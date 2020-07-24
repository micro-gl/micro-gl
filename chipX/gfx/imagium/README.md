# Imagium lib & Imagium-CLI
project contains two components  
- `imagium` is a c++ library, that processes images andre-shapes and packs/unpacks  
new pixels inside power of 2 data and returns `.c header files` strings out of them.  
- `imagium-cli` is a command line interface, that uses `imagium` to write `.c header files`    
export formats so you can embed/use them in your application.
  
### supported image formats
- `PNG`, `JPEG`, `BMP`, `TGA`, `GIF`, `PSD`, `PIC`, `PNM` are supported.
- for palette export only `PNG` is supported with embedded palette, that you  
have created in an image editing software.

### notes
- channels, that are reshaped into 0 bits are discarded in output to save storage.
- palette, is only supported for `png` format

#### imagium lib
you can use the lib as follows
```c++
imagium::Imagium lib{};
Config config = Config::getDefault();
using bytearray = std::vector<unsigned char>;

// change the config
config.r=8; config.g=2; config.b=1; config.a=0;

// here you can load font data into the bytearray from disk or memory
bytearray image{};

// compute the c-array string
imagium::converter::result res = lib.produce(&image, config);
res.data; // is the bytearray result, currently represents a string
res.size_bytes; // represent what size in bytes the data will occupy in memory once loaded

```

building can be done with `cmake`
```text
cd imagium
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target imagium
// libfontium file should be in your folder

```

you can also add `imagium` as a sub-directory and link it in your
cmake project
```cmake
cmake_minimum_required(VERSION 3.14)
project(your_project)
...
...
add_subdirectory(imagium)
target_link_libraries(${PROJECT_NAME} imagium)

```

#### imagium cli
run `imagium-cli -h` for help and instructions.  
build `imagium-cli` with cmake
```cmake
cd imagium
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target imagium-cli
cd ../bin
./imagium-cli -h

```
