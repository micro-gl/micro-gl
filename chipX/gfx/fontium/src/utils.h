#pragma once

#include <codecvt>
#include <locale>

using namespace std;

//#if _MSC_VER >= 1900
//wstring_convert<codecvt_utf8<int32_t>, int32_t> utf32conv;
//    auto utf32 = utf32conv.from_bytes("The quick brown fox jumped over the lazy dog.");
//    // use reinterpret_cast<const char32_t *>(utf32.c_str())
//#else
//std::wstring_convert<codecvt_utf8<char32_t>, char32_t> utf32conv;
//auto utf32 = utf32conv.from_bytes("The quick brown fox jumped over the lazy dog.");
//// use utf32.c_str()
//#endif

std::u32string utf_8_to_32(const std::string & val);