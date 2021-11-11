#========================================================================================
# Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
# All Rights Reserved.
#
# this should help find the microgl headers-only package and define the microgl target that was
# installed on your system and does not include CMakeLists.txt file, so you can easily link to it.
# If successful, the following will happen:
# 1. microgl_INCLUDE_DIR will be defined
# 2. microgl::microgl target will be defined so you can link to it using target_link_libraries(..)
#========================================================================================
include(GNUInstallDirs)
include(FindPackageHandleStandardArgs)

find_path(microgl_INCLUDE_DIR
        NAMES microgl
        HINTS ${CMAKE_INSTALL_INCLUDEDIR}
        PATH_SUFFIXES clippers bitmaps samplers)
set(microgl_LIBRARY "/dont/use")
find_package_handle_standard_args(microgl DEFAULT_MSG
        microgl_LIBRARY microgl_INCLUDE_DIR)

if(microgl_FOUND)
    message("MICROGL was found !!!")
else(microgl_FOUND)
    message("MICROGL was NOT found !!!")
endif(microgl_FOUND)

if(microgl_FOUND AND NOT TARGET microgl::microgl)
    # build the target
    add_library(microgl::microgl INTERFACE IMPORTED)
    set_target_properties(
            microgl::microgl
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${microgl_INCLUDE_DIR}")
endif()