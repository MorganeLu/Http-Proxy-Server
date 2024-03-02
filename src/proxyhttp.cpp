#include "proxyhttp.hpp"
#include "cache.hpp"
extern Logger logger;
FIFOCache myCache(100);

void ProxyHTTP::multiRun(){
    
}

void ProxyHTTP::run(){
    Server server(port);
    int ready = server.buildServer();
    // cout<<"starting..."<<endl;
    if(ready == EXIT_FAILURE){
        string msg = "Cannot build proxy server.";
        logger.log_message(3, 0, msg);
        exit(EXIT_FAILURE);
    }
    // printf("before while\n");
    // server.connect2Client();
    // printf("after connect");
    int client_id = -1;
    while (true) {
        std::pair<int, std::string> temp;
        temp = server.connect2Client();
        // printf("after connect in proxy\n");
        // cout<<temp.first<<endl;
        if(temp.first == -1){
            logger.log_message(3, -1, temp.second);
            continue;
        }
        // cout<<temp.first<<", "<<temp.second<<endl;
        int client_fd = temp.first;
        std::string client_ip = temp.second;

        client_id += 1;

        ClientInfo clientInfo(client_fd, client_id, client_ip);
        // clientInfo.printInfo();
        // clientInfo.set_fd(client_fd);
        // clientInfo.set_id(client_id);
        // clientInfo.set_ip(client_ip);
        // cout << "******************************************"<<endl;
        // clientInfo.printInfo();

        pthread_t thread;
        pthread_create(&thread, NULL, handle, &clientInfo);
    }
}



void ProxyHTTP::Log502(int client_fd, int requestId){
    // std::string response = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    // int status = send(client_fd, response.c_str(), response.size(), 0);
    // if (status == -1) {
    //     std::string msg = "Cannot send 502 Bad Gateway.";
    //     logger.log_message(3, requestId, msg);
    // }
    std::string msg = "502 Bad Gateway";
    logger.log_message(2, requestId, msg);
}
void ProxyHTTP::Log503(int client_fd, int requestId){
    // std::string response = "HTTP/1.1 503 Service Unavailable\r\n\r\n";
    // int status = send(client_fd, response.c_str(), response.size(), 0);
    // if (status == -1) {
    //     std::string msg = "Cannot send 503 Service Unavailable.";
    //     logger.log_message(3, requestId, msg);
    // }
    std::string msg = "503 Service Unavailable";
    logger.log_message(2, requestId, msg);
}
void ProxyHTTP::Log400(int client_fd, int requestId){
    // std::string response = "HTTP/1.1 400 Bad Request\r\n\r\n";
    // int status = send(client_fd, response.c_str(), response.size(), 0);
    // if (status == -1) {
    //     std::string msg = "Cannot send 400 error.";
    //     logger.log_message(3, requestId, msg);
    // }
    std::string msg = "400 Bad Request";
    logger.log_message(2, requestId, msg);
}
void ProxyHTTP::Log404(int client_fd, int requestId){
    // std::string response = "HTTP/1.1 404 Not Found\r\n\r\n";
    // int status = send(client_fd, response.c_str(), response.size(), 0);
    // if (status == -1) {
    //     std::string msg = "Cannot send 404 Not Found.";
    //     logger.log_message(3, requestId, msg);
    // }
    std::string msg = "404 Not Found";
    logger.log_message(2, requestId, msg);
}

void * ProxyHTTP::handle(void *clientInfo){
    ClientInfo *client_info = (ClientInfo *)clientInfo;
    int client_fd = client_info->get_fd();
    int requestId = client_info->get_id();
    std::string client_ip = client_info->get_ip();

    std::vector<char> message(1024 * 1024);
    int clientLen = recv(client_fd, &message.data()[0], 1024*1024, 0);
    // int clientLen = recv(client_fd, &test, sizeof(test), 0);
    // std::cout<<test<<endl;
    if (clientLen <= 0) {
        close(client_fd);
        return NULL;
    }
    message.data()[clientLen] = '\0';
    std::cout<<message.data()<<endl;
    string initRequest(message.begin(), message.end());
    // make Request object
    HttpRequest req(initRequest, requestId);

    // time_t now = time(nullptr);
    // std::tm * utc_time = std::gmtime(&now);
    std::string firstLine = req.getFirstLine();
    // std::cout <<"firstLine***********************:"<<firstLine<<"****************"<<endl;
    // cout<<"log_requestId: "<<requestId<<endl;
    // std::cout << "Host: "<<req.getHost().c_str()<<"~~~~~~~~~~"<<endl;
    //LOG
    logger.log_newRequest(requestId, firstLine, client_ip);
    // std::cout<<"after log"<<endl;

    if(req.getMethod() == "GET"){
        getHttp(req,client_fd,requestId);
    }else if(req.getMethod() == "POST"){
        postHttp(req,client_fd,requestId);
    }else if(req.getMethod() == "CONNECT"){
        connectHttp(req,client_fd);
        logger.log_closeTunnel(requestId);
    }else {
        Log400(client_fd, requestId);
    }
    close(client_fd);
    pthread_exit(NULL);

    return NULL;
}


// handle GET method
void ProxyHTTP::getHttp(HttpRequest request, int client_fd,int requestId){
    // cout<<"entering getHttp.............."<<endl;
    // cout<<"HOST:"<<request.getHost().c_str()<<endl;
    string tmpHost = request.getHost();
    const char * tmp = tmpHost.c_str();
    cout<<"Char*:"<<tmp<<endl;
    cout<<strlen(request.getHost().c_str())<<endl;
    cout<<sizeof(tmpHost)<<endl;
    Client myServer(tmp, request.getPort().c_str());
    // Client myServer("www.httpwatch.com",request.getPort().c_str());
    // std::cout << "Host: "<<request.getHost().c_str()<<"~~~~~~~~~~"<<endl;
    // std::cout << "Host: "<<request.getPort().c_str()<<"-not char"<<endl;

    // cout<<"GET=======================compare length:"<<sizeof(request.getHost().c_str())<<", "<<sizeof("www.artsci.utoronto.ca")<<endl;
    // if(request.getHost().c_str() == request.getHost()){
    //     cout<<"The same.\n";
    // }
    //connect to server
    myServer.buildClient();
    int myServer_fd = myServer.connect2Server();

    if(myServer_fd == -1){
        std::string msg = "ERROR Cannot connect to the socket.";
        logger.log_message(3,request.getRequestId(), msg);
        // return;
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
            logger.log_respondClient(request.getRequestId(), resfirstLine);
            close(myServer_fd);
            return;
        }
    }

    // Not in cache
    // cout << "NOT IN CACHE!!!!!!!!!!!!!!!!!!!!!!!!!\n";

    //LOG INPUT
    logger.log_getRequest(1, request.getRequestId());

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
    
    logger.log_contactServer(request.getRequestId(), firstLine, serverip);
    char first[1024];
    // Get from server the first time
    int serverLen = recv(myServer_fd,first,1024,0);
    if(serverLen == -1){
        Log502(client_fd,requestId);
        close(myServer_fd);
        return;
    }
    // cout <<"first len: "<<serverLen<<endl;
    // cout<<"first response: "<<first<<endl;
    string response_mesage;
    response_mesage.append(first,serverLen);
    HttpResponse myResponse(response_mesage);
    // myResponse.printRes();

    int unreceivedLen = myResponse.getHeadLength() + myResponse.getContentLength() - serverLen;
    bool isComplete = false;
    char continueResponse[1024];

    // cout<<"getContentLength():"<< myResponse.getContentLength()<<endl;
    // cout << "unreceivedLen: "<<unreceivedLen<<endl;
    // cout << "getHeadLength: "<<myResponse.getHeadLength()<<endl;
    // cout << "getContentLenongth: "<<myResponse.getContentLength()<<endl;


    while(!isComplete){
        // cout<<"in chunk!!!!!"<<endl;
        if(myResponse.getContentLength() != 0 && unreceivedLen <=0){
            isComplete = true;
            break;
        }
        int serverLen = recv(myServer_fd,continueResponse,1024,0);
        // cout<<serverLen;
        if(serverLen < 0){
            Log502(client_fd,requestId);
            close(myServer_fd);
            return;
        }else if(serverLen == 0 || serverLen == 5 || serverLen == 3){
            isComplete = true;
            break;
        }else{
            response_mesage.append(continueResponse,serverLen);
            unreceivedLen -= serverLen;
        }
    }

    // cout<<"END!!!!!"<<endl;
    // cout << "\n";

    // // Check if the last chunk has been received
    // size_t last_chunk_pos = response_mesage.find("\r\n0\r\n");
    // if (last_chunk_pos != std::string::npos) {
    //     // Erase the chunk extension and trailing CRLF
    //     response_mesage.erase(last_chunk_pos);
    // }
    
    HttpResponse finalResponse(response_mesage);
    // finalResponse.printRes();
    if(finalResponse.getCode() == "502"){
        Log502(client_fd, requestId);
    }
    if(finalResponse.getCode() == "400"){
        Log400(client_fd, requestId);
    }
    if(finalResponse.getCode() == "503"){
        Log503(client_fd, requestId);
    }
    if(finalResponse.getCode() == "404"){
        Log404(client_fd, requestId);
    }

    // cout<<"RESPONSE_MESSAGE:"<<response_mesage<<endl;
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
    if(finalResponse.getCode() == "200"){
        myCache.addCache(request, finalResponse);
    }
    

    // // if response is not chunked
    // if(!finalResponse.getChunked()){
    //     myCache.addCache(request, finalResponse);
    // }else{
    //     //Log
    //     logger.log_response200(1,request.getRequestId(),"response is chunked");
    // }
    logger.log_respondClient(request.getRequestId(), finalResponse.getFirstLine());

    close(myServer_fd);
}


// handle POST method
void ProxyHTTP::postHttp(HttpRequest request, int client_fd,int requestId){
    string tmpHost = request.getHost();
    const char * tmp = tmpHost.c_str();
    // cout<<"Char*:"<<tmp<<endl;
    // cout<<strlen(request.getHost().c_str())<<endl;
    // cout<<sizeof(tmpHost)<<endl;
    Client myServer(tmp, request.getPort().c_str());

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
    if(finalResponse.getCode() == "502"){
        Log502(client_fd, requestId);
    }
    if(finalResponse.getCode() == "400"){
        Log400(client_fd, requestId);
    }
    if(finalResponse.getCode() == "503"){
        Log503(client_fd, requestId);
    }
    if(finalResponse.getCode() == "404"){
        Log404(client_fd, requestId);
    }
    

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
    string tmpHost = request.getHost();
    const char * tmp = tmpHost.c_str();
    // cout<<"Char*:"<<tmp<<endl;
    // cout<<strlen(request.getHost().c_str())<<endl;
    // cout<<sizeof(tmpHost)<<endl;
    Client myServer(tmp, request.getPort().c_str());
    
    // cout<<"CONNECT=========================compare length:"<<sizeof(request.getHost())<<","<<sizeof("www.httpwatch.com")<<endl;


    //connect to server
    myServer.buildClient();
    int myServer_fd = myServer.connect2Server();

    // Send a success response to the client
    string response = "HTTP/1.1 200 OK\r\n\r\n";
    send(client_fd, response.c_str(), response.length(), 0);

    std::vector<int> client_fds;
    client_fds.push_back(client_fd);
    client_fds.push_back(myServer_fd);

    fd_set read_fds;
    int max_fd = -1;

    while(true){
        FD_ZERO(&read_fds);
        for (size_t i = 0; i < client_fds.size(); ++i) {
            FD_SET(client_fds[i], &read_fds);
            if (client_fds[i] > max_fd) {
                max_fd = client_fds[i];
            }
        }

        // struct timeval timeout;
        // timeout.tv_sec = 1000;
        // timeout.tv_usec = 0;

        int ready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (ready == -1) {
            perror("select");
            return;
        } else if (ready == 0) {
            std::cout << "No neighbor is ready to send or receive potato." << std::endl;
            return;
        }

        // Find the first ready client and send potato
        if(FD_ISSET(client_fd, &read_fds)){
            forwardData(request, client_fd, myServer_fd);
        }
        if(FD_ISSET(myServer_fd, &read_fds)){
            forwardData(request, myServer_fd, client_fd);
        }
    }

     // close 
     close(myServer_fd);
}

// connect helper function
void ProxyHTTP::forwardData(HttpRequest request, int source_fd, int dest_fd) {
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
}

