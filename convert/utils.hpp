#pragma once

#include <algorithm>

namespace ranges {

template <typename Container, typename F>
auto min_element(Container& container, F f)
{
    return std::min_element(begin(container), end(container), f);
}

}  // namespace ranges
