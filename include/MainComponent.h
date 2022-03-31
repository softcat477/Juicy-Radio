#ifndef MAINCOMPONENT_H
#define MAINCOMPONENT_H

#include <juce_audio_utils/juce_audio_utils.h>

#include <thread>
#include "../include/ThreadInternet.h"
#include "../include/ThreadDecoder.h"
#include "../include/ThreadChannel.h"
#include "../include/RingBuffer.h"
#include "../include/CondVar.h"

namespace AudioApp
{
class MainComponent : public juce::AudioAppComponent, public juce::Slider::Listener{
public:
    MainComponent(size_t sample_per_frame, size_t max_frame_count);
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void shutdown();

    void sliderValueChanged(juce::Slider* slider) override;

private:
    void _openButtonClicked();
    void _clearButtonClicked();

    juce::TextButton _open_button;
    juce::TextButton _clear_button;
    juce::Slider _db_slider;

    double _output_gain;

    size_t _sample_per_frame;
    size_t _max_frame_count;

    std::thread _thread_internet;
    std::thread _thread_decoder;
    std::thread _thread_channel;

    // Connect to a radio station and receive mp3 frames with LIBCURL.
    // Hold a RingBuffer to store chunks of data, and use _cond_mp3 to inform
    // the holder that there are new data in the ring buffer.
    ThreadInternet _internet_manager;
    // In charge of decoding data in a ring buffer to waveform.
    // Connect to ThreadInternet::RingBuffer and decode mp3 frames into floating points
    // and store them into TWO ring buffers (left channel and right channel)
    ThreadDecoder _decoder_manager; // RingBuffer for pcm data, _cond_pcm

    ThreadChannel _channel_manager;


    // Read from these two RingBuffer pointer and send to the audio device.
    // Point to the ring buffer holded by ThreadDecoder
    RingBuffer<float>* _buffer_pcm_L;
    // Point to the ring buffer holded by ThreadDecoder
    RingBuffer<float>* _buffer_pcm_R;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
}

#endif