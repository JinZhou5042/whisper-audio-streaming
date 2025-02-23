# Find required packages
find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

# Automatically find all source files
file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

# Create executable
message(STATUS "SOURCES found: ${SOURCES}")
message(STATUS "Runtime output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_executable(stream ${SOURCES})

# Link libraries
target_link_libraries(stream PRIVATE
    whisper
    ${SDL2_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
)