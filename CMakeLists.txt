cmake_minimum_required(VERSION 3.10)
project(NoMoreVolumeKeyPain)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define the executable target
add_executable(NoMoreVolumeKeyPain src/main.cpp res/app.rc)

# Set the subsystem to Windows
set_target_properties(NoMoreVolumeKeyPain PROPERTIES WIN32_EXECUTABLE YES)

# Link Windows libraries
target_link_libraries(NoMoreVolumeKeyPain
    winmm             # For multimedia API functions
    ole32             # For COM library functions like CoInitialize and CoCreateInstance
    uuid              # For generating GUIDs used in COM interfaces
    comctl32          # For common controls (if needed in the future)
    user32            # For Windows user interface functions
    gdi32             # For GDI functions like SetTextColor and DrawTextA
)

# Enable Unicode on Windows
add_definitions(-DUNICODE -D_UNICODE)
