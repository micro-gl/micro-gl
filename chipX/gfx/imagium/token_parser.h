#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <map>

class token_parser {
private:
    using str=std::string;
    using tokens=std::vector<str>;
    using dictionary=std::map<str, str>;

public:
    token_parser()= delete;

    static
    dictionary parse (const int &argc, char **argv){
        tokens tokens{};
        dictionary dic{};
        int index=1;
        str waiting_key="VOID_KEY";
        str aggregate_values;
        while(index<argc) {
            const str token=str{argv[index]};
            const bool is_key= isKey(token);
            if(is_key) {
                dic[waiting_key]=aggregate_values;
                waiting_key=token.substr(1);
                aggregate_values="";
            } else { // is a value
                aggregate_values += " " + token;
            }
            index++;
        }
        return dic;
    }

    static
    bool isKey(const str & val) {
        if(val.empty()) return false;
        return val[0]=='-';
    }

};