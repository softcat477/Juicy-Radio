#include "../include/ChannelStrip.h"
ChannelStrip::ChannelStrip():_channel_setting(0.0, 0.0, 0.0){

}
ChannelStrip::ChannelStrip(ChannelStripSetting channel_strip_setting):
                    _channel_setting(channel_strip_setting){

}
ChannelStrip::~ChannelStrip(){

}

bool ChannelStrip::connect(ChannelStrip* channel_strip){
    _input_strips.emplace_back(channel_strip);
    return true;
}
bool ChannelStrip::connect(IEncoderStream* encoder_stream){
    _input_enc_streams.emplace_back(encoder_stream);
    return true;
}

bool ChannelStrip::remove(ChannelStrip* channel_strip){
    printf ("ChannelStrip::remove(ChannelStrip*) not implemented!\n");
    return false;
}
bool ChannelStrip::remove(IEncoderStream* encoder_stream){
    printf ("ChannelStrip::remove(IEncoderStream*) not implemented!\n");
    return false;
}
void ChannelStrip::setPreGain(float pre_db){
    _channel_setting._pre_dB = pre_db;
}
void ChannelStrip::setPostGain(float post_db){
    _channel_setting._post_dB = post_db;
}
void ChannelStrip::setPan(float pan){
    _channel_setting._pan = pan;
}

void ChannelStrip::processAndMixAudio(juce::AudioBuffer<float>* out_buffer, int& out_success_sample_L, int& out_success_sample_R, size_t required_samples){
    // Fill <sample_count> * <channels> sample in <out_buf>
    // 1. Sum all connected strips and connected events to the left/right channel.
    // 2. Pre gain
    // 3. Apply Audio FXs
    // 4. Post gain
    // 5. Pan

    // 1. Connected strips and encoder streams
    //for (ChannelStrip* input_strip : _input_strips){
    //    input_strip->processAndMixAudio(out_buffer, required_samples);
    //}

    EncoderInputData enc_input_data_dummy{required_samples};
    EncoderOutputData enc_output_data{out_buffer};
    for (IEncoderStream* enc_stream : _input_enc_streams){
        enc_stream->encodeSumIntoBuffer(&enc_input_data_dummy, &enc_output_data);
        out_success_sample_L = enc_output_data.sample_count_L;
        out_success_sample_R = enc_output_data.sample_count_R;
    }

    // 2. pre-gain
    //out_buffer->applyGain(0, static_cast<int>(required_samples), juce::Decibels::decibelsToGain(_channel_setting._pre_dB));

    // 3. Apply Audio FXs
    // Skip

    // 4. Apply post gain
    //out_buffer->applyGain(0, static_cast<int>(required_samples), juce::Decibels::decibelsToGain(_channel_setting._post_dB));

    // 5. Pan
    // Skip
}
