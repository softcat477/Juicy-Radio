#ifndef MAINCOMPONENT_H
#define MAINCOMPONENT_H

#include <thread>

#include <juce_audio_utils/juce_audio_utils.h>

#include "StereoOut.h"
#include "Internet.h"
#include "Mp3Decoder.h"
#include "ChannelGui.h"

namespace AudioApp
{
class MainComponent : public juce::AudioAppComponent {
public:
    MainComponent(size_t sample_per_frame, size_t max_frame_count);
    ~MainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void releaseResources() override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;

    void paint(juce::Graphics&) override;
    void resized() override;

    void shutdown();
private:
    ChannelGui* _stereo_out_gui;

    size_t _sample_per_frame;
    size_t _max_frame_count;

    std::thread _thread_internet;
    std::thread _thread_decoder;

    // Connect to a radio station and receive mp3 frames with LIBCURL.
    // Hold a RingBuffer to store chunks of data, and use _cond_mp3 to inform
    // the holder that there are new data in the ring buffer.
    Internet _internet;
    // In charge of decoding data in a ring buffer to waveform.
    // and store them into TWO ring buffers (left channel and right channel)
    // ThreadDecoder _mp3_decoder; // RingBuffer for pcm data, _cond_pcm
    Mp3Decoder _mp3_decoder; // RingBuffer for pcm data, _cond_pcm

    StereoOut _stereo_out;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
}

#endif