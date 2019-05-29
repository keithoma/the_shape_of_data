#pragma once

#include <algorithm>
#include <iterator>

namespace ranges {

template <typename Container, typename F>
inline auto min_element(Container& container, F f)
{
    return std::min_element(begin(container), end(container), f);
}

template <typename Container, typename OutputIterator>
inline void copy(Container& input, OutputIterator output)
{
    std::copy(begin(input), end(input), output);
}

template <typename Container, typename F>
inline void for_each(Container& container, F f)
{
    std::for_each(begin(container), end(container), f)
}

template <typename Container, typename F>
inline void for_each(Container const& container, F f)
{
    std::for_each(cbegin(container), cend(container), f);
}

template <typename C, typename D, typename F>
inline void transform(C const& c, D& d, F f)
{
    std::transform(begin(c), end(c), d, f)
}

}  // namespace ranges
