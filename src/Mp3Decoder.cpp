#include "../include/Mp3Decoder.h"

#include <vector>
#include <utility>

Mp3Decoder::Mp3Decoder() {
}
Mp3Decoder::~Mp3Decoder(){
}

enum mad_flow Mp3Decoder::input(void *data, struct mad_stream *stream){
    Mp3Decoder* thread_decoder = static_cast<Mp3Decoder*>(data);

    // Terminate by user
    if (thread_decoder->isStopped()){
        return MAD_FLOW_STOP;
    }

    // Wait until there's mp3 frames. Need to do this since MAD_FLOW_IGNORE is not working.
    if ((thread_decoder->_aggregator).canRead() == false){
        (thread_decoder->_aggregator).wait();
    }

    // Terminate by user
    if (thread_decoder->isStopped()){
        return MAD_FLOW_STOP;
    }

    // Read upstream, only need the left channel (Radio receiver is in mono but with two channels)
    size_t target_read_length = (thread_decoder->_aggregator).getSamplesPerFrame();

    std::vector<char> tmpL (target_read_length, '\0');
    size_t success_read_length = (thread_decoder->_aggregator).popAudio(tmpL.data(), target_read_length, 0);

    std::vector<char> tmpR (target_read_length, '\0');
    (thread_decoder->_aggregator).popAudio(tmpR.data(), target_read_length, 1);

    // Bytes from the last round that haven't been decoded yet
    size_t length_l = static_cast<size_t>(stream->bufend - stream->next_frame);

    if (length_l+success_read_length != 0){
        unsigned char* ptr = (unsigned char*)malloc(length_l + success_read_length);

        // Bytes from the last round
        memcpy(ptr, stream->next_frame, length_l);
        // Bytes received in this round
        memcpy(ptr+length_l, tmpL.data(), success_read_length);

        mad_stream_buffer(stream, ptr, (unsigned long)(length_l + success_read_length)); // The address to the location storing the binary data.
    }
    else{
        printf ("Buggy in Mp3Decoder::input. Shutdown decoder.\n");
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
        // output sample(s) in 16-bit signed little-endian PCM
        auto pcmInt_pcmFloat = Mp3Decoder::scale(*left_ch++);
        signed int sample = pcmInt_pcmFloat.first;
        float pcm_float = pcmInt_pcmFloat.second;

        *(buf_L+buf_idx) = pcm_float;

        if (nchannels == 2) {
            pcmInt_pcmFloat = Mp3Decoder::scale(*right_ch++);
            sample = pcmInt_pcmFloat.first;
            pcm_float = pcmInt_pcmFloat.second;

            *(buf_R+buf_idx) = pcm_float;
        }
        buf_idx++;
    }

    // Push to distributor
    Mp3Decoder* thread_decoder = static_cast<Mp3Decoder*>(data);
    (thread_decoder->_distributor).pushAudio(buf_L, buf_idx, 0);
    (thread_decoder->_distributor).pushAudio(buf_R, buf_idx, 1);

    free(buf_L);
    free(buf_R);
    return MAD_FLOW_CONTINUE;
}
enum mad_flow Mp3Decoder::error(void *data, struct mad_stream *stream, struct mad_frame *frame){
    fprintf(stderr, "decoding error 0x%04x (%s)\n",
        stream->error, mad_stream_errorstr(stream));

    return MAD_FLOW_CONTINUE;
}
std::pair<signed int, float> Mp3Decoder::scale(mad_fixed_t sample){
    // mad_fixed_t : signed int
    /* round */
    //sample += (1L << (MAD_F_FRACBITS - 16));

    // clip to 1.0 ~ -1.0
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1; // 0.9~
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE; // -1.0

    float pcm_f = (float)sample / (float)MAD_F_ONE;

    /// quantize to to signed int again, float
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