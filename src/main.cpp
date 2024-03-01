#include "proxyhttp.hpp"
#include "logger.hpp"

Logger logger("../var/log/erss/proxy.log"); 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int main() {
    const char * port = "12345";
    ProxyHTTP myProxy(port);
    myProxy.run();

  return 0;
}