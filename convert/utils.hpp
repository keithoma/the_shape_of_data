#pragma once

#include <algorithm>
#include <iterator>

namespace ranges {

template <typename Container, typename F>
auto min_element(Container& container, F f)
{
    return std::min_element(begin(container), end(container), f);
}

template <typename Container, typename OutputIterator>
void copy(Container& input, OutputIterator output)
{
    std::copy(begin(input), end(input), output);
}

template <typename Container, typename F>
void for_each(Container& container, F f)
{
    std::for_each(begin(container), end(container), f)
}

template <typename Container, typename F>
void for_each(Container const& container, F f)
{
    std::for_each(cbegin(container), cend(container), f);
}

}  // namespace ranges
