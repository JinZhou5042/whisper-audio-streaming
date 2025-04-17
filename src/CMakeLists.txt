find_package(SDL2 REQUIRED)
find_package(CURL REQUIRED)

file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
)

message(STATUS "SOURCES found: ${SOURCES}")
message(STATUS "Runtime output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

add_executable(livestreaming ${SOURCES})

target_link_libraries(livestreaming PRIVATE
    ${SDL2_LIBRARIES}
    ${CURL_LIBRARIES}
    nlohmann_json::nlohmann_json
    whisper
    # simpleble::simpleble
)
