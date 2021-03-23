cmake_minimum_required(VERSION 3.14)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

message("===============\n${PROJECT_NAME} project \n===============")

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

add_library(${PROJECT_NAME} INTERFACE include/public/microgl/pixel_coders/RGBA_PACKED.h include/public/microgl/stdint.h)

# expose public include directory
target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${PROJECT_SOURCE_DIR}/include/public
        )

# Don't make the install target depend on the all target.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
install(TARGETS microgl LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin")
install(DIRECTORY include/public/microgl DESTINATION include/microgl)

# add the targets examples project
add_subdirectory(examples)
