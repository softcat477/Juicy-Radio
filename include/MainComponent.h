#ifndef MAINCOMPONENT_H
#define MAINCOMPONENT_H

#include <thread>
#include <memory>

#include <juce_audio_utils/juce_audio_utils.h>

#include "StereoOut.h"
#include "RadioReceiver.h"
#include "Mp3Decoder.h"
#include "ChannelGui.h"

#include "Osc.h"

#include "Wire.h"
#include "Aggregator.h"
#include "Distributor.h"

namespace AudioApp
{
class MainComponent : public juce::AudioAppComponent {
public:
    MainComponent(size_t sample_per_frame_radio, size_t max_frame_count_radio,
        size_t sample_per_frame, size_t max_frame_count);
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void shutdown();
private:
    ChannelGui* _stereo_out_gui;
    ChannelGui* _gui1;
    ChannelGui* _gui2;

    size_t _sample_per_frame;
    size_t _max_frame_count;

    // left radio
    std::thread _thread_internet1;
    std::thread _thread_decoder1;
    std::thread _thread_st_out1;
    RadioReceiver _radioReceiver1;
    Mp3Decoder _mp3_decoder1;
    StereoOut _stereo_out1;

    // right radio
    std::thread _thread_internet2;
    std::thread _thread_decoder2;
    std::thread _thread_st_out2;
    RadioReceiver _radioReceiver2;
    Mp3Decoder _mp3_decoder2;
    StereoOut _stereo_out2;

    // Sine wave osc
    std::thread _thread_osc;
    Osc _osc;

    // st-out
    std::thread _thread_st_out;
    StereoOut _st_out;

    // Wires
    std::vector<WirePtr<char, 2>> _wires_c2;
    std::vector<WirePtr<float, 2>> _wires_f2;

    juce::AudioIODevice* _device;

    OutJackPtr<float, 2> _stereo_out_wire;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
}

#endif