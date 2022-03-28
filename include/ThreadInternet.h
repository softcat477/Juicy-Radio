#ifndef THREADINTERNET_H
#define THREADINTERNET_H

#include <iostream>
#include <atomic>
#include <thread>
#include <curl/curl.h>
#include "RingBuffer.h"
#include "CondVar.h"

class ThreadInternet{
public:
    ThreadInternet() = delete;
    ThreadInternet(size_t write_buf_size, size_t buf_max_frame_count, CondVar* cond_mp3);
    ~ThreadInternet();

    void start();
    static size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata);
    size_t write_function(char *data, size_t size, size_t nmemb);

    void setStop();
    void setStart();

    RingBuffer<char>* ring_buffer;
private:
    // A variable trigerred by main thread to infomr this thread to stop.
    std::atomic<bool> _isStopped;
    // The number of samples in the ring buffer. This is also the buffer size in CURL.
    size_t _write_buf_size;

    // Two-stage writing in write_function()
    char* _write_buf;
    size_t _write_buf_available_data_count;
};

#endif