cmake_minimum_required(VERSION 3.21.1)
project("micro-tess"
        VERSION
        1.0.0
        DESCRIPTION
        "fast, super slim, embeddable, allocator-aware, headers files only **`C++11`** geometry tesselation library for vector graphics"
        HOMEPAGE_URL
        "https://micro-gl.github.io/docs")

enable_language(CXX)
set (CMAKE_CXX_STANDARD 11)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
message("===============\n${PROJECT_NAME} v${PROJECT_VERSION} project \n===============")

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

# expose public include directory
#target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_SOURCE_DIR}/include)
# define target includes for both direct build via add_subdirectory() and for
# defined installation configuration to allow easier package find
target_include_directories(
        ${PROJECT_NAME} INTERFACE
        $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

#target_link_libraries(${PROJECT_NAME} INTERFACE micro-alloc micro-tess)

# Don't make the install target depend on the all target.
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

# add the targets examples project
add_subdirectory(examples)

### install
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}_Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION
        ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
# install cmake config files for targets of this lib, so they can be found more easily
# without a custom find script, but with find_package(micro-tess CONFIG) command
install(EXPORT ${PROJECT_NAME}_Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)

install(FILES
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME}/cmake)
# install include files
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/micro-tess DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
