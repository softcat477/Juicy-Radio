#ifndef THREADINTERNET_H
#define THREADINTERNET_H

#include <iostream>
#include <atomic>
#include <thread>
#include <curl/curl.h>
#include "RingBuffer.h"
#include "CondVar.h"
#include "IThreadManager.h"

class ThreadInternet: public IThreadManager{
//class ThreadInternet{
public:
    ThreadInternet() = delete;
    ThreadInternet(size_t write_buf_size, size_t buf_max_frame_count);
    ~ThreadInternet() override;

    void start() override;
    static size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata);
    size_t write_function(char *data, size_t size, size_t nmemb);

    RingBuffer<char>* ring_buffer;
private:
    // The number of samples in the ring buffer. This is also the buffer size in CURL.
    size_t _write_buf_size;
};

#endif