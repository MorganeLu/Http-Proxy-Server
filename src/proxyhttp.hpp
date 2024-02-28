#ifndef PROXYHTTP_HPP
#define PROXYHTTP_HPP

#include<cstdlib>
#include<cstdio>
#include <cstring>
#include <iostream>
#include<vector>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include<thread>
#include <fstream>
#include "server.hpp"
#include "cache.hpp"
#include "client.hpp"
#include "logger.hpp"
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"



class ProxyHTTP{
private:
  const char *hostname;
  const char *port;

public:
    ProxyHTTP():hostname(NULL), port(NULL){}
    ProxyHTTP(const char *port):hostname(NULL), port(port){}

    void multiRun();
    void run();
    static void Log502(int client_fd, int requestId);
    static void Log503(int client_fd, int requestId);
    static void Log400(int client_fd, int requestId);
    static void Log404(int client_fd, int requestId);
    static void *handle(void *args);
    static void getHttp(HttpRequest request, int client_fd,int requestId);
    static void postHttp(HttpRequest request, int client_fd,int requestId);
    static void connectHttp(HttpRequest request, int client_fd);
    static void forwardData(HttpRequest request, int source_fd, int dest_fd);

};


#endif