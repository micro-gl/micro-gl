cmake_minimum_required(VERSION 3.12)
project(fontium)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-fno-rtti -fstrict-aliasing -O3 -s -flto -Wstrict-aliasing -fno-math-errno -ffast-math -fno-threadsafe-statics -Wno-unused-parameter")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

add_subdirectory(libs/freetype-2.10.0)
include_directories("${PROJECT_SOURCE_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src/")
include_directories("${PROJECT_SOURCE_DIR}/libs/lodepng")
include_directories("/libs/freetype-2.10.0/include")

set(SOURCES_LIB
        src/Fontium.cpp
        src/utils.cpp
        src/FontRenderer.cpp
        src/AbstractLayout.cpp
        src/exporters/BMFExporter.cpp
        src/layouters/boxlayouter.cpp
        src/layouters/boxlayouteroptimized.cpp
        src/layouters/gridlayouter.cpp
        src/layouters/gridlinelayouter.cpp
        src/layouters/linelayouter.cpp
        libs/lodepng/lodepng.cpp
        )

set(SOURCES_CLI ${SOURCES_LIB}
        cli/utils.cpp
        cli/fontium-cli.cpp)

set(TARGET_LIB  ${PROJECT_NAME})
set(TARGET_CLI ${PROJECT_NAME}-cli)
message(${TARGET_CLI})
link_libraries(freetype)
add_executable(${TARGET_CLI} ${SOURCES_CLI})
add_library(${TARGET_LIB} ${SOURCES_LIB})
target_include_directories(${TARGET_LIB}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include)
target_include_directories(${TARGET_CLI}
        PRIVATE
        ${PROJECT_SOURCE_DIR}/include)

add_custom_command(TARGET ${TARGET_CLI} POST_BUILD COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${TARGET_CLI}>/assets)