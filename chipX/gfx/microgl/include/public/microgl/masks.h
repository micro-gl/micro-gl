#pragma once

namespace microgl {
    namespace masks {
        enum class chrome_mode {
            // B&W image - black shades are converted to transparency
            black_transparent,
            // B&W image - white shades are converted to transparency
            white_transparent,
            // regular image - transparency represents transparency as is
            regular
        };
    }
}