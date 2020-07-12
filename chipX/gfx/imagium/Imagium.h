#pragma once

#include <options.h>
#include <ImageWriterWorker.h>
#include <png_true_color_worker.h>
#include <map>

namespace imagium {
    class Imagium {
    private:
        using generator= std::function<ImageWriterWorker * (void)>;
        std::map<std::string, generator> repo;
    public:
        Imagium() : repo{} {
            repo["png_true_color"]= []() { return new png_true_color_worker(); };
        };

        template <typename ...ARGS>
        ImageWriterWorker * instantiateWorkerByTag(const std::string & tag, ARGS... args) {
            const auto pair=repo.find(tag);
            if(pair==repo.end())
                return nullptr;
            else
                return pair->second(args...);
        }

        byte_array produce(byte_array * data, const options & options) {
            const auto tag= options.toString();
            const ImageWriterWorker * worker= instantiateWorkerByTag<>(tag);
            return worker->write(data, options);
        }
    };

}
