#include "../include/MainComponent.h"
#include "../include/RingBuffer.h"
#include "../include/IOParams.h"

#include <stdio.h>
#include <fstream>

#include <juce_audio_basics/juce_audio_basics.h>

namespace AudioApp
{
MainComponent::MainComponent(size_t sample_per_frame, size_t max_frame_count): // 8192, 128
                        _sample_per_frame(sample_per_frame),
                        _max_frame_count(max_frame_count),
                        _internet{sample_per_frame, max_frame_count}, // 8192, 128
                        _mp3_decoder(1152, 512, &_internet), // pass, 1152, 128
                        _stereo_out(&_mp3_decoder)
{
    // stereo out
    _stereo_out_gui = _stereo_out.getStereoOut();
    addAndMakeVisible(*_stereo_out_gui);

    // Start two threads
    this->_thread_internet = std::thread(&Internet::start, &_internet);
    this->_thread_decoder = std::thread(&Mp3Decoder::start, &_mp3_decoder);

    setSize(640, 480);

    setAudioChannels(0, 2);

    printf ("Gain 0.5 = %f dB\n",  juce::Decibels::gainToDecibels(0.5));
}
MainComponent::~MainComponent()
{
    shutdownAudio();
}
void MainComponent::shutdown(){
    _internet.setStop();
    _mp3_decoder.setStop();
    this->_thread_internet.join();
    this->_thread_decoder.join();
}
void MainComponent::paint(juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
void MainComponent::resized()
{
    auto area = getLocalBounds();
    _stereo_out_gui->setBounds(area.removeFromLeft(_stereo_out_gui->channel_width));
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    printf ("Prepare to play with %d/%f\n", samplesPerBlockExpected, sampleRate);
}
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Clear buffer
    bufferToFill.clearActiveBufferRegion();
    int startSample = bufferToFill.startSample;
    int numSamples = bufferToFill.numSamples;

    // Copy to buffer
    int success_sample_L = 0;
    int success_sample_R = 0;
    _stereo_out.getNextAudioBlock(bufferToFill.buffer, bufferToFill.numSamples, success_sample_L, success_sample_R);
}
void MainComponent::releaseResources()
{
    printf ("Release resources\n");
}
} // namespace GuiApp