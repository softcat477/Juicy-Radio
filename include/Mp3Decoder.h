#ifndef MP3DECODER_H
#define MP3DECODER_H

#include <cstdio>
#include <mad.h>

#include "IThread.h"
#include "Channel.h"

class Mp3Decoder: public Channel<char, float, 2>, public IThread{
public:
    Mp3Decoder();
    ~Mp3Decoder() override;

    Mp3Decoder(Mp3Decoder& other) = delete;
    Mp3Decoder& operator=(Mp3Decoder& other) = delete;
    Mp3Decoder(Mp3Decoder&& other) = delete;
    Mp3Decoder& operator=(Mp3Decoder&& other) = delete;

    void start() override;

    static enum mad_flow input(void *data, struct mad_stream *stream);
    static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
    static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);
    static std::pair<signed int, float> scale(mad_fixed_t sample);
};

#endif