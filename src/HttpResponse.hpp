#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>



using namespace std;

class HttpResponse {
private:
    string responseContent = "";
    string status = ""; //OK
    string code = ""; //200
    time_t date = 0; // default value for a time_t variable is typically 0
    time_t expireDate = 0;
    string etag = "";
    string firstLine = "";
    string lastModifyDate = "";
    size_t headLength = 0;
    size_t contentLength = 0;
    size_t max_age = 0; // how long a cached response is considered fresh without re-validation.
    size_t max_stale = 0; //the time still can be accepted after expiration
    bool hasMustRevalidate = false;
    bool no_cache;
    bool no_store;
    bool is_private;
    bool is_public;
    bool has_max_age;
    bool isChunked = false;
    bool is_Etag = false;

    void parseCodeAndStatus();
    void parseEtag();
    void parseDate();
    void parseExpireDate();
    void parseLastModify();

    void parseCacheControl();
    void parseChunked();
    // void parseHeadLength();
    // void parseContentLength();

public:
    HttpResponse() {}

    explicit HttpResponse(const string& initRequest);

    // Getter methods
    time_t getDate() const;
    time_t getExpires() const;
    string getContent() const;
    string getEtag() const;
    string getStatus() const;
    string getLastModify() const;
    string getCode() const;
    size_t getMaxAge() const;
    size_t getMaxStale() const;
    size_t getContentLength() const;
    size_t getHeadLength() const;
    bool isNocache() const;
    bool isNostore() const;
    bool isPrivate() const;
    bool ishasMustRevalidate() const;
    bool getChunked() const;
    string getFirstLine();
    void printRes();

    time_t getValidExpireTime(); 
};

#endif
