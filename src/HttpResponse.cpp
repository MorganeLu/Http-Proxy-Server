#include "HttpResponse.hpp"

// Costructor
HttpResponse::HttpResponse(const string& initResponse){
    responseContent = initResponse;
    parseCodeAndStatus();
    parseEtag();
    parseDate();
    parseExpireDate();
    parseLastModify();
    parseCacheControl();
}


// parse the max-age in cache-control headers (max-age in the Cache-Control header is not guaranteed)
void HttpResponse::parseCacheControl()
{
    size_t cacheControlPos = responseContent.find("Cache-Control: ");

    if (cacheControlPos != std::string::npos)
    {
        size_t cacheControlStart = cacheControlPos + 15;
        size_t cacheControlEnd = responseContent.find("\r\n", cacheControlStart);
        std::string cacheControlValue = responseContent.substr(cacheControlStart, cacheControlEnd - cacheControlStart);

        // Check for the presence of "must-revalidate"
        hasMustRevalidate = (cacheControlValue.find("must-revalidate") != string::npos);
        no_cache = (cacheControlValue.find("no-cache") != string::npos);
        // Find the position of "max-age="
        size_t maxAgePos = cacheControlValue.find("max-age=");

        if (maxAgePos != std::string::npos)
        {
            string maxAgeSubstring = cacheControlValue.substr(maxAgePos + 8);
            size_t maxAgeEnd = maxAgeSubstring.find_first_not_of("0123456789");

            // Extract the max-age value as a string
            string maxAgeValue = maxAgeSubstring.substr(0, maxAgeEnd);
            try
            {
                // Convert the string to a size_t
                max_age = stoul(maxAgeValue);
            }
            catch (const std::invalid_argument &e)
            {
                // Handle invalid_argument exception (e.g., non-numeric string)
                std::cerr << "Error: " << e.what() << std::endl;
            }
            catch (const std::out_of_range &e)
            {
                // Handle out_of_range exception (e.g., value out of range for size_t)
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }

        // Find the position of "max-stale="
        size_t maxStalePos = cacheControlValue.find("max-stale=");

        if (maxStalePos != std::string::npos)
        {
            string maxStaleSubstring = cacheControlValue.substr(maxAgePos + 10);
            size_t maxStaleEnd = maxStaleSubstring.find_first_not_of("0123456789");

            // Extract the max-stale value as a string
            string maxStaleValue = maxStaleSubstring.substr(0, maxStaleEnd);
            try
            {
                // Convert the string to a size_t
                max_stale = stoul(maxStaleValue);
            }
            catch (const std::invalid_argument &e)
            {
                // Handle invalid_argument exception (e.g., non-numeric string)
                std::cerr << "Error: " << e.what() << std::endl;
            }
            catch (const std::out_of_range &e)
            {
                // Handle out_of_range exception (e.g., value out of range for size_t)
                std::cerr << "Error: " << e.what() << std::endl;
            }
        }
    }
}

// parse the Last Modify
void HttpResponse::parseLastModify()
{
    size_t datePos = responseContent.find("Last-Modified: ");
    if (datePos != std::string::npos)
    {
        size_t dateStart = datePos + 15; // Move past "Last-Modified: "
        size_t dateEnd = responseContent.find("\r\n", dateStart);
        string dateString = responseContent.substr(dateStart, dateEnd - dateStart);
        lastModifyDate = dateString;
        // // Convert the date string to a time structure
        // struct tm tm = {};
        // istringstream dateStream(dateString);
        // dateStream >> get_time(&tm, "%a, %d %b %Y %H:%M:%S %Z");

        // if (!dateStream.fail())
        // {
        //     // Convert the time structure to a time_t value
        //     lastModifyDate = mktime(&tm);

        //     // Print the parsed time_t value
        //     std::cout << "Parsed time_t value: " << date << std::endl;
        // }
        // else
        // {
        //     // Handle the case where parsing the date string fails
        //     std::cerr << "Error parsing Last-Modified-Date string." << std::endl;
        // }
    }
    // return NULL;
}

// parse the Expire Date
void HttpResponse::parseExpireDate()
{
    size_t datePos = responseContent.find("Expires: ");
    if (datePos != std::string::npos)
    {
        size_t dateStart = datePos + 9; // Move past "Expires: "
        size_t dateEnd = responseContent.find("\r\n", dateStart);
        string dateString = responseContent.substr(dateStart, dateEnd - dateStart);

        // Convert the date string to a time structure
        struct tm tm = {};
        istringstream dateStream(dateString);
        dateStream >> get_time(&tm, "%a, %d %b %Y %H:%M:%S %Z");

        if (!dateStream.fail())
        {
            // Convert the time structure to a time_t value
            expireDate = mktime(&tm);

            // Print the parsed time_t value
            std::cout << "Parsed time_t value: " << date << std::endl;
        }
        else
        {
            // Handle the case where parsing the date string fails
            std::cerr << "Error parsing Date string." << std::endl;
        }
    }
}

// parse the Etag
void HttpResponse::parseEtag()
{
    // Find the position of the "ETag" header in the response
    size_t etagPos = responseContent.find("ETag: ");

    if (etagPos != std::string::npos)
    {
        size_t etagStart = etagPos + 6;
        size_t etagEnd = responseContent.find("\r\n", etagStart);
        etag = responseContent.substr(etagStart, etagEnd - etagStart);
    }
    else
    {
        // Handle the case where the "ETag" header is not found in the response
        std::cerr << "ETag header not found in the response." << std::endl;
    }
}
// parse the date
void HttpResponse::parseDate()
{
    size_t datePos = responseContent.find("Date: ");
    if (datePos != std::string::npos)
    {
        size_t dateStart = datePos + 6; // Move past "Date: "
        size_t dateEnd = responseContent.find("\r\n", dateStart);
        string dateString = responseContent.substr(dateStart, dateEnd - dateStart);

        // Convert the date string to a time structure
        struct tm tm = {};
        istringstream dateStream(dateString);
        dateStream >> get_time(&tm, "%a, %d %b %Y %H:%M:%S %Z");

        if (!dateStream.fail())
        {
            // Convert the time structure to a time_t value
            time_t date = mktime(&tm);

            // Print the parsed time_t value
            std::cout << "Parsed time_t value: " << date << std::endl;
        }
        else
        {
            // Handle the case where parsing the date string fails
            std::cerr << "Error parsing Date string." << std::endl;
        }
    }
}

// parse Code and Status eg.200 OK
void HttpResponse::parseCodeAndStatus()
{
    size_t spacePos = responseContent.find(' ');
    // int status_code;
    // Extract the status code substring 3-digit numerical code
    string statusCodeStr = responseContent.substr(spacePos + 1, 3);
    code = statusCodeStr;
    // TBD
    // try
    // {
    //     int status_code = std::stoi(statusCodeStr);
    // }
    // catch (const std::invalid_argument &e)
    // {
    //     // Handle the case where the status code is not a valid integer
    //     std::cerr << "Error parsing status code: " << e.what() << std::endl;
    // }
    // catch (const std::out_of_range &e)
    // {
    //     // Handle the case where the status code is out of the valid range for int
    //     std::cerr << "Error parsing status code: " << e.what() << std::endl;
    // }

    // Extract the status phrase substring
    size_t phraseStart = spacePos + 4; // Move past the space and status code
    size_t phraseEnd = responseContent.find('\r', phraseStart);
    status = responseContent.substr(phraseStart, phraseEnd - phraseStart);
}

time_t HttpResponse::getDate() const {
    return date;
}

time_t HttpResponse::getExpires() const {
    return expireDate;
}

string HttpResponse::getContent() const {
    return responseContent;
}

string HttpResponse::getEtag() const {
    return etag;
}

string HttpResponse::getStatus() const {
    return status;
}

string HttpResponse::getLastModify() const {
    return lastModifyDate;
}

string HttpResponse::getCode() const {
    return code;
}

size_t HttpResponse::getMaxAge() const {
    return max_age;
}

size_t HttpResponse::getMaxStale() const {
    return max_stale;
}

bool HttpResponse::isNocache() const{
    return no_cache;
}
bool HttpResponse::isNostore() const{
    return no_store;
}
bool HttpResponse::isPrivate() const{
    return is_private;
}
bool HttpResponse::ishasMustRevalidate() const{
    return hasMustRevalidate;
}

string HttpResponse::getFirstLine() const{
    istringstream responseStream(responseContent);
    string firstLine;
    
    // Get the first line from the stream
    getline(responseStream, firstLine);

    return firstLine;
}

bool HttpResponse::getChunked() const{
    size_t transferEncodingPos = responseContent.find("Transfer-Encoding: chunked");
    if (transferEncodingPos != string::npos) {
        // Check if "chunked" is present after "Transfer-Encoding:"
        size_t chunkedPos = responseContent.find("chunked", transferEncodingPos + 23);
        if (chunkedPos != string::npos) {
            return true;
        }
    }
    return false;
}

size_t HttpResponse::getContentLength() const{
    size_t contentLengthPos = responseContent.find("Content-Length: ");
    if (contentLengthPos != string::npos) {
        // Extract the value of the "Content-Length" header
        std::istringstream iss(responseContent.substr(contentLengthPos + 16));
        size_t contentLength = 0;
        iss >> contentLength;
        return contentLength;
    }

    return 0;
}

size_t HttpResponse::getHeadLength() const{
    size_t headerEnd = responseContent.find("\r\n\r\n");
     return (headerEnd != string::npos) ? headerEnd + 4 : 0;
}