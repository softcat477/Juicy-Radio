#ifndef CHANNELGUI_H
#define CHANNELGUI_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include <queue>
#include <vector>

#include "IOParams.h"

class AudioMeter: public juce::Component, public juce::Timer{
public:
    AudioMeter() = delete;
    AudioMeter(float sr, float window_size_ms);
    ~AudioMeter() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void audioBuffer2Rms(const juce::AudioBuffer<float>* out_buffer, int out_success_sample_L, int out_success_sample_R);
    void timerCallback() override;

    int meter_width = 40;
private:
    float _rms2Db(float current_rms);
    //ChannelStripSetting* strip_setting;
    float _max_queue_size;
    float _last_rms_L;
    float _last_rms_R;

    //std::queue<float> _queue;
    std::vector<float> _queue_L;
    std::vector<float> _queue_R;
};

class ChannelGui: public juce::Component{
public:
    ChannelGui() = delete;
    ChannelGui(ChannelStripSetting*);
    ~ChannelGui();

    void paint(juce::Graphics& g);
    void resized();

    int channel_width = 120;
    void updateAudioMeter(const juce::AudioBuffer<float>* out_buffer, int out_success_sample_L, int out_success_sample_R);
private:
    void _panCallback();
    void _faderCallback();

    ChannelStripSetting* strip_setting;

    juce::Slider pan_gui;
    juce::Label plugins;
    juce::Slider fader_gui;
    AudioMeter meter_gui;
    int margin = 10;
    int margin_mini = 5;

    int LR = 0;

    int _channel_shadow_margin;
    juce::Colour _c_channel_bg;
    juce::Colour _c_channel_shadow;
};

#endif