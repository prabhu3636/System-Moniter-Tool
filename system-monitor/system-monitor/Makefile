CXX = clang++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I/opt/homebrew/opt/ncurses/include
LDFLAGS = -L/opt/homebrew/opt/ncurses/lib -lncurses
SRC = src/main.cpp src/Process.cpp src/SystemMonitor.cpp src/Ui.cpp
BIN = bin/system-monitor

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin
	$(CXX) $(CXXFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

clean:
	rm -rf bin
