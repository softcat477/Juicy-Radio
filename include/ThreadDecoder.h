#ifndef THREADDECODER_H
#define THREADDECODER_H

#include <stdio.h>
#include <thread>
#include <atomic>
#include <mad.h>

#include "RingBuffer.h"
#include "CondVar.h"
//#include "IEncoderStream.h"

//class ThreadDecoder : public IEncoderStream{
class ThreadDecoder{
public:
    ThreadDecoder() = delete;
    ThreadDecoder(RingBuffer<char>* mp3_buffer, size_t sample_per_frame, size_t pcm_buf_size, CondVar* cond_mp3, CondVar* cond_pcm);
    ~ThreadDecoder();
    //~ThreadDecoder() override;

    //size_t EncodeAndSumIntoBuffer(FMixerEncoderOutputData*) override;

    void start();
    void setStop();
    void setStart();
    bool isStopped();

    static enum mad_flow input(void *data, struct mad_stream *stream);
    static enum mad_flow output(void *data, struct mad_header const *header, struct mad_pcm *pcm);
    static enum mad_flow error(void *data, struct mad_stream *stream, struct mad_frame *frame);
    static std::pair<signed int, float> scale(mad_fixed_t sample);

    // Read mp3 frames from here
    RingBuffer<char>*   _mp3_buffer;
    // Store decoded floating points to here.
    RingBuffer<float>* _pcm_buffer_L;
    RingBuffer<float>* _pcm_buffer_R;
private:
    // A variable trigerred by main thread to infomr this thread to stop.
    std::atomic<bool> _isStopped;
    // Subscribe to this condition variable, and we are notified whenever ther's new frame in this cond_var.
    CondVar* _cond_mp3;
};

#endif