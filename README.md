# CSCE412 Project 3: Load Balancer Simulation

## Overview
This project simulates a load balancer that distributes requests across web servers, scales server count based on queue pressure, and filters blocked IP ranges.

It supports:
- Single load balancer mode (core requirement)
- Bonus switch mode that routes `P` and `S` jobs to separate load balancers

## Files
- `main.cpp`: Driver, config parsing, mode selection
- `Request.h/.cpp`: Request data structure
- `WebServer.h/.cpp`: Web server worker logic
- `LoadBalancer.h/.cpp`: Queueing, scheduling, scaling, firewall, logging
- `Switch.h/.cpp`: Bonus routing layer for job-type-based dispatch
- `config.txt`: Runtime configuration
- `Makefile`: Build and clean commands
- `Doxyfile`: Doxygen configuration
- `docs/html/`: Generated Doxygen HTML output

## Build
```bash
make
```

## Run
Single load balancer mode:
```bash
./loadbalancer config.txt
```

Switch mode (bonus):
1. Set `useSwitch=true` in `config.txt`
2. Run:
```bash
./loadbalancer config.txt
```

## Key Config Options (`config.txt`)
- `initialServers`: Initial server count (single mode)
- `runTime`: Number of cycles to simulate
- `initialQueueMultiplier`: Initial queue = servers * multiplier
- `minThresholdFactor`: Lower scaling threshold factor
- `maxThresholdFactor`: Upper scaling threshold factor
- `scalingCooldownCycles`: Wait cycles between scale actions
- `requestProbability`: Percent chance of new arrivals each cycle
- `maxNewRequestsPerCycle`: Max new arrivals per cycle
- `maxProcessTime`: Max processing time per request
- `blockedPrefixes`: Comma-separated blocked IP prefixes
- `useSwitch`: Enable/disable bonus switch mode
- `processingServers`: Processing LB server count in switch mode
- `streamingServers`: Streaming LB server count in switch mode
- `logFilePath`, `processingLogFilePath`, `streamingLogFilePath`: Log file destinations

## Output and Logs
- Console output includes colorized events:
  - Red: blocked request
  - Yellow: server added
  - Cyan: server removed
  - Green: summary
- CSV log files include per-cycle metrics plus a final summary line.

## Required 10 Server / 10000 Cycle Run
Default `config.txt` is already set for:
- `initialServers=10`
- `runTime=10000`

Run:
```bash
./loadbalancer config.txt
```
This produces `log.txt` for submission.

## Doxygen Documentation
Generate docs:
```bash
doxygen Doxyfile
```
Main page after generation:
- `docs/html/index.html`

## Packaging for Submission
Clean old objects/binary:
```bash
make clean
```

Create zip with source and build files (no executables):
```bash
zip -r project3_submission.zip *.h *.cpp Makefile config.txt Doxyfile README.md docs
```

## Suggested Demo Flow (Video)
1. Show `config.txt` and explain parameters.
2. Run single mode and show scaling + firewall events.
3. Show `log.txt` summary.
4. Enable `useSwitch=true`, rerun, and show separate processing/streaming summaries.
5. Open `docs/html/index.html` to show generated documentation.
