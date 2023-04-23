#include <cstdio>
//#include <juce_audio_basics/juce_audio_basics.h>

#include "../include/MainComponent.h"

namespace AudioApp
{
MainComponent::MainComponent(size_t sample_per_frame_radio, size_t max_frame_count_radio, size_t sample_per_frame, size_t max_frame_count): // 8192, 128
                        _radioReceiver1{sample_per_frame_radio}, // 8192, magic(?) number for curl
                        _mp3_decoder1(), // 1152, 128
                        _stereo_out1(),
                        _radioReceiver2{sample_per_frame_radio}, // 8192, magic(?) number for curl
                        _mp3_decoder2(), // 1152, 128
                        _stereo_out2(),
                        _osc{},
                        _st_out()
{
    // Get samples per frame
    setAudioChannels(0, 2);
    _device = deviceManager.getCurrentAudioDevice();
    sample_per_frame = static_cast<size_t>(_device->getCurrentBufferSizeSamples());

    // Connect left radio channel
    _wires_c2.push_back(std::make_shared<Wire<char, 2>>(sample_per_frame_radio, max_frame_count_radio));
    _radioReceiver1.connectOut(_wires_c2.back());
    _mp3_decoder1.connectIn(_wires_c2.back());
    _mp3_decoder1.listenTo(_radioReceiver1.getSignal());

    _wires_f2.push_back(std::make_shared<Wire<float, 2>>(sample_per_frame, max_frame_count));
    _mp3_decoder1.connectOut(_wires_f2.back());
    _stereo_out1.connectIn(_wires_f2.back());

    _wires_f2.push_back(std::make_shared<Wire<float, 2>>(sample_per_frame, max_frame_count));
    _stereo_out1.connectOut(_wires_f2.back());
    _st_out.connectIn(_wires_f2.back());
    
    // Conncet OSC
    _wires_f2.push_back(std::make_shared<Wire<float, 2>>(sample_per_frame, max_frame_count));
    _osc.connectOut(_wires_f2.back());
    _st_out.connectIn(_wires_f2.back());

    // stereo-out
    _wires_f2.push_back(std::make_shared<Wire<float, 2>>(sample_per_frame, max_frame_count));
    _st_out.connectOut(_wires_f2.back());
    _stereo_out_wire = _wires_f2.back()->getOutPtr();

    // Make Guis visible
    _gui1 = _stereo_out1.getStereoOut();
    //_gui2 = _stereo_out2.getStereoOut();
    _gui2 = _osc.getStereoOut();
    _stereo_out_gui = _st_out.getStereoOut();
    addAndMakeVisible(_gui1);
    addAndMakeVisible(_gui2);
    addAndMakeVisible(_stereo_out_gui);

    setSize(360, 480);

    // Start threads
    this->_thread_internet1 = std::thread(&RadioReceiver::start, &_radioReceiver1);
    this->_thread_decoder1 = std::thread(&Mp3Decoder::start, &_mp3_decoder1);
    this->_thread_st_out1 = std::thread(&StereoOut::start, &_stereo_out1);

    this->_thread_osc = std::thread(&Osc::start, &_osc);

    this->_thread_st_out = std::thread(&StereoOut::start, &_st_out);
}
MainComponent::~MainComponent()
{
    shutdownAudio();
}
void MainComponent::shutdown(){
    _radioReceiver1.setStop();
    _mp3_decoder1.setStop();
    _stereo_out1.setStop();

    _osc.setStop();

    _st_out.setStop();

    this->_thread_internet1.join();
    this->_thread_decoder1.join();
    this->_thread_st_out1.join();

    this->_thread_osc.join();

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
    _gui1->setBounds(area.removeFromLeft(_gui1->channel_width));
    _gui2->setBounds(area.removeFromLeft(_gui2->channel_width));
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    printf ("Prepare to play with %d/%f\n", samplesPerBlockExpected, sampleRate);
    _device = deviceManager.getCurrentAudioDevice();
    size_t sample_per_frame = static_cast<size_t>(_device->getCurrentBufferSizeSamples());
    _stereo_out1.setUpdateCycle((samplesPerBlockExpected/sampleRate)*1000.0);
    _stereo_out2.setUpdateCycle((samplesPerBlockExpected/sampleRate)*1000.0);
    _st_out.setUpdateCycle((samplesPerBlockExpected/sampleRate)*1000.0);
    _osc.setUpdateCycle((samplesPerBlockExpected/sampleRate)*1000.0);
    _osc.setF(440.0);
    _osc.setFs(_device->getCurrentSampleRate());

    _osc.prepareTable();
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