#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <atomic>
#include <algorithm>
#include <iomanip>

#include <vector>

#include "CondVar.h"
#include <iostream>

#include <memory>


template <typename DataType>
class RingBuffer{
public:
    /*
    Can store <sample_per_frame> * <max_frame_count> - 1 items of <DataType>
    *Input:
    *    <sample_per_frame>: The number of samples in each frame.
    *    <max_frame_count>: The number of frames.
    */
    RingBuffer() {
        std::cout << "Default constructor in Ring Buffer\n";
    }
    RingBuffer(size_t sample_per_frame, size_t max_frame_count):
                                                            _buffer(sample_per_frame*max_frame_count),
                                                            _samples_per_frame(sample_per_frame),
                                                            _max_frame_count(max_frame_count),
                                                            _read_idx{0},
                                                            _write_idx{0},
                                                            _capacity{sample_per_frame*max_frame_count}{
        //_cond_var = new CondVar();
    }
    ~RingBuffer(){
        //if (_cond_var != nullptr)
        //    _cond_var->signal();
        //delete _cond_var;
        std::shared_ptr<CondVar> ptr = _cond_var.lock();
        if (ptr)
            ptr->signal();
        _cond_var.reset();
    }

    void addCondVar(std::shared_ptr<CondVar> var) {
        _cond_var = var;
    }

    RingBuffer(const RingBuffer& other) :
        _buffer{other._buffer},
        _samples_per_frame{other._samples_per_frame},
        _max_frame_count{other._max_frame_count},
        _read_idx{other._read_idx.load()},
        _write_idx{other._write_idx.load()},
        _capacity{other._capacity},
        _cond_var{other._cond_var}
    {

    }
    RingBuffer& operator=(const RingBuffer& other) {
        if (&other == this)
            return *this;

        _buffer = other._buffer;
        _samples_per_frame = other._samples_per_frame;
        _max_frame_count = other._max_frame_count;
        _read_idx = other._read_idx.load();
        _write_idx = other._write_idx.load();
        _capacity = other._capacity;
        _cond_var = other._cond_var;

        return *this;
    }
    RingBuffer(RingBuffer&& other) :
        _buffer{std::move(other._buffer)},
        _samples_per_frame{std::move(other._samples_per_frame)},
        _max_frame_count{std::move(other._max_frame_count)},
        _read_idx{std::move(other._read_idx.load())},
        _write_idx{std::move(other._write_idx.load())},
        _capacity{std::move(other._capacity)},
        _cond_var{other._cond_var} {

        other.isActive = false;
        other.signal();
        //other._buffer.erase();
        other._samples_per_frame = 0;
        other._max_frame_count = 0;
        other._read_idx.store(0);
        other._write_idx.store(0);
        other._capacity = 0;
        other._cond_var.reset();
    }
    RingBuffer& operator=(RingBuffer&& other) {
        if (&other == this)
            return *this;
        _cond_var = other._cond_var;
    }

    /*
    Return true when there's at least a frame to read
    */
    bool canRead(){
        size_t count_ = getStored();
        count_ = count_ / _samples_per_frame;
        return count_;
    }

    /*
    Write <write_sample_length> to buffer
    */
    size_t write(const DataType* input, size_t write_samples_length){
        if (!isActive)
            return 0;
        const size_t write_index = this->_write_idx.load();
        DataType* buffer_ptr = _buffer.data();

        // The number of samples we can write
        size_t ret_write_sample_length = std::min(write_samples_length, this->getSpace());
        // The number of samples we can write to input + write_index:end
        size_t write_to_end = std::min(ret_write_sample_length, this->_capacity - write_index);
        // The number of samples we can write starting from input:<an arbitrary position before read>
        size_t write_from_start = ret_write_sample_length - write_to_end;

        // Copy data
        memcpy(&buffer_ptr[write_index], input, sizeof(DataType) * write_to_end);
        memcpy(&buffer_ptr[0], &input[write_to_end], sizeof(DataType) * write_from_start);

        // update write_index_
        _write_idx.store((write_index + ret_write_sample_length) % _capacity);

        // ThreadDecoder is waiting for the next frame, we have send a signal to let it know there's a new frame.
        if (ret_write_sample_length > 0) {
            if (canRead()) {
                std::shared_ptr<CondVar> ptr = _cond_var.lock();
                if (ptr) {
                    ptr->signal();
                }
            }
        }

        return ret_write_sample_length;
    }

    /*
    Read <num_samples> to <output_buffer>
    */
    size_t read(std::vector<DataType>* output_buffer, size_t num_samples){
        if (!isActive)
            return 0;
        const size_t read_index = this->_read_idx.load();
        // const size_t write_index = this->_write_idx.load();
        DataType* read_buffer_ptr = _buffer.data();
        DataType* write_buffer_ptr = output_buffer->data();

        // The number of samples we can read
        size_t ret_read_sample_length = std::min(num_samples, this->getStored());
        // The number of samples we can read from read_index:end
        size_t read_to_end = std::min(ret_read_sample_length, this->_capacity - read_index);
        // The number of samples we can read starting from _buffer:<an arbitrary position before read>
        size_t read_from_start = ret_read_sample_length - read_to_end;

        // Copy data
        memcpy(&write_buffer_ptr[0], &read_buffer_ptr[read_index], sizeof(DataType) * read_to_end);
        memcpy(&write_buffer_ptr[read_to_end], &read_buffer_ptr[0], sizeof(DataType) * read_from_start);

        _read_idx.store((read_index + ret_read_sample_length) % _capacity);

        return ret_read_sample_length;
    }
    size_t read(DataType* output_buffer, size_t num_samples){
        if (!isActive)
            return 0;
        const size_t read_index = this->_read_idx.load();
        // const size_t write_index = this->_write_idx.load();
        DataType* read_buffer_ptr = _buffer.data();

        // The number of samples we can read
        size_t ret_read_sample_length = std::min(num_samples, this->getStored());
        // The number of samples we can read from read_index:end
        size_t read_to_end = std::min(ret_read_sample_length, this->_capacity - read_index);
        // The number of samples we can read starting from _buffer:<an arbitrary position before read>
        size_t read_from_start = ret_read_sample_length - read_to_end;

        // Copy data
        memcpy(&output_buffer[0], &read_buffer_ptr[read_index], sizeof(DataType) * read_to_end);
        memcpy(&output_buffer[read_to_end], &read_buffer_ptr[0], sizeof(DataType) * read_from_start);

        _read_idx.store((read_index + ret_read_sample_length) % _capacity);

        return ret_read_sample_length;
    }

    size_t getSamplesPerFrame(){
        return _samples_per_frame;
    }
    size_t getMaxFrameCount(){
        return _max_frame_count;
    }

    // Need these two functions to let ThreadDecoder know there's at least one frame in the buffer.
    void signal(){
        std::shared_ptr<CondVar> ptr = _cond_var.lock();
        if (ptr) {
            //if (_cond_var != nullptr)
            ptr->signal();
        }
    }
    void wait(){
        std::shared_ptr<CondVar> ptr = _cond_var.lock();
        if (ptr) {
            //if (_cond_var != nullptr)
            ptr->wait();
        }
    }

    /*
    Return how much spaces are left in the ring buffer
    */
    size_t getSpace() const{
        const size_t read_idx = _read_idx.load();
        const size_t write_idx = _write_idx.load();

        if (read_idx == write_idx) {
            // Empty
            return _capacity-1;
        }
        else {
            size_t space = (_capacity - (write_idx - read_idx)) % _capacity;
            return space - 1;
        }
    }
    /*
    Stored data in the buffer
    */
    size_t getStored() const {
        const size_t read_idx = _read_idx.load();
        const size_t write_idx = _write_idx.load();

        if (write_idx >= read_idx){
            return write_idx - read_idx;
        }
        else{
            return _capacity + (write_idx - read_idx);
        }
    }
    std::vector<DataType> status() {
        const size_t read_idx = _read_idx.load();
        const size_t write_idx = _write_idx.load();

        auto cap = getSpace();
        auto store = getStored();

        //std::cout << "Read/Write/total : " << read_idx << "/" << write_idx << "/" << _samples_per_frame * _max_frame_count;
        for (auto i = 0; i < _capacity; i++) {
            // within
            // if (i == read_idx)
            //     std::cout << "r,";
            // else if (i == write_idx)
            //     std::cout << "w,";
            // else if (i > read_idx && i < write_idx)
            //     std::cout << "*,";
            // else
            //     std::cout << "_,";
            std::cout << std::setw(5);
            if (read_idx <= write_idx) {
                if (i >= read_idx && i < write_idx)
                    std::cout << _buffer.at(i) << ",";
                else
                    std::cout << "_" << std::setw(1) << ",";
            }
            else {
                if (i >= read_idx || i < write_idx)
                    std::cout << _buffer.at(i) << ",";
                else
                    std::cout << "_" << std::setw(1) << ",";
            }
        }
        std::cout << " space/store : " << cap << "/" << store << " read/write : " << read_idx << "/" << write_idx << "\n";
        return _buffer;
    }
private:
    // Stores the data
    std::vector<DataType> _buffer;

    size_t _samples_per_frame;
    size_t _max_frame_count;

    std::atomic<size_t> _read_idx;
    std::atomic<size_t> _write_idx;
    size_t _capacity;

    // A condition variable pointer to inform its holder (downstream units' aggregator) that new data is available.
    std::weak_ptr<CondVar> _cond_var;

    bool isActive = true;
};

#endif