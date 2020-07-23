#pragma once

#include <string>
#include <algorithm>
#include <vector>
#include <map>
#include <iostream>

using str=std::string;
using tokens=std::vector<str>;
using dictionary=std::map<str, str>;

class token_parser {
private:

public:
    token_parser()= delete;

    static
    dictionary parse (const int &argc, char **argv){
        dictionary dic{};
        int index=1;
        str waiting_key="VOID_KEY";
        str aggregate_values;

        while(index<argc) {
            const str token=str{argv[index]};
            const bool is_key= isKey(token);
            if(is_key) {
                //std::cout << waiting_key << " = " << aggregate_values << std::endl;
                dic[waiting_key]=aggregate_values;
                waiting_key=token.substr(1);
                aggregate_values="";
            } else { // is a value
                str space=aggregate_values.empty() ? "" : " ";
                aggregate_values += space + token;
            }
            index++;
        }
        dic[waiting_key]=aggregate_values;
        // std::cout << waiting_key << " = " << aggregate_values << std::endl;
        return dic;
    }

    static
    bool isKey(const str & val) {
        if(val.empty()) return false;
        return val[0]=='-';
    }

};