cmake_minimum_required(VERSION 3.12)
project(micro-alloc-examples)
message(\n===========\n${PROJECT_NAME} \n===========\n)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS_DEBUG  ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS_RELEASE " -fno-rtti -fno-exceptions -fno-unwind-tables \
                            -fstrict-aliasing -O3  -flto \
                            -Wstrict-aliasing -fno-math-errno \
                            -ffast-math -fno-threadsafe-statics \
                            -fdata-sections -ffunction-sections \
                            " CACHE INTERNAL "" FORCE)
#-nostdlib
message("using compiler: ${CMAKE_CXX_COMPILER_ID}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin")
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

set(libs micro-alloc)

message(${CMAKE_BINARY_DIR})
message(${CMAKE_MODULE_PATH})
message(${PROJECT_SOURCE_DIR})
message(${PROJECT_BINARY_DIR})

set(SOURCES
        test_allocators.cpp
        )

set(SOURCES_SHARED "")

foreach( testsourcefile ${SOURCES} )
    # I used a simple string replace, to cut off .cpp.
    string( REPLACE ".cpp" "" testname ${testsourcefile} )
    include_directories("${PROJECT_BINARY_DIR}")

    add_executable( ${testname} ${testsourcefile} ${SOURCES_SHARED} )
    # Make sure YourLib is linked to each app
    target_link_libraries( ${testname} ${libs} )
endforeach( testsourcefile ${SOURCES} )

