#ifndef SWITCH_H
#define SWITCH_H

#include <random>
#include <string>
#include <vector>

#include "LoadBalancer.h"
#include "Request.h"

/**
 * @file Switch.h
 * @brief Defines a higher-level router that dispatches by job type.
 */

/**
 * @struct SwitchConfig
 * @brief Runtime settings for the bonus switch mode.
 */
struct SwitchConfig {
    int runTime = 10000;
    int initialQueueMultiplier = 100;
    int requestProbability = 40;
    int maxNewRequestsPerCycle = 3;
    int maxProcessTime = 20;
    bool colorOutput = true;
};

/**
 * @class Switch
 * @brief Routes processing and streaming requests to dedicated load balancers.
 */
class Switch {
private:
    LoadBalancer processingLB;
    LoadBalancer streamingLB;
    SwitchConfig config;
    int processingInitialServers;
    int streamingInitialServers;

    Request generateRandomRequest(std::mt19937& rng) const;
    std::string randomIP(std::mt19937& rng) const;
    void routeRequest(const Request& request);

public:
    /**
     * @brief Constructs switch with two load balancers and routing config.
     * @param processingConfig Config for processing load balancer.
     * @param streamingConfig Config for streaming load balancer.
     * @param blockedPrefixes Firewall prefix list applied to both load balancers.
     * @param switchConfig Shared switch/router configuration.
     */
    Switch(const SimulationConfig& processingConfig,
           const SimulationConfig& streamingConfig,
           const std::vector<std::string>& blockedPrefixes,
           const SwitchConfig& switchConfig);

    /**
     * @brief Executes routed simulation over configured runtime.
     */
    void simulate();

    /**
     * @brief Prints per-balancer and aggregate summaries.
     */
    void printSummary() const;
};

#endif
