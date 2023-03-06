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

    ChannelStripSetting& operator=(ChannelStripSetting& other) = delete;
    ChannelStripSetting(ChannelStripSetting&& other) = delete;
    ChannelStripSetting& operator=(ChannelStripSetting&& other) = delete;
};

#endif
