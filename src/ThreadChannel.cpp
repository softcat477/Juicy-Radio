#include "../include/ThreadChannel.h"
#include "../include/IOParams.h"

#include <juce_core/juce_core.h>
#include <thread>

ThreadChannel::ThreadChannel(IChannel<float>* mp3_decoder):
                _channel_setting(0.0, 0.0, 0.0, 44100.0, 300),
                _channel_gui(&_channel_setting){ 
    _input_enc_streams.emplace_back(mp3_decoder);
}
ThreadChannel::~ThreadChannel(){
}
/*
void ThreadChannel::start(){
    while (true){
        if (this->_isStopped.load() == true){
            break;
        }

        juce::AudioBuffer<float> out_buffer{2, static_cast<int>(_buf_size)};
        out_buffer.clear();

        int success_sample_L = 0;
        int success_sample_R = 0;
        _stereo_out.processAndMixAudio(&out_buffer, success_sample_L, success_sample_R, _buf_size);

        if (success_sample_L == 0 && success_sample_R == 0){
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        // Write to to ring_buffer
        if (success_sample_L > 0){
            size_t success_L = stereo_out_L->write(out_buffer.getReadPointer(0), success_sample_L);
            printf ("SUS\n");
        }
        if (success_sample_R > 0){
            size_t success_R = stereo_out_R->write(out_buffer.getReadPointer(1), success_sample_R );
        }
        continue;

        // Decoder stream
        // Skip

        // Debug: A noise generator, SUCCESS!
        juce::Random random{};
        juce::AudioBuffer<float> out_buffer{2, static_cast<int>(_buf_size)};
        out_buffer.clear();
        for (auto channel = 0; channel < out_buffer.getNumChannels(); ++channel){
            // Get a pointer to the start sample in the buffer for this audio output channel
            auto* buffer = out_buffer.getWritePointer (channel, 0);
             // Fill the required number of samples with noise between -0.125 and +0.125
            for (auto sample = 0; sample < _buf_size; ++sample)
                buffer[sample] = random.nextFloat() * 1.0f - 0.5f;
        }
    }
    printf ("Quit ThreadChannel::start()\n");
}
*/

/*
1. Read from upstream
2. Pre-gain
3. Audio FX
4. Pan
5. Post-gain
*/
void ThreadChannel::getNextAudioBlock(juce::AudioBuffer<float>* out_buffer, int num_samples, int& success_sample_L, int& success_sample_R){
    // Read from Upstream
    const size_t required_samples = static_cast<size_t>(num_samples);
    std::vector<float> left(required_samples);
    std::vector<float> right(required_samples);
    auto [_success_sample_L, _success_sample_R] = _input_enc_streams[0]->popAudio(&left, &right, required_samples);

    // Add to the output buffer
    out_buffer->addFrom(0, 0, left.data(), static_cast<int>(_success_sample_L));
    out_buffer->addFrom(1, 0, right.data(), static_cast<int>(_success_sample_R));
    success_sample_L = _success_sample_L;
    success_sample_R = _success_sample_R;

    // 2. pre-gain
    // out_buffer->applyGain(0, static_cast<int>(required_samples), juce::Decibels::decibelsToGain(_channel_setting._pre_dB));

    // 3. Apply Audio FXs
    // Pan
    float pan = _channel_setting.getPan();
    float theta = juce::jmap(pan, static_cast<float>(-1.0), static_cast<float>(1.0),
                             static_cast<float>(0.0), static_cast<float>(M_PI/2.0));
    float pan_gan_L = cos(theta);
    float pan_gan_R = sin(theta);
    out_buffer->applyGain(0, 0, static_cast<int>(_success_sample_L), pan_gan_L);
    out_buffer->applyGain(1, 0, static_cast<int>(_success_sample_R), pan_gan_R);

    // 4. Apply post gain
    float post_dB = _channel_setting.getPostDb();
    out_buffer->applyGain(0, 0, static_cast<int>(_success_sample_L), juce::Decibels::decibelsToGain(post_dB));
    out_buffer->applyGain(1, 0, static_cast<int>(_success_sample_R), juce::Decibels::decibelsToGain(post_dB));
    _channel_gui.updateAudioMeter(out_buffer, _success_sample_R, _success_sample_R);
}