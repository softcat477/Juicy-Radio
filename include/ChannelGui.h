#ifndef CHANNELGUI_H
#define CHANNELGUI_H

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>
#include <queue>
#include <vector>
#include <memory>

#include "IOParams.h"

class AudioMeter: public juce::Component, public juce::Timer{
public:
    AudioMeter() = delete;
    AudioMeter(float sr, float window_size_ms);
    ~AudioMeter() override;

    AudioMeter(AudioMeter& other) = delete;
    AudioMeter& operator=(AudioMeter& other)=delete;
    AudioMeter(AudioMeter&& other) = delete;
    AudioMeter& operator=(AudioMeter&& other)=delete;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void audioBuffer2Rms(const juce::AudioBuffer<float>* out_buffer, int out_success_sample_L, int out_success_sample_R);
    void timerCallback() override;

    int meter_width = 35;
private:
    float _rms2Db(float current_rms);
    //ChannelStripSetting* strip_setting;
    float _max_queue_size;
    float _last_rms_L;
    float _last_rms_R;

    //std::queue<float> _queue;
    std::vector<float> _queue_L;
    std::vector<float> _queue_R;

    juce::Colour _c_foreground = juce::Colour{105, 185, 102};
    juce::Colour _c_background = juce::Colour{50, 50, 50};

    juce::ColourGradient _gradient;
};

class AudioMeasure: public juce::Component {
public:
    AudioMeasure();
    ~AudioMeasure() override;

    AudioMeasure(AudioMeasure& other) = delete;
    AudioMeasure& operator=(AudioMeasure& other)=delete;
    AudioMeasure(AudioMeasure&& other) = delete;
    AudioMeasure& operator=(AudioMeasure&& other)=delete;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Colour _c_foreground = juce::Colour{105, 185, 102};
    //juce::Colour _c_background = juce::Colour{50, 50, 50};
    juce::Colour _c_background = juce::Colour{72, 72, 72};

    std::vector<std::shared_ptr<juce::Label>> _labels;

    int _ticks =30;
    float _dist;
};

class ChannelGui: public juce::Component{
public:
    ChannelGui() = delete;
    ChannelGui(ChannelStripSetting*);
    ~ChannelGui();

    ChannelGui(ChannelGui& other) = delete;
    ChannelGui& operator=(ChannelGui& other) = delete;
    ChannelGui(ChannelGui&& other) = delete;
    ChannelGui& operator=(ChannelGui&& other) = delete;

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
    AudioMeasure measure_gui;
    // int margin = 10;
    // int margin_mini = 5;
    int margin = 10;
    int margin_mini = 5;

    int LR = 0;

    int _channel_shadow_margin = 0;
    juce::Colour _c_channel_bg = juce::Colour{72, 72, 72};
    juce::Colour _c_channel_shadow = juce::Colour{33, 131, 128};
    juce::Colour _c_interior_fill = juce::Colour{50, 50, 50};
};

#endif