#ifndef WIRE_H
#define WIRE_H

#include "RingBuffer.h"
#include "CondVar.h"

#include <vector>
#include <algorithm>
#include <memory>

/*
API for aggregator.
Always paired with an InJack.
*/
template <typename DType, size_t channels>
class OutJack {
public:
    // The special constructor for libmad
    OutJack(size_t sample_per_frame, size_t max_frame_count): 
        _channels{channels},
        _sample_per_frame{sample_per_frame},
        _max_frame_count{max_frame_count},
        _buffer_size{sample_per_frame * max_frame_count}{

        _buffers.reserve(channels);
        for (size_t i = 0; i < channels; i++) {
            _buffers.push_back(RingBuffer<DType>(sample_per_frame, max_frame_count));
        }
    }
    // For general use
    OutJack(size_t buffer_size): 
        _channels{channels},
        _sample_per_frame{buffer_size},
        _max_frame_count{1},
        _buffer_size{buffer_size} {
        _buffers.reserve(channels);
        for (size_t i = 0; i < channels; i++) {
            _buffers.push_back(RingBuffer<int>(buffer_size, 1));
        }
    }

    ~OutJack(){

    }

    /*
    Return true if there's at lease one buffer having samples more than <_sample_per_frame>.
    For libmad usage.
    */
    bool canRead() {
        bool ret = false;
        for (RingBuffer<DType> _buffer : _buffers) {
            ret = ret | _buffer.canRead();
        }
        return ret;
    }

    /*
    Return #samples/frame in buffers.
    For libmad usage.
    */
    size_t getSamplesPerFrame() const {
        return _sample_per_frame;
    }

    /*
    Pop from the first channel / a channel
    */
    size_t popAudio(DType* output_buffer, size_t output_samples, size_t channel_index = 0) {
        if (channel_index >= _channels)
            return 0;
        size_t success_samples = _buffers[channel_index].read(output_buffer, output_samples);
        return success_samples;
    }

    /*
    Pop all channels
    */
    std::vector<size_t> popAudio(std::vector<DType*>* output_buffers, size_t output_samples) {
        std::vector<size_t> success_samples{};
        for (size_t i = 0; i < output_buffers->size(); i++) {
            size_t tmp = popAudio(output_buffers->at(i), output_samples, i);
            success_samples.emplace_back(tmp);
        }
        return success_samples;
    }

    /*
    Pop from the <channel_index> channel and add to <output_buffer>
    */
    size_t popAndSumAudio(DType* output_buffer, size_t output_samples, size_t channel_index = 0) {
        if (channel_index >= _channels)
            return 0;

        std::vector<DType> tmp_buffer(output_samples, DType());
        size_t success_samples = _buffers[channel_index].read(tmp_buffer.data(), output_samples);

        std::transform(tmp_buffer.cbegin(), tmp_buffer.cend(), output_buffer, output_buffer, 
            [](const DType& a, const DType& b){
                return a + b;});
        return success_samples;
    }

    /*
    Pop all channels and add them into <output_buffers>
    */
    std::vector<size_t> popAndSumAudio(std::vector<DType*>* output_buffers, size_t output_samples) {
        std::vector<size_t> success_samples{};
        for (size_t i = 0; i < output_buffers->size(); i++) {
            size_t tmp_samples = popAndSumAudio(output_buffers->at(i), output_samples, i);

            success_samples.emplace_back(tmp_samples);
        }
        return success_samples;
    }

    size_t getChannels() const {
        return _channels;
    }

    /*
    Return available spaces in <channel_index>th buffer/
    */
    size_t getSpace(size_t channel_index) const {
        if (channel_index >= _channels)
            return 0;
        return _buffers[channel_index].getSpace();
    }

    /*
    Return the number of stored data in <channel_index>th buffer/
    */
    size_t getStored(size_t channel_index) const {
        if (channel_index >= _channels)
            return 0;
        return _buffers[channel_index].getStored();
    }

    /*
    For ctest. Return buffers.
    */
    std::vector<std::vector<DType>> status() const {
        std::vector<std::vector<DType>> ret {};
        for (auto buffer : _buffers){
            ret.push_back(buffer.status());
        }
        return ret;
    }

    std::vector<RingBuffer<DType>> _buffers;
private:
    size_t _channels;

    size_t _sample_per_frame;
    size_t _max_frame_count;

    // _buffer_size = _sample_per_frame * _max_frame_count
    size_t _buffer_size;
};

template <typename DType, size_t channels>
using OutJackPtr = std::shared_ptr<OutJack<DType, channels>>;


/*
API for distributor.
Always paired with an OutJack.
*/
template <typename DType, size_t channels>
class InJack{ 
public:
    InJack(std::shared_ptr<OutJack<DType, channels>> out_ptr) :
        _out_ptr{out_ptr},
        _channels{out_ptr->getChannels()}{

    }
    InJack() = delete;
    ~InJack() {

    }

    /*
    Push <input_buffer> into the ring buffer owned by _output_ptr
    */
    size_t pushAudio(const DType* input_buffer, size_t input_samples, size_t channel_index = 0) {
        std::shared_ptr<OutJack<DType, channels>> ptr = _out_ptr.lock();
        if (!ptr)
            return 0;

        if (channel_index >= ptr->getChannels())
            return 0;

        size_t ret_samples = (ptr->_buffers[channel_index]).write(input_buffer, input_samples);
        return ret_samples;
    }

    /*
    Push <input_buffers into ring buffers owned by _out_ptr
    */
    std::vector<size_t> pushAudio(const std::vector<DType*>* input_buffers, size_t input_samples) {
        std::vector<size_t> ret_samples{};

        for (size_t i = 0; i < input_buffers->size(); i++) {
            size_t tmp = pushAudio(input_buffers->at(i), input_samples, i);
            ret_samples.emplace_back(tmp);
        }
        return ret_samples;
    }

    size_t getSpace(size_t channel_index) const {
        if (channel_index >= _channels)
            return 0;
        std::shared_ptr<OutJack<DType, channels>> ptr = _out_ptr.lock();
        if (!ptr)
            return 0;
        return ptr->getSpace(channel_index);
    }

    size_t getStored(size_t channel_index) const {
        if (channel_index >= _channels)
            return 0;
        std::shared_ptr<OutJack<DType, channels>> ptr = _out_ptr.lock();
        if (!ptr)
            return 0;
        return ptr->getStored(channel_index);
    }

    size_t getChannels() const {
        return _channels;
    }

    bool canRead() {
        std::shared_ptr<OutJack<DType, channels>> ptr = _out_ptr.lock();
        if (!ptr)
            return false;

        return ptr->canRead();
    }

private:
    std::weak_ptr<OutJack<DType, channels>> _out_ptr;
    size_t _channels;
};

template <typename DType, size_t channels>
using InJackPtr = std::shared_ptr<InJack<DType, channels>>;

/*
A wire owns an InJack and OutJack. Aggregators and distributors talk to In/Out jack
to read/write audio samples across units.
*/
template <typename DType, size_t channels>
class Wire {
public:
    Wire(size_t sample_per_frame, size_t max_frame_count): 
        _channels{channels},
        _out_jack_ptr(std::make_shared<OutJack<DType, channels>>(sample_per_frame, max_frame_count)),
        _in_jack_ptr(std::make_shared<InJack<DType, channels>>(_out_jack_ptr))
    {
    }

    Wire(size_t buffer_size): 
        _channels{channels},
        _out_jack_ptr(std::make_shared<OutJack<DType, channels>>(buffer_size)),
        _in_jack_ptr(std::make_shared<InJack<DType, channels>>(_out_jack_ptr))
    {}

    // Wire() = delete;
    // Wire(const Wire& other) = delete;
    // Wire& operator=(const Wire& other) = delete;
    // Wire(const Wire&& other) = delete;
    // Wire& operator=(const Wire&& other) = delete;

    size_t getChannels() const {
        return _channels;
    }

    std::shared_ptr<OutJack<DType, channels>> getOutPtr() const {
        return _out_jack_ptr;
    }

    std::shared_ptr<InJack<DType, channels>> getInPtr() const {
        return _in_jack_ptr;
    }

    /*
    For ctest.
    */
    void status() const {
        _out_jack_ptr->status();
    }
private:
    size_t _channels;
    std::shared_ptr<OutJack<DType, channels>> _out_jack_ptr;

    std::shared_ptr<InJack<DType, channels>> _in_jack_ptr;
};

template <typename DType, size_t channels>
using WirePtr = std::shared_ptr<Wire<DType, channels>>;
#endif