#pragma once

#include <types.h>

namespace imagium {
    channel convert_channel(channel val, bits bits_from, bits bits_to);
    str infer_type_needed_for_bits(bits val);
    bits infer_bits_used_from_unsigned_byte_value(ubyte val);
    ubyte infer_power_of_2_bytes_needed_from_bits(bits bits_val);
    ubyte infer_power_of_2_bytes_needed_for_rgba(bits r, bits g, bits b, bits a=0);
    ubyte infer_power_of_2_bits_needed_from_bits(bits bits_val);
    ubyte infer_power_of_2_bits_needed_for_rgba(bits r, bits g, bits b, bits a=0);

}