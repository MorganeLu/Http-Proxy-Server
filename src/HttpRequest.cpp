#include "HttpRequest.hpp"


// Constructor
HttpRequest::HttpRequest(string initRequest, int requestId)
    : requestContent(initRequest), requestId(requestId)
{
    parseMethod();
    parseURI();
    parseHost();
    parseFirstLine();
    parseDate();
}

// parse the request date
void HttpRequest::parseDate()
{
    size_t datePos = requestContent.find("Date: ");
    if (datePos != std::string::npos)
    {
        size_t dateStart = datePos + 6; // Move past "Date: "
        size_t dateEnd = requestContent.find("\r\n", dateStart);
        string dateString = requestContent.substr(dateStart, dateEnd - dateStart);

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
// parse the firstLine
void HttpRequest::parseFirstLine()
{
    size_t firstLineEnd = requestContent.find("\r\n");
    firstLine = requestContent.substr(0, firstLineEnd);
}

// parse URI
void HttpRequest::parseURI()
{
    size_t uriStart = requestContent.find(" ") + 1;
    size_t uriEnd = requestContent.find(" ", uriStart);
    URI = requestContent.substr(uriStart, uriEnd - uriStart);
}

// parse host and port
void HttpRequest::parseHost()
{
    size_t hostStart = requestContent.find("Host: ");
    if (hostStart != std::string::npos)
    {
        hostStart += 6; // Move past "Host: "
        size_t hostEnd = requestContent.find("\r\n", hostStart);
        if (hostEnd != std::string::npos)
        {
            std::string hostWithPort = requestContent.substr(hostStart, hostEnd - hostStart);

            // Check if the host contains a colon (indicating a port)
            size_t colonPos = hostWithPort.find(":");
            if (colonPos != std::string::npos)
            {
                // Extract host and port separately
                host = hostWithPort.substr(0, colonPos);
                port = hostWithPort.substr(colonPos + 1);
            }
            else
            {
                // No port specified, use default
                host = hostWithPort;
                port = "80"; // Default HTTP port
            }
        }
        else
        {
            std::cerr << "Error: Host header is incomplete." << std::endl;
        }
    }
    else
    {
        std::cerr << "Error: Host header not found." << std::endl;
    }
}

// parse request method
void HttpRequest::parseMethod()
{
    if (requestContent.find("GET") != string::npos)
    {
        method = "GET";
    }
    else if (requestContent.find("POST") != string::npos)
    {
        method = "POST";
    }
    else if (requestContent.find("CONNECT") != string::npos)
    {
        method = "CONNECT";
    }
    else
    {
        size_t pos = requestContent.find(' ');
        method = requestContent.substr(0, pos);
    }
}

string HttpRequest::getMethod()
{
    return method;
}
string HttpRequest::getContent()
{
    return requestContent;
}
string HttpRequest::getHost()
{
    return host;
}
string HttpRequest::getPort()
{
    return port;
}
string HttpRequest::getURI()
{
    return URI;
}
string HttpRequest::getFirstLine()
{
    return firstLine;
}
time_t HttpRequest::getDate(){
    return date;
}

size_t HttpRequest::getRequestId(){
    return requestId;
}