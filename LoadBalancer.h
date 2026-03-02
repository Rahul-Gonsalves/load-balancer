#ifndef LOADBALANCER_H
#define LOADBALANCER_H

#include <fstream>
#include <queue>
#include <random>
#include <string>
#include <vector>

#include "Request.h"
#include "WebServer.h"

/**
 * @file LoadBalancer.h
 * @brief Defines the LoadBalancer class and simulation configuration.
 */

/**
 * @struct SimulationConfig
 * @brief Stores runtime parameters for the load balancer simulation.
 */
struct SimulationConfig {
    int initialServers = 10;
    int runTime = 10000;
    int initialQueueMultiplier = 100;

    int minThresholdFactor = 50;
    int maxThresholdFactor = 80;
    int scalingCooldownCycles = 5;

    int requestProbability = 40;
    int maxNewRequestsPerCycle = 3;
    int maxProcessTime = 20;

    std::string logFilePath = "log.txt";
    bool colorOutput = true;
};

/**
 * @struct SimulationStats
 * @brief Aggregates metrics collected during simulation.
 */
struct SimulationStats {
    int totalGenerated = 0;
    int totalBlocked = 0;
    int totalProcessed = 0;

    int serversAdded = 0;
    int serversRemoved = 0;

    int maxQueueSize = 0;
    int finalQueueSize = 0;
    int finalServerCount = 0;
};

/**
 * @class LoadBalancer
 * @brief Manages request queue, server pool, scaling, and firewall filtering.
 */
class LoadBalancer {
private:
    std::vector<WebServer> servers;
    std::queue<Request> requestQueue;
    std::vector<std::string> blockedPrefixes;

    SimulationConfig config;
    SimulationStats stats;

    int clockCycle;
    int scalingCooldownRemaining;
    int nextServerID;

    std::ofstream logFile;

    std::string randomIP(std::mt19937& rng) const;
    Request generateRandomRequest(std::mt19937& rng) const;
    bool isBlocked(const std::string& ip) const;

    bool tryAcceptRequest(const Request& request, bool printBlocked);

    void maybeGenerateNewRequests(std::mt19937& rng);
    void assignRequests();
    void processServers();
    void checkScaling();
    void updateMaxQueue();

    void logCycle();
    void logEvent(const std::string& event);

public:
    /**
     * @brief Constructs a load balancer with the given configuration and block list.
     * @param cfg Simulation settings.
     * @param blockedIPPrefixes Prefix patterns to block (example: "192.168.").
     */
    LoadBalancer(const SimulationConfig& cfg, const std::vector<std::string>& blockedIPPrefixes);

    /**
     * @brief Initializes queue with random requests.
     * @param size Number of requests to seed.
     */
    void initializeQueue(int size);

    /**
     * @brief Adds one request into this load balancer, respecting firewall rules.
     * @param request Request to enqueue.
     * @param countAsGenerated Whether to increment generated statistics.
     * @param printBlocked Whether to print blocked request events.
     * @return True if request was queued.
     */
    bool enqueueRequest(const Request& request, bool countAsGenerated = true, bool printBlocked = true);

    /**
     * @brief Runs one simulation cycle.
     * @param allowRandomArrivals If true, random requests are generated this cycle.
     * @param rng Optional RNG used for random arrivals when enabled.
     */
    void runOneCycle(bool allowRandomArrivals, std::mt19937* rng = nullptr);

    /**
     * @brief Runs the simulation for the configured runtime.
     */
    void simulate();

    /**
     * @brief Finalizes end-of-run statistics and writes summary to log.
     */
    void finalize();

    /**
     * @brief Gets final simulation statistics.
     * @return Collected stats.
     */
    SimulationStats getStats() const;

    /**
     * @brief Prints a human-readable summary to stdout.
     */
    void printSummary() const;
};

#endif
