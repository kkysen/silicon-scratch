//
// Created by Khyber on 1/17/2019.
//

#ifndef SiliconScratch_strings_H
#define SiliconScratch_strings_H

#include <memory>

template <
        typename T,
        template <typename> typename Traits = std::char_traits,
        template <typename> typename Allocator = std::allocator
>
std::basic_string<T, Traits<T>, Allocator<T>> operator+(
        const std::basic_string<T, Traits<T>, Allocator<T>>& lhs,
        std::basic_string_view<T, Traits<T>> rhs
) {
    std::string result;
    result += lhs;
    result += rhs;
    return result;
}

#endif // SiliconScratch_strings_H
