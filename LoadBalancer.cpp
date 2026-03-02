#include "LoadBalancer.h"

#include <algorithm>
#include <ctime>
#include <iostream>
#include <sstream>

namespace {
constexpr const char* RESET = "\033[0m";
constexpr const char* GREEN = "\033[32m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* RED = "\033[31m";
constexpr const char* CYAN = "\033[36m";
}

LoadBalancer::LoadBalancer(const SimulationConfig& cfg, const std::vector<std::string>& blockedIPPrefixes)
    : blockedPrefixes(blockedIPPrefixes),
      config(cfg),
      clockCycle(0),
      scalingCooldownRemaining(0),
      nextServerID(1) {
    if (config.initialServers < 1) {
        config.initialServers = 1;
    }
    if (config.maxNewRequestsPerCycle < 1) {
        config.maxNewRequestsPerCycle = 1;
    }
    if (config.maxProcessTime < 1) {
        config.maxProcessTime = 1;
    }
    if (config.scalingCooldownCycles < 0) {
        config.scalingCooldownCycles = 0;
    }

    for (int i = 0; i < config.initialServers; ++i) {
        servers.emplace_back(nextServerID++);
    }
    stats.taskTimeMin = 1;
    stats.taskTimeMax = config.maxProcessTime;

    logFile.open(config.logFilePath);
    if (logFile.is_open()) {
        logFile << "cycle,queueSize,serverCount,totalGenerated,totalBlocked,totalProcessed\n";
    }
}

std::string LoadBalancer::randomIP(std::mt19937& rng) const {
    std::uniform_int_distribution<int> octet(0, 255);
    std::ostringstream oss;
    oss << octet(rng) << '.' << octet(rng) << '.' << octet(rng) << '.' << octet(rng);
    return oss.str();
}

Request LoadBalancer::generateRandomRequest(std::mt19937& rng) const {
    std::uniform_int_distribution<int> timeDist(1, config.maxProcessTime);
    std::uniform_int_distribution<int> typeDist(0, 1);

    Request req;
    req.ipIn = randomIP(rng);
    req.ipOut = randomIP(rng);
    req.timeRequired = timeDist(rng);
    req.jobType = (typeDist(rng) == 0) ? 'P' : 'S';
    return req;
}

bool LoadBalancer::isBlocked(const std::string& ip) const {
    for (const std::string& prefix : blockedPrefixes) {
        if (!prefix.empty() && ip.rfind(prefix, 0) == 0) {
            return true;
        }
    }
    return false;
}

bool LoadBalancer::tryAcceptRequest(const Request& request, bool printBlocked) {
    if (isBlocked(request.ipIn)) {
        ++stats.totalBlocked;
        if (printBlocked && config.colorOutput) {
            std::cout << RED << "[cycle " << clockCycle << "] blocked request from " << request.ipIn << RESET
                      << '\n';
        }
        return false;
    }

    requestQueue.push(request);
    updateMaxQueue();
    return true;
}

void LoadBalancer::initializeQueue(int size) {
    if (size <= 0) {
        return;
    }

    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
    for (int i = 0; i < size; ++i) {
        Request req = generateRandomRequest(rng);
        ++stats.totalGenerated;
        tryAcceptRequest(req, false);
    }
    stats.startingQueueSize = static_cast<int>(requestQueue.size());
}

bool LoadBalancer::enqueueRequest(const Request& request, bool countAsGenerated, bool printBlocked) {
    if (countAsGenerated) {
        ++stats.totalGenerated;
    }
    return tryAcceptRequest(request, printBlocked);
}

void LoadBalancer::maybeGenerateNewRequests(std::mt19937& rng) {
    std::uniform_int_distribution<int> chance(1, 100);
    std::uniform_int_distribution<int> arrivals(1, config.maxNewRequestsPerCycle);

    if (chance(rng) > config.requestProbability) {
        return;
    }

    int newRequests = arrivals(rng);
    for (int i = 0; i < newRequests; ++i) {
        Request req = generateRandomRequest(rng);
        ++stats.totalGenerated;
        tryAcceptRequest(req, true);
    }
}

void LoadBalancer::assignRequests() {
    for (WebServer& server : servers) {
        if (requestQueue.empty()) {
            break;
        }
        if (server.isIdle()) {
            server.assignRequest(requestQueue.front());
            requestQueue.pop();
        }
    }
}

void LoadBalancer::processServers() {
    for (WebServer& server : servers) {
        if (server.processOneCycle()) {
            ++stats.totalProcessed;
        }
    }
}

void LoadBalancer::checkScaling() {
    if (scalingCooldownRemaining > 0) {
        --scalingCooldownRemaining;
        return;
    }

    const int serverCount = static_cast<int>(servers.size());
    const int queueSize = static_cast<int>(requestQueue.size());
    const int upperBound = config.maxThresholdFactor * serverCount;
    const int lowerBound = config.minThresholdFactor * serverCount;

    if (queueSize > upperBound) {
        servers.emplace_back(nextServerID++);
        ++stats.serversAdded;
        scalingCooldownRemaining = config.scalingCooldownCycles;

        if (config.colorOutput) {
            std::cout << YELLOW << "[cycle " << clockCycle << "] added server; total=" << servers.size() << RESET
                      << '\n';
        }
        logEvent("added server");
        return;
    }

    if (queueSize < lowerBound && serverCount > 1) {
        auto idleIt = std::find_if(servers.rbegin(), servers.rend(), [](const WebServer& server) {
            return server.isIdle();
        });

        if (idleIt != servers.rend()) {
            std::size_t idx = static_cast<std::size_t>(std::distance(idleIt, servers.rend()) - 1);
            servers.erase(servers.begin() + static_cast<std::ptrdiff_t>(idx));
            ++stats.serversRemoved;
            scalingCooldownRemaining = config.scalingCooldownCycles;

            if (config.colorOutput) {
                std::cout << CYAN << "[cycle " << clockCycle << "] removed idle server; total=" << servers.size()
                          << RESET << '\n';
            }
            logEvent("removed server");
        }
    }
}

void LoadBalancer::updateMaxQueue() {
    int queueSize = static_cast<int>(requestQueue.size());
    if (queueSize > stats.maxQueueSize) {
        stats.maxQueueSize = queueSize;
    }
}

void LoadBalancer::logCycle() {
    if (!logFile.is_open()) {
        return;
    }
    logFile << clockCycle << ',' << requestQueue.size() << ',' << servers.size() << ',' << stats.totalGenerated << ','
            << stats.totalBlocked << ',' << stats.totalProcessed << '\n';
}

void LoadBalancer::logEvent(const std::string& event) {
    if (!logFile.is_open()) {
        return;
    }
    logFile << "# cycle " << clockCycle << ": " << event << '\n';
}

void LoadBalancer::runOneCycle(bool allowRandomArrivals, std::mt19937* rng) {
    ++clockCycle;

    if (allowRandomArrivals && rng != nullptr) {
        maybeGenerateNewRequests(*rng);
    }

    assignRequests();
    processServers();
    checkScaling();
    updateMaxQueue();
    logCycle();
}

void LoadBalancer::simulate() {
    std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));

    for (int i = 0; i < config.runTime; ++i) {
        runOneCycle(true, &rng);
    }

    finalize();
}

void LoadBalancer::finalize() {
    stats.finalQueueSize = static_cast<int>(requestQueue.size());
    stats.finalServerCount = static_cast<int>(servers.size());
    stats.inactiveServersAtEnd = 0;
    for (const WebServer& server : servers) {
        if (server.isIdle()) {
            ++stats.inactiveServersAtEnd;
        }
    }
    stats.activeServersAtEnd = stats.finalServerCount - stats.inactiveServersAtEnd;

    if (logFile.is_open()) {
        logFile << "# summary totalGenerated=" << stats.totalGenerated << " totalBlocked=" << stats.totalBlocked
                << " totalProcessed=" << stats.totalProcessed << " serversAdded=" << stats.serversAdded
                << " serversRemoved=" << stats.serversRemoved << " maxQueueSize=" << stats.maxQueueSize
                << " startingQueueSize=" << stats.startingQueueSize << " finalQueueSize=" << stats.finalQueueSize
                << " taskTimeRange=" << stats.taskTimeMin << ".." << stats.taskTimeMax
                << " activeServersAtEnd=" << stats.activeServersAtEnd
                << " inactiveServersAtEnd=" << stats.inactiveServersAtEnd
                << " finalServerCount=" << stats.finalServerCount << '\n';
        logFile << "# startingQueueSize=" << stats.startingQueueSize << '\n';
        logFile << "# endingQueueSize=" << stats.finalQueueSize << '\n';
        logFile << "# taskTimeRange=" << stats.taskTimeMin << ".." << stats.taskTimeMax << '\n';
        logFile << "# rejectedRequests=" << stats.totalBlocked << '\n';
        logFile << "# activeServersAtEnd=" << stats.activeServersAtEnd << '\n';
        logFile << "# inactiveServersAtEnd=" << stats.inactiveServersAtEnd << '\n';
    }
}

SimulationStats LoadBalancer::getStats() const {
    return stats;
}

void LoadBalancer::printSummary() const {
    const char* color = config.colorOutput ? GREEN : "";
    const char* reset = config.colorOutput ? RESET : "";

    std::cout << color << "\nSimulation Summary\n" << reset;
    std::cout << "Starting queue size:      " << stats.startingQueueSize << '\n';
    std::cout << "Task time range:          " << stats.taskTimeMin << ".." << stats.taskTimeMax << '\n';
    std::cout << "Total generated requests: " << stats.totalGenerated << '\n';
    std::cout << "Total blocked requests:   " << stats.totalBlocked << '\n';
    std::cout << "Total processed requests: " << stats.totalProcessed << '\n';
    std::cout << "Servers added:            " << stats.serversAdded << '\n';
    std::cout << "Servers removed:          " << stats.serversRemoved << '\n';
    std::cout << "Max queue size:           " << stats.maxQueueSize << '\n';
    std::cout << "Final queue size:         " << stats.finalQueueSize << '\n';
    std::cout << "Final server count:       " << stats.finalServerCount << '\n';
    std::cout << "Active servers at end:    " << stats.activeServersAtEnd << '\n';
    std::cout << "Inactive servers at end:  " << stats.inactiveServersAtEnd << '\n';
    std::cout << "Log file:                 " << config.logFilePath << '\n';
}
