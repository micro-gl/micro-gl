#pragma once

#include <ImageWriterWorker.h>
#include <lodepng.h>

namespace imagium {

    class png_true_color_worker : public ImageWriterWorker {
    public:
        png_true_color_worker()= default;

        std::vector<ubyte> write(byte_array * data, const options & options) const override {
            std::vector<unsigned char> image, image2; //the raw pixels
            unsigned width, height;
            lodepng::State state; //optionally customize this one
//            state.info_raw.colortype=LodePNGColorType::LCT_PALETTE;
            unsigned error = lodepng::decode(image, width, height, state, *data);
//            state.info_png.color.colortype=LodePNGColorType::LCT_PALETTE; 
//            state.encoder.force_palette=1;
            error = lodepng::encode(image2, image, width, height, state);

            //if there's an error, display it
            if(error) std::cout << "decoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
        };
    };
}
