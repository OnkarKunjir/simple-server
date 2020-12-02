#include "../include/utils.hpp"
#include <string>
#include <vector>
#include <iostream>

std::vector<std::string> split(std::string &src, std::string delimiter){
    // function to split string according to delimiter.
    std::vector<std::string>tokens;
    std::string token;
    int start = 0, end = src.find(delimiter);

    while (end != std::string::npos){
        token = src.substr(start, end - start);
        if(token.length() > 0)
            tokens.push_back(token);

        start = end + delimiter.length();
        end = src.find(delimiter, start);
    }

    token = src.substr(start, end);
    if(token.length() > 0)
        tokens.push_back(token);

    return tokens;
}
