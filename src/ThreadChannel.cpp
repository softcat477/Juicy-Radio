#include "../include/ThreadChannel.h"
#include "../include/IOParams.h"

#include <juce_core/juce_core.h>
#include <thread>

ThreadChannel::ThreadChannel(size_t buf_size, size_t buf_max_frame, IEncoderStream* mp3_decoder):
                _buf_size(buf_size), _stereo_out(), tmp_mp3_decoder(mp3_decoder){
    stereo_out_L = new RingBuffer<float>(_buf_size, buf_max_frame);
    stereo_out_R = new RingBuffer<float>(_buf_size, buf_max_frame);

    _stereo_out.connect(mp3_decoder);
}
ThreadChannel::~ThreadChannel(){
    delete stereo_out_L;
    delete stereo_out_R;
}

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
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }

        // Write to to ring_buffer
        if (success_sample_L > 0){
            size_t success_L = stereo_out_L->lazySmartWrite(out_buffer.getReadPointer(0), success_sample_L);
        }
        if (success_sample_R > 0){
            size_t success_R = stereo_out_R->lazySmartWrite(out_buffer.getReadPointer(1), success_sample_R );
        }
        continue;

        // Decoder stream
        // Skip

        // Debug: A noise generator, SUCCESS!
        /*
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
        */
    }
    printf ("Quit ThreadChannel::start()\n");
}