// This file is part of the "convert" project, https://github.com/keithoma>
//   (c) 2019 Kei Thoma <thomakei@gmail.com>
//   (c) 2019 Christian Parpart <christian@parpart.family>
//
// Design idea originated from the x0 project, https://github.com/chrisitanparpart/x0.
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#include "pipeline.hpp"
#include "bitstream.hpp"
#include "huffman.hpp"
#include "utils.hpp"

#include <sgfx/color.hpp>
#include <sgfx/image.hpp>
#include <sgfx/ppm.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

#include <cassert>
#include <cmath>
#include <cstdlib>

using namespace std;

namespace pipeline {

// -------------------------------------------------------------------------
// private helper

namespace {

template <typename T>
T read(std::istream& source)
{
    T value;
    source.read((char*) &value, sizeof(value));
    return value;
}

template <typename T>
void write(pipeline::Buffer& os, T const& value)
{
    static_assert(sizeof(T) <= 8);

    if constexpr (sizeof(T) >= 8)
        os.push_back((value >> 56) & 0xFF);

    if constexpr (sizeof(T) >= 7)
        os.push_back((value >> 48) & 0xFF);

    if constexpr (sizeof(T) >= 6)
        os.push_back((value >> 40) & 0xFF);

    if constexpr (sizeof(T) >= 5)
        os.push_back((value >> 32) & 0xFF);

    if constexpr (sizeof(T) >= 4)
        os.push_back((value >> 24) & 0xFF);

    if constexpr (sizeof(T) >= 3)
        os.push_back((value >> 16) & 0xFF);

    if constexpr (sizeof(T) >= 2)
        os.push_back((value >> 8) & 0xFF);

    os.push_back(value & 0xFF);
}

}  // namespace

// -------------------------------------------------------------------------
// Filter API

Buffer& apply(list<Filter> const& filters, Buffer const& input, Buffer& output, bool last)
{
    auto i = filters.begin();
    auto e = filters.end();

    if (i == e)
        output = input;  // too bad we didn't decide for non-const input, then we could swap() here, too.
    else
    {
        output.clear();

        (*i)(input, output, last);
        i++;

        Buffer backBuffer;
        while (i != e)
        {
            backBuffer.swap(output);
            (*i)(backBuffer, output, last);
            i++;
        }
    }

    return output;
}

// -------------------------------------------------------------------------
// PPM Encoder & Decoder

void PPMDecoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    ranges::copy(input, back_inserter(cache_));

    if (last)
    {
        ranges::copy(input, back_inserter(cache_));
        sgfx::canvas const canvas = sgfx::ppm::Parser{}.parseString(cache_);

        auto out = back_inserter(output);

        // encode 16-bit width
        *out++ = canvas.width() & 0xFF;
        *out++ = (canvas.width() >> 8) & 0xFF;

        // encode 16-bit height
        *out++ = canvas.height() & 0xFF;
        *out++ = (canvas.height() >> 8) & 0xFF;

        for (sgfx::color::rgb_color color : canvas.pixels())
        {
            *out++ = color.red();
            *out++ = color.green();
            *out++ = color.blue();
        }
    }
}

void PPMEncoder::write(Buffer& output, char const* text)
{
    while (*text)
        output.push_back(*text++);
}

void PPMEncoder::write(Buffer& output, unsigned value)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%u", value);
    write(output, buf);
}

void PPMEncoder::write(Buffer& output, char ch)
{
    output.push_back(ch);
}

void PPMEncoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    ranges::copy(input, back_inserter(cache_));

    if (last)
    {
        auto const& input = cache_;

        unsigned const width = input[0] | (input[1] << 8);
        unsigned const height = input[2] | (input[3] << 8);

        write(output, "P3\n");
        write(output, width);
        write(output, ' ');
        write(output, height);
        write(output, '\n');
        write(output, "255\n");  // largest value

        for (size_t i = 4; i + 2 < input.size(); i += 3)
        {
            char buf[128];
            snprintf(buf, sizeof(buf), "%u %u %u\n", input[i + 0], input[i + 1], input[i + 2]);
            write(output, buf);
        }
    }
}

// -------------------------------------------------------------------------
// RLE Encoder & Decoder

void RLEDecoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    ranges::copy(input, back_inserter(cache_));

    if (last)
    {
        auto in = stringstream{cache_};
        auto out = back_inserter(output);

        auto const width = read<uint16_t>(in);
        *out++ = width & 0xFF;
        *out++ = (width >> 8) & 0xFF;

        auto const height = read<uint16_t>(in);
        *out++ = height & 0xFF;
        *out++ = (height >> 8) & 0xFF;

        for (auto lineNo = 0; lineNo < height; ++lineNo)
        {
            auto const runsOnThisLine = read<uint16_t>(in);

            for (auto i = 0; i < runsOnThisLine; ++i)
            {
                auto const length = read<uint8_t>(in);
                auto const red = read<uint8_t>(in);
                auto const green = read<uint8_t>(in);
                auto const blue = read<uint8_t>(in);

                for (auto k = 0; k < length; ++k)
                {
                    *out++ = red;
                    *out++ = green;
                    *out++ = blue;
                }
            }
        }
    }
}

void RLEEncoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    for (auto const symbol : input)
    {
        switch (state_)
        {
            case RLEState::Width1:
                width_ |= symbol & 0xFF;
                output.push_back(symbol);
                state_ = RLEState::Width2;
                break;
            case RLEState::Width2:
                width_ |= (symbol << 8) & 0xFF00;
                output.push_back(symbol);
                state_ = RLEState::Height1;
                break;
            case RLEState::Height1:
                height_ |= symbol & 0xFF;
                output.push_back(symbol);
                state_ = RLEState::Height2;
                break;
            case RLEState::Height2:
                height_ |= (symbol << 8) & 0xFF00;
                output.push_back(symbol);
                state_ = RLEState::PixelRed;
                break;
            case RLEState::PixelRed:
                cache_.push_back(symbol);
                state_ = RLEState::PixelGreen;
                break;
            case RLEState::PixelGreen:
                cache_.push_back(symbol);
                state_ = RLEState::PixelBlue;
                break;
            case RLEState::PixelBlue:
                cache_.push_back(symbol);
                ++currentColumn_;
                if (currentColumn_ == width_)
                {
                    sgfx::rle_image::encodeLine(cache_, output);
                    currentColumn_ = 0;
                    currentLine_++;
                    cache_.clear();
                }
                state_ = RLEState::PixelRed;
                break;
            default:
                assert(!"Internal Bug. Please report me.");
                abort();
        }
    }
}

void HuffmanDecoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    uint64_t const originalSize =
        static_cast<uint64_t>(input[0]) << 56 | static_cast<uint64_t>(input[1]) << 48
        | static_cast<uint64_t>(input[2]) << 40 | static_cast<uint64_t>(input[3]) << 32
        | static_cast<uint64_t>(input[4]) << 24 | static_cast<uint64_t>(input[5]) << 16
        | static_cast<uint64_t>(input[6]) << 8 | static_cast<uint64_t>(input[7]);

    auto reader = bitstream::BitStreamReader{input};
    reader.skip(64);

    // TODO: read code table

    // TODO: read data payload
    size_t decodedByteCount = 0;

    assert(decodedByteCount == originalSize);
}

void HuffmanEncoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    ranges::copy(input, back_inserter(cache_));

    if (last)
        encode(cache_, output, dotfile_, debug_);
}

void HuffmanEncoder::encode(Buffer const& input, Buffer& output, string const& dotfileName, bool debug)
{
    auto const static debugCode = [](uint8_t code, huffman::BitVector const& bits,
                                     vector<uint8_t> const& bytesPadded) {
        if (!bytesPadded.empty())
        {
            printf("[%03u]", code);
            for (auto const byteValue : bytesPadded)
                printf(" %02x", byteValue);
            printf(" ");
            for (size_t i = 0; i < bits.size(); ++i)
            {
                if ((i % 4) == 0)
                    printf(" ");
                printf("%c", bits[i] ? '1' : '0');
            }
            printf("\n");
        }
    };

    auto const static debugSym = [](uint8_t sym, huffman::BitVector const& bits) {
        printf("sym: %02x; bit seq:", sym);
        for (size_t i = 0; i < bits.size(); ++i)
        {
            if ((i % 4) == 0)
                printf(" ");
            printf("%c", bits[i] ? '1' : '0');
        }
        printf("\n");
    };

    auto const static debugFlush = [](byte const* data, size_t count) {
        printf("  flush: %zu bytes:\n", count);
        for (size_t i = 0; i < count; ++i)
        {
            printf("    %02x", static_cast<unsigned>(data[i]));
            for (size_t k = 0; k < 8; ++k)
            {
                if ((k % 4) == 0)
                    printf(" ");
                printf("%c", ((to_integer<uint8_t>(data[i]) & (1 << (7 - k))) != 0) ? '1' : '0');
            }
            printf("\n");
        }
    };

    auto const static flusher = [](Buffer& output, bool debug) {
        return [debug, output = ref(output)](byte const* data, size_t count) mutable {
            if (debug)
                debugFlush(data, count);

            auto const static phi = [](byte value) { return to_integer<uint8_t>(value); };
            ranges::transform(util::span{data, count}, back_inserter(output.get()), phi);
        };
    };

    auto const root = huffman::encode(input);
    auto const codeTable = huffman::CodeTable{huffman::encode(root)};
    auto writer = bitstream::BitStreamWriter{flusher(output, debug)};

    if (!dotfileName.empty())
        ofstream{dotfileName, ios::trunc} << huffman::to_dot(root) << '\n';

    // original filesize
    writer.writeAligned<uint64_t>(input.size());

    // code table
    if (debug)
        printf("Code Table:\n");
    for (auto&& [code, bits] : ranges::indexed(codeTable))
    {
        auto const bytesPadded = huffman::to_bytes(bits);

        if (debug)
            debugCode(static_cast<uint8_t>(code), bits, bytesPadded);

        writer.writeAligned<uint16_t>(static_cast<uint16_t>(bits.size()));
        for (auto const b : bytesPadded)
            writer.writeAligned<uint8_t>(b);
    }

    // payload
    if (debug)
        printf("Data:\n");
    for (auto const sym : input)
    {
        if (debug)
            debugSym(sym, codeTable[sym]);
        writer.write(codeTable[sym]);
    }
    writer.flush();
}

}  // namespace pipeline
