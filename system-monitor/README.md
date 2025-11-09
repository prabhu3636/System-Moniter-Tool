# 🖥️ System Monitor Tool

A **real-time system monitoring application** built in **C++** that displays CPU usage, memory utilization, and running processes — similar to the Linux `top` command.  
This version is designed to run smoothly on **macOS**, using **ncurses** for a terminal-based UI.

---

## 🎯 Objective

Create a system monitor tool in C++ that displays real-time information about system processes, memory usage, and CPU load, similar to the 'top' command.

---

## 🚀 Features

- 🔄 **Real-time updates** of CPU and memory usage  
- 🧠 **Displays process information:** PID, User, Command, Threads, CPU%, and Memory  
- ⚙️ **Sort processes** by CPU, Memory, or PID  
- 💀 **Kill processes** (SIGTERM or SIGKILL) directly from the interface  
- ⏱️ **Auto-refresh every few seconds**  
- 🎨 **Ncurses-based interactive UI**

---

## 🧰 Technologies Used

- **Programming Language:** C++17  
- **Libraries:** ncurses, libproc, mach  
- **Platform:** macOS  
- **Build Tool:** Make / CMake  
- **IDE:** Visual Studio Code  

---

## 🗂️ Project Structure

```
system-monitor/
├── src/
│   ├── main.cpp
│   ├── Ui.cpp
│   ├── Ui.h
│   ├── Process.cpp
│   ├── Process.h
│   ├── SystemMonitor.cpp
│   └── SystemMonitor.h
├── Makefile
├── README.md
└── screenshots/
    └── output.png
```

---

## ⚙️ Installation (Windows)

### 1️⃣ Install Dependencies
Install required packages using **Homebrew**:
```bash
brew install ncurses cmake
```

### 2️⃣ Clone the Repository
```bash
git clone https://github.com/<your-username>/system-monitor.git
cd system-monitor
```

### 3️⃣ Build the Project
```bash
make clean
make
```

### 4️⃣ Run the Application
```bash
./bin/system-monitor
```

---

## 🎮 Controls

| Key | Function |
|-----|-----------|
| ↑ / ↓ | Navigate process list |
| Enter | Show process details |
| c | Sort by CPU usage |
| m | Sort by Memory usage |
| p | Sort by Process ID |
| k | Send SIGTERM |
| K | Send SIGKILL |
| + / - | Increase or decrease refresh interval |
| q | Quit the program |

---

## 🖼️ Screenshot

| System Monitor Output |
|:----------------------:|
| ![Output Screenshot](./screenshots/output.png) |
| ![Output Screenshot](./screenshots/output1.png) |
---

## 🧠 Future Enhancements

- Add **per-process CPU usage percentage tracking**
- Include **disk I/O and network usage**
- Enable **cross-platform support (Linux, Windows)**
- Add **logging and export to CSV**

---

## 🧩 Example Output

```
System Monitor Tool (press 'q' to quit)  Refresh: 2s  Sort: CPU
CPU: 12.45%   Mem: 8192000 KB total, 2100340 KB used
 No.   PID    USER     CPU%   TIME+   MEM(KB)   COMMAND
--------------------------------------------------------
  1    1234   narayan  1.2    0:03     4321     system-monitor
  2    4321   root     0.7    0:01     1234     zsh
```

---

## 👨‍💻 Author

**Name:** Kumar Premsai  
**Institute:** ITER, Siksha 'O' Anusandhan  
**Project Title:** System Monitor Tool  

---

## 💡 Acknowledgements

- The **ncurses** library for terminal interface management  
- **Apple libproc** and **Mach APIs** for process and system data retrieval  
- Inspiration from the **Linux `top` command**

---

## 📄 License

This project is released under the **MIT License**.
