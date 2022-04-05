#ifndef THREADCHANNEL_H
#define THREADCHANNEL_H

#include "IThreadManager.h"
#include "RingBuffer.h"
#include "ChannelStrip.h"
#include "IOParams.h"
#include "ChannelGui.h"

#include <juce_audio_devices/juce_audio_devices.h>

class ThreadChannel:public IThreadManager{
public:
    ThreadChannel(size_t buf_size, size_t buf_max_frame, IEncoderStream* mp3_decoder);
    ~ThreadChannel() override;

    void start() override;
    ChannelGui* getStereoOut(){return _stereo_out.getChanelGui();}

    void getNextAudioBlock(juce::AudioBuffer<float>* out_buffer, int num_samples, int& success_sample_L, int& success_sample_R);

    RingBuffer<float>* stereo_out_L;
    RingBuffer<float>* stereo_out_R;
private:
    size_t _buf_size;
    ChannelStrip _stereo_out;
    std::vector<ChannelStrip> _channels;
};

#endif