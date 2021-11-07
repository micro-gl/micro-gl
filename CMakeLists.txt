cmake_minimum_required(VERSION 3.21.1)
project(microgl)

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)

message("===============\n${PROJECT_NAME} project \n===============")

#add_subdirectory(include/public/microgl/micro-alloc)
#add_subdirectory(include/public/microgl/micro-tess)


add_library(${PROJECT_NAME} INTERFACE)

# expose public include directory
target_include_directories(${PROJECT_NAME}
        INTERFACE
        ${PROJECT_SOURCE_DIR}/include/public
        )
#target_link_libraries(${PROJECT_NAME} INTERFACE micro-alloc micro-tess)

# Don't make the install target depend on the all target.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)
install(TARGETS microgl LIBRARY DESTINATION "lib"
        ARCHIVE DESTINATION "lib"
        RUNTIME DESTINATION "bin")
install(DIRECTORY include/public/microgl DESTINATION include/microgl)

# add the targets examples project
add_subdirectory(examples)
