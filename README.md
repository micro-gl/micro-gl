## micro{gl}
<hr/>

fast, super slim (mostly headers files only) `c++11` graphics library, that  
can run on any 32 bits computer without FPU or GPU.

check out our website at [micro-gl.github.io](micro-gl.github.io)

## running examples
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