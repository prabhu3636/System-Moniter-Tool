#ifndef UI_H
#define UI_H

#include "SystemMonitor.h"
#include <vector>

enum class SortMode { CPU, MEM, PID };

class Ui {
public:
    Ui(SystemMonitor &mon);
    ~Ui();
    void run(); // main loop (blocking)

    // setters for initial CLI configuration
    void set_sort_mode(SortMode m) { sort_mode = m; }
    void set_refresh_interval(int s) { if (s >= 1) refresh_interval = s; }

private:
    SystemMonitor &monitor;
    SortMode sort_mode = SortMode::CPU;
    int selected_index = 0;
    int refresh_interval = 2; // seconds
    void draw();
    void draw_header(int rows, int cols);
    void draw_processes(int rows, int cols);
    void handle_input();
    bool running = true;
};

#endif // UI_H
