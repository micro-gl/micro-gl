# micro{tess}

fast, super slim, embeddable, allocator-aware, headers files only **`C++11`** geometry tesselation library for vector graphics, that can run on any 32/64 bits computer with or without **FPU**. **no standard library required**.

check out our website at [micro-gl.github.io/docs/micro-tess/](micro-gl.github.io/)

# Usage
Because `micro-tess` is a headers only library, installing just means copying the `include/`
folder to your system search path or another folder and then set the include path in your build.
the `cmake` file also has an install target, that will copy everything in your system via ```cmake install```.

## Using *CMake*
Download the project to a sub folder of your project. inside your **`CMakeLists.txt`** add
```cmake
add_subdirectory(/path/to/micro-tess)
target_link_libraries(your_app micro-tess)
```
If you installed **`micro{tess}`** (see above) at your system, you can instead
```cmake
target_include_directories(app path/to/micro-tess/install/include/)
```

# Running Examples and testing
lib is tested when embedded with one of the `micro-gl` libraries for rendering

```text
Author: Tomer Shalev, tomer.shalev@gmail.com
```