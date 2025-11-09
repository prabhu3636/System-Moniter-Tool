#include "SystemMonitor.h"
#include <libproc.h>
#include <sys/sysctl.h>
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/vm_statistics.h>
#include <unistd.h>
#include <pwd.h>
#include <vector>
#include <sstream>
#include <iostream>

SystemMonitor::SystemMonitor() {
    update();
}

// Get total system memory in KB
uint64_t SystemMonitor::get_mem_total_kb() const {
    int64_t mem;
    size_t len = sizeof(mem);
    if (sysctlbyname("hw.memsize", &mem, &len, NULL, 0) == 0)
        return mem / 1024;
    return 0;
}

// Get used memory in KB
uint64_t SystemMonitor::get_mem_used_kb() const {
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    vm_statistics64_data_t vmstat;
    kern_return_t kr = host_statistics64(
        mach_host_self(),
        HOST_VM_INFO64,
        (host_info64_t)&vmstat,
        &count
    );

    if (kr != KERN_SUCCESS) return 0;

    uint64_t used =
        (vmstat.active_count +
         vmstat.inactive_count +
         vmstat.wire_count) *
        getpagesize();
    return used / 1024;
}

// Get overall CPU usage percentage
double SystemMonitor::get_cpu_usage_percent() const {
    host_cpu_load_info_data_t cpuinfo;
    mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
    kern_return_t kr = host_statistics(
        mach_host_self(),
        HOST_CPU_LOAD_INFO,
        (host_info_t)&cpuinfo,
        &count
    );

    if (kr != KERN_SUCCESS) return 0.0;

    static uint64_t prev_total = 0, prev_idle = 0;
    uint64_t user = cpuinfo.cpu_ticks[CPU_STATE_USER];
    uint64_t sys = cpuinfo.cpu_ticks[CPU_STATE_SYSTEM];
    uint64_t idle = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    uint64_t nice = cpuinfo.cpu_ticks[CPU_STATE_NICE];
    uint64_t total = user + sys + idle + nice;

    double diff_total = total - prev_total;
    double diff_idle = idle - prev_idle;
    prev_total = total;
    prev_idle = idle;

    if (diff_total <= 0) return 0.0;
    return 100.0 * (1.0 - (diff_idle / diff_total));
}

// Collect and update process information
void SystemMonitor::update() {
    processes.clear();

    int bufsize = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    std::vector<pid_t> pids(bufsize / sizeof(pid_t));
    proc_listpids(PROC_ALL_PIDS, 0, &pids[0], bufsize);

    for (pid_t pid : pids) {
        if (pid <= 0) continue;

        struct proc_bsdinfo bsdinfo;
        if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &bsdinfo, sizeof(bsdinfo)) <= 0)
            continue;

        struct proc_taskinfo taskinfo;
        if (proc_pidinfo(pid, PROC_PIDTASKINFO, 0, &taskinfo, sizeof(taskinfo)) <= 0)
            continue;

        Process p;
        p.pid = pid;
        p.name = bsdinfo.pbi_comm;

        struct passwd *pw = getpwuid(bsdinfo.pbi_uid);
        p.user = pw ? pw->pw_name : std::to_string(bsdinfo.pbi_uid);

        p.mem_kb = taskinfo.pti_resident_size / 1024;
        p.time_seconds = (taskinfo.pti_total_user + taskinfo.pti_total_system) / 1e9;
        p.num_threads = taskinfo.pti_threadnum;
        p.nice_value = bsdinfo.pbi_nice;
        p.cpu_percent = 0.0; // placeholder (optional to sample later)

        // Get process command path
        char pathbuf[PROC_PIDPATHINFO_MAXSIZE];
        if (proc_pidpath(pid, pathbuf, sizeof(pathbuf)) > 0)
            p.cmdline = pathbuf;
        else
            p.cmdline = p.name;

        processes.push_back(p);
    }
}
