#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <vector>

namespace bitstream {

class BitStreamWriter {
  public:
    using Writer = std::function<void(std::byte const*, std::size_t)>;
    explicit BitStreamWriter(Writer writer) : cache_{0}, current_{0}, writer_{move(writer)} {}
    explicit BitStreamWriter(std::ostream& output) :
		BitStreamWriter{
			[out = std::ref(output)](std::byte const* data, size_t count) mutable
			{
              out.get().write((char const*) data, count);
			}
		}
    {
    }

    void write(std::vector<bool> const& bits)
    {
        for (bool bit : bits)
            write(bit);
    }

    void write(bool bit)
    {
        if (bit)
            cache_ |= (1llu << ((sizeof(cache_) * 8) - current_));

        current_++;

        if (current_ == sizeof(cache_) * 8)
            flush();
    }

    void flush()
    {
        writer_((std::byte const *) &cache_, sizeof(cache_));
        current_ = 0;
        cache_ = 0;
    }

  private:
    uint64_t cache_;
    size_t current_;
    Writer writer_;
};

class BitStreamReader {
  public:
    using Reader = std::function<void(std::byte*, std::size_t)>;

    explicit BitStreamReader(Reader reader) : cache_{}, current_{0}, reader_{move(reader)} {}

    explicit BitStreamReader(std::istream& input)
        : BitStreamReader{[in = std::ref(input)](std::byte* data, std::size_t count) mutable {
              in.get().read((char*) data, count);
          }}
    {
    }

    void read(std::vector<bool> bits, size_t count)
    {
        for (size_t i = 0; i < count; ++i)
            bits.push_back(read());
    }

    bool read()
    {
        if (current_ != sizeof(cache_) * 8)
            return (cache_ & (1llu << current_)) != 0;

        current_ = 0;
        reader_((std::byte*) &cache_, sizeof(cache_));
    }

  private:
    // Not using vector<bool> here, as I cannot bulk-read into a vector<bool> conviniently.
    uint64_t cache_;
    size_t current_;
    Reader reader_;
};

}  // namespace bitstream
