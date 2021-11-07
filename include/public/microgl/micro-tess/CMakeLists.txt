cmake_minimum_required(VERSION 3.21.1)
project(micro-tess)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

message("===============\n${PROJECT_NAME} project \n===============")

add_library(${PROJECT_NAME} INTERFACE)

# expose public include directory
target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${PROJECT_SOURCE_DIR}/include
        )

# Don't make the install target depend on the all target.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
install(TARGETS micro-tess LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin")
install(DIRECTORY include/micro-tess DESTINATION include/micro-tess)
