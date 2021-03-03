# micro{gl}

fast, super slim, headers files only `C++11` graphics library, that  
can run on any 32 bits computer without FPU or GPU.

check out our website at [micro-gl.github.io](micro-gl.github.io)

## install *microGL* with *CMake*

```bash
$ cd path/to/microgl
$ mkdir cmake-build-release
$ cd cmake-build-release

# use this to install globally (usually /usr/local/ on *nix)
$ cmake -DCMAKE_BUILD_TYPE=Release ..
# use this to install at a folder you want
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/path/for/install ..

$ cmake --build . --target install
```

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