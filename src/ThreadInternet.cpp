#include "../include/ThreadInternet.h"

#include <curl/curl.h>
#include <cstring>
#include <algorithm>


struct memory{
    char* response;
    size_t size;
};

ThreadInternet::ThreadInternet(size_t write_buf_size, size_t buf_max_frame_count): // 8192, 128
                                        _isStopped(false),
                                        _write_buf_size(write_buf_size),
                                        _write_buf_available_data_count(0){
    /*
     * Input:
     *  <write_buf_size>: The number of samples in the ring buffer. This is also the buffer size in CURL.
     *  <buf_max_frame_count>: The number of frames in the ring buffer.
    */
    _write_buf = (char*)malloc(sizeof(char)*static_cast<unsigned long long>(write_buf_size));
    ring_buffer = new RingBuffer<char>(write_buf_size, buf_max_frame_count);
}
ThreadInternet::~ThreadInternet(){
    free(_write_buf);
    delete ring_buffer;
}
void ThreadInternet::start(){
    // Start this thread
    CURL *curl = curl_easy_init();

    //long write_buf_size = 4096;
    //char url[] = "http://ice.stream.frequence3.net/frequence3-128.mp3";
    char url[] = "https://stream.live.vc.bbcmedia.co.uk/bbc_radio_three";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, ThreadInternet::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)this);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, _write_buf_size);

    if(curl) {
      CURLcode res;
      res = curl_easy_perform(curl);
    }

    curl_easy_cleanup(curl);
    ring_buffer->signal();
    printf ("Quit ThreadInternet::start().\n");
}
size_t ThreadInternet::write_callback(char *data, size_t size, size_t nmemb, void *userdata){
    return ((ThreadInternet*)userdata)->write_function(data, size, nmemb);
}
size_t ThreadInternet::write_function(char *data, size_t size, size_t nmemb){
    /*
     * https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
     * Params:
     *  <ptr> points to the delivered data.
     *  <size> is always 1.
     *  <nmemb> is the size of the delivered data.
     * Return:
     *  The number of bytes actually taken care of. If the return number differs from
     *  the amount passed to the callback function, it will signal an error and cause the transfer to abort
     *  and return CURLE_WRITE_ERROR.
     */
    if (this->_isStopped.load() == true){
        return (size_t)-1;
    }

    size_t realsize = size * nmemb;

    // 1. Copy part of the data to fill the write_buf
    size_t available_space = _write_buf_size - _write_buf_available_data_count;
    size_t pre_write_num = std::min(available_space, realsize);
    size_t post_write_num = realsize - pre_write_num;
    memcpy(_write_buf+_write_buf_available_data_count, data, pre_write_num);
    _write_buf_available_data_count += pre_write_num;

    // Dump fulled write_buf to the ring buffer.
    // Here we're not using smartWrite(), so make sure that the ring buffer and the _write_buf have the same size.
    if (_write_buf_available_data_count == _write_buf_size){
        if (this->ring_buffer->canWrite()){
            char* write_ptr = this->ring_buffer->getWritePtr();
            memcpy(write_ptr, _write_buf, _write_buf_size);
            this->ring_buffer->finishWrite(_write_buf_size);
        }
        else{
            printf ("Can't write to ring_buffer.\n");
            return (size_t)-1;
        }
        memset(_write_buf, 0, _write_buf_size);
        _write_buf_available_data_count = 0;
    }

    // 2. Copy the rest of the data.
    if (post_write_num > 0){
        memcpy(_write_buf, data+pre_write_num, post_write_num);
        _write_buf_available_data_count = post_write_num;
    }

    return realsize;
}
void ThreadInternet::setStop(){
    _isStopped.store(true);
}
void ThreadInternet::setStart(){
    _isStopped.store(false);
}