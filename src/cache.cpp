#include "cache.hpp"
extern Logger logger;

bool FIFOCache::inCache(HttpRequest req){
        std::map<std::string, HttpResponse>::iterator cache_it = cacheMap.find(req.getURI());
        if (cache_it == cacheMap.end() ) { 
            return false; 
        }
        return true;
    }
    
void FIFOCache::removeCache(){
    std::pair<std::string, HttpResponse> temp = FIFOQue.front();
    std::string toRemove = temp.first;
    // remove from both
    FIFOQue.pop_front();
    cacheMap.erase(toRemove);
}

int FIFOCache:: addCache(HttpRequest req, HttpResponse res){
    // not prevent caching entirely; it just requires caches to revalidate the response before serving it again
    if (res.isNocache() || res.isNostore()) {
        cout<<"situation 1 cache\n";
        return -1; 
    }
    if (res.isPrivate() || res.getEtag().empty() || res.getLastModify().empty()) {
        cout<<"situation 2 cache\n";
        return -1;
    }

    for(size_t i = 0;i<FIFOQue.size();i++){
        std::pair<std::string, HttpResponse> frontElement = FIFOQue.front();
        if(frontElement.first == req.getFirstLine()){
            FIFOQue.erase(FIFOQue.begin() + i);
        }
    }
    cout<<"Put in cache\n";
    // giving out a space FULL
    if(FIFOQue.size() == capacity){
        removeCache();
    }

    cacheMap[req.getURI()] = res;
    FIFOQue.push_back({req.getURI(), res});
    return 0;
}

// should prove it is in cache
bool FIFOCache::checkValid(HttpRequest req, HttpResponse res, int requestId){
    if(!inCache(req)){
        return false;
    }
    
    std::time_t now = time(nullptr);
    std::time_t expireTime = res.getExpires();

    // ID: in cache, requires validation
    if (res.isNocache() || res.ishasMustRevalidate()) {
        logger.log_getRequest(3, requestId);
        return false;
    }
    // not sure how to compare the time_t??
    if(expireTime < now){
        logger.log_getRequest(2, requestId, expireTime);
        return false;
    }

    // ID: in cache, valid
    logger.log_getRequest(4, requestId);
    return true;
}

HttpResponse *FIFOCache::getCache(HttpRequest req, int socket_fd){
    HttpResponse * resCache = &cacheMap[req.getURI()];
    int requestId = req.getRequestId();

    // if this response is in cache and valid
    if(checkValid(req, *resCache, requestId)){
        return resCache;
    }
    // If not valid
    // send to server
    int status = send(socket_fd, req.getContent().c_str(), req.getContent().length(), 0);
    if (status == -1) {
        std::string msg = "Cannot send request to server for re-validation.";
        logger.log_message(3, req.getRequestId(), msg);
        exit(EXIT_FAILURE);
    }
    std::string firstLine = req.getFirstLine();
    std::string server = req.getHost();
    logger.log_contactServer(req.getRequestId(), firstLine, server);

    // receive from server
    std::vector<char> res_char(1024);
    // int status = 0;
    status = recv(socket_fd, res_char.data(), res_char.size(), 0);
    if(status == -1){
        std::string msg = "Cannot receive revalidate source, 503 Gateway Timeout";
        logger.log_message(3, req.getRequestId(), msg);
    }
    std::string serverRes(res_char.data(), res_char.size());

    HttpResponse * resValid = new HttpResponse(serverRes);
    int ready = addCache(req, *resValid);
    if(ready == -1){
        std::string msg = "Cannot Add in cache";
        logger.log_message(1, req.getRequestId(), msg);
        return NULL;
    }


    // ID: Received "xx" from serverip
    // std::string server = req.getHost();
    // std::string resStr = resValid->getFirstLine();
    // logger.log_recvServer(req.getRequestId(), resStr, server);

    // 304: source not modify
    if (resValid->getCode() == "304") {
        return resCache;
    }
    // 200:have successfully get the needed source
    else if (resValid->getCode() == "200") {
        cacheMap[req.getURI()] = *resValid;
        return resValid;
    }
    // cannot revalidate
    else {
        std::string msg = "503 Gateway Timeout";
        logger.log_message(3, req.getRequestId(), msg);
        exit(EXIT_FAILURE);
    }

    }