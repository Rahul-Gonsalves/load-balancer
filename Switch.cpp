#include "Switch.h"

#include <ctime>
#include <iostream>
#include <sstream>

namespace {
constexpr const char* RESET = "\033[0m";
constexpr const char* MAGENTA = "\033[35m";
}

Switch::Switch(const SimulationConfig& processingConfig,
               const SimulationConfig& streamingConfig,
               const std::vector<std::string>& blockedPrefixes,
               const SwitchConfig& switchConfig)
    : processingLB(processingConfig, blockedPrefixes),
      streamingLB(streamingConfig, blockedPrefixes),
      config(switchConfig),
      processingInitialServers(processingConfig.initialServers),
      streamingInitialServers(streamingConfig.initialServers) {}

std::string Switch::randomIP(std::mt19937& rng) const {
    std::uniform_int_distribution<int> octet(0, 255);
    std::ostringstream oss;
    oss << octet(rng) << '.' << octet(rng) << '.' << octet(rng) << '.' << octet(rng);
    return oss.str();
}

Request Switch::generateRandomRequest(std::mt19937& rng) const {
    std::uniform_int_distribution<int> timeDist(1, config.maxProcessTime);
    std::uniform_int_distribution<int> typeDist(0, 1);

    Request req;
    req.ipIn = randomIP(rng);
    req.ipOut = randomIP(rng);
    req.timeRequired = timeDist(rng);
    req.jobType = (typeDist(rng) == 0) ? 'P' : 'S';
    return req;
}

void Switch::routeRequest(const Request& request) {
    if (request.jobType == 'P') {
        processingLB.enqueueRequest(request, true, true);
    } else {
        streamingLB.enqueueRequest(request, true, true);
    }
}

void Switch::simulate() {
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    const int initialProc = config.initialQueueMultiplier * processingInitialServers;
    const int initialStream = config.initialQueueMultiplier * streamingInitialServers;

    for (int i = 0; i < initialProc; ++i) {
        Request req = generateRandomRequest(rng);
        req.jobType = 'P';
        routeRequest(req);
    }
    for (int i = 0; i < initialStream; ++i) {
        Request req = generateRandomRequest(rng);
        req.jobType = 'S';
        routeRequest(req);
    }

    std::uniform_int_distribution<int> chance(1, 100);
    std::uniform_int_distribution<int> arrivals(1, config.maxNewRequestsPerCycle);

    for (int cycle = 0; cycle < config.runTime; ++cycle) {
        if (chance(rng) <= config.requestProbability) {
            int numArrivals = arrivals(rng);
            for (int i = 0; i < numArrivals; ++i) {
                routeRequest(generateRandomRequest(rng));
            }
        }

        processingLB.runOneCycle(false, nullptr);
        streamingLB.runOneCycle(false, nullptr);
    }

    processingLB.finalize();
    streamingLB.finalize();
}

void Switch::printSummary() const {
    if (config.colorOutput) {
        std::cout << MAGENTA << "\nSwitch Mode Summary\n" << RESET;
    } else {
        std::cout << "\nSwitch Mode Summary\n";
    }

    std::cout << "\n[Processing Load Balancer]\n";
    processingLB.printSummary();

    std::cout << "\n[Streaming Load Balancer]\n";
    streamingLB.printSummary();

    const SimulationStats p = processingLB.getStats();
    const SimulationStats s = streamingLB.getStats();

    std::cout << "\n[Aggregate]\n";
    std::cout << "Total generated: " << (p.totalGenerated + s.totalGenerated) << '\n';
    std::cout << "Total blocked:   " << (p.totalBlocked + s.totalBlocked) << '\n';
    std::cout << "Total processed: " << (p.totalProcessed + s.totalProcessed) << '\n';
    std::cout << "Servers added:   " << (p.serversAdded + s.serversAdded) << '\n';
    std::cout << "Servers removed: " << (p.serversRemoved + s.serversRemoved) << '\n';
}
