#pragma once

#include <token_parser.h>

namespace imagium {

    class bundle {
    public:
        using dictionary=std::map<str, str>;
        using Key=const str &;
    private:
        dictionary _dic;
    public:
        static bundle fromTokens(const int &argc, char **argv) {
            return bundle{token_parser::parse(argc, argv)};
        }

        explicit bundle(dictionary && dic) : _dic{std::move(dic)} { }
        explicit bundle(dictionary & dic) : _dic{(dic)} { }
        bundle(bundle &&bundle) noexcept : _dic{std::move(bundle._dic)} { }
        bundle(bundle & bundle) = default;
        bundle() = default;

        bool hasKey(const str &key) {
            return _dic.find(key)!=_dic.end();
        }

        int getValueAsInteger(Key key, int default_value=0) {
            const auto as_str=getValueAsString(key, std::to_string(default_value));
            return std::stoi(as_str);
        }

        float getValueAsFloat(Key key, float default_value=0.0f) {
            const auto as_str=getValueAsString(key, std::to_string(default_value));
            return std::stof(as_str);
        }

        float getValueAsBoolean(Key key, bool default_value=false) {
            const auto as_str=getValueAsString(key, std::to_string(int(default_value)));
            return as_str!="0";
        }

        str getValueAsString(Key key, str default_value="") {
            const auto it= _dic.find(key);
            if(it==_dic.end()) return default_value;
            return it->second;
        }

        template <typename Value>
        void putValue(Key key, const Value & value) {
            _dic[key]=stringify<Value>(value);
        }

        template <typename Value>
        void removeKey(Key key) {
            const auto it= _dic.find(key);
            _dic.erase(it);
        }

    private:
        template <typename Value>
        str stringify(const Value & value) { return std::to_string(value); }
        template <>
        str stringify<str>(const str & value) { return value; }
    };
}
