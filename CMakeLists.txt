cmake_minimum_required(VERSION 3.5...4.1)
project(54ChanPlayer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add allolib as a subdirectory (assumes it's in the parent directory)
set(ALLOLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../allolib)
add_subdirectory(${ALLOLIB_DIR} ${CMAKE_CURRENT_BINARY_DIR}/allolib)

# Create the executable
add_executable(mainplayer mainplayer.cpp)

# Link allolib
target_link_libraries(mainplayer PRIVATE al)

# Copy audio files to build directory (optional)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/sourceAudio)