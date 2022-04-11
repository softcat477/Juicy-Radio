#ifndef CHANNELSTRIP_H
#define CHANNELSTRIP_H

#include <vector>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

#include "IOParams.h"
#include "IEncoderStream.h"
#include "ChannelGui.h"

class ChannelStrip{
public:
    ChannelStrip();
    ChannelStrip(juce::AudioDeviceManager::AudioDeviceSetup device_spec);
    ChannelStrip(ChannelStripSetting);
    ~ChannelStrip();

    ChannelStrip(ChannelStrip& other) = delete;
    ChannelStrip& operator=(ChannelStrip& other) = delete;
    ChannelStrip(ChannelStrip&& other) = delete;
    ChannelStrip& operator=(ChannelStrip&& other) = delete;

    bool connect(ChannelStrip*);
    bool connect(IEncoderStream*);

    bool remove(ChannelStrip*);
    bool remove(IEncoderStream*);
    /*
    void setPreDb(float pre_db);
    void setPostDb(float post_db);
    void setPan(float pan);
    */
    float getPreDb();
    float getPostDb();
    float getPan();

    void processAndMixAudio(juce::AudioBuffer<float>*, int&, int&, size_t);

    ChannelGui* getChanelGui(){return &_channel_gui;}
private:
    ChannelStripSetting _channel_setting;
    ChannelGui _channel_gui;
    std::vector<ChannelStrip*> _input_strips;
    std::vector<IEncoderStream*> _input_enc_streams;
};
#endif