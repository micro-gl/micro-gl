#include "utils.h"

std::u32string utf_8_to_32(const std::string &val) {
    std::wstring_convert<codecvt_utf8<char32_t>, char32_t> utf32conv;
//    auto utf32 = utf32conv.from_bytes("The quick brown fox jumped over the lazy dog.");
    auto utf32 = utf32conv.from_bytes(val.c_str());
    return utf32;
}
