// This file is part of the "convert" project, https://github.com/keithoma>
//   (c) 2019 Kei Thoma <thomakei@gmail.com>
//   (c) 2019 Christian Parpart <christian@parpart.family>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#pragma once

#include <algorithm>
#include <iterator>

namespace util {

template <typename T>
class span {
  public:
	using value_type = T;

    span(T const* data, std::size_t size) : data_{data}, size_{size} {}

    struct iterator {
        using difference_type = long;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::forward_iterator_tag;

        T const* current;

        T const& operator*() const noexcept { return *current; }
        T const* operator->() const noexcept { return current; }

        iterator& operator++() noexcept
        {
            ++current;
            return *this;
        }

        iterator& operator++(int) noexcept { return ++*this; }

        bool operator==(iterator const& other) const noexcept { return current == other.current; }
        bool operator!=(iterator const& other) const noexcept { return !(*this == other); }
    };

    iterator begin() const noexcept { return iterator{data_}; }
    iterator end() const noexcept { return iterator{data_ + size_}; }

  private:
    T const* data_;
    size_t size_;
};

// deduction guide
template <typename T> span(T const* data, std::size_t size)->span<T>;

}  // namespace util

namespace ranges {

template <typename Container, typename F>
inline auto min_element(Container const& container, F f)
{
    return std::min_element(begin(container), end(container), f);
}

template <typename Container, typename OutputIterator>
inline void copy(Container const& input, OutputIterator output)
{
    std::copy(begin(input), end(input), output);
}

template <typename Container, typename F>
inline void for_each(Container& container, F f)
{
    std::for_each(begin(container), end(container), f);
}

template <typename Container, typename F>
inline void for_each(Container const& container, F f)
{
    std::for_each(cbegin(container), cend(container), f);
}

template <typename C, typename D, typename F>
inline void transform(C const& c, D d, F f)
{
    std::transform(begin(c), end(c), d, f);
}

}  // namespace ranges
