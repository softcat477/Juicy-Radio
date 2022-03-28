#include "../include/ThreadDecoder.h"

#include <algorithm>

#include <mad.h>
#include <mutex>
#include <utility>
#include <condition_variable>
#include "../include/RingBuffer.h"
#include "../include/CondVar.h"

#include <fstream>

ThreadDecoder::ThreadDecoder(RingBuffer<char>* mp3_buffer, size_t sample_per_frame, size_t pcm_buf_size, 
                             CondVar* cond_mp3, CondVar* cond_pcm):
                mp3_buffer(mp3_buffer), _cond_mp3(cond_mp3){
    /*
     * Input:
     *  <mp3_buffer>: Read mp3 frames from this buffer and decode frames into floating points.
     *  <sample_per_frame>: The number of samples in ring buffers for LR channels.
     *  <pcm_buf_size>: The number of frames in the ring buffer for LR channels.
     *  <cond_mp3>: Subscribe to this condition variable, and we are notified whenever ther's new frame in this cond_var.
     *  <cond_pcm>: No usage.
    */
    pcm_buffer_L = new RingBuffer<float>(sample_per_frame, pcm_buf_size, cond_pcm); // sample_per_Frame=1152 for mp3
    pcm_buffer_R = new RingBuffer<float>(sample_per_frame, pcm_buf_size, cond_pcm); // sample_per_Frame=1152 for mp3
}
ThreadDecoder::~ThreadDecoder(){
    delete pcm_buffer_L;
    delete pcm_buffer_R;
}
/*
size_t ThreadDecoder::EncodeAndSumIntoBuffer(FMixerEncoderOutputData* encoder_output_struct){
    juce::AudioBuffer<float>* audio_buffer = encoder_output_struct->audio_buffer;
    size_t sample_count = encoder_output_struct->sample_count;
    size_t count_L = 0;
    size_t count_R = 0;

    float* tmp_ptr = (float*)malloc(sizeof(float)*sample_count);
    if (this->pcm_buffer_L->canSmartRead()){
        count_L = pcm_buffer_L->getSmartRead(tmp_ptr, sample_count);
        if (count_L != sample_count){
            printf("MainComponent::getNextAudioBlock: not enough samples in tmp_ptr, need %d get %d\n", sample_count, count_L);
        }
        memcpy(audio_buffer->getWritePointer(0, 0), tmp_ptr, sizeof(float) * count_L);
        // DEBUG, write decoded floating points to a file and synthesis .wav with python.
        //for (auto di = 0; di < count_L; di++){
        //    fp_device << *(tmp_ptr + di) << "\n";
        //}
    }
    if (this->pcm_buffer_R->canSmartRead()){
        count_R = pcm_buffer_R->getSmartRead(tmp_ptr, sample_count);
        if (count_R != sample_count){
            printf("MainComponent::getNextAudioBlock: not enough samples in tmp_ptr, need %d get %d\n", sample_count, count_R);
        }
        // DEBUG, write decoded floating points to a file and synthesis .wav with python.
        memcpy(audio_buffer->getWritePointer(1, 0), tmp_ptr, sizeof(float) * count_R);
    }
    free (tmp_ptr);

    return std::min(count_L, count_R);
}
*/

enum mad_flow ThreadDecoder::input(void *data, struct mad_stream *stream){
    ThreadDecoder* thread_decoder = static_cast<ThreadDecoder*>(data);

    // Terminate by user
    if (thread_decoder->isStopped()){
        return MAD_FLOW_STOP;
    }

    // Wait until there's mp3 frames.
    if (thread_decoder->mp3_buffer->canRead() == false){
        thread_decoder->_cond_mp3->wait();
    }

    // Terminate by user
    if (thread_decoder->isStopped()){
        return MAD_FLOW_STOP;
    }

    // Available frames
    if (thread_decoder->mp3_buffer->canRead()){
        // V2. Good PCM, https://stackoverflow.com/questions/39803572/libmad-playback-too-fast-if-read-in-chunks
        // 0. Incomplete frames from the last decode round.
        size_t length_l = stream->bufend - stream->next_frame;
        if (length_l != 0){
            // 1. The next chunk of data from LIBCURL
            const char* ptr_r = thread_decoder->mp3_buffer->getReadPtr();
            size_t length_r = thread_decoder->mp3_buffer->getReadLength();
            // 2. Merge step0 and step 1 together. Step 0 is on the left side and step 1 is on the right
            unsigned char* ptr = (unsigned char*)malloc(length_l + length_r);
            memcpy(ptr, stream->next_frame, length_l);
            memcpy(ptr+length_l, ptr_r, length_r);

            mad_stream_buffer(stream, ptr, (unsigned long)(length_l + length_r)); // The address to the location storing the binary data.
        }
        else{
            const char* read_ptr = thread_decoder->mp3_buffer->getReadPtr();
            size_t read_size = thread_decoder->mp3_buffer->getReadLength();
            mad_stream_buffer(stream, reinterpret_cast<const unsigned char*>(read_ptr), read_size);
        }

        thread_decoder->mp3_buffer->finishRead(true);
    }
    // Skip this frame
    else{
        printf ("Buggy in ThreadDecoder\n");
    }

    return MAD_FLOW_CONTINUE;
}
enum mad_flow ThreadDecoder::output(void *data, struct mad_header const *header, struct mad_pcm *pcm){
    unsigned int nchannels, nsamples;
    mad_fixed_t const *left_ch, *right_ch;
    nchannels = pcm->channels;
    nsamples  = pcm->length;
    left_ch   = pcm->samples[0];
    right_ch  = pcm->samples[1];

    // Debug
    //int FrameLen = int((144 * header->bitrate / header->samplerate) + int(header->flags&MAD_FLAG_PADDING)); // bytes
    //printf ("%d | Flag : %d, Bytes : %d, Bitrate : %lu, Sample Rate : %d, Decoded sample length: %d, buffer_size : %u\n", -99, header->flags&MAD_FLAG_PADDING, FrameLen, header->bitrate, header->samplerate, nsamples, nchannels * 2 * nsamples);

    // DEBUG, dump to .pcm.
    unsigned char* buf;
    unsigned int buf_size = nchannels * 2 * nsamples;
    buf = static_cast<unsigned char*>(malloc(buf_size));

    float* buf_L;
    buf_L = static_cast<float*>(malloc(buf_size));
    float* buf_R;
    buf_R = static_cast<float*>(malloc(buf_size));

    size_t buf_idx = 0;
    while (nsamples--) {
        /* output sample(s) in 16-bit signed little-endian PCM */
        auto pcmInt_pcmFloat = ThreadDecoder::scale(*left_ch++);
        signed int sample = pcmInt_pcmFloat.first;
        float pcm_float = pcmInt_pcmFloat.second;

        // DEBUG, Write to PCM
        *(buf+buf_idx*4 + 0) = (sample >> 0) & 0xff;
        *(buf+buf_idx*4 + 1) = (sample >> 8) & 0xff;

        *(buf_L+buf_idx) = pcm_float;

        if (nchannels == 2) {
            //sample = scale(*right_ch++);
            pcmInt_pcmFloat = ThreadDecoder::scale(*right_ch++);
            sample = pcmInt_pcmFloat.first;
            pcm_float = pcmInt_pcmFloat.second;

            // Write to file
            // DEBUG, Write to PCM
            *(buf+buf_idx*4 + 2) = (sample >> 0) & 0xff;
            *(buf+buf_idx*4 + 3) = (sample >> 8) & 0xff;

            *(buf_R+buf_idx) = pcm_float;
        }
        buf_idx++;
    }

    ThreadDecoder* thread_decoder = static_cast<ThreadDecoder*>(data);
    if (thread_decoder->pcm_buffer_L->canSmartWrite()){
        size_t ret = thread_decoder->pcm_buffer_L->smartWrite(buf_L, buf_idx);
        if (ret != buf_idx){
            printf ("pcm_buffer_L, Write %zu/%zu\n", ret, buf_idx);
        }
        thread_decoder->pcm_buffer_L->finishSmartWrite();
    }
    else{
        printf ("ThreadDecoder.cpp:output: Can't write to pcm_buffer_L\n");
    }

    if (thread_decoder->pcm_buffer_R->canSmartWrite()){
        size_t ret = thread_decoder->pcm_buffer_R->smartWrite(buf_R, buf_idx);
        if (ret != buf_idx){
            printf ("pcm_buffer_R, Write %zu/%zu\n", ret, buf_idx);
        }
        thread_decoder->pcm_buffer_R->finishSmartWrite();
    }
    else{
        printf ("ThreadDecoder.cpp:output: Can't write to pcm_buffer_R\n");
    }

    free(buf);
    free(buf_L);
    free(buf_R);
    return MAD_FLOW_CONTINUE;
}
enum mad_flow ThreadDecoder::error(void *data, struct mad_stream *stream, struct mad_frame *frame){
    fprintf(stderr, "decoding error 0x%04x (%s)\n",
        stream->error, mad_stream_errorstr(stream));

    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
    return MAD_FLOW_CONTINUE;
}
std::pair<signed int, float> ThreadDecoder::scale(mad_fixed_t sample){
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
    sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
    sample = -MAD_F_ONE;

    float pcm_f = (float)sample / (float)MAD_F_ONE;

    /* quantize */
    return std::make_pair(sample >> (MAD_F_FRACBITS + 1 - 16), pcm_f);
}

void ThreadDecoder::start(){
    struct mad_decoder decoder;
    mad_decoder_init(&decoder, this,
		   ThreadDecoder::input, nullptr, nullptr, ThreadDecoder::output,
		   ThreadDecoder::error, nullptr);
    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    /* release the decoder */
    mad_decoder_finish(&decoder);

    printf ("ThreadDecoder::start stopped with code %d\n", result);
}
void ThreadDecoder::setStop(){
    _isStopped.store(true);
}
void ThreadDecoder::setStart(){
    _isStopped.store(false);
}
bool ThreadDecoder::isStopped(){
    return _isStopped.load();
}