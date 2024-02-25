#ifndef CACHE_HPP
#define CACHE_HPP

#include<cstdlib>
#include<cstdio>
#include <cstring>
#include <iostream>
#include <map>
#include <deque>
#include <vector>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "logger.hpp"
class FIFOCache{
private:
    std::map<std::string, HttpResponse> cacheMap;
    std::deque<std::pair<std::string, HttpResponse> > FIFOQue;
    size_t capacity;
    // Logger logger;

public:
    // FIFOCache():capacity(0){}
    FIFOCache(size_t capacity):capacity(capacity){}
    bool inCache(HttpRequest req);
    void removeCache();
    int addCache(HttpRequest req, HttpResponse res);
    // should prove it is in cache
    bool checkValid(HttpRequest req, HttpResponse res, int requestId);
    HttpResponse *getCache(HttpRequest req, int socket_fd);
};


#endif