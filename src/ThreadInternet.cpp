#include "../include/ThreadInternet.h"

#include <curl/curl.h>
#include <cstring>
#include <algorithm>


struct memory{
    char* response;
    size_t size;
};

ThreadInternet::ThreadInternet(size_t write_buf_size, size_t buf_max_frame_count): // 8192, 128
                                        _write_buf_size(write_buf_size){
    /*
     * Input:
     *  <write_buf_size>: The number of samples in the ring buffer. This is also the buffer size in CURL.
     *  <buf_max_frame_count>: The number of frames in the ring buffer.
    */
    ring_buffer = new RingBuffer<char>(write_buf_size, buf_max_frame_count);
}
ThreadInternet::~ThreadInternet(){
    //free(_write_buf);
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

    size_t success_write_length = ring_buffer->write(data, realsize);
    return realsize;
}