#pragma once

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

namespace bitstream {

class BitStreamWriter {
  public:
    explicit BitStreamWriter(std::ostream& output) : cache_{0}, current_{0}, output_{output} {}

    void write(std::vector<bool> const& bits)
    {
        for (bool bit : bits)
            write(bit);
    }

    void write(bool bit)
    {
        if (bit)
            cache_ |= (1 << ((sizeof(cache_) * 8) - current_));

        current_++;

        if (current_ == sizeof(cache_) * 8)
            flush();
    }

    void flush()
    {
        output_.write((char const*) &cache_, sizeof(cache_));
        current_ = 0;
        cache_ = 0;
    }

  private:
    uint64_t cache_;
    size_t current_;
    std::ostream& output_;
};

class BitStreamReader {
  public:
    explicit BitStreamReader(std::istream& input) : cache_{}, input_{input} {}

    void read(std::vector<bool> bits, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
            bits.push_back(read());
    }

    bool read()
    {
        if (current_ != sizeof(cache_) * 8)
            return (cache_ & (1 << current_)) != 0;

        current_ = 0;
        input_.read((char*) &cache_, sizeof(cache_));
    }

  private:
    // Not using vector<bool> here, as I cannot bulk-read into a vector<bool> conviniently.
    uint64_t cache_;
    size_t current_;
    std::istream& input_;
};

}  // namespace bitstream
