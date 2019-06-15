cmake_minimum_required(VERSION 3.14)
project(chipX_desktop)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# I added FindRtAudio.cmake module so it can find sdl2 in a platform agnostic manner
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

find_package(PORTAUDIO REQUIRED)

# link against these shared libraries
set(libs ${PORTAUDIO_LIBRARIES})
set(include_dirs ${PORTAUDIO_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/src ${PROJECT_BINARY_DIR})

#message("${include_dirs}")

add_executable(${PROJECT_NAME}
                main.cpp
                src/DesktopAudioEngine.cpp
                src/DesktopAudioEngine.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${include_dirs} )
target_link_libraries(${PROJECT_NAME} ${libs} chipxengine)
# CAN ALSO USE THIS --> add_subdirectory(../engine build), but this is more convenient
#target_link_libraries(${PROJECT_NAME} PRIVATE chipxengine)

message("${INCLUDE_DIRECTORIES} 2221")

