// This file is part of the "convert" project, http://github.com/keithoma>
//   (c) 2019 Kei Thoma <thomakei@gmail.com>
//   (c) 2019 Christian Parpart <christian@parpart.family>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace bitstream {

class BitStreamWriter {
  public:
    using Writer = std::function<void(std::byte const*, std::size_t)>;

    explicit BitStreamWriter(Writer writer) : cache_{0}, current_{0}, writer_{move(writer)} {}

    explicit BitStreamWriter(std::ostream& output)
        : BitStreamWriter{[out = std::ref(output)](std::byte const* data, size_t count) mutable {
              out.get().write((char const*) data, count);
          }}
    {
    }

    ~BitStreamWriter()
    {
        if (current_ != 0)
            flush();
    }

    /// Writes POD value into thre stream.
    /// The current bit position must be aligned.
    template <typename T>
    void writeAligned(T const& value)
    {
        if (current_ != 0)
            throw logic_error{"Bitstream must be aligned in order to write non-bits."};

        writer_((std::byte const*) &value, sizeof(value));
    }

    /// Writes a vector of bits into the stream.
    void write(std::vector<bool> const& bits)
    {
        for (bool bit : bits)
            write(bit);
    }

    /// Writes a single bit into the stream.
    void write(bool bit)
    {
		cache_ <<= 1;
        if (bit)
			cache_ |= 1;

        current_++;

        if (current_ == sizeof(cache_) * 8)
            flush();
    }

    /// Flushes all pending bits into the underlying stream, potentially adding zero-padding.
    void flush()
    {
        writer_((std::byte const*) &cache_, sizeof(cache_));
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

    explicit BitStreamReader(std::vector<uint8_t> const& bytes)
        : BitStreamReader{[bytes, i = size_t{0}](std::byte* data, std::size_t count) mutable {
              if (i + count < bytes.size())
              {
                  for (size_t k = 0; k < count; ++k)
                      data[k] = static_cast<std::byte>(bytes[i + k]);
                  i += count;
              }
              else
                  throw std::runtime_error{"Reading beyond bit-stream."};
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
        if (current_ == sizeof(cache_) * 8)
        {
            current_ = 0;
            cache_ = 0;
            reader_((std::byte*) &cache_, sizeof(cache_));
        }

        return (cache_ & (1llu << current_++)) != 0;
    }

    void skip(size_t count)
    {
        for (size_t i = 0; i < count; ++i)
            read();
    }

  private:
    // Not using vector<bool> here, as I cannot bulk-read into a vector<bool> conviniently.
    uint64_t cache_;
    size_t current_;
    Reader reader_;
};

}  // namespace bitstream
