#pragma once

#include <string>

namespace bts { namespace utilities {

/**
 * A non-threadsafe simple libcURL-easy based HTTP downloader
 */
class http_downloader {
public:
    http_downloader();
    ~http_downloader();
    /**
     * Download a file using HTTP GET and store in in a std::string
     * @param url The URL to download
     * @return The download result
     */
    std::string download(const std::string& url);
private:
    void* curl;
};

} }
