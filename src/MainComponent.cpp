#include "../include/MainComponent.h"
#include "../include/RingBuffer.h"
#include "../include/ThreadInternet.h"

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
                        _decoder_manager(_internet_manager.ring_buffer, 1152, 512) // pass, 1152, 128
{
    // Points to ring buffers that store the decoded floating points.
    _buffer_pcm_L = _decoder_manager.pcm_buffer_L;
    _buffer_pcm_R = _decoder_manager.pcm_buffer_R;

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

    // Start two threads
    this->_thread_internet = std::thread(&ThreadInternet::start, &_internet_manager);
    this->_thread_decoder = std::thread(&ThreadDecoder::start, &_decoder_manager);

    setSize(300, 200);

    setAudioChannels(0, 2);
}
MainComponent::~MainComponent()
{
    shutdownAudio();
}
void MainComponent::shutdown(){
    _internet_manager.setStop();
    _decoder_manager.setStop();
    this->_thread_internet.join();
    this->_thread_decoder.join();
}
void MainComponent::paint(juce::Graphics& g)
{
    //g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}
void MainComponent::resized()
{
    _open_button.setBounds(10, 10, getWidth()-20, 20);
    _clear_button.setBounds(10, 40, getWidth()-20, 20);
    _db_slider.setBounds(10, 70, getWidth()-20, 20);
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
    
    // func()

    float* tmp_ptr = (float*)malloc(sizeof(float)*static_cast<unsigned long>(numSamples));
    if (this->_buffer_pcm_L->canSmartRead()){
        size_t count = _buffer_pcm_L->getSmartRead(tmp_ptr, static_cast<size_t>(numSamples));
        if (count != static_cast<size_t>(numSamples)){
            printf("MainComponent::getNextAudioBlock: not enough samples in tmp_ptr, need %d get %zu, %zu/%zu\n", numSamples, count, _buffer_pcm_L->getFrameCount(), _buffer_pcm_L->getMaxFrameCount());
        }
        memcpy(bufferToFill.buffer->getWritePointer(0, startSample), tmp_ptr, sizeof(float) * count);
        bufferToFill.buffer->applyGain(0, 0, static_cast<int>(count), static_cast<float>(_output_gain));
    }
    if (this->_buffer_pcm_R->canSmartRead()){
        size_t count = _buffer_pcm_R->getSmartRead(tmp_ptr, static_cast<size_t>(numSamples));
        if (count != static_cast<size_t>(numSamples)){
            printf("MainComponent::getNextAudioBlock: not enough samples in tmp_ptr, need %d get %zu, %zu/%zu\n", numSamples, count, _buffer_pcm_R->getFrameCount(), _buffer_pcm_R->getMaxFrameCount());
        }
        // DEBUG, write decoded floating points to a file and synthesis .wav with python.
        memcpy(bufferToFill.buffer->getWritePointer(1, startSample), tmp_ptr, sizeof(float) * count);
        bufferToFill.buffer->applyGain(1, 0, static_cast<int>(count), static_cast<float>(_output_gain));
    }
    free (tmp_ptr);
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