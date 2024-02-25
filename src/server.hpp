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
    Server():hostname(NULL), port("4444"){}
    Server(const char *port):hostname(NULL), port(port){}

    int buildServer(){
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;
        host_info.ai_flags    = AI_PASSIVE;
        memset(&host_info, 0, sizeof(host_info));

        int status;

        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0) {
            std::cerr << "Error: cannot get address info for host" << std::endl;
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            return EXIT_FAILURE;
        } //if

        socket_fd = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        if (socket_fd == -1) {
            std::cerr << "Error: cannot create socket" << std::endl;
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            return EXIT_FAILURE;
        } //if

        int yes = 1;
        status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
        status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            std::cerr << "Error: cannot bind socket" << std::endl;
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            return EXIT_FAILURE;
        } //if

        status = listen(socket_fd, 512);
        if (status == -1) {
            std::cerr << "Error: cannot listen on socket" << std::endl; 
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            return EXIT_FAILURE;
        } //if
        return EXIT_SUCCESS;
    }

    std::pair<int, std::string> connect2Client(){
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        int client_connection_fd;
        client_connection_fd = accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        if (client_connection_fd == -1) {
            std::cerr << "Error: cannot accept connection on socket" << std::endl;
            return {-1, "cannot accept connection on socket."};
        } //if

        struct sockaddr_in * addr_ip = (struct sockaddr_in *)&socket_addr;
        std::string ip = inet_ntoa(addr_ip->sin_addr);

        return {client_connection_fd, ip};
    }

    // void run(){
    //     buildServer();
    // }

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
};

#endif