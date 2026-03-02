#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "LoadBalancer.h"
#include "Switch.h"

/**
 * @file main.cpp
 * @brief Driver for Project 3 load balancer simulation.
 */

namespace {
struct AppConfig {
    SimulationConfig base;

    bool useSwitch = false;
    int processingServers = 5;
    int streamingServers = 5;
    std::string processingLogFilePath = "processing_log.txt";
    std::string streamingLogFilePath = "streaming_log.txt";

    std::vector<std::string> blockedPrefixes = {"10."};
};

std::string trim(const std::string& s) {
    std::size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }

    std::size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }

    return s.substr(start, end - start);
}

std::vector<std::string> splitCSV(const std::string& csv) {
    std::vector<std::string> parts;
    std::stringstream ss(csv);
    std::string token;
    while (std::getline(ss, token, ',')) {
        token = trim(token);
        if (!token.empty()) {
            parts.push_back(token);
        }
    }
    return parts;
}

bool toBool(const std::string& value) {
    std::string lower = value;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return (lower == "1" || lower == "true" || lower == "yes" || lower == "on");
}

void loadConfigFile(const std::string& path, AppConfig& appConfig) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open config file: " << path << "\nUsing default values.\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) {
            continue;
        }

        std::string key = trim(line.substr(0, eqPos));
        std::string value = trim(line.substr(eqPos + 1));

        if (key == "initialServers") {
            appConfig.base.initialServers = std::stoi(value);
        } else if (key == "runTime") {
            appConfig.base.runTime = std::stoi(value);
        } else if (key == "initialQueueMultiplier") {
            appConfig.base.initialQueueMultiplier = std::stoi(value);
        } else if (key == "minThresholdFactor") {
            appConfig.base.minThresholdFactor = std::stoi(value);
        } else if (key == "maxThresholdFactor") {
            appConfig.base.maxThresholdFactor = std::stoi(value);
        } else if (key == "scalingCooldownCycles") {
            appConfig.base.scalingCooldownCycles = std::stoi(value);
        } else if (key == "requestProbability") {
            appConfig.base.requestProbability = std::stoi(value);
        } else if (key == "maxNewRequestsPerCycle") {
            appConfig.base.maxNewRequestsPerCycle = std::stoi(value);
        } else if (key == "maxProcessTime") {
            appConfig.base.maxProcessTime = std::stoi(value);
        } else if (key == "logFilePath") {
            appConfig.base.logFilePath = value;
        } else if (key == "colorOutput") {
            appConfig.base.colorOutput = toBool(value);
        } else if (key == "blockedPrefixes") {
            appConfig.blockedPrefixes = splitCSV(value);
        } else if (key == "useSwitch") {
            appConfig.useSwitch = toBool(value);
        } else if (key == "processingServers") {
            appConfig.processingServers = std::stoi(value);
        } else if (key == "streamingServers") {
            appConfig.streamingServers = std::stoi(value);
        } else if (key == "processingLogFilePath") {
            appConfig.processingLogFilePath = value;
        } else if (key == "streamingLogFilePath") {
            appConfig.streamingLogFilePath = value;
        }
    }
}
}  // namespace

/**
 * @brief Program entry point.
 * @param argc Number of arguments.
 * @param argv Argument values (optional config path as argv[1]).
 * @return Exit code.
 */
int main(int argc, char** argv) {
    AppConfig appConfig;

    std::string configPath = "config.txt";
    if (argc >= 2) {
        configPath = argv[1];
    }

    loadConfigFile(configPath, appConfig);

    if (!appConfig.useSwitch) {
        std::cout << "Starting single load balancer simulation with " << appConfig.base.initialServers
                  << " initial servers for " << appConfig.base.runTime << " cycles.\n";

        LoadBalancer lb(appConfig.base, appConfig.blockedPrefixes);
        lb.initializeQueue(appConfig.base.initialServers * appConfig.base.initialQueueMultiplier);
        lb.simulate();
        lb.printSummary();
        return 0;
    }

    SimulationConfig processingConfig = appConfig.base;
    SimulationConfig streamingConfig = appConfig.base;

    processingConfig.initialServers = std::max(1, appConfig.processingServers);
    streamingConfig.initialServers = std::max(1, appConfig.streamingServers);
    processingConfig.logFilePath = appConfig.processingLogFilePath;
    streamingConfig.logFilePath = appConfig.streamingLogFilePath;

    SwitchConfig switchConfig;
    switchConfig.runTime = appConfig.base.runTime;
    switchConfig.initialQueueMultiplier = appConfig.base.initialQueueMultiplier;
    switchConfig.requestProbability = appConfig.base.requestProbability;
    switchConfig.maxNewRequestsPerCycle = appConfig.base.maxNewRequestsPerCycle;
    switchConfig.maxProcessTime = appConfig.base.maxProcessTime;
    switchConfig.colorOutput = appConfig.base.colorOutput;

    std::cout << "Starting switch-mode simulation for " << appConfig.base.runTime << " cycles (processingServers="
              << processingConfig.initialServers << ", streamingServers=" << streamingConfig.initialServers
              << ").\n";

    Switch sw(processingConfig, streamingConfig, appConfig.blockedPrefixes, switchConfig);
    sw.simulate();
    sw.printSummary();

    return 0;
}
