#ifndef SYSTEM_MONITOR_H
#define SYSTEM_MONITOR_H

#include "Process.h"
#include <vector>

class SystemMonitor {
public:
    SystemMonitor();
    void update();
    std::vector<Process> get_processes() const { return processes; }

    double get_cpu_usage_percent() const;
    uint64_t get_mem_total_kb() const;
    uint64_t get_mem_used_kb() const;

private:
    std::vector<Process> processes;
};

#endif
