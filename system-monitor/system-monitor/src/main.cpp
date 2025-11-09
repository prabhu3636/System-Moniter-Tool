#include "SystemMonitor.h"
#include "Ui.h"
#include <iostream>
#include <string>

int main(int argc, char **argv) {
    int default_interval = 2;
    std::string sort_flag = "cpu";
    // simple arg parse: -i <seconds>  -s cpu|mem|pid
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "-i" && i + 1 < argc) {
            default_interval = std::stoi(argv[++i]);
            if (default_interval < 1) default_interval = 1;
        } else if (a == "-s" && i + 1 < argc) {
            sort_flag = argv[++i];
        } else if (a == "-h" || a == "--help") {
            std::cout << "Usage: " << argv[0] << " [-i seconds] [-s cpu|mem|pid]\n";
            return 0;
        }
    }

    try {
        SystemMonitor monitor;
        Ui ui(monitor);
        // apply parsed defaults
        if (sort_flag == "cpu") ui.set_sort_mode(SortMode::CPU);
        else if (sort_flag == "mem") ui.set_sort_mode(SortMode::MEM);
        else ui.set_sort_mode(SortMode::PID);
        ui.set_refresh_interval(default_interval);
        ui.run();
    } catch (const std::exception &e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
