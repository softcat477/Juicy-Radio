#ifndef CHANNELGUI_H
#define CHANNELGUI_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "IOParams.h"

class ChannelGui: public juce::Component{
public:
    ChannelGui() = delete;
    ChannelGui(ChannelStripSetting*);
    ~ChannelGui();

    void paint(juce::Graphics& g);
    void resized();

    int channel_width = 100;
private:
    void _panCallback();
    void _faderCallback();

    ChannelStripSetting* strip_setting;

    juce::Slider pan_gui;
    juce::Label plugins;
    juce::Slider fader_gui;
    int margin = 10;
    int margin_mini = 5;

    int LR = 0;
};

#endif