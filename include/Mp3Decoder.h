#ifndef MP3DECODER_H
#define MP3DECODER_H

#include <cstdio>
#include <mad.h>

//#include "IThread.h"
#include "IChannel.h"

class Mp3Decoder: public IChannel<float>{
public:
    Mp3Decoder() = delete;
    Mp3Decoder(size_t sample_per_frame, size_t pcm_buf_size, IChannel<char>* upstream);
    ~Mp3Decoder() override;

    Mp3Decoder(Mp3Decoder& other) = delete;
    Mp3Decoder& operator=(Mp3Decoder& other) = delete;
    Mp3Decoder(Mp3Decoder&& other) = delete;
    Mp3Decoder& operator=(Mp3Decoder&& other) = delete;

    void start() override;
    std::pair<size_t, size_t> popAudio(std::vector<float>* output_bufferL, 
        std::vector<float>* output_bufferR,
        size_t output_sample_count) override;

    static enum mad_flow input(void *data, struct mad_stream *stream);
    static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
    static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);
    static std::pair<signed int, float> scale(mad_fixed_t sample);

    IChannel<char>* upstream;
};

#endif