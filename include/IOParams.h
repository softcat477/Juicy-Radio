#ifndef IOPARAMS_H
#define IOPARAMS_H

#include <vector>
#include <atomic>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_audio_devices/juce_audio_devices.h>

struct ChannelStripSetting{
    std::atomic<float> pre_dB;
    //td::vector<AudioFxId> _audio_fx_ids;
    std::atomic<float> post_dB;
    std::atomic<float> pan;

    double sample_rate;
    int rms_window_ms;
    juce::AudioDeviceManager::AudioDeviceSetup device_spec;


    float getPreDb() const {return pre_dB.load();}
    float getPostDb() const {return post_dB.load();}
    float getPan() const {return pan.load();}
    void setPreDb(float in_pre_dB){pre_dB.store(in_pre_dB);}
    void setPostDb(float in_post_dB){post_dB.store(in_post_dB);}
    void setPan(float in_pan){pan.store(in_pan);}

    ChannelStripSetting() = delete;
    ChannelStripSetting(float in_pre_dB, float in_post_dB, float in_pan, double in_sample_rate, int in_rms_window_ms):
                    pre_dB(in_pre_dB),
                    post_dB(in_post_dB),
                    pan(in_pan),
                    sample_rate(in_sample_rate),
                    rms_window_ms(in_rms_window_ms){
    }
    ChannelStripSetting(float in_pre_dB, float in_post_dB, float in_pan, juce::AudioDeviceManager::AudioDeviceSetup in_device_spec):
                    pre_dB(in_pre_dB),
                    post_dB(in_post_dB),
                    pan(in_pan),
                    device_spec(in_device_spec){
    }
    ChannelStripSetting(const ChannelStripSetting& copied){
        setPreDb(copied.getPreDb());
        setPostDb(copied.getPostDb());
        setPan(copied.getPan());
        sample_rate = copied.sample_rate;
        rms_window_ms = copied.rms_window_ms;
        device_spec = copied.device_spec;
        //pre_dB = copied.pre_dB;
        //post_dB = copied.post_dB;
        //pan = copied.pan;
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
                    audio_buffer(in_audio_buffer), sample_count_L(0), sample_count_R(0){}
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
