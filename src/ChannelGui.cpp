#include "../include/ChannelGui.h"
#include "math.h"
#include <cmath>
#include <valarray>
#include <juce_core/juce_core.h>

AudioMeter::AudioMeter(float sr, float window_size_ms):
            _max_queue_size(static_cast<int>(window_size_ms/1000.0*sr)),
            _last_rms_L(0.0),
            _last_rms_R(0.0){
    startTimer(100);
}
AudioMeter::~AudioMeter(){
    stopTimer();
}

void AudioMeter::paint(juce::Graphics& g){
    g.fillAll(juce::Colours::white);

    auto meter_R = getLocalBounds();
    auto meter_L = meter_R.removeFromLeft(static_cast<int>(meter_R.getWidth()/2));
    g.setColour(juce::Colour{195, 219, 197});
    g.fillRect(meter_R.removeFromLeft(1.0));
    g.fillRect(meter_L.removeFromRight(1.0));

    g.setColour(juce::Colour{235, 186, 185});
    float db_L = static_cast<float>(std::fmax(_rms2Db(_last_rms_L), -45.0));
    int height_L = static_cast<int>(juce::jmap(db_L, static_cast<float>(-45.0), static_cast<float>(0.0), 
                                      static_cast<float>(0.0), static_cast<float>(meter_L.getHeight())));
    g.fillRect(meter_L.removeFromBottom(height_L));

    float db_R = static_cast<float>(std::fmax(_rms2Db(_last_rms_R), -45.0));
    int height_R = static_cast<int>(juce::jmap(db_R, static_cast<float>(-45.0), static_cast<float>(0.0), 
                                      static_cast<float>(0.0), static_cast<float>(meter_R.getHeight())));
    g.fillRect(meter_R.removeFromBottom(height_R));
}
void AudioMeter::resized(){
    // Do nothing here
}
void AudioMeter::timerCallback(){
    repaint();
}

void AudioMeter::audioBuffer2Rms(const juce::AudioBuffer<float>* out_buffer, int out_success_sample_L, int out_success_sample_R){
    if (out_success_sample_L > 0){
        std::vector<float> vector_L{out_buffer->getReadPointer(0), out_buffer->getReadPointer(0)+out_success_sample_L};
        // Erase exceeding elements
        int exceed_size_L = _queue_L.size()+out_success_sample_L-_max_queue_size;
        if (exceed_size_L > 0){
            _queue_L.erase(_queue_L.begin(), _queue_L.begin()+exceed_size_L);
        }
        // Insert all elements to the queue
        _queue_L.insert(_queue_L.end(), vector_L.begin(), vector_L.end());
        // RMS
        std::valarray<float> valarray_L{_queue_L.data(), _queue_L.size()};
        std::valarray<float> valarray_pow_L = std::pow(valarray_L, 2.0);
        _last_rms_L = sqrt(valarray_pow_L.sum()/valarray_pow_L.size());
    }

    if (out_success_sample_R > 0){
        std::vector<float> vector_R{out_buffer->getReadPointer(1), out_buffer->getReadPointer(1)+out_success_sample_R};
        // Erase exceeding elements
        int exceed_size_R = _queue_R.size()+out_success_sample_R-_max_queue_size;
        if (exceed_size_R > 0){
            _queue_R.erase(_queue_R.begin(), _queue_R.begin()+exceed_size_R);
        }
        // Insert all elements to the queue
        _queue_R.insert(_queue_R.end(), vector_R.begin(), vector_R.end());
        // RMS
        std::valarray<float> valarray_R{_queue_R.data(), _queue_R.size()};
        std::valarray<float> valarray_pow_R = std::pow(valarray_R, 2.0);
        _last_rms_R = sqrt(valarray_pow_R.sum()/valarray_pow_R.size());
    }

    /*
    float removed = 0;
    if (_queue.size() == _max_queue_size){
        removed = _queue.front();
        _queue.pop();
        _queue.push(current_peak);
    }
    else{
        _queue.push(current_peak);
    }

    float tmp = _max_queue_size * _last_rms - pow(removed, 2.0);
    _last_rms = sqrt((tmp + pow(current_peak, 2.0))/_max_queue_size);
    */
}
float AudioMeter::_rms2Db(float current_rms){
    if (current_rms == 0.0)
        current_rms = static_cast<float>(1e-5);
    return 20.0*log10f(current_rms/1.0);
}

ChannelGui::ChannelGui(ChannelStripSetting* in_strip_setting):strip_setting(in_strip_setting), 
                    meter_gui(in_strip_setting->sample_rate, in_strip_setting->rms_window_ms){
    LR = int((channel_width-2*margin-margin_mini)/2);
    pan_gui.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalDrag);
    //pan_gui.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox);
    pan_gui.setTextBoxStyle (juce::Slider::TextBoxBelow, true, channel_width, 20);
    pan_gui.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colour{240, 162, 2});
    pan_gui.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour{235, 186, 185});
    pan_gui.setColour(juce::Slider::ColourIds::textBoxOutlineColourId , juce::Colour{235, 186, 185});
    pan_gui.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::white);
    pan_gui.setRange(-1.0, 1.0);
    pan_gui.setValue(0.0);
    pan_gui.onValueChange = [this]{_panCallback();};
    addAndMakeVisible(pan_gui);

    plugins.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour{235, 186, 185});
    addAndMakeVisible(plugins);

    fader_gui.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    //fader_gui.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    fader_gui.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colour{235, 186, 185});
    fader_gui.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colour{240, 162, 2});
    fader_gui.setTextBoxStyle (juce::Slider::TextBoxBelow, true, LR, 20*2);
    fader_gui.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour{235, 186, 185});
    fader_gui.setColour(juce::Slider::ColourIds::textBoxOutlineColourId, juce::Colour{235, 186, 185});
    fader_gui.setColour(juce::Slider::ColourIds::textBoxTextColourId, juce::Colours::white);
    fader_gui.setRange(-36.0, 16.0);
    fader_gui.setValue(0.0);
    fader_gui.setTextValueSuffix ("dB");
    fader_gui.setNumDecimalPlacesToDisplay(1);
    fader_gui.onValueChange = [this]{_faderCallback();};
    addAndMakeVisible(fader_gui);

    addAndMakeVisible(meter_gui);

    _channel_shadow_margin = 5;
    _c_channel_bg = juce::Colour{195, 219, 197};
    _c_channel_shadow = juce::Colour{33, 131, 128};;
}
ChannelGui::~ChannelGui(){

}

void ChannelGui::paint(juce::Graphics& g){
    g.fillAll(_c_channel_bg);
    auto area = getLocalBounds();
    g.setColour(_c_channel_shadow);
    g.fillRect(area.removeFromRight(_channel_shadow_margin));
    g.fillRect(area.removeFromBottom(_channel_shadow_margin));
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
    //area.removeFromRight(margin_mini);
    meter_gui.setBounds(area.removeFromRight(meter_gui.meter_width));
    return;
}
void ChannelGui::_panCallback(){
    double a = pan_gui.getValue();
    strip_setting->setPan(static_cast<float>(a));
}
void ChannelGui::_faderCallback(){
    double a = fader_gui.getValue();
    strip_setting->setPostDb(static_cast<float>(a));
}
void ChannelGui::updateAudioMeter(const juce::AudioBuffer<float>* out_buffer, int out_success_sample_L, int out_success_sample_R){
    meter_gui.audioBuffer2Rms(out_buffer, out_success_sample_L, out_success_sample_R);
}