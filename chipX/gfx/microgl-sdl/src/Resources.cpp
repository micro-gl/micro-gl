#include "Resources.h"

Resources::Resources() {
    _asset_folder = "assets/";
//    stbi_set_flip_vertically_on_load(true);
    stbi_set_flip_vertically_on_load(false);
};

Resources::~Resources() = default;;

bool Resources::init() {
    return true;
}

Resources::image_info_t Resources::loadImageFromCompressedPath(const std::string &path, const std::string &name) {

    auto * byte_array = loadFileAsByteArray(path);
    const image_info_t & result = loadImageFromCompressedMemory(byte_array->data(),
                                                                static_cast<unsigned int>(byte_array->size()),
                                                                name);
    delete byte_array;
    return result;
}

Resources::image_info_t Resources::loadImageFromCompressedMemory(unsigned char *byte_array,
                                                                 unsigned int length_bytes,
                                                                 const std::string & name) {
    int width, height, nrChannels;
    unsigned char * data = stbi_load_from_memory(byte_array, length_bytes, &width, &height, &nrChannels, 0);
    image_info_t info { "", name, width, height, nrChannels, data };
    return info;
}

std::vector<unsigned char> * Resources::loadFileAsByteArray(const std::string &file_name) {
    std::ifstream ifs(getAssetFolder() + file_name, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    auto isGood = ifs.good();
    if(!isGood) return nullptr;
//        throw std::runtime_error("error loading file - " + file_name);
    auto length = static_cast<size_t>(ifs.tellg());
    auto *ret = new std::vector<unsigned char>(length);
    ifs.seekg(0, std::ios::beg);
    ifs.read(reinterpret_cast<char *>(ret->data()), length);
    ifs.close();
    return ret;
}

void Resources::loadXML(const std::string &file_name, rapidxml::xml_document<> & doc) {
    doc.clear();
    auto * ba= loadFileAsByteArray(file_name);
    ba->push_back('\0');
    doc.parse<0>(reinterpret_cast<char *>(ba->data()));
}

std::string Resources::loadTextFile(const std::string &file_name) {
    auto * ba = loadFileAsByteArray(file_name);
    std::string str(ba->begin(), ba->end());
    ba->clear();
    delete ba;
    return str;
}

std::string & Resources::getAssetFolder() {
    return _asset_folder;
}



