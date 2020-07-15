#pragma once

#include <utils.h>
#include <utility>
#include <iostream>

namespace imagium {
    std::vector<unsigned char> * loadFileAsByteArray(const std::string &file_name) {
        std::ifstream ifs(file_name, std::ios::binary);
        ifs.seekg(0, std::ios::end);
        auto isGood = ifs.good();
        if(!isGood)
            throw std::runtime_error("error loading file - " + file_name);
        auto length = static_cast<size_t>(ifs.tellg());
        auto *ret = new std::vector<unsigned char>(length);
        ifs.seekg(0, std::ios::beg);
        ifs.read(reinterpret_cast<char *>(ret->data()), length);
        ifs.close();
        return ret;
    }

    std::vector<std::string> split(std::string s, std::string delimiter) {
        std::vector<std::string> list;
        split(std::move(s), std::move(delimiter), list);
        return list;
    }

    void split(std::string s, const std::string& delimiter, std::vector<std::string> &list) {
        size_t pos = 0;
        std::string token;
        while ((pos = s.find(delimiter)) != std::string::npos) {
            token = s.substr(0, pos);
            list.push_back(token);
            s.erase(0, pos + delimiter.length());
        }
        list.push_back(s);
    }

}