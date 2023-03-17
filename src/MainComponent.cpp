#include <cstdio>
//#include <juce_audio_basics/juce_audio_basics.h>

#include "../include/MainComponent.h"

namespace AudioApp
{
MainComponent::MainComponent(size_t sample_per_frame_radio, size_t max_frame_count_radio,
    size_t sample_per_frame, size_t max_frame_count): // 8192, 128
                        _radioReceiver{sample_per_frame_radio}, // 8192, magic(?) number for curl
                        _mp3_decoder(), // 1152, 128
                        _stereo_out()
{
    // Get samples per frame
    setAudioChannels(0, 2);
    _device = deviceManager.getCurrentAudioDevice();
    sample_per_frame = static_cast<size_t>(_device->getCurrentBufferSizeSamples());
    _stereo_out.setUpdateCycle((sample_per_frame/_device->getCurrentSampleRate())*1000.0);

    // Connect Wires
    _wires_c2.push_back(std::make_shared<Wire<char, 2>>(sample_per_frame_radio, max_frame_count_radio));
    _radioReceiver.connectOut(_wires_c2.back());
    _mp3_decoder.connectIn(_wires_c2.back());
    _mp3_decoder.listenTo(_radioReceiver.getSignal());

    _wires_f2.push_back(std::make_shared<Wire<float, 2>>(sample_per_frame, max_frame_count));
    _mp3_decoder.connectOut(_wires_f2.back());
    _stereo_out.connectIn(_wires_f2.back());

    // The output jack of the output wire from _stereo_out
    _wires_f2.push_back(std::make_shared<Wire<float, 2>>(sample_per_frame, max_frame_count));
    _stereo_out.connectOut(_wires_f2.back());
    _stereo_out_wire = _wires_f2.back()->getOutPtr();

    // stereo out
    _stereo_out_gui = _stereo_out.getStereoOut();
    addAndMakeVisible(*_stereo_out_gui);

    // Start two threads
    this->_thread_internet = std::thread(&RadioReceiver::start, &_radioReceiver);
    this->_thread_decoder = std::thread(&Mp3Decoder::start, &_mp3_decoder);
    this->_thread_st_out = std::thread(&StereoOut::start, &_stereo_out);

    setSize(120, 480);
}
MainComponent::~MainComponent()
{
    shutdownAudio();
}
void MainComponent::shutdown(){
    _radioReceiver.setStop();
    _mp3_decoder.setStop();
    _stereo_out.setStop();
    this->_thread_internet.join();
    this->_thread_decoder.join();
    this->_thread_st_out.join();
}
void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::azure);
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
    //int startSample = bufferToFill.startSample;
    //int numSamples = bufferToFill.numSamples;

    // Copy to buffer
    int success_sample_L = 0;
    int success_sample_R = 0;

    if (!_stereo_out_wire)
        return;

    // Read from Upstream
    const size_t required_samples = static_cast<size_t>(bufferToFill.numSamples);
    std::vector<float> left(required_samples);
    std::vector<float> right(required_samples);
    std::vector<float*> LR = {left.data(), right.data()};

    std::vector<size_t> success_sample = _stereo_out_wire->popAudio(&LR, required_samples);
    (bufferToFill.buffer)->addFrom(0, 0, left.data(), static_cast<int>(success_sample[0]));
    (bufferToFill.buffer)->addFrom(1, 0, right.data(), static_cast<int>(success_sample[1]));
}
void MainComponent::releaseResources()
{
    printf ("Release resources\n");
}
} // namespace GuiApp