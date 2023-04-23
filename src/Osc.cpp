#include "../include/Osc.h"
#include "../include/IOParams.h"

#include <cmath>    
#include <math.h>
#include <chrono>
#include <thread>

#include <iostream>

Osc::Osc():
    _channel_setting(0.0, 0.0, 0.0, 44100.0, 300),
    _channel_gui(&_channel_setting){
}
Osc::~Osc(){
}

void Osc::prepareTable() {
    // Wave table
    _tsize = 256;
    _table.reserve(_tsize);
    double hop = juce::MathConstants<double>::twoPi / static_cast<double>(_tsize - 1);
    for (int i = 0; i < _tsize; i++) {
        float val = std::sin(hop * i);
        _table[i] = val;
        std::cout << "idx " << i << " " << _table[i] << "\n";
    }

    _prev = 0;
    std::cout << "_prev/_f/_fs: " << _prev << "/" << _f << "/" << _fs << "\n";
}

void Osc::start() {
    const std::chrono::duration<double, std::milli> elapsed  (_update_ms/1.35);
    printf ("Elapsed ms : %f\n", _update_ms/1.35);
    while (true) {
        if (this->_isStopped.load() == true)
            break;

        getNextAudioBlock();

        std::this_thread::sleep_for(elapsed);
    }

    printf ("Quit Osc::start()\n");
}
/*
1. Read from upstream
2. Pre-gain
3. Audio FX
4. Pan
5. Post-gain
*/
void Osc::getNextAudioBlock(){
    int num_samples = 512;
    // Read from Upstream
    const size_t required_samples = static_cast<size_t>(num_samples);

    // A dummy buffer to do all the operation
    juce::AudioBuffer<float> cache {2, num_samples};
    size_t _success_sample_L = num_samples;
    size_t _success_sample_R = num_samples;

    // TODO: Synthesize <num_samples> samples
    float kinc = _f / _fs * static_cast<float>(_tsize);
    for (int i = 0; i < required_samples; i++) {
        float cur = fmod((_prev + kinc), static_cast<float>(_tsize));
        _prev = cur;
        // Lookup
        float y = _table[static_cast<int>(cur)];
        cache.setSample(0, i, y);
        cache.setSample(1, i, y);
    }

    // 3. Apply Audio FXs
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