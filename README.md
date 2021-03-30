# micro{gl}

fast, super slim, embeddable, headers files only `C++11` graphics library, that  
can run on any 32/64 bits computer without FPU or GPU. no standard library required.

check out our website at [micro-gl.github.io](micro-gl.github.io)

# Usage
Because `microgl` is a headers only library, installing just means copying the `include/public/microgl`
folder to your system search path or another folder and then set the include path in your build.
the `cmake` file also has an install target.

## using *CMake*
download the project to a sub folder of your project. inside your **`CMakeLists.txt`** add
```cmake
add_subdirectory(/path/to/microgl)
target_link_libraries(your_app microgl)
```
if you installed **`micro{gl}`** (see above) at your system, you can instead
```cmake
target_include_directories(app path/to/microgl/install/include/)
```

# running examples
first make sure you have 
 - [SDL2](https://www.libsdl.org/) installed at your system.  
 - [cmake](https://cmake.org/download/) installed at your system.

there are two ways:
1. use your favourite IDE to load the root `CMakeLists.txt` file, and then it   
   will pick up all of the targets, including the examples
2. using the command line:
```bash
$ mkdir cmake-build-release
$ cd cmake-build-release
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --build . --target <example_name>
$ ../examples/bin/example_clear
```

```text
Author: Tomer Shalev, tomer.shalev@gmail.com
```