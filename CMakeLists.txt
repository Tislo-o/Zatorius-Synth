cmake_minimum_required(VERSION 3.21)
project(Audio)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_DIR "C:/SFML-3.0.0/lib/cmake/SFML" CACHE PATH "SFML CMake config directory")

find_package(SFML 3 REQUIRED COMPONENTS Graphics Audio)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(Audio main.cpp graphics.cpp)

target_link_libraries(Audio PRIVATE SFML::Graphics SFML::Audio)

add_custom_command(TARGET Audio POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:Audio>/assets)

set_target_properties(Audio PROPERTIES WIN32_EXECUTABLE TRUE)
