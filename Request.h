#ifndef REQUEST_H
#define REQUEST_H

#include <string>

/**
 * @file Request.h
 * @brief Defines the Request data structure used by the load balancer simulation.
 */

/**
 * @struct Request
 * @brief Represents a single inbound web request.
 */
struct Request {
    std::string ipIn;
    std::string ipOut;
    int timeRequired;
    char jobType;

    /**
     * @brief Constructs a default request.
     */
    Request();

    /**
     * @brief Constructs a request with all fields set.
     * @param in Source IP address.
     * @param out Destination IP address.
     * @param time Processing time in clock cycles.
     * @param type Job type ('P' for processing, 'S' for streaming).
     */
    Request(const std::string& in, const std::string& out, int time, char type);
};

#endif
