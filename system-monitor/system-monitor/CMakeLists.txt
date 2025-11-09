cmake_minimum_required(VERSION 3.5)
project(system_monitor CXX)

set(CMAKE_CXX_STANDARD 17)
set(SOURCE_FILES
    src/main.cpp
    src/Process.cpp
    src/SystemMonitor.cpp
    src/Ui.cpp
)

add_executable(system-monitor ${SOURCE_FILES} src/Process.h src/SystemMonitor.h src/Ui.h)
target_include_directories(system-monitor PRIVATE src)
target_link_libraries(system-monitor PRIVATE ncurses)
set_target_properties(system-monitor PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
