#========================================================================================
# Copyright (2021), Tomer Shalev (tomer.shalev@gmail.com, https://github.com/HendrixString).
# All Rights Reserved.
#
# this should help find the microgl headers-only package and define the microgl target that was
# installed on your system and does not include CMakeLists.txt file, so you can easily link to it.
# If successful, the following will happen:
# 1. micro-tess_INCLUDE_DIR will be defined
# 2. micro-tess::micro-tess target will be defined so you can link to it using target_link_libraries(..)
#========================================================================================
include(GNUInstallDirs)
include(FindPackageHandleStandardArgs)

find_path(micro-tess_INCLUDE_DIR
        NAMES micro-tess
        HINTS ${CMAKE_INSTALL_INCLUDEDIR}
        PATH_SUFFIXES clippers bitmaps samplers)
set(microgl_LIBRARY "/dont/use")
find_package_handle_standard_args(micro-tess DEFAULT_MSG
        micro-tess_LIBRARY micro-tess_INCLUDE_DIR)

if(micro-tess_FOUND)
    message("micro-tess was found !!!")
else(micro-tess_FOUND)
    message("micro-tess was NOT found !!!")
endif(micro-tess_FOUND)

if(micro-tess_FOUND AND NOT TARGET micro-tess::micro-tess)
    # build the target
    add_library(micro-tess::micro-tess INTERFACE IMPORTED)
    set_target_properties(
            micro-tess::micro-tess
            PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${micro-tess_INCLUDE_DIR}")
endif()