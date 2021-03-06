#include "../include/MainComponent.h"
#include "../include/RingBuffer.h"
#include "../include/ThreadInternet.h"
#include "../include/IOParams.h"

#include <stdio.h>
#include <fstream>

#include <juce_audio_basics/juce_audio_basics.h>

namespace AudioApp
{
MainComponent::MainComponent(size_t sample_per_frame, size_t max_frame_count): // 8192, 128
                        _output_gain(0.0),
                        _sample_per_frame(sample_per_frame),
                        _max_frame_count(max_frame_count),
                        _internet_manager(sample_per_frame, max_frame_count), // 8192, 128
                        _decoder_manager(_internet_manager.ring_buffer, 1152, 512), // pass, 1152, 128
                        _channel_manager(512, 1152, &_decoder_manager)
{
    /*
    // Points to ring buffers that store the decoded floating points.
    _buffer_pcm_L = _channel_manager.stereo_out_L;
    _buffer_pcm_R = _channel_manager.stereo_out_R;

    // Open Button
    addAndMakeVisible(_open_button);
    _open_button.setButtonText("Open");
    _open_button.onClick = [this]{_openButtonClicked();};

    // Close Button
    addAndMakeVisible(_clear_button);
    _clear_button.setButtonText("Close");
    _clear_button.onClick = [this]{_clearButtonClicked();};

    // dB Slider
    addAndMakeVisible(_db_slider);
    _db_slider.setRange(-36.0, 3.0);
    _db_slider.setValue(-36.0);
    _db_slider.addListener(this);
    _db_slider.setTextValueSuffix (" dB");
    _db_slider.setNumDecimalPlacesToDisplay(2);
    */

    // stereo out
    _stereo_out_gui = _channel_manager.getStereoOut();
    addAndMakeVisible(*_stereo_out_gui);

    // Start two threads
    this->_thread_internet = std::thread(&ThreadInternet::start, &_internet_manager);
    this->_thread_decoder = std::thread(&ThreadDecoder::start, &_decoder_manager);
    //this->_thread_channel = std::thread(&ThreadChannel::start, &_channel_manager);

    setSize(640, 480);

    setAudioChannels(0, 2);

    printf ("Gain 0.5 = %f dB\n",  juce::Decibels::gainToDecibels(0.5));
}
MainComponent::~MainComponent()
{
    shutdownAudio();
}
void MainComponent::shutdown(){
    _internet_manager.setStop();
    _decoder_manager.setStop();
    //_channel_manager.setStop();
    this->_thread_internet.join();
    this->_thread_decoder.join();
    //this->_thread_channel.join();
}
void MainComponent::paint(juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
void MainComponent::resized()
{
    /*
    _open_button.setBounds(10, 10, getWidth()-20, 20);
    _clear_button.setBounds(10, 40, getWidth()-20, 20);
    _db_slider.setBounds(10, 70, getWidth()-20, 20);
    */

    auto area = getLocalBounds();
    _stereo_out_gui->setBounds(area.removeFromLeft(_stereo_out_gui->channel_width));
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    printf ("Prepare to play with %d/%f\n", samplesPerBlockExpected, sampleRate);
}
void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();
    int startSample = bufferToFill.startSample;
    int numSamples = bufferToFill.numSamples;

    juce::AudioBuffer<float> tmp_audiobuffer{2, numSamples};
    tmp_audiobuffer.clear();
    int success_sample_L = 0;
    int success_sample_R = 0;
    _channel_manager.getNextAudioBlock(&tmp_audiobuffer, numSamples, success_sample_L, success_sample_R);

    bufferToFill.buffer->copyFrom(0, startSample, tmp_audiobuffer, 0, 0, success_sample_L);
    bufferToFill.buffer->copyFrom(1, startSample, tmp_audiobuffer, 1, 0, success_sample_R);
    /* Submix thread
    size_t success_read_L = this->_buffer_pcm_L->lazySmartRead(bufferToFill.buffer, startSample, numSamples, 0);
    bufferToFill.buffer->applyGain(0, 0, static_cast<int>(success_read_L), static_cast<float>(_output_gain));
    size_t success_read_R = this->_buffer_pcm_R->lazySmartRead(bufferToFill.buffer, startSample, numSamples, 1);
    bufferToFill.buffer->applyGain(1, 0, static_cast<int>(success_read_R), static_cast<float>(_output_gain));
    */
}
void MainComponent::releaseResources()
{
    printf ("Release resources\n");
}
void MainComponent::sliderValueChanged(juce::Slider* slider){
    double new_db = slider->getValue();
    _output_gain = juce::Decibels::decibelsToGain(new_db);
}

void MainComponent::_openButtonClicked()
{
    printf("Open Button\n");
}
void MainComponent::_clearButtonClicked()
{
    //_internet_manager.setStop();
    printf("Clear Button\n");
}
} // namespace GuiApp