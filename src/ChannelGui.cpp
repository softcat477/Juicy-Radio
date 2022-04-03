#include "../include/ChannelGui.h"

ChannelGui::ChannelGui(ChannelStripSetting* in_strip_setting):strip_setting(in_strip_setting){
    LR = int((channel_width-2*margin-margin_mini)/2);
    pan_gui.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    //pan_gui.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox);
    pan_gui.setTextBoxStyle (juce::Slider::TextBoxBelow, true, channel_width, 20);
    pan_gui.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::coral);
    pan_gui.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colours::coral);
    pan_gui.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::white);
    pan_gui.setRange(-1.0, 1.0);
    pan_gui.setValue(0.0);
    pan_gui.onValueChange = [this]{_panCallback();};
    addAndMakeVisible(pan_gui);

    plugins.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colours::salmon);
    addAndMakeVisible(plugins);

    fader_gui.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    //fader_gui.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    fader_gui.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::olive);
    fader_gui.setTextBoxStyle (juce::Slider::TextBoxBelow, true, LR, 20*2);
    fader_gui.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colours::olive);
    fader_gui.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::white);
    fader_gui.setRange(-36.0, 8.0);
    fader_gui.setValue(0.0);
    fader_gui.setTextValueSuffix ("dB");
    fader_gui.setNumDecimalPlacesToDisplay(1);
    fader_gui.onValueChange = [this]{_faderCallback();};
    addAndMakeVisible(fader_gui);
}
ChannelGui::~ChannelGui(){

}

void ChannelGui::paint(juce::Graphics& g){
    g.fillAll(juce::Colours::floralwhite);
}
void ChannelGui::resized(){
    auto area = getLocalBounds();
    area.removeFromBottom(margin);
    area.removeFromTop(margin);
    area.removeFromLeft(margin);
    area.removeFromRight(margin);

    pan_gui.setBounds(area.removeFromTop(channel_width));
    area.removeFromTop(margin_mini);
    plugins.setBounds(area.removeFromTop(100));
    area.removeFromTop(margin_mini);
    fader_gui.setBounds(area.removeFromLeft(LR));
    return;
}
void ChannelGui::_panCallback(){
    double a = pan_gui.getValue();
    printf("pan: %f\n", a);
}
void ChannelGui::_faderCallback(){
    double a = fader_gui.getValue();
    strip_setting->setPostDb(a);
}