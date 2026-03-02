#include "WebServer.h"

WebServer::WebServer(int id) : serverID(id), busy(false), currentRequest(), remainingTime(0) {}

bool WebServer::assignRequest(const Request& request) {
    if (busy) {
        return false;
    }
    currentRequest = request;
    remainingTime = request.timeRequired;
    busy = true;
    return true;
}

bool WebServer::processOneCycle() {
    if (!busy) {
        return false;
    }

    --remainingTime;
    if (remainingTime <= 0) {
        clearRequest();
        return true;
    }
    return false;
}

bool WebServer::isIdle() const {
    return !busy;
}

void WebServer::clearRequest() {
    busy = false;
    remainingTime = 0;
    currentRequest = Request();
}

int WebServer::getServerID() const {
    return serverID;
}

const Request& WebServer::getCurrentRequest() const {
    return currentRequest;
}
