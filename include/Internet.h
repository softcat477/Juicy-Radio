#ifndef INTERNET_H
#define INTERNET_H

#include "IChannel.h"
#include <curl/curl.h>

class Internet : public IChannel<char> {
public:
    Internet() = delete;
    Internet(size_t write_buf_size, size_t buf_max_frame_count);
    ~Internet() override;

    Internet(Internet& other) = delete;
    Internet& operator=(Internet& other) = delete;
    Internet(Internet&& other) = delete;
    Internet& operator=(Internet&& other) = delete;

    void start() override;
    size_t popAudio(std::vector<char>* output_buffer, size_t output_sample_count) override;

    static size_t write_callback(char *data, size_t size, size_t nmemb, void *userdata);
    size_t write_function(char *data, size_t size, size_t nmemb);

    size_t _write_buf_size;

};

#endif