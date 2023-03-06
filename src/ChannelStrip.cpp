#include "../include/ChannelStrip.h"
#include <math.h>
ChannelStrip::ChannelStrip(juce::AudioDeviceManager::AudioDeviceSetup device_spec):_channel_setting(0.0, 0.0, 0.0, device_spec),
                             _channel_gui(&_channel_setting){

}
ChannelStrip::ChannelStrip():_channel_setting(0.0, 0.0, 0.0, 44100.0, 300),
                             _channel_gui(&_channel_setting){

}
ChannelStrip::ChannelStrip(ChannelStripSetting channel_strip_setting):
                    _channel_setting(channel_strip_setting),
                    _channel_gui(&_channel_setting){

}
ChannelStrip::~ChannelStrip(){

}

bool ChannelStrip::connect(ChannelStrip* channel_strip){
    _input_strips.emplace_back(channel_strip);
    return true;
}
bool ChannelStrip::connect(IChannel<float>* encoder_stream){
    _input_enc_streams.emplace_back(encoder_stream);
    return true;
}

bool ChannelStrip::remove(ChannelStrip* channel_strip){
    printf ("ChannelStrip::remove(ChannelStrip*) not implemented!\n");
    return false;
}
bool ChannelStrip::remove(IChannel<float>* encoder_stream){
    printf ("ChannelStrip::remove(IEncoderStream*) not implemented!\n");
    return false;
}
/*
void ChannelStrip::setPreDb(float pre_db){
    _channel_setting.setPreDb(pre_db);
}
void ChannelStrip::setPostDb(float post_db){
    _channel_setting.setPostDb(post_db);
}
void ChannelStrip::setPan(float pan){
    _channel_setting.setPan(pan);
}
*/
float ChannelStrip::getPreDb(){
    return _channel_setting.getPreDb();
}
float ChannelStrip::getPostDb(){
        return _channel_setting.getPostDb();
}
float ChannelStrip::getPan(){
    return _channel_setting.getPan();
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

    // === TODO: Unpack this line
    //_input_enc_streams[0]->encodeSumIntoBuffer(&enc_input_data_dummy, &enc_output_data);
    // === to these lines
    size_t required_sample_count = enc_input_data_dummy.required_sample_count;
    juce::AudioBuffer<float> intermediate_buf{2, static_cast<int>(required_sample_count)};
    intermediate_buf.clear();

    std::vector<float> left(required_sample_count);
    std::vector<float> right(required_sample_count);

    auto [success_sample_L, success_sample_R] = _input_enc_streams[0]->popAudio(&left, &right, required_sample_count);

    memcpy(intermediate_buf.getWritePointer(0, 0), left.data(), sizeof(float) * success_sample_L);
    memcpy(intermediate_buf.getWritePointer(1, 0), right.data(), sizeof(float) * success_sample_R);

    // Add to the output buffer
    juce::AudioBuffer<float>* audio_buffer = enc_output_data.audio_buffer;
    audio_buffer->addFrom(0, 0, intermediate_buf, 0, 0, static_cast<int>(success_sample_L));
    audio_buffer->addFrom(1, 0, intermediate_buf, 1, 0, static_cast<int>(success_sample_R));
    enc_output_data.sample_count_L = success_sample_L;
    enc_output_data.sample_count_R = success_sample_R;
    // ===
    out_success_sample_L = enc_output_data.sample_count_L;
    out_success_sample_R = enc_output_data.sample_count_R;

    // 2. pre-gain
    //out_buffer->applyGain(0, static_cast<int>(required_samples), juce::Decibels::decibelsToGain(_channel_setting._pre_dB));

    // 3. Apply Audio FXs
    // Skip

    // Pan
    float pan = getPan();
    float theta = juce::jmap(pan, static_cast<float>(-1.0), static_cast<float>(1.0),
                             static_cast<float>(0.0), static_cast<float>(M_PI/2.0));
    float pan_gan_L = cos(theta);
    float pan_gan_R = sin(theta);
    out_buffer->applyGain(0, 0, static_cast<int>(out_success_sample_L), pan_gan_L);
    out_buffer->applyGain(1, 0, static_cast<int>(out_success_sample_R), pan_gan_R);

    // 4. Apply post gain
    float post_dB = getPostDb();
    out_buffer->applyGain(0, 0, static_cast<int>(out_success_sample_L), juce::Decibels::decibelsToGain(post_dB));
    out_buffer->applyGain(1, 0, static_cast<int>(out_success_sample_R), juce::Decibels::decibelsToGain(post_dB));
    _channel_gui.updateAudioMeter(out_buffer, out_success_sample_R, out_success_sample_R);

    // 5. Pan
    // Skip
}
