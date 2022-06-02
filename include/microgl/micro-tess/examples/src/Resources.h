/**
 * Author: Tomer Shalev
 *
 */
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <microgl/text/bitmap_font.h>
#include "../libs/stb_image/stb_image.h"

using std::cout;
using std::endl;

/**
 * basic resources manager, includes built in json and image support.
 *
 */
class Resources  {

public:
    /**
     * basic uncompressed image info structure
     */
    struct image_info_t {
        // path or name
        std::string path;
        std::string name;
        int width;
        int height;
        int channels;
        unsigned char * data;
    };

    Resources()=delete;

    /**
     * asset folder path
     *
     */
    static std::string getAssetFolder() {
        return "assets/";
    }

    /**
     * load an image from_sampler path
     *
     * @param path relative to assets folder
     * @param name name of the image
     *
     * @return  image_info_t instance
     */
    static
    image_info_t loadImageFromCompressedPath(const std::string &path, const std::string &name = "");

    /**
     * load an image from_sampler raw byte array of memory (decode PNG)
     *
     * @param byte_array byte array of the image
     * @param path length_bytes size
     * @param name name of the image
     *
     * @return  image_info_t instance
     */
    static image_info_t loadImageFromCompressedMemory(unsigned char *byte_array, unsigned int length_bytes, const std::string & name) ;

    /**
     * load a file from_sampler disk as a byte array
     *
     * @param file_name the file path
     *
     * @return a vector as a byte array
     */
    static std::vector<unsigned char> * loadFileAsByteArray(const std::string &file_name);



    /**
     * load text file
     *
     * @param file_name the file path relative to assets folder
     * @param save save the contents in resources memory ?
     *
     * @return true/false on failure
     */
    static
    std::string loadTextFile(const std::string &file_name);

private:
    std::string _asset_folder;
};
