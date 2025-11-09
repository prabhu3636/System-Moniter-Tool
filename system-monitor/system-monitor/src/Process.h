#ifndef PROCESS_H
#define PROCESS_H

#include <string>
#include <cstdint>

struct Process {
    int pid = 0;
    std::string user;
    std::string name;
    std::string cmdline;
    uint64_t utime = 0;
    uint64_t stime = 0;
    uint64_t total_time = 0;
    double time_seconds = 0.0;
    double cpu_percent = 0.0;
    uint64_t mem_kb = 0;
    unsigned long vsize = 0;
    long num_threads = 0;
    uint64_t starttime = 0;
    int nice_value = 0;   // ✅ <--- add this line
};

#endif
