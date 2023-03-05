#include "../include/ThreadChannel.h"
#include "../include/IOParams.h"

#include <juce_core/juce_core.h>
#include <thread>

ThreadChannel::ThreadChannel(size_t buf_size, size_t buf_max_frame, IEncoderStream* mp3_decoder):
                _buf_size(buf_size), _stereo_out(){
    _stereo_out.connect(mp3_decoder);
}
ThreadChannel::~ThreadChannel(){
}
void ThreadChannel::start(){
    printf ("ThreadChannel::start() is not implemented yet :<");
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
void ThreadChannel::getNextAudioBlock(juce::AudioBuffer<float>* out_buffer, int num_samples, int& success_sample_L, int& success_sample_R){
    _stereo_out.processAndMixAudio(out_buffer, success_sample_L, success_sample_R, static_cast<size_t>(num_samples));

}