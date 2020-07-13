#pragma once

#include <utils.h>

namespace imagium {
    std::vector<unsigned char> * loadFileAsByteArray(const std::string &file_name) {
        std::ifstream ifs(file_name, std::ios::binary);
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

}