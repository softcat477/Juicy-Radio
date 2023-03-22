#include "../include/StereoOut.h"
#include "../include/IOParams.h"

#include <chrono>
#include <thread>

StereoOut::StereoOut():
                _channel_setting(0.0, 0.0, 0.0, 44100.0, 300),
                _channel_gui(&_channel_setting){ 
}
StereoOut::~StereoOut(){
}

void StereoOut::start() {
    const std::chrono::duration<double, std::milli> elapsed  (_update_ms/1.35);
    printf ("Elapsed ms : %f\n", _update_ms/1.35);
    while (true) {
        if (this->_isStopped.load() == true)
            break;

        getNextAudioBlock();

        std::this_thread::sleep_for(elapsed);
    }

    printf ("Quit StereoOut::start()\n");
}
/*
1. Read from upstream
2. Pre-gain
3. Audio FX
4. Pan
5. Post-gain
*/
void StereoOut::getNextAudioBlock(){
    int num_samples = 512;
    // Read from Upstream
    const size_t required_samples = static_cast<size_t>(num_samples);
    std::vector<float> left(required_samples);
    std::vector<float> right(required_samples);
    std::vector<float*> LR = {left.data(), right.data()};

    auto success_sample = _aggregator.popAudio(&LR, required_samples);
    size_t _success_sample_L = success_sample[0];
    size_t _success_sample_R = success_sample[1];

    // A dummy buffer to do all the operation
    juce::AudioBuffer<float> cache {2, num_samples};
    cache.copyFrom(0, 0, left.data(), static_cast<int>(_success_sample_L));
    cache.copyFrom(1, 0, right.data(), static_cast<int>(_success_sample_R));

    // 2. pre-gain
    // out_buffer->applyGain(0, static_cast<int>(required_samples), juce::Decibels::decibelsToGain(_channel_setting._pre_dB));

    // 3. Apply Audio FXs
    // Pan
    float pan = _channel_setting.getPan();
    float theta = juce::jmap(pan, static_cast<float>(-1.0), static_cast<float>(1.0),
                            static_cast<float>(0.0), static_cast<float>(M_PI/2.0));
    float pan_gan_L = cos(theta);
    float pan_gan_R = sin(theta);
    cache.applyGain(0, 0, static_cast<int>(_success_sample_L), pan_gan_L);
    cache.applyGain(1, 0, static_cast<int>(_success_sample_R), pan_gan_R);

    // 4. Apply post gain
    float post_dB = _channel_setting.getPostDb();
    cache.applyGain(0, 0, static_cast<int>(_success_sample_L), juce::Decibels::decibelsToGain(post_dB));
    cache.applyGain(1, 0, static_cast<int>(_success_sample_R), juce::Decibels::decibelsToGain(post_dB));
    _channel_gui.updateAudioMeter(&cache, _success_sample_R, _success_sample_R);

    // Push to distributor
    _distributor.pushAudio(cache.getReadPointer(0), _success_sample_L, 0);
    _distributor.pushAudio(cache.getReadPointer(1), _success_sample_R, 1);
}