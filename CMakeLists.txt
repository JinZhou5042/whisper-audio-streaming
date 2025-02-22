cmake_minimum_required(VERSION 3.15)
project(whisper_streaming 
    VERSION 1.0
    LANGUAGES C CXX)

add_definitions(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}")
add_definitions(-DWHISPER_MODEL_PATH="${CMAKE_SOURCE_DIR}/third_party/whisper.cpp/models")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add Homebrew paths for macOS
if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

# For Linux
if(UNIX)
    include_directories(/usr/include)
    link_directories(/usr/lib)
endif()

# Add nlohmann/json
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# Add whisper.cpp as subdirectory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/whisper.cpp ${CMAKE_BINARY_DIR}/whisper)

# Set the install prefix to the current directory's bin folder
set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/bin" CACHE PATH "Install path" FORCE)

# Add src directory
add_subdirectory(src)