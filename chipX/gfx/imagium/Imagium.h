#pragma once

#include <options.h>
#include <utils.h>
#include <converter.h>
#include <png_true_color_converter.h>
#include <map>

namespace imagium {
    class Imagium {
    private:
        using generator= std::function<converter * (void)>;
        std::map<std::string, generator> repo;
    public:
        Imagium() : repo{} {
            repo["png_true_color"]= []() { return new png_true_color_converter(); };
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
            const auto tag= options.toString();
            const converter * worker= instantiateWorkerByTag<>(converter_tag);
            return worker->write(data, options);
        }
    };

}
