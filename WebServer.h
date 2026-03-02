#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Request.h"

/**
 * @file WebServer.h
 * @brief Defines a simulated web server that processes one request at a time.
 */

/**
 * @class WebServer
 * @brief Models a server worker in the load balancer simulation.
 */
class WebServer {
private:
    int serverID;
    bool busy;
    Request currentRequest;
    int remainingTime;

public:
    /**
     * @brief Constructs a server with the provided identifier.
     * @param id Unique server identifier.
     */
    explicit WebServer(int id = 0);

    /**
     * @brief Assigns a request to this server if it is idle.
     * @param request Request to process.
     * @return True if assignment succeeded.
     */
    bool assignRequest(const Request& request);

    /**
     * @brief Processes one clock cycle.
     * @return True if a request completed on this cycle.
     */
    bool processOneCycle();

    /**
     * @brief Returns whether the server is currently idle.
     * @return True if idle.
     */
    bool isIdle() const;

    /**
     * @brief Clears the server's current request state.
     */
    void clearRequest();

    /**
     * @brief Gets the server identifier.
     * @return The server identifier.
     */
    int getServerID() const;

    /**
     * @brief Gets the current request.
     * @return The active request being processed.
     */
    const Request& getCurrentRequest() const;
};

#endif
