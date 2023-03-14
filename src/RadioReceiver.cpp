#include "../include/RadioReceiver.h"
#include <curl/curl.h>

struct memory{
    char* response;
    size_t size;
};

RadioReceiver::RadioReceiver(size_t write_buf_size) {
        _write_buf_size = write_buf_size;
}
RadioReceiver::~RadioReceiver() {

}

void RadioReceiver::start() {
    CURL *curl = curl_easy_init();

    char url[] = "https://stream.live.vc.bbcmedia.co.uk/bbc_radio_three";
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RadioReceiver::write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)this);
    curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, _write_buf_size);

    if(curl) {
        CURLcode res;
        res = curl_easy_perform(curl);
        if (res != 0) {
            printf ("curl exit with error code %d\n", res);
        }
    }

    curl_easy_cleanup(curl);
    _distributor.signal();

    printf ("Quit RadioReceiver::start().\n");
}

size_t RadioReceiver::write_callback(char *data, size_t size, size_t nmemb, void *userdata){
    return ((RadioReceiver*)userdata)->write_function(data, size, nmemb);
}
size_t RadioReceiver::write_function(char *data, size_t size, size_t nmemb){
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

    size_t ret_size = _distributor.pushAudio(data, realsize, 0);
    _distributor.pushAudio(data, realsize, 1);
    return ret_size;
}