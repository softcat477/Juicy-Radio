#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <atomic>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <condition_variable>
#include <mutex>

#include <juce_audio_utils/juce_audio_utils.h>

#include "CondVar.h"
#include <fstream>


template <typename DataType>
class RingBuffer{
public:
    RingBuffer(size_t sample_per_frame, size_t max_frame_count, CondVar* cond_var):
                                                            _juce_buffer(1, static_cast<int>(sample_per_frame*max_frame_count)),
                                                            _samples_per_frame(sample_per_frame),
                                                            _max_frame_count(max_frame_count),
                                                            _read_idx(0),
                                                            _write_idx(0),
                                                            _frame_count(0),
                                                            _cond_var(cond_var){
        /*
         *Input:
         *    <sample_per_frame>: The number of samples in each frame.
         *    <max_frame_count>: The number of frames.
         *    <cond_var>: The pointer to a condition variable. The ring buffer will inform users subscribing to this cond_var
         *                whenever there's new data available in the ring buffer.
         *    <debug>: True to dump DataType* input int smartWrite() to a file.
         */
        _buffer_length = (size_t*)malloc(sizeof(size_t)*_max_frame_count);
        memset(_buffer_length, 0, sizeof(size_t)*_max_frame_count);
    }
    ~RingBuffer(){
        free(_buffer_length);
    }

    bool canWrite(){
        return _frame_count.load() != _max_frame_count;
    }
    DataType* getWritePtr(){
        return _juce_buffer.getWritePointer(0, static_cast<int>(_write_idx * _samples_per_frame));
    }
    void finishWrite(size_t write_length){
        *(_buffer_length+_write_idx) = write_length;
        _write_idx = (_write_idx + 1) % _max_frame_count;

        //std::scoped_lock lp{this->m};
        _frame_count.fetch_add(1);
        //this->condvar.notify_all();
        _cond_var->signal();
    }
    bool canSmartWrite(){
        return canWrite();
    }
    size_t smartWrite(DataType* input, size_t write_samples_length){
        /*
         * Write <write_length> data from input to the ring buffer.
         * Return :
         *  The number of succesfully written data. <write_length> == <written> means all 
         *  data are written to the buffer.
         * Note : 
         *  MIGHT BE BUGGY :<
        */
        DataType* write_ptr = nullptr;
        size_t available_length = (size_t)_samples_per_frame - (size_t)*(_buffer_length+_write_idx);
        size_t a_length = std::min(available_length, write_samples_length);

        size_t ret_length = 0;

        if (a_length > 0){
            // Write to fill this frame
            write_ptr =  _juce_buffer.getWritePointer(0, static_cast<int>(_write_idx * _samples_per_frame));
            memcpy(write_ptr + *(_buffer_length+_write_idx), input, sizeof(DataType) * a_length);
            ret_length += a_length;
            *(_buffer_length+_write_idx) += a_length;
        }
        // Move to next frame
        if (*(_buffer_length+_write_idx) == _samples_per_frame){
            _write_idx = (_write_idx + 1) % _max_frame_count;
            _frame_count.fetch_add(1);
        }
        // Continue writing until
        //  1. Write all data.
        //  2. No frame left.
        while (ret_length < write_samples_length && _frame_count.load() != _max_frame_count){
            // Write to this frame.
            write_ptr =  _juce_buffer.getWritePointer(0, static_cast<int>(_write_idx * _samples_per_frame));
            // and copy data in it.
            size_t b_length = std::min((size_t)(this->_samples_per_frame), write_samples_length - ret_length);
            memcpy(write_ptr, input + ret_length, sizeof(DataType) * b_length);
            ret_length += b_length;
            *(_buffer_length+_write_idx) += b_length;
            // Finally, update the frame count and write index.
            if (*(_buffer_length+_write_idx) == _samples_per_frame){
                _write_idx = (_write_idx + 1) % _max_frame_count;
                _frame_count.fetch_add(1);
            }
        }
        return ret_length;
    }
    void finishSmartWrite(){
        // A dummy function that does nothing.
        _cond_var->signal();
        return;
    }

    bool canRead(bool debug = false){
        if (debug)
            printf ("Before count\n");
        size_t count = _frame_count.load();
        if (debug)
            printf ("Get count %zu\n", count);
        return count;
    }
    const DataType* getReadPtr(){
        return _juce_buffer.getReadPointer(0, static_cast<int>(_read_idx * _samples_per_frame));
    }
    size_t getReadLength(){
        // Return the length of available data in this frame.
        return *(_buffer_length+_read_idx);
    }
    void finishRead(bool decrement=false){
        auto old_read_idx = _read_idx;
        _read_idx = (_read_idx + 1) % _max_frame_count;
        if (decrement){
            _frame_count.fetch_sub(1);
            // Clean up unused position.
            // This is for the smartWrite function
            DataType* old_read_ptr = _juce_buffer.getWritePointer(0, static_cast<int>(old_read_idx * _samples_per_frame));
            memset(old_read_ptr, 0, sizeof(DataType) * this->_samples_per_frame);
            *(_buffer_length+old_read_idx) = 0;
        }
    }
    bool canSmartRead(bool debug=false){
        return canRead(debug);
    }
    size_t getSmartRead(DataType* output_ptr, size_t output_samples_length){
        /*
         * Read <output_samples_length> into <output_ptr>.
         * Return : 
         *  The number of read data.
         */
        size_t ret_samples_length = 0;

        while (ret_samples_length < output_samples_length){
            auto available_samples_length = *(_buffer_length+_read_idx);
            const DataType* cur_ptr =  _juce_buffer.getReadPointer(0, static_cast<int>(_read_idx * _samples_per_frame));

            // Copy to output_ptr
            auto copy_samples_length = std::min((size_t)available_samples_length, output_samples_length - ret_samples_length);
            memcpy(output_ptr + ret_samples_length, cur_ptr + (_samples_per_frame - available_samples_length), sizeof(DataType) * copy_samples_length);
            ret_samples_length += copy_samples_length;

            // If we just read part of the data, update the number of available data in this frame.
            if (copy_samples_length != available_samples_length){
                *(_buffer_length+_read_idx) -= copy_samples_length;
            }
            else{
            // Move on to the next frame.
                finishRead(true);
            }
            if (_frame_count.load() == 0)
                break;
        }
        return ret_samples_length;
    }
    void finishSmartRead(){
        return;
    }

    int getSamplesPerFrame(){
        return _samples_per_frame;
    }
    int getFrameCount(){
        return _frame_count.load();
    }
    int getMaxFrameCount(){
        return _max_frame_count;
    }

    std::condition_variable condvar;
    std::mutex m;
private:
    // Stores the data
    juce::AudioBuffer<DataType> _juce_buffer;
    // Store the available data in each frame
    size_t* _buffer_length;

    size_t _samples_per_frame;
    size_t _max_frame_count;

    size_t _read_idx;
    size_t _write_idx;
    std::atomic<size_t> _frame_count;

    // A condition variable pointer to inform its holder that new data is available.
    CondVar* _cond_var;
};

#endif