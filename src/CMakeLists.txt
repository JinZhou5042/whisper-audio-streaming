# Find required packages
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

# Automatically find all source files
file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Print found source files (optional, for debugging)
message(STATUS "Found source files: ${SOURCES}")

# Create executable
add_executable(stream ${SOURCES})

# Set rpath to find the whisper library
set_target_properties(stream PROPERTIES
    INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)

# Include directories
target_include_directories(stream PRIVATE
    ${SDL2_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/submodules/whisper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link libraries

target_link_libraries(stream PRIVATE
    whisper
    ${SDL2_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
)

# Install target
install(TARGETS stream
    RUNTIME DESTINATION .
)