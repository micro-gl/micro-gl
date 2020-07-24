## Fontium lib & Fontium-CLI
fontium is a c++ library, that creates bitmap fonts with custom layout algorithms  
and custom export formats so you can embed/use them in your application. Under  
the hood, it uses `freetype 2`  
  
### supported fonts formats
`TrueType`, `CFF`, `WOFF`, `OpenType`, `SFNT`, `PCF`, `BDF`, `FNT`, `PFR`  
- note, for `BDF`, make sure that file has a new line at the end.

### supported export formats
bitmap font image is `png` format and the following data formats will be added  
- BMF (Angel-Code's format, which is used in many frameworks)
- C header file with code
- many others I found from fontbuilder and I add when I have time

#### fontium lib
you can use the lib as follows
```c++
#include <fontium/Fontium.h>

using bytearray = std::vector<unsigned char>;
FontConfig fontConfig = FontConfig::getDefault();
LayoutConfig layoutConfig = LayoutConfig::getDefault();
// here you can load font data into the bytearray from disk or memory
bytearray font{};
// change some config
fontConfig.size= 16;
fontConfig.antialiasing= Antialiasing::Normal;
fontConfig.characters= "abcd,ABCD";
layoutConfig.layout_type=LayoutType::box;
// create the bitmap font, which contains a single channel frame buffer
// and layout data and font data of each glyph
bitmap_font bm_font = Fontium::create(
        "bitmap font name",
        font,
        fontConfig,
        layoutConfig);
```

building can be done with `cmake`
build `fontium-cli` with cmake
```text
cd fontium
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target fontium
// libfontium file should be in your folder

```

and linked in your project with the following
```cmake
cmake_minimum_required(VERSION 3.14)
project(your_project)
...
...
add_subdirectory(fontium)
target_link_libraries(${PROJECT_NAME} fontium)

```

#### fontium cli
run `fontium-cli -h` for help and instructions.  
build `fontium-cli` with cmake
```text
cd fontium
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target fontium-cli
cd ../bin
./fontium-cli -h

```

<img src="assets/glyph.png">