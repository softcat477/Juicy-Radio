#ifndef CHANNELSTRIP_H
#define CHANNELSTRIP_H

#include <vector>
#include <juce_audio_utils/juce_audio_utils.h>

#include "IOParams.h"
#include "IEncoderStream.h"

class ChannelStrip{
public:
    ChannelStrip();
    ChannelStrip(ChannelStripSetting);
    ~ChannelStrip();

    bool connect(ChannelStrip*);
    bool connect(IEncoderStream*);

    bool remove(ChannelStrip*);
    bool remove(IEncoderStream*);

    void setPreGain(float pre_db);
    void setPostGain(float post_db);
    void setPan(float pan);

    void processAndMixAudio(juce::AudioBuffer<float>*, int&, int&, size_t);
private:
    ChannelStripSetting _channel_setting;
    std::vector<ChannelStrip*> _input_strips;
    std::vector<IEncoderStream*> _input_enc_streams;
};
#endif