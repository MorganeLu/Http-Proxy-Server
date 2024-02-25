#include "proxyhttp.hpp"
#include "cache.hpp"
extern Logger logger;
FIFOCache myCache(100);

void ProxyHTTP::multiRun(){
    
}

void ProxyHTTP::run(){
    Server server(port);
    int ready = server.buildServer();
    cout<<"starting..."<<endl;
    if(ready == EXIT_FAILURE){
        string msg = "Cannot build proxy server.";
        logger.log_message(3, 0, msg);
        exit(EXIT_FAILURE);
    }
    printf("before while\n");
    // server.connect2Client();
    // printf("after connect");
    int client_id = -1;
    while (true) {
        std::pair<int, std::string> temp;
        temp = server.connect2Client();
        printf("after connect");
        if(temp.first == -1){
            logger.log_message(3, -1, temp.second);
            continue;
        }
        int client_fd = temp.first;
        std::string client_ip = temp.second;

        client_id += 1;

        ClientInfo clientInfo;
        clientInfo.set_fd(client_fd);
        clientInfo.set_id(client_id);
        clientInfo.set_ip(client_ip);
        printf("before thread\n");

        pthread_t thread;
        pthread_create(&thread, NULL, handle, &clientInfo);
    }
}



void ProxyHTTP::Log502(int client_fd, int requestId){
    std::string response = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    int status = send(client_fd, response.c_str(), response.size(), 0);
    if (status == -1) {
        std::string msg = "Cannot send 502 Bad Gateway.";
        logger.log_message(3, requestId, msg);
    }
}
void ProxyHTTP::Log503(int client_fd, int requestId){
    std::string response = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
    int status = send(client_fd, response.c_str(), response.size(), 0);
    if (status == -1) {
        std::string msg = "Cannot send 503 Service Unavailable.";
        logger.log_message(3, requestId, msg);
    }
}
void ProxyHTTP::Log400(int client_fd, int requestId){
    std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    int status = send(client_fd, response.c_str(), response.size(), 0);
    if (status == -1) {
        std::string msg = "Cannot send 400 error.";
        logger.log_message(3, requestId, msg);
    }
}
void ProxyHTTP::Log404(int client_fd, int requestId){
    std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
    int status = send(client_fd, response.c_str(), response.size(), 0);
    if (status == -1) {
        std::string msg = "Cannot send 404 Not Found.";
        logger.log_message(3, requestId, msg);
    }
}

void * ProxyHTTP::handle(void *clientInfo){
    ClientInfo *client_info = (ClientInfo *)clientInfo;
    int client_fd = client_info->get_fd();
    int requestId = client_info->get_id();
    std::string client_ip = client_info->get_ip();

    std::vector<char> message;
    int clientLen = recv(client_fd, &message.data()[0], message.size(), 0);
    if (clientLen <= 0) {
        close(client_fd);
        return NULL;
    }
    message.data()[clientLen] = '\0';
    string initRequest(message.begin(), message.end());
    // make Request object
    HttpRequest req(initRequest, requestId);

    // time_t now = time(nullptr);
    // std::tm * utc_time = std::gmtime(&now);
    std::string firstLine = req.getFirstLine();
    logger.log_newRequest(requestId, firstLine, client_ip);

    if(req.getMethod() == "GET"){
        getHttp(req,client_fd,requestId); ///TBD
    }else if(req.getMethod() == "POST"){
        postHttp(req,client_fd,requestId); ///TBD
    }else if(req.getMethod() == "Connect"){
        connectHttp(req,client_fd);
        logger.log_closeTunnel(requestId);
    }else {
        // handle error, which one??????
        // Log404(client_fd, requestId);
        Log400(client_fd, requestId);
    }
    close(client_fd);
    pthread_exit(NULL);

    return NULL;
}


// handle GET method
void ProxyHTTP::getHttp(HttpRequest request, int client_fd,int requestId){
    Client myServer(request.getHost().c_str(),request.getPort().c_str());

    //connect to server
    myServer.buildClient();
    int myServer_fd = myServer.connect2Server();

    if(myServer_fd == -1){
        std::string msg = "ERROR Cannot connect to the socket.";
        logger.log_message(3,request.getRequestId(), msg);
        return;
    }

    // bool succeed = true;
    //IF in cache
    if(myCache.inCache(request)){
        HttpResponse *http = myCache.getCache(request, myServer_fd);
        if(http == NULL){
            // succeed = false;
        }else{
            HttpResponse resCache = *http;
            int sent = send(client_fd, resCache.getContent().c_str(), resCache.getContent().length(), 0);
            if(sent == -1){
                //LOG 
                std::string msg = "ERROR Sending response to client failed.";
                logger.log_message(3,request.getRequestId(), msg);
                close(myServer_fd);
                return;
            }
            std::string serverip = request.getHost();
            std::string resfirstLine = resCache.getFirstLine();
            logger.log_contactServer(request.getRequestId(), resfirstLine, serverip);
            close(myServer_fd);
            return;
        }

    }

    // Not in cache

    //LOG INPUT
    logger.log_response200(1,request.getRequestId(),"not in cache");

    string content = request.getContent();
    const char *request_message = content.c_str();

    // send to server
    int clinetLen = send(myServer_fd,request_message,strlen(request_message),0);
    if(clinetLen == -1){
        //LOG 
        std::string msg = "ERROR Sending response to server failed.";
        logger.log_message(3,request.getRequestId(), msg);
        close(myServer_fd);
        exit(EXIT_FAILURE);
    }
    //Log
    std::string firstLine = request.getFirstLine();
    std::string serverip = request.getHost();
    logger.log_recvServer(request.getRequestId(), firstLine, serverip);

    char first[1024];
    // Get from server the first time
    int serverLen = recv(myServer_fd,first,1024,0);
    if(serverLen == -1){
        Log502(client_fd,requestId);
        close(myServer_fd);
        return;
    }

    string response_mesage;
    response_mesage.append(first,serverLen);
    HttpResponse myResponse(response_mesage);

    int unreceivedLen =myResponse.getHeadLength() + myResponse.getContentLength() - serverLen;
    // bool isComplete = false;
    char continueResponse[1024];


    while(unreceivedLen>0){
        int serverLen = recv(myServer_fd,continueResponse,1024,0);
        if(serverLen < 0){
            Log502(client_fd,requestId);
            close(myServer_fd);
            return;
        }else if(serverLen == 0){
            break;
        }else{
            response_mesage.append(continueResponse,serverLen);
            unreceivedLen -= serverLen;
        }
    }

    // Check if the last chunk has been received
    size_t last_chunk_pos = response_mesage.find("\r\n0\r\n");
    if (last_chunk_pos != std::string::npos) {
        // Erase the chunk extension and trailing CRLF
        response_mesage.erase(last_chunk_pos);
    }
    
    HttpResponse finalResponse(response_mesage);

    std::string finalfirstLine = finalResponse.getFirstLine();
    // std::string serverip = request.getHost();
    logger.log_recvServer(request.getRequestId(), finalfirstLine, serverip);

    // send message from server to client

    int sent = send(client_fd, response_mesage.c_str(), response_mesage.length(), 0);
    if(sent == -1){
        //LOG 
        std::string msg = "ERROR Sending response to client failed.";
        logger.log_message(3,request.getRequestId(), msg);
        close(myServer_fd);
        return;
    }

    // if response is not chunked
    if(!finalResponse.getChunked()){
        // add cache 
        myCache.addCache(request,finalResponse);
    }else{
        //Log
        logger.log_response200(1,request.getRequestId(),"not cacheable because response is chunked");
    }
  close(myServer_fd);
}


// handle POST method
void ProxyHTTP::postHttp(HttpRequest request, int client_fd,int requestId){
    Client myServer(request.getHost().c_str(),request.getPort().c_str());

    //connect to server
    myServer.buildClient();
    int myServer_fd = myServer.connect2Server();

    if(myServer_fd == -1){
        std::string msg = "ERROR Cannot connect to the socket.";
        logger.log_message(3,request.getRequestId(), msg);
        return;
    }

    string content = request.getContent();
    const char *request_message = content.c_str();

    int clinetLen = send(myServer_fd,request_message,strlen(request_message),0);
    if(clinetLen == -1){
        //LOG 
        std::string msg = "ERROR Sending response to server failed.";
        logger.log_message(3,request.getRequestId(), msg);
        close(myServer_fd);
        return;
    }
    //Log
    std::string firstLine = request.getFirstLine();
    std::string serverip = request.getHost();
    logger.log_recvServer(request.getRequestId(),firstLine, serverip);
  
    
    char first[1024];
    // Get from server the first time
    int serverLen = recv(myServer_fd,first,1024,0);
    if(serverLen == -1){
        Log502(client_fd,requestId);
        close(myServer_fd);
        return;
    }

    // char firstResponse[1024];
    // serverLen = recv(myServer_fd,firstResponse,1024,0);
    // if(serverLen == -1){
    //     Log502(client_fd,requestId);
    //     close(myServer_fd);
    //     return;
    // }
    
    // start receive message...
    string response_mesage;
    response_mesage.append(first,serverLen);
    HttpResponse myResponse(response_mesage);

    // string response_mesage;
    // response_mesage.append(first,serverLen);
    // HttpResponse myResponse(response_mesage);

    int unreceivedLen =myResponse.getHeadLength() + myResponse.getContentLength() - serverLen;
    // bool isComplete = false;
    char continueResponse[1024];


    while(unreceivedLen>0){
        int serverLen = recv(myServer_fd,continueResponse,1024,0);
        if(serverLen < 0){
            Log502(client_fd,requestId);
            close(myServer_fd);
            return;
        }else if(serverLen == 0){
            break;
        }else{
            response_mesage.append(continueResponse,serverLen);
            unreceivedLen -= serverLen;
        }
    }
    // Check if the last chunk has been received
    size_t last_chunk_pos = response_mesage.find("\r\n0\r\n");
    if (last_chunk_pos != std::string::npos) {
        // Erase the chunk extension and trailing CRLF
        response_mesage.erase(last_chunk_pos);
    }
    
    // receive all messages
    HttpResponse finalResponse(response_mesage);

    // LOG
    std::string finalfirstLine = finalResponse.getFirstLine();
    logger.log_recvServer(request.getRequestId(), finalfirstLine, serverip);

    // send message from server to client
    int sent = send(client_fd, response_mesage.c_str(), response_mesage.length(), 0);
    if(sent == -1){
        std::string msg = "ERROR Sending response to client failed.";
        logger.log_message(3, request.getRequestId(), msg);
        close(myServer_fd);
        return;
    }

    // ID: Responding "RESPONSE"
    logger.log_respondClient(request.getRequestId(), finalResponse.getStatus());
    close(myServer_fd);
}




// handle connect method
void ProxyHTTP::connectHttp(HttpRequest request, int client_fd){
    Client myServer(request.getHost().c_str(),request.getPort().c_str());

    //connect to server
    myServer.buildClient();
    int myServer_fd = myServer.connect2Server();

    if(myServer_fd == -1){
        std::string msg = "ERROR Cannot connect to the socket.";
        logger.log_message(3,request.getRequestId(), msg);
        return;
    }

    // Send a success response to the client
    string response = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, response.c_str(), response.length(), 0);

    // Forward data between client and target server
    forwardData(request, client_fd, myServer_fd, "Forwarding data from client to server");
    forwardData(request, myServer_fd, client_fd, "Forwarding data from server to client");

     // close 
     close(myServer_fd);
}

// connect helper function
void ProxyHTTP::forwardData(HttpRequest request, int source_fd, int dest_fd, const string& logMessage) {
    char buffer[65536];
    int bytes_received = recv(source_fd, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        int bytes_sent = send(dest_fd, buffer, bytes_received, 0);
        if (bytes_sent <= 0) {
            //LOG
            std::string msg = "ERROR Forwarding response to client failed.";
            logger.log_message(3,request.getRequestId(), msg);
        }
    }
    std::string msg = logMessage;
    logger.log_message(1, request.getRequestId(), msg);
}

