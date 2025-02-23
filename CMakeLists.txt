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

# Disable whisper installation
set(WHISPER_INSTALL OFF CACHE BOOL "Disable whisper installation" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)

# Enable CoreML backend for Whisper on MacOS, this is a huge performance boost
if (APPLE)
    set(WHISPER_COREML ON CACHE BOOL "Enable CoreML backend for Whisper" FORCE)
endif()

# Add whisper.cpp as subdirectory
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/whisper.cpp ${CMAKE_BINARY_DIR}/whisper EXCLUDE_FROM_ALL)

# Add src directory
add_subdirectory(src)

# Ensure src executables are also placed in bin/
set_target_properties(stream PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)