#pragma once

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <vector>

namespace bitstream {

using BitVector = std::vector<bool>;

class BitStreamWriter {
  public:
    explicit BitStreamWriter(std::ostream& output) : cache_{}, output_{output} {}

    void write(std::vector<bool> const& bits) { cache_.insert(cache_.end(), bits.begin(), bits.end()); }

    void flush(bool bytePadding = false)
    {
        size_t const numFullBytes = cache_.size() / 8;
        size_t const numRemainingBits = cache_.size() % 8;

        size_t i = 0;
        for (; i + 7 < numFullBytes; i += 8)
        {
            auto const b = static_cast<std::byte>(
                (cache_[i + 0] << 7) | (cache_[i + 1] << 6) | (cache_[i + 2] << 5) | (cache_[i + 3] << 4)
                | (cache_[i + 4] << 3) | (cache_[i + 5] << 2) | (cache_[i + 6] << 1) | (cache_[i + 7] << 0));
            output_.write((char const*) &b, sizeof(b));
        }

        if (!numRemainingBits)
            cache_.clear();
        else if (bytePadding)
        {
            // NB: Left-most bit always goes in & right-most bit is always zero.
            auto const rem = [&]() {
                uint8_t rem = cache_[i] << 7;
                for (size_t k = 6; k >= 1; --k)  // for each bit from 6 to 1
                    if (++i < cache_.size())
                        rem |= cache_[i] << k;
                return rem;
            }();

            output_.write((char const*) &rem, sizeof(rem));
            cache_.clear();
        }
        else
        {
            rotate(begin(cache_), next(begin(cache_), cache_.size() - numRemainingBits), end(cache_));
            cache_.resize(numRemainingBits);
        }
    }

  private:
    std::vector<bool> cache_;
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
