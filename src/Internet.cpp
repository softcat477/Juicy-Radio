#include "../include/Internet.h"

struct memory{
    char* response;
    size_t size;
};

Internet::Internet(size_t write_buf_size, size_t buf_max_frame_count):
    IChannel{write_buf_size, buf_max_frame_count} {
        _write_buf_size = write_buf_size;
}
Internet::~Internet() {

}

void Internet::start() {
    // Start this thread
    CURL *curl = curl_easy_init();

    //char url[] = "http://ice.stream.frequence3.net/frequence3-128.mp3";
    char url[] = "https://stream.live.vc.bbcmedia.co.uk/bbc_radio_three";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Internet::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)this);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, _write_buf_size);

    if(curl) {
      CURLcode res;
      res = curl_easy_perform(curl);
    }

    curl_easy_cleanup(curl);
    buffer.signal();
    printf ("Quit Internet::start().\n");
}
size_t Internet::popAudio(std::vector<char>* output_buffer, size_t output_sample_count) {
    size_t ret_pop_length = buffer.read(output_buffer, output_sample_count);
    return ret_pop_length;
}

size_t Internet::write_callback(char *data, size_t size, size_t nmemb, void *userdata){
    return ((Internet*)userdata)->write_function(data, size, nmemb);
}
size_t Internet::write_function(char *data, size_t size, size_t nmemb){
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

    size_t success_write_length = buffer.write(data, realsize);
    return realsize;
}