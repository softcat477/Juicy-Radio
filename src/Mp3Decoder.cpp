#include "../include/Mp3Decoder.h"

#include <vector>
#include <utility>

Mp3Decoder::Mp3Decoder(size_t sample_per_frame, size_t pcm_buf_size, IChannel<char>* upstream): // 1152, 128
                IChannel{sample_per_frame, pcm_buf_size},
                upstream{upstream}{
    /*
    * Input:
     *  <sample_per_frame>: The number of samples in ring buffers for LR channels.
     *  <pcm_buf_size>: The number of frames in the ring buffer for LR channels.
    */
}
Mp3Decoder::~Mp3Decoder(){
}

enum mad_flow Mp3Decoder::input(void *data, struct mad_stream *stream){
    Mp3Decoder* thread_decoder = static_cast<Mp3Decoder*>(data);

    // Terminate by user
    if (thread_decoder->isStopped()){
        return MAD_FLOW_STOP;
    }

    // Wait until there's mp3 frames.
    if (thread_decoder->upstream->canRead() == false){
        thread_decoder->upstream->wait();
    }

    // Terminate by user
    if (thread_decoder->isStopped()){
        return MAD_FLOW_STOP;
    }

    // With SmartRead()
    size_t target_read_length = thread_decoder->upstream->getSamplesPerFrame();
    //juce::AudioBuffer<char> tmp_audioBuffer{1, static_cast<int>(target_read_length)};
    std::vector<char> tmptmp (target_read_length, '0');

    auto [success_read_length, _tmp] = thread_decoder->upstream->popAudio(&tmptmp, &tmptmp, static_cast<int>(target_read_length));
    //memcpy(tmp_audioBuffer.getWritePointer(0,0), tmptmp.data(), sizeof(char) * success_read_length);
    size_t length_l = stream->bufend - stream->next_frame;

    if (length_l+success_read_length != 0){
        unsigned char* ptr = (unsigned char*)malloc(length_l + success_read_length);
        memcpy(ptr, stream->next_frame, length_l);
        //memcpy(ptr+length_l, tmp_audioBuffer.getReadPointer(0), success_read_length);
        memcpy(ptr+length_l, tmptmp.data(), success_read_length);

        mad_stream_buffer(stream, ptr, (unsigned long)(length_l + success_read_length)); // The address to the location storing the binary data.
    }
    else{
        printf ("Buggy in Mp3Decoder::input return \n");
        return MAD_FLOW_IGNORE;
    }

    return MAD_FLOW_CONTINUE;
}
enum mad_flow Mp3Decoder::output(void *data, struct mad_header const *header, struct mad_pcm *pcm){
    unsigned int nchannels, nsamples;
    mad_fixed_t const *left_ch, *right_ch;
    nchannels = pcm->channels;
    nsamples  = pcm->length;
    left_ch   = pcm->samples[0];
    right_ch  = pcm->samples[1];

    // unsigned char* buf;
    unsigned int buf_size = nchannels * 2 * nsamples;

    float* buf_L;
    buf_L = static_cast<float*>(malloc(buf_size));
    float* buf_R;
    buf_R = static_cast<float*>(malloc(buf_size));

    size_t buf_idx = 0;
    while (nsamples--) {
        /* output sample(s) in 16-bit signed little-endian PCM */
        auto pcmInt_pcmFloat = Mp3Decoder::scale(*left_ch++);
        signed int sample = pcmInt_pcmFloat.first;
        float pcm_float = pcmInt_pcmFloat.second;

        *(buf_L+buf_idx) = pcm_float;

        if (nchannels == 2) {
            //sample = scale(*right_ch++);
            pcmInt_pcmFloat = Mp3Decoder::scale(*right_ch++);
            sample = pcmInt_pcmFloat.first;
            pcm_float = pcmInt_pcmFloat.second;

            *(buf_R+buf_idx) = pcm_float;
        }
        buf_idx++;
    }

    Mp3Decoder* thread_decoder = static_cast<Mp3Decoder*>(data);
    size_t success_written_length_L = (thread_decoder->bufferL).write(buf_L, buf_idx);
    size_t success_written_length_R = (thread_decoder->bufferR).write(buf_R, buf_idx);

    free(buf_L);
    free(buf_R);
    return MAD_FLOW_CONTINUE;
}
std::pair<size_t, size_t> Mp3Decoder::popAudio(std::vector<float>* output_bufferL, 
    std::vector<float>* output_bufferR,
    size_t output_sample_count) {

    size_t success_sample_L = bufferL.read(output_bufferL, static_cast<int>(output_sample_count));
    size_t success_sample_R = bufferR.read(output_bufferR, static_cast<int>(output_sample_count));

    return std::make_pair(success_sample_L, success_sample_R);
}
enum mad_flow Mp3Decoder::error(void *data, struct mad_stream *stream, struct mad_frame *frame){
    fprintf(stderr, "decoding error 0x%04x (%s)\n",
        stream->error, mad_stream_errorstr(stream));

    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */
    return MAD_FLOW_CONTINUE;
}
std::pair<signed int, float> Mp3Decoder::scale(mad_fixed_t sample){
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

void Mp3Decoder::start(){
    struct mad_decoder decoder;
    mad_decoder_init(&decoder, this,
		   Mp3Decoder::input, nullptr, nullptr, Mp3Decoder::output,
		   Mp3Decoder::error, nullptr);
    int result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    /* release the decoder */
    mad_decoder_finish(&decoder);

    printf ("Quit Mp3Decoder::start() with code %d\n", result);
}