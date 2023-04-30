#pragma once

#include <string>
#include <iostream>
#include <vector>
#include <set>
#include <string_view>

std::vector<std::string_view> SplitIntoWords(std::string_view str);

template <typename StringContainer>
std::set<std::string, std::less<>> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    std::set<std::string, std::less<>> non_empty_strings;
    for (const  auto& str : strings) {
        if ( str.size() != 0) {
            std::string my_str(str);
            non_empty_strings.emplace(my_str);
        }
    }
    return non_empty_strings;
}
