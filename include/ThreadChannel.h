#ifndef THREADCHANNEL_H
#define THREADCHANNEL_H

#include "IThreadManager.h"
#include "RingBuffer.h"
#include "ChannelStrip.h"
#include "IOParams.h"
#include "ChannelGui.h"

#include "IChannel.h"

#include <juce_audio_devices/juce_audio_devices.h>

class ThreadChannel:public IThreadManager{
public:
    ThreadChannel(IChannel<float>* mp3_decoder);
    ~ThreadChannel() override;

    ThreadChannel(ThreadChannel& other) = delete;
    ThreadChannel& operator=(ThreadChannel& other) = delete;
    ThreadChannel(ThreadChannel&& other) = delete;
    ThreadChannel& operator=(ThreadChannel&& other) = delete;

    void start() override;
    ChannelGui* getStereoOut(){return &_channel_gui;}

    void getNextAudioBlock(juce::AudioBuffer<float>* out_buffer, int num_samples, int& success_sample_L, int& success_sample_R);
private:
    std::vector<IChannel<float>*> _input_enc_streams;
    ChannelStripSetting _channel_setting;
    ChannelGui _channel_gui;
};

#endif