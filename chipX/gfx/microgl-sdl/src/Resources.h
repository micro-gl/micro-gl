/**
 * Author: Tomer Shalev
 *
 */
#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include <libs/stb_image/stb_image.h>

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

    Resources();

    virtual ~Resources();

    /**
     * init the damn thing
     *
     * @return true/false on failure
     */
    virtual bool init();

    /**
     * asset folder path
     *
     */
    std::string & getAssetFolder();

    /**
     * load an image from path
     *
     * @param path relative to assets folder
     * @param name name of the image
     *
     * @return  image_info_t instance
     */
    image_info_t loadImageFromCompressedPath(const std::string &path, const std::string &name = "");

    /**
     * load an image from raw byte array of memory (decode PNG)
     *
     * @param byte_array byte array of the image
     * @param path length_bytes size
     * @param name name of the image
     *
     * @return  image_info_t instance
     */
    image_info_t loadImageFromCompressedMemory(unsigned char *byte_array, unsigned int length_bytes, const std::string & name);

    /**
     * load a file from disk as a byte array
     *
     * @param file_name the file path
     *
     * @return a vector as a byte array
     */
    std::vector<unsigned char> * loadFileAsByteArray(const std::string &file_name);

    /**
     * load text file
     *
     * @param file_name the file path relative to assets folder
     * @param save save the contents in resources memory ?
     *
     * @return true/false on failure
     */
    std::string loadTextFile(const std::string &file_name);

protected:

private:
    std::string _asset_folder;
};
