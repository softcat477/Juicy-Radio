#ifndef STEREOOUT_H
#define STEREOOUT_H

#include <juce_audio_devices/juce_audio_devices.h>

#include "IOParams.h"
#include "IThread.h"
#include "ChannelGui.h"
#include "Channel.h"

class StereoOut: public Channel<float, float, 2>, public IThread {
public:
    StereoOut();
    ~StereoOut();

    StereoOut(StereoOut& other) = delete;
    StereoOut& operator=(StereoOut& other) = delete;
    StereoOut(StereoOut&& other) = delete;
    StereoOut& operator=(StereoOut&& other) = delete;

    ChannelGui* getStereoOut(){return &_channel_gui;}
    void start() override;

    void getNextAudioBlock();
private:
    ChannelStripSetting _channel_setting;
    ChannelGui _channel_gui;
};

#endif