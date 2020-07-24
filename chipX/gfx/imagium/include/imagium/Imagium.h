#pragma once

#include "Config.h"
#include "../src/converter.h"
#include "../src/regular_converter.h"
#include "../src/png_palette_converter.h"

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

        converter::result produce(const str & converter_tag, byte_array * data, const Config & config) {
            const converter * worker= instantiateWorkerByTag<>(converter_tag);
            if(worker==nullptr)
                throw std::runtime_error("could not find a matching converter for this data !!! ");
            return worker->write(data, config);
        }

        converter::result produce(byte_array * data, const Config & config) {
            str tag=config.converter;
            if(tag.empty()) {
                tag="regular_converter";
                if(config.use_palette)
                    tag=config.image_format+"_palette_converter";
            }
            return produce(tag, data, config);
        }
    };

}
