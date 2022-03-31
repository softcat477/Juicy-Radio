#ifndef THREADDECODER_H
#define THREADDECODER_H

#include <stdio.h>
#include <thread>
#include <atomic>
#include <mad.h>

#include "RingBuffer.h"
#include "CondVar.h"
#include "IEncoderStream.h"
#include "IThreadManager.h"
#include "IOParams.h"

//class ThreadDecoder : public IEncoderStream{
class ThreadDecoder:public IThreadManager, public IEncoderStream{
public:
    ThreadDecoder() = delete;
    ThreadDecoder(RingBuffer<char>* mp3_buffer, size_t sample_per_frame, size_t pcm_buf_size);
    ~ThreadDecoder() override;

    void encodeSumIntoBuffer(EncoderInputData* e_in, EncoderOutputData* e_out) override;

    void start() override;

    static enum mad_flow input(void *data, struct mad_stream *stream);
    static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
    static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);
    static std::pair<signed int, float> scale(mad_fixed_t sample);

    // Read mp3 frames from here
    RingBuffer<char>*   mp3_buffer;
    // Store decoded floating points to here.
    RingBuffer<float>* pcm_buffer_L;
    RingBuffer<float>* pcm_buffer_R;
};

#endif
