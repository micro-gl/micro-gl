# Fontium lib & Fontium-CLI
project contains two components  
- `fontium` is a c++ library, that creates bitmap fonts with custom layout algorithms.  
it also has built-in exporters to other formats as strings so you can embed them.
- `fontium-cli` is a command line interface, that uses `fontium` to also save the  
bitmap fonts as custom export formats so you can embed/use them in your application.  
Under the hood, it uses `freetype 2`  

### supported fonts formats
`TrueType`, `CFF`, `WOFF`, `OpenType`, `SFNT`, `PCF`, `BDF`, `FNT`, `PFR`  
- note, for `BDF`, make sure that file has a new line at the end.

### supported layout formats
`box`, `line`, `box_optimal`, `grid`, `gridline`

### supported export formats for `fontium-cli`
this only applies for the `fontium-cli`  
the generated image is `png` format and the following data formats can be selected  
- **BMF** (Angel-Code's format, which is used in many frameworks)
- **C header file** with code
- many others I found from fontbuilder software and I add when I have time

#### fontium lib
you can use the lib as follows
```c++
#include <fontium/Fontium.h>

using namespace fontium;

// input data
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

to export your `bitmap_font` result into another format, follow
```c++
#include <fontium/Fontium.h>
#include <fontium/ExportFactory.h>

using namespace fontium;
using str = std::string;

bitmap_font bm_font= ...; // created in previous step

// export as BMF ( Angel Code's format xml)
str output_export_type = "bmf";
auto * exporter = ExportFactory::create(output_export_type);
str result = exporter->apply(bm_font);
str data_file_name= "font." + exporter->fileExtension();

// write data file
std::ofstream out(data_file_name);
out << result;
out.close();
std::cout << std::endl << "created data file :: " 
          << data_file_name << std::endl;

// free memory
delete exporter;

```

building can be done with `cmake`
```text
cd fontium
mkdir cmake_build
cd cmake_build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target fontium
// libfontium file should be in your folder

```

you can also add `fontium` as a sub-directory and link it in your
cmake project
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
