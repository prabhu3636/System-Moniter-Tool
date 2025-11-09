#include "Ui.h"
#include <ncurses.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <sys/wait.h>
#include <iostream>
#include <cmath>
#include <ctime>

Ui::Ui(SystemMonitor &mon) : monitor(mon) {
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    nodelay(stdscr, TRUE); // non-blocking getch
    curs_set(0);
    start_color();
    use_default_colors();
    init_pair(1, COLOR_CYAN, -1);
    init_pair(2, COLOR_YELLOW, -1);
    init_pair(3, COLOR_RED, -1);
}

Ui::~Ui() {
    endwin();
}

static std::string seconds_to_hms(double secs) {
    int total = (int)std::round(secs);
    int h = total / 3600;
    int m = (total % 3600) / 60;
    int s = total % 60;
    std::ostringstream oss;
    if (h > 0) oss << h << ":" << (m < 10 ? "0" : "") << m << ":" << (s < 10 ? "0" : "") << s;
    else oss << m << ":" << (s < 10 ? "0" : "") << s;
    return oss.str();
}

void Ui::run() {
    while (running) {
        monitor.update();
        draw();
        // handle input repeated for refresh_interval seconds with small sleeps
        int ms_total = refresh_interval * 1000;
        int step = 100;
        int elapsed = 0;
        while (elapsed < ms_total) {
            handle_input();
            if (!running) break;
            usleep(step * 1000);
            elapsed += step;
        }
        if (!running) break;
    }
}

static std::string pad_or_trim(const std::string &s, size_t width) {
    if (s.size() > width) return s.substr(0, width - 3) + "...";
    std::string out = s;
    while (out.size() < width) out += ' ';
    return out;
}

void Ui::draw_header(int rows, int cols) {
    attron(A_BOLD);
    mvprintw(0, 0, "System Monitor Tool (press 'q' to quit)  Refresh: %ds  Sort: %s",
             refresh_interval,
             (sort_mode == SortMode::CPU ? "CPU"
              : sort_mode == SortMode::MEM ? "MEM" : "PID"));
    attroff(A_BOLD);

    double cpu = monitor.get_cpu_usage_percent();
    uint64_t mem_total = monitor.get_mem_total_kb();
    uint64_t mem_used = monitor.get_mem_used_kb();
    mvprintw(1, 0, "CPU: %.2f%%   Mem: %llu KB total, %llu KB used",
             cpu,
             (unsigned long long)mem_total,
             (unsigned long long)mem_used);

    attron(COLOR_PAIR(1));
    mvprintw(3, 0, "%4s %6s %8s %7s %8s %8s %s",
             "No.", "PID", "USER", "CPU%", "TIME+", "MEM(KB)", "COMMAND");
    attroff(COLOR_PAIR(1));
}

void Ui::draw_processes(int rows, int cols) {
    auto procs = monitor.get_processes();

    // sort based on mode
    if (sort_mode == SortMode::CPU) {
        std::sort(procs.begin(), procs.end(),
                  [](const Process &a, const Process &b) {
                      return a.cpu_percent > b.cpu_percent;
                  });
    } else if (sort_mode == SortMode::MEM) {
        std::sort(procs.begin(), procs.end(),
                  [](const Process &a, const Process &b) {
                      return a.mem_kb > b.mem_kb;
                  });
    } else {
        std::sort(procs.begin(), procs.end(),
                  [](const Process &a, const Process &b) {
                      return a.pid < b.pid;
                  });
    }

    int max_display = rows - 6;
    if (max_display < 1) return;
    if (selected_index < 0) selected_index = 0;
    if (selected_index >= (int)procs.size())
        selected_index = (int)procs.size() - 1;

    int offset = 0;
    if (selected_index >= max_display) {
        offset = selected_index - max_display + 1;
    }

    for (int i = 0; i < max_display && (i + offset) < (int)procs.size(); ++i) {
        int idx = i + offset;
        int row = 4 + i;
        bool is_selected = (idx == selected_index);
        if (is_selected) attron(A_REVERSE);
        mvprintw(row, 0, "%4d %6d %8s %6.2f %8s %8llu %s",
                 idx + 1,
                 procs[idx].pid,
                 procs[idx].user.c_str(),
                 procs[idx].cpu_percent,
                 seconds_to_hms(procs[idx].time_seconds).c_str(),
                 (unsigned long long)procs[idx].mem_kb,
                 pad_or_trim(procs[idx].name, (size_t)cols - 50).c_str());
        if (is_selected) attroff(A_REVERSE);
    }

    // clear remaining lines
    for (int i = std::min((int)procs.size(), max_display); i < max_display; ++i) {
        int row = 4 + i;
        move(row, 0);
        clrtoeol();
    }
}

static void show_process_popup(WINDOW *parent, const Process &p) {
    int pr, pc;
    getmaxyx(parent, pr, pc);
    int w = std::min(80, pc - 4);
    int h = std::min(12, pr - 4);
    int starty = (pr - h) / 2;
    int startx = (pc - w) / 2;
    WINDOW *win = newwin(h, w, starty, startx);
    box(win, 0, 0);

    mvwprintw(win, 1, 2, "PID: %d  User: %s", p.pid, p.user.c_str());
    mvwprintw(win, 2, 2, "Command: %s", p.cmdline.c_str());
    mvwprintw(win, 3, 2, "CPU: %.2f%%  TIME+: %s  Mem: %llu KB",
              p.cpu_percent, seconds_to_hms(p.time_seconds).c_str(),
              (unsigned long long)p.mem_kb);
    mvwprintw(win, 4, 2, "Threads: %ld  Nice: %d",
              p.num_threads, p.nice_value);
    mvwprintw(win, 5, 2, "Started: (not available on macOS)");

    mvwprintw(win, h - 3, 2,
              "Press any key to close, 'k' to SIGTERM, 'K' to SIGKILL");
    wrefresh(win);

    int ch = wgetch(win);
    if (ch == 'k' || ch == 'K') {
        int sig = (ch == 'k') ? SIGTERM : SIGKILL;
        int ret = kill(p.pid, sig);
        werase(win);
        box(win, 0, 0);
        if (ret == 0)
            mvwprintw(win, 1, 2, "Signal sent to PID %d (%s).",
                      p.pid, (sig == SIGTERM ? "SIGTERM" : "SIGKILL"));
        else
            mvwprintw(win, 1, 2, "Failed to send signal to PID %d (errno %d).",
                      p.pid, errno);
        mvwprintw(win, 3, 2, "Press any key to continue.");
        wrefresh(win);
        wgetch(win);
    }

    delwin(win);
}

void Ui::draw() {
    clear();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    draw_header(rows, cols);
    draw_processes(rows, cols);
    mvprintw(rows - 1, 0,
             "Keys: ↑/↓ select  Enter=details  c=CPU  m=Mem  p=PID  k/K=Kill  +/-=Speed  q=Quit");
    refresh();
}

void Ui::handle_input() {
    int ch = getch();
    if (ch == ERR) return;
    auto procs = monitor.get_processes();

    // sort same as draw
    if (sort_mode == SortMode::CPU) {
        std::sort(procs.begin(), procs.end(),
                  [](const Process &a, const Process &b) {
                      return a.cpu_percent > b.cpu_percent;
                  });
    } else if (sort_mode == SortMode::MEM) {
        std::sort(procs.begin(), procs.end(),
                  [](const Process &a, const Process &b) {
                      return a.mem_kb > b.mem_kb;
                  });
    } else {
        std::sort(procs.begin(), procs.end(),
                  [](const Process &a, const Process &b) {
                      return a.pid < b.pid;
                  });
    }

    int display_count = (int)procs.size();
    switch (ch) {
        case 'q':
        case 'Q':
            running = false;
            break;
        case KEY_UP:
            if (selected_index > 0) --selected_index;
            break;
        case KEY_DOWN:
            if (selected_index + 1 < display_count) ++selected_index;
            break;
        case '\n':
        case KEY_ENTER:
            if (display_count > 0 && selected_index >= 0 &&
                selected_index < display_count) {
                show_process_popup(stdscr, procs[selected_index]);
                draw();
            }
            break;
        case 'c':
        case 'C':
            sort_mode = SortMode::CPU;
            selected_index = 0;
            break;
        case 'm':
        case 'M':
            sort_mode = SortMode::MEM;
            selected_index = 0;
            break;
        case 'p':
        case 'P':
            sort_mode = SortMode::PID;
            selected_index = 0;
            break;
        case '+':
            if (refresh_interval < 30) refresh_interval++;
            break;
        case '-':
            if (refresh_interval > 1) refresh_interval--;
            break;
        case 'k':
        case 'K': {
            if (display_count == 0) break;
            if (selected_index < 0 || selected_index >= display_count) break;
            Process p = procs[selected_index];
            echo();
            nodelay(stdscr, FALSE);
            int sig = (ch == 'k') ? SIGTERM : SIGKILL;
            mvprintw(2, 0, "Kill PID %d (%s) with %s? (y/N): ",
                     p.pid, p.name.c_str(),
                     (sig == SIGTERM ? "SIGTERM" : "SIGKILL"));
            clrtoeol();
            int c = getch();
            if (c == 'y' || c == 'Y') {
                int ret = kill(p.pid, sig);
                if (ret == 0)
                    mvprintw(2, 0,
                             "Signal sent to PID %d. Press any key to continue.",
                             p.pid);
                else
                    mvprintw(2, 0,
                             "Failed to send signal to PID %d (errno %d). Press any key.",
                             p.pid, errno);
                getch();
            }
            nodelay(stdscr, TRUE);
            noecho();
            break;
        }
        default:
            break;
    }
}
