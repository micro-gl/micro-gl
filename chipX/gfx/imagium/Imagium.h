#pragma once

#include <options.h>
#include <utils.h>
#include <converter.h>
#include <regular_converter.h>
#include <png_palette_converter.h>
#include <map>

namespace imagium {
    class Imagium {
    private:
        using generator= std::function<converter * (void)>;
        std::map<std::string, generator> repo;
    public:
        Imagium() : repo{} {
            repo["regular_converter"]= []() { return new regular_converter(); };
            repo["png_palette_converter"]= []() { return new png_palette_converter(); };
        };

        template <typename ...ARGS>
        converter * instantiateWorkerByTag(const std::string & tag, ARGS... args) {
            const auto pair=repo.find(tag);
            if(pair==repo.end())
                return nullptr;
            else
                return pair->second(args...);
        }

        byte_array produce(const str & converter_tag, byte_array * data, const options & options) {
            const converter * worker= instantiateWorkerByTag<>(converter_tag);
            return worker->write(data, options);
        }

        byte_array produce(byte_array * data, const options & options) {
            str tag=options.converter;
            if(tag.empty()) {
                tag="regular_converter";
                if(options.palette>0)
                    tag="palette_converter";
            }
            return produce(tag, data, options);
        }
    };

}
