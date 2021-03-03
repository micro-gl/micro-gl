cmake_minimum_required(VERSION 3.14)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

message("===============\n${PROJECT_NAME} project \n===============")

set(sources
#        src/PixelCoder.cpp
        src/triangles.cpp
        src/qsort.cpp)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_library(${PROJECT_NAME} STATIC ${sources} include/public/microgl/rgba_t.h include/public/microgl/pixel_coder.h include/public/microgl/convert_channel.h)
add_library(${PROJECT_NAME}-shared SHARED ${sources})

# expose public include directory
target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/public
        )

target_include_directories(${PROJECT_NAME}-shared
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include/public
        )

# Don't make the install target depend on the all target.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
install(TARGETS microgl microgl-shared LIBRARY DESTINATION "lib"
                                       ARCHIVE DESTINATION "lib"
                                       RUNTIME DESTINATION "bin")
install(DIRECTORY include/public/microgl DESTINATION include)

# add the targets examples project
add_subdirectory(examples)
