#ifndef IOPARAMS_H
#define IOPARAMS_H

#include <vector>
#include <juce_audio_utils/juce_audio_utils.h>

struct ChannelStripSetting{
    float _pre_dB;
    //td::vector<AudioFxId> _audio_fx_ids;
    float _post_dB;
    float _pan;

    ChannelStripSetting(float pre_dB, float post_dB, float pan):
                    _pre_dB(pre_dB),
                    _post_dB(post_dB),
                    _pan(pan){
    }
    ChannelStripSetting(const ChannelStripSetting& copied){
        _pre_dB = copied._pre_dB;
        _post_dB = copied._post_dB;
        _pan = copied._pan;
    }
};

// = = = = =
// Encoder Stream Data
// From audio metadata(ex: filename) to raw audio waveform(float)
struct EncoderInputData
{
    // The source to encode audio from.
    //FSoundSource InputSource;
    // this will point to the settings of the submix this callback
    // is encoding to.
    //MixerSubmixSettingsBase* InputSettings;
    size_t required_sample_count;

    //EncoderInputData():required_sample_count(0){}
    EncoderInputData(size_t in_required_sample_count):required_sample_count(in_required_sample_count){}
};
struct EncoderOutputData
{
    // Buffer that the encoding stream will sum into.
    juce::AudioBuffer<float>* audio_buffer;
    size_t sample_count_L;
    size_t sample_count_R;

    EncoderOutputData() = delete;
    EncoderOutputData(juce::AudioBuffer<float>* in_audio_buffer):
                    audio_buffer(in_audio_buffer), sample_count_L(-1), sample_count_R(-1){}
};
// = = = = =
// Decoder Stream
// Render(?) audio according to the SPEAKER setting.(5.1? stereo headphone? mono?)
// More like the mastering stem in DAW.
// Decoder Stream Data:
struct DecoderPositionalData
{
    int OutputNumChannels;

    // FVector is a struct containing three floats
    // representing cartesian coordinates in 3D space.
    //std::vector<FVector> OutputChannelPositions;
    //FQuat ListenerRotation;
};
struct DecoderInputData
{
    // Encoded stream data.
    // Buffer that the encoding stream will sum into.
    juce::AudioBuffer<float>* audio_buffer;
    size_t sample_count;
    // this will point to the settings of the submix this stream
    // was encoded with.
    //MixerSubmixSettingsBase* InputSettings;
    //FMixerDecoderPositionalData& OutputChannelPositions;
};
struct DecoderOutputData
{
    juce::AudioBuffer<float>* audio_buffer;
    size_t sample_count;
};

// = = = = =
// Transcoder Stream
// Convert between different channels. This is a little bit like the decoder stream.
// Transcoder Stream Data
struct TranscoderCallbackData
{
    // encoded stream data.
    // We already have enough space allocated here for
    // the larger of the two streams we are transcoding between.
    juce::AudioBuffer<float>* audio_buffer;
    size_t sample_count;
    // Settings of the submix we are transcoding from.
    //MixerSubmixSettingsBase* SourceStreamSettings;
    // Settings of the submix we are transcoding to.
    //MixerSubmixSettingsBase* DestinationStreamSettings;
};

#endif
