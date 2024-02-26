#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <ctime>

using namespace std;

extern pthread_mutex_t lock;
class Logger {
private:
    std::ofstream m_logfile;
    std::string m_filename;

public:
    Logger(const std::string& filename) : m_filename(filename) {
        m_logfile.open(filename, std::ios::app); // Open file in append mode
    }

    ~Logger() {
        if (m_logfile.is_open())
            m_logfile.close();
    }

    void log_newRequest(size_t id, string &firstLine, string &host) {
        std::time_t currentTime = std::time(nullptr);
        struct tm *localTime = localtime(&currentTime);
        char* time_char = asctime(localTime);
        string timeStr(time_char);

        // ID:	"REQUEST"	from	IPFROM	@	TIME
        std::string idStr = std::to_string(id);
        std::string message = idStr + ": \"" + firstLine + "\" from" + host + "@" + timeStr;
        printf("in logger\n");
        if (m_logfile.is_open()) {
            printf("can wirte in log\n");
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    void log_getRequest(int status, size_t id, time_t expireTime=std::time(nullptr)){
        std::string situation;
        if(status == 1){ // not in cache	
            situation = ": not in cache";
        }else if(status == 2){
            struct tm *localTime = localtime(&expireTime);
            char* time_char = asctime(localTime);
            string timeStr(time_char);
            situation = ": in cache, but expired at" + timeStr;
        }else if(status == 3){
            situation = ": in cache, requires validation";
        }else if(status == 4){
            situation = ": in cache, valid";
        }else{
            situation = ": Cache part error";
        }

        std::string idStr = std::to_string(id);
        std::string message = idStr + situation;
        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    void log_contactServer(size_t id, string &firstLine, string &serverStr){
        std::string idStr = std::to_string(id);
        // std::string serverStr(server);

        std::string message = idStr + ": Requesting \"" + firstLine + "\" from " + serverStr;
        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    void log_recvServer(size_t id, string &firstLine, string &serverStr){
        std::string idStr = std::to_string(id);
        // std::string serverStr(server);

        std::string message = idStr + ": Received \"" + firstLine + "\" from " + serverStr;
        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    void log_response200(int status, size_t id, string reason, time_t expireTime=std::time(nullptr)){
        std::string situation;
        if(status == 1){
            situation = reason;
        }else if(status == 2){
            struct tm *localTime = localtime(&expireTime);
            char* time_char = asctime(localTime);
            string timeStr(time_char);
            situation = "cached, expires at" + timeStr;
        }else if(status == 3){
            situation = "cached, but requires re-validation";
        }
        std::string idStr = std::to_string(id);
        std::string message = idStr + situation;
        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    void log_respondClient(size_t id, string response){
        std::string idStr = std::to_string(id);
        std::string message = idStr + "Responding \"" + response + "\"";

        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    void log_closeTunnel(size_t id){
        std::string idStr = std::to_string(id);
        std::string message = idStr + ": Tunnel closed";

        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }

    // 1->note, 2->warning, 3->error
    void log_message(int status, size_t id, std::string &message){
        std::string idStr = std::to_string(id);
        std::string situation;
        if(status == 1){
            situation = idStr + ": NOTE " + message;
        }else if(status == 2){
            situation = idStr + ": WARNING " + message;
        }else if(status == 3){
            situation = idStr + ": ERROR " + message;
        }

        if (m_logfile.is_open()) {
            pthread_mutex_lock(&lock);
            m_logfile << message << std::endl;
            pthread_mutex_unlock(&lock);
        }
    }
};


#endif