#ifndef OSC_H
#define OSC_H

#include <vector>

#include <juce_audio_devices/juce_audio_devices.h>

#include "IOParams.h"
#include "IThread.h"
#include "ChannelGui.h"
#include "Channel.h"

class Osc: public Channel<float, float, 2>, public IThread {
public:
    Osc();
    ~Osc();

    Osc(Osc& other) = delete;
    Osc& operator=(Osc& other) = delete;
    Osc(Osc&& other) = delete;
    Osc& operator=(Osc&& other) = delete;

    ChannelGui* getStereoOut(){return &_channel_gui;}
    void start() override;
    void prepareTable();

    void getNextAudioBlock();

    void setF(float f) {_f = f;}
    void setFs(float fs) {_fs = fs;}
private:
    ChannelStripSetting _channel_setting;
    ChannelGui _channel_gui;

    float _f;
    float _fs;

    std::vector<float> _table;
    size_t _tsize = 256;

    float _prev = 0;
};

#endif