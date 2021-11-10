# micro{alloc}

fast, super slim, embeddable, headers files only **`C++11`** memory allocation library.

check out our website at [micro-gl.github.io/docs/micro-alloc/](micro-gl.github.io/docs/micro-alloc)

# Usage
Because `micro-alloc` is a headers only library, installing just means copying the `include/`
folder to your system search path or another folder and then set the include path in your build.
the `cmake` file also has an install target, that will copy everything in your system via 
```bash
$ mkdir cmake-build-release
$ cd cmake-build-release
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ cmake --install .
```

## Using *CMake*
Download the project to a sub folder of your project. inside your **`CMakeLists.txt`** add
```cmake
add_subdirectory(/path/to/micro-alloc)
target_link_libraries(your_app micro-alloc)
```
If you installed **`micro{alloc}`** (see above) at your system, you can instead
```cmake
target_include_directories(app path/to/micro-alloc/install/include/)
```

# Running Examples and testing
TODO

# How to use
see `examples` folder or website
```text
Author: Tomer Shalev, tomer.shalev@gmail.com, all rights reserved (2021)
```