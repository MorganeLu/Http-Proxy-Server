#ifndef CLIENT_HPP
#define CLIENT_HPP

#include<cstdlib>
#include<cstdio>
#include <cstring>
#include <iostream>
#include<vector>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>


class Client{
private:
    const char *hostname;
    const char *port;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    int socket_fd;

public:
    Client():hostname(NULL), port(NULL){}
    Client(const char * host, const char * port): hostname(host), port(port){}

    int buildClient(){
        memset(&host_info, 0, sizeof(host_info)); // init
        
        host_info.ai_family   = AF_UNSPEC;
        host_info.ai_socktype = SOCK_STREAM;

        int status;
        // cout<<"start building client"<<endl;
        cout<<"myServiering....:"<<hostname<<endl;
        status = getaddrinfo(hostname, port, &host_info, &host_info_list);
        if (status != 0) {
            std::cerr << "Error: cannot get address info for host" << std::endl;
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            // return EXIT_FAILURE;
        } // get address info for host

        socket_fd = socket(host_info_list->ai_family, 
                    host_info_list->ai_socktype, 
                    host_info_list->ai_protocol);
        if (socket_fd == -1) {
            std::cerr << "Error: cannot create socket" << std::endl;
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            // return EXIT_FAILURE;
        } // get socket

        return EXIT_SUCCESS;
    }

    int connect2Server() {
        std::cout << "Connecting to " << hostname << " on port " << port << "..." << std::endl;
    
        int status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if (status == -1) {
            std::cerr << "Error: cannot connect to socket" << std::endl;
            std::cerr << "  (" << hostname << "," << port << ")" << std::endl;
            // return -1;
        } // connect to the server
        return socket_fd;
    }

    ~Client() {
        free(host_info_list);
        // close(socket_fd);
    }

};

#endif