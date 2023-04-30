#include "read_input_functions.h"

std::string_view ReadLine() {
    std::string s;
    getline(std::cin, s);
    std::string_view str(s);
    return str;
}

int ReadLineWithNumber() {
    int result;
    std::cin >> result;
    ReadLine();
    return result;
}
