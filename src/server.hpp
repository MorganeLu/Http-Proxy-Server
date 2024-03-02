#ifndef SERVER_HPP
#define SERVER_HPP

#include<cstdlib>
#include<cstdio>
#include <cstring>
#include <iostream>
#include<vector>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>


class Server {
private:
    const char *hostname;
    const char *port;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    // std::string ip;
    // std::vector<int> client_fds;
    int socket_fd;

public:
    Server():hostname(NULL), port("12345"){}
    Server(const char *port):hostname(NULL), port(port){}

    int buildServer();

    std::pair<int, std::string> connect2Client();

    ~Server() {
        free(host_info_list);
        close(socket_fd);
    }
};

class ClientInfo{
private:
    int client_fd;
    int client_id;
    std::string client_ip;
public:
    ClientInfo(): client_fd(-1), client_id(-1), client_ip(NULL){}
    ClientInfo(int client_fd, int client_id, std::string client_ip): client_fd(client_fd), client_id(client_id), client_ip(client_ip){}
    void set_fd(int fd){
        client_fd = fd;
    }
    void set_id(int id){
        client_id = id;
    }
    void set_ip(std::string &ip){
        client_ip = ip;
    }
    int get_fd(){
        return client_fd;
    }
    int get_id(){
        return client_id;
    }
    std::string get_ip(){
        return client_ip;
    }
    void printInfo(){
        std::cout << "client fd: "<<client_fd<<std::endl;
        std::cout << "client id: "<<client_id<<std::endl;
        std::cout << "client ip: "<<client_ip<<std::endl;

    }
};

#endif