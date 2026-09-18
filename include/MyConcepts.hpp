#ifndef MYCONCEPTS_HPP
#define MYCONCEPTS_HPP

#include "types.hpp"
#include <concepts>
#include <type_traits>

template<class T>
concept SizeType = std::is_same_v<usize, T>;

template<SizeType T>
consteval bool is_power_of_two(T n) {
    return (n & (n - 1)) == 0;
}

#endif
