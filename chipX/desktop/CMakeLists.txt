cmake_minimum_required(VERSION 3.14)
project(chipX_desktop)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# I added FindRtAudio.cmake module so it can find sdl2 in a platform agnostic manner
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(RTAUDIO REQUIRED)


# link against these shared libraries
set(libs ${RTAUDIO_LIBRARY})
set(include_dirs ${RTAUDIO_INCLUDE_DIR} src ${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR})

#message("${include_dirs}")

add_subdirectory(../engine build)

include_directories(${include_dirs})

add_executable(chipX_desktop main.cpp)
target_link_libraries(chipX_desktop ${libs} chipxengine)

message("${CHIPXENGINE_INCLUDE_DIR}")

