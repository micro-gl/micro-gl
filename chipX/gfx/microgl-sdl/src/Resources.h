/**
 * Author: Tomer Shalev
 *
 */
#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <microgl/text/bitmap_font.h>
#include <libs/stb_image/stb_image.h>
#include <libs/rapidxml/rapidxml.hpp>

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
    const std::string & getAssetFolder() const;

    /**
     * load an image from path
     *
     * @param path relative to assets folder
     * @param name name of the image
     *
     * @return  image_info_t instance
     */
    image_info_t loadImageFromCompressedPath(const std::string &path, const std::string &name = "") const;

    /**
     * load an image from raw byte array of memory (decode PNG)
     *
     * @param byte_array byte array of the image
     * @param path length_bytes size
     * @param name name of the image
     *
     * @return  image_info_t instance
     */
    image_info_t loadImageFromCompressedMemory(unsigned char *byte_array, unsigned int length_bytes, const std::string & name) const;

    /**
     * load a file from disk as a byte array
     *
     * @param file_name the file path
     *
     * @return a vector as a byte array
     */
    std::vector<unsigned char> * loadFileAsByteArray(const std::string &file_name) const;



    /**
     * load text file
     *
     * @param file_name the file path relative to assets folder
     * @param save save the contents in resources memory ?
     *
     * @return true/false on failure
     */
    std::string loadTextFile(const std::string &file_name);

    void loadXML(const std::string &file_name, rapidxml::xml_document<> & doc);

    template<typename BITMAP>
    microgl::text::bitmap_font<BITMAP> loadFont(const std::string &name) {
        microgl::text::bitmap_font<BITMAP> font;
        stbi_set_flip_vertically_on_load(false);
        rapidxml::xml_document<> d;
        loadXML("fonts/"+name+"/font.fnt", d);
        auto * f= d.first_node("font");
        auto * f_info= f->first_node("info");
        auto * f_common= f->first_node("common");
        auto * f_chars= f->first_node("chars");
        strncpy(font.name, f_info->first_attribute("face")->value(), 10);
        font.nativeSize=atoi(f_info->first_attribute("size")->value());
        font.lineHeight=atoi(f_common->first_attribute("lineHeight")->value());
        font.baseline=atoi(f_common->first_attribute("base")->value());
        font.width=atoi(f_common->first_attribute("scaleW")->value());
        font.height=atoi(f_common->first_attribute("scaleH")->value());
        font.glyphs_count=atoi(f_chars->first_attribute("count")->value());
        auto * iter= f_chars->first_node("char");
        do {
            int id=atoi(iter->first_attribute("id")->value());
            int x=atoi(iter->first_attribute("x")->value());
            int y=atoi(iter->first_attribute("y")->value());
            int w=atoi(iter->first_attribute("width")->value());
            int h=atoi(iter->first_attribute("height")->value());
            int xoffset=atoi(iter->first_attribute("xoffset")->value());
            int yoffset=atoi(iter->first_attribute("yoffset")->value());
            int xadvance=atoi(iter->first_attribute("xadvance")->value());
            font.addChar(id, x, y, w, h, xoffset, yoffset, xadvance);
            iter = iter->next_sibling();
        } while (iter);
        // load bitmap
        auto img_font = loadImageFromCompressedPath("fonts/"+name+"/font.png");
        auto *bmp_font = new BITMAP(img_font.data, img_font.width, img_font.height);
        font._bitmap=bmp_font;
        stbi_set_flip_vertically_on_load(true);
        return font;
    }
protected:

private:
    std::string _asset_folder;

};
