#include "cache.hpp"
#include <mutex>
std::mutex cacheMutex;
extern Logger logger;

bool FIFOCache::inCache(HttpRequest req){
        std::map<std::string, HttpResponse>::iterator cache_it = cacheMap.find(req.getURI());
        if (cache_it == cacheMap.end() ) { 
            return false; 
        }
        return true;
    }
    
void FIFOCache::removeCache(){
    lock_guard<mutex> guard(cacheMutex);
    std::pair<std::string, HttpResponse> temp = FIFOQue.front();
    std::string toRemove = temp.first;
    // remove from both
    FIFOQue.pop_front();
    cacheMap.erase(toRemove);
}

int FIFOCache:: addCache(HttpRequest req, HttpResponse res){
    if(res.isNostore()){
        logger.log_response200(1, req.getRequestId(), "have no-store");
        return -1;
    }
    if(res.isPrivate()){
        logger.log_response200(1, req.getRequestId(), "have private tag");
        return -1;
    }
    if(res.getEtag().empty()){
        logger.log_response200(1, req.getRequestId(), "not have Etag");
        return -1;
    }
    if(res.getLastModify().empty()){
        logger.log_response200(1, req.getRequestId(), "Last Modify is empty");
        return -1;
    }
    
    lock_guard<mutex> guard(cacheMutex);
    for(size_t i = 0;i<FIFOQue.size();i++){
        std::pair<std::string, HttpResponse> frontElement = FIFOQue.front();
        if(frontElement.first == req.getFirstLine()){
            FIFOQue.erase(FIFOQue.begin() + i);
        }
    }
    // giving out a space FULL
    if(FIFOQue.size() == capacity){
        removeCache();
    }

    cacheMap[req.getURI()] = res;
    FIFOQue.push_back({req.getURI(), res});
    
    // if(cacheMap.count(req.getURI())){
    //     cacheMap[req.getURI()].printRes();
    //     cout << "Now new response is in cache============================\n\n\n";
    // }
    // printCache();
    //log
    std::time_t now = time(nullptr);
    std::time_t validTime = res.getValidExpireTime();
    std::time_t expireTime = res.getExpires();

    if(expireTime < now){
        logger.log_response200(2, req.getRequestId(), "", expireTime);
    }else if(res.ishasMustRevalidate() || res.isNocache() || res.getMaxAge() == 0 || validTime < now){
        logger.log_response200(3, req.getRequestId(), "");
    }
    return 0;
}

// should prove it is in cache
bool FIFOCache::checkValid(HttpRequest req, HttpResponse res, int requestId){
    if(!inCache(req)){
        return false;
    }
    if(res.isNocache()){
        logger.log_getRequest(3, requestId);
        return false;
    }
    if(res.getMaxAge() == 0){
        logger.log_getRequest(3, requestId);
        return false;
    }
    
    std::time_t now = time(nullptr);
    std::time_t validTime = res.getValidExpireTime();
    std::time_t expireTime = res.getExpires();

    // not sure how to compare the time_t??
    if(expireTime < now){
        logger.log_getRequest(2, requestId, expireTime);
        return false;
    }
    // ID: in cache, requires validation
    if (res.ishasMustRevalidate() || validTime < now) {
        logger.log_getRequest(3, requestId);
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
    string newReq = req.getContent();
    if (!(resCache->getEtag()).empty()) {
        newReq += "If-None-Match: " + resCache->getEtag() + "\r\n";
    }
    if (!(resCache->getLastModify()).empty()) {
        newReq += "If-Modified-Since: " + resCache->getLastModify() + "\r\n";
    }
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
    std::string resStr = resValid->getFirstLine();
    logger.log_recvServer(req.getRequestId(), resStr, server);

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

void FIFOCache::printCache(){
    cout<<"Start printing cache...................\n";
    std::deque<std::pair<std::string, HttpResponse> >::iterator itQue = FIFOQue.begin();
    // std::map<std::string, HttpResponse>::iterator mapit = cacheMap.begin();
    if(FIFOQue.size() != cacheMap.size()){
        cout << "Cache size not matching\n";
        cout << "MAP:"<<cacheMap.size()<<endl;
        cout << "que:" <<FIFOQue.size()<<endl;
        return;
    }
    cout<<"IN QUE: "<<endl;
    while(itQue != FIFOQue.end()){
        cout << itQue->first <<endl;
        itQue->second.printRes();
        if(cacheMap.count(itQue->first)){
            cacheMap[itQue->first].printRes();
        }else{
            cout<<"NOT find in map\n";
        }
        ++itQue;
    }
}