#ifndef RADIORECEIVER_H
#define RADIORECEIVER_H

#include "Channel.h"
#include "IThread.h"

class RadioReceiver : public Channel<char, char, 2>, public IThread{
public:
    RadioReceiver() = delete;
    RadioReceiver(size_t write_buf_size);
    ~RadioReceiver() override;

    RadioReceiver(RadioReceiver& other) = delete;
    RadioReceiver& operator=(RadioReceiver& other) = delete;
    RadioReceiver(RadioReceiver&& other) = delete;
    RadioReceiver& operator=(RadioReceiver&& other) = delete;

    void start() override;

    static size_t write_callback(char* data, size_t size, size_t nmemb, void* userdata);
    size_t write_function(char* data, size_t size, size_t nmemb);

    size_t _write_buf_size;
};

#endif