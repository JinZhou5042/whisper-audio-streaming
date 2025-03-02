cmake_minimum_required(VERSION 3.15)
project(whisper_streaming 
    VERSION 1.0
    LANGUAGES C CXX)

add_definitions(-DPROJECT_ROOT="${CMAKE_SOURCE_DIR}")
add_definitions(-DWHISPER_MODEL_PATH="${CMAKE_SOURCE_DIR}/third_party/whisper.cpp/models")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(APPLE)
    include_directories(/opt/homebrew/include)
    link_directories(/opt/homebrew/lib)
endif()

if(UNIX)
    include_directories(/usr/include)
    link_directories(/usr/lib)
endif()

# add nlohmann/json
include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(json)

# disable whisper installation
set(WHISPER_INSTALL OFF CACHE BOOL "Disable whisper installation" FORCE)
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)

# enable CoreML backend for Whisper on MacOS, this is a huge performance boost
if (APPLE)
    set(WHISPER_COREML ON CACHE BOOL "Enable CoreML backend for Whisper" FORCE)
    endif()

# enable BLAS to speed up
set(DGGML_BLAS ON CACHE BOOL "Enable BLAS" FORCE)

# the source code
add_subdirectory(src)

# submodules
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/whisper.cpp ${CMAKE_BINARY_DIR}/whisper EXCLUDE_FROM_ALL)

# bluetooth support
if (APPLE)
    find_library(COREBLUETOOTH_FRAMEWORK CoreBluetooth)
    target_link_libraries(livestreaming PRIVATE ${COREBLUETOOTH_FRAMEWORK})
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/simpleble/simpleble ${CMAKE_BINARY_DIR}/simpleble EXCLUDE_FROM_ALL)
# Add the "-idirafter" option to ensure that the build directory's "simpleble" folder 
# is searched for headers only after system headers. This prevents conflicts with the 
# standard C++ <version> header, which would otherwise be mistakenly shadowed by the 
# "VERSION" file in the "simpleble" directory.
target_compile_options(simpleble PRIVATE "-idirafter" "${CMAKE_BINARY_DIR}/simpleble")

set_target_properties(livestreaming PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)