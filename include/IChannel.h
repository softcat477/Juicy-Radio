#ifndef ICHANNEL_H
#define ICHANNEL_H
#include "IThreadManager.h"
#include "RingBuffer.h"

#include <vector>

// target_read_length = thread_decoder->mp3_buffer->getSamplesPerFrame();

template<typename DataType>
class IChannel : public IThreadManager {
public:
    IChannel(size_t sample_per_frame, size_t max_frame_count) :
        buffer{sample_per_frame, max_frame_count},
        upstreams{}
    {}

    IChannel() {}

    size_t getSamplesPerFrame() {
        return buffer.getSamplesPerFrame();
    }

    bool canRead() {
        return buffer.canRead();
    }

    void wait(){
        buffer.wait();
    }

    virtual ~IChannel(){}

    void connectUpstream(IChannel* upstream) {
        upstreams.emplace_back(upstream);
    }

    virtual size_t popAudio(std::vector<DataType>* output_buffer, size_t output_sample_count) = 0;

    RingBuffer<DataType> buffer;
    std::vector<IChannel*> upstreams;
};


#endif
