# Adaptive Load Balancer Simulator

A configurable C++ simulation of an adaptive load balancer designed to model queue-driven request distribution, dynamic server scaling, and basic firewall behavior.

## System Summary
The simulator models production-style request handling with these capabilities:
- Request queue management with per-cycle scheduling
- Dynamic horizontal scaling of server workers based on queue pressure
- Cooldown-protected scaling decisions to reduce thrashing
- Prefix-based source IP blocking (firewall guardrail)
- Structured logging for cycle metrics and run-end status
- Optional switch routing mode that separates streaming and processing workloads

## Architecture
Core components:
- `Request`: immutable request payload (source IP, destination IP, processing time, job type)
- `WebServer`: single-worker execution unit, processes one request at a time
- `LoadBalancer`: queue owner and orchestrator (assignment, scaling, filtering, metrics)
- `Switch` (optional): routes `P` (processing) and `S` (streaming) jobs to dedicated load balancers

## Runtime Model
Each clock cycle:
1. New requests may arrive based on configured probability.
2. Blocked source IP prefixes are filtered before queue admission.
3. Idle servers pull from the request queue.
4. Active servers process one cycle of work.
5. Scaling logic evaluates queue pressure:
   - scale up if `queueSize > maxThresholdFactor * serverCount`
   - scale down if `queueSize < minThresholdFactor * serverCount`
6. Cooldown enforces wait cycles between scaling actions.
7. Per-cycle metrics are appended to log output.

## Build
```bash
make
```

## Run
Default:
```bash
./loadbalancer config.txt
```

Switch mode:
1. Set `useSwitch=true` in `config.txt`
2. Run `./loadbalancer config.txt`

## Configuration Reference
`config.txt` controls runtime behavior without code changes.

Main controls:
- `initialServers`: starting server count (single mode)
- `runTime`: number of simulation cycles
- `initialQueueMultiplier`: initial queue = `initialServers * initialQueueMultiplier`
- `minThresholdFactor`, `maxThresholdFactor`: scaling bounds
- `scalingCooldownCycles`: minimum delay between scale actions
- `requestProbability`: chance of new arrivals each cycle (0-100)
- `maxNewRequestsPerCycle`: upper bound for new arrivals in a cycle
- `maxProcessTime`: max request processing time (cycles)
- `blockedPrefixes`: comma-separated IP prefixes to reject
- `colorOutput`: ANSI colorized runtime events

Switch controls:
- `useSwitch`: enable workload routing mode
- `processingServers`, `streamingServers`: initial server pools
- `processingLogFilePath`, `streamingLogFilePath`: per-balancer logs

## Log Contract
Cycle log columns:
- `cycle,queueSize,serverCount,totalGenerated,totalBlocked,totalProcessed`

Run-end summary includes:
- start/end queue size
- task time range
- total generated/blocked/processed
- servers added/removed
- active/inactive servers at end
- final server count

## Documentation
Generate API/reference docs:
```bash
doxygen Doxyfile
```
Entry page:
- `docs/html/index.html`

## Repository Layout
- `main.cpp`
- `Request.h`, `Request.cpp`
- `WebServer.h`, `WebServer.cpp`
- `LoadBalancer.h`, `LoadBalancer.cpp`
- `Switch.h`, `Switch.cpp`
- `config.txt`
- `Makefile`
- `Doxyfile`
- `docs/html/`
