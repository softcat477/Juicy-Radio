#ifndef INTERNET_H
#define INTERNET_H

#include "Channel.h"
#include "IThread.h"

class RadioReceiver : public Channel<char, char, 2>, public IThread{
public:
    RadioReceiver() = delete;
    RadioReceiver(size_t write_buf_size, size_t buf_max_frame_count);
    ~RadioReceiver() override;

    RadioReceiver(RadioReceiver& other) = delete;
    RadioReceiver& operator=(RadioReceiver& other) = delete;
    RadioReceiver(RadioReceiver&& other) = delete;
    RadioReceiver& operator=(RadioReceiver&& other) = delete;

    // std::pair<size_t, size_t> popAudio(std::vector<char>* output_bufferL, 
    //     std::vector<char>* output_bufferR,
    //     size_t output_sample_count);

    void start() override;
    static size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata);
    size_t write_function(char *data, size_t size, size_t nmemb);

    size_t _write_buf_size;
};

#endif