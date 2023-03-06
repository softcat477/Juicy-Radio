#ifndef STEREOOUT_H
#define STEREOOUT_H

//#include "IThread.h"
// #include "RingBuffer.h"
// #include "ChannelStrip.h"
#include "IOParams.h"
#include "ChannelGui.h"

#include "IChannel.h"

#include <juce_audio_devices/juce_audio_devices.h>

class StereoOut {
public:
    StereoOut(IChannel<float>* mp3_decoder);
    ~StereoOut();

    StereoOut(StereoOut& other) = delete;
    StereoOut& operator=(StereoOut& other) = delete;
    StereoOut(StereoOut&& other) = delete;
    StereoOut& operator=(StereoOut&& other) = delete;

    ChannelGui* getStereoOut(){return &_channel_gui;}

    void getNextAudioBlock(juce::AudioBuffer<float>* out_buffer, int num_samples, int& success_sample_L, int& success_sample_R);
private:
    std::vector<IChannel<float>*> _input_enc_streams;
    ChannelStripSetting _channel_setting;
    ChannelGui _channel_gui;
};

#endif