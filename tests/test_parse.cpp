#include <string>
#include <iostream>
#include "json.h"


int main(){
    std::string str = "{\"check\": 123.5e10, \"2893h\":\"ok\", \"arr\": [\"sd\", null]}";
    haha::json::Json json;
    bool ok = true;
    ok = json.fromString(str);
    std::cout << ok << std::endl;

    std::string filePath = "./test.json";
    ok = json.fromFile(filePath);
    std::cout << ok << std::endl;

    return 0;
}