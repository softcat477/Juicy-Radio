#ifndef ICHANNEL_H
#define ICHANNEL_H
#include "IThread.h"
#include "RingBuffer.h"

#include <vector>
#include <utility>

// target_read_length = thread_decoder->mp3_buffer->getSamplesPerFrame();

template<typename DataType>
class IChannel : public IThread {
public:
    IChannel(size_t sample_per_frame, size_t max_frame_count) :
        bufferL{sample_per_frame, max_frame_count},
        bufferR{sample_per_frame, max_frame_count},
        upstreams{}
    {}

    IChannel() {}

    size_t getSamplesPerFrame() {
        return bufferL.getSamplesPerFrame();
    }

    bool canRead() {
        return bufferL.canRead();
    }

    void wait(){
        bufferL.wait();
    }

    virtual ~IChannel(){}

    void connectUpstream(IChannel* upstream) {
        upstreams.emplace_back(upstream);
    }

    virtual std::pair<size_t, size_t> popAudio(std::vector<DataType>* output_bufferL, 
        std::vector<DataType>* output_bufferR,
        size_t output_sample_count) = 0;

    RingBuffer<DataType> bufferL;
    RingBuffer<DataType> bufferR;
    std::vector<IChannel*> upstreams;
};


#endif
