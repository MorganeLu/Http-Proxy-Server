#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <cstring>
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace std;

class HttpRequest
{
    string method;
    string host;
    string port;
    string requestContent;
    string URI;
    string firstLine;
    time_t date;
    size_t requestId;

public:
    HttpRequest(string initRequest, int requestId);
    void parseFirstLine();
    void parseMethod();
    void parseHost();
    void parseURI();
    void parseDate();
    string getFirstLine();
    string getContent();
    string getMethod();
    string getHost();
    string getPort();
    string getURI();
    time_t getDate();
    size_t getRequestId();
};

#endif
