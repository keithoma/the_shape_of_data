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
#include "huffman.hpp"
#include "utils.hpp"

#include <sgfx/color.hpp>
#include <sgfx/image.hpp>
#include <sgfx/ppm.hpp>

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
        using Run = sgfx::rle_image::Run;

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
    for (size_t i = 0; i < input.size(); ++i)
    {
        switch (state_)
        {
            case RLEState::Width1:
                width_ |= input[i] & 0xFF;
                output.push_back(input[i]);
                state_ = RLEState::Width2;
                break;
            case RLEState::Width2:
                width_ |= (input[i] << 8) & 0xFF00;
                output.push_back(input[i]);
                state_ = RLEState::Height1;
                break;
            case RLEState::Height1:
                height_ |= input[i] & 0xFF;
                output.push_back(input[i]);
                state_ = RLEState::Height2;
                break;
            case RLEState::Height2:
                height_ |= (input[i] << 8) & 0xFF00;
                output.push_back(input[i]);
                state_ = RLEState::PixelRed;
                break;
            case RLEState::PixelRed:
                cache_.push_back(input[i]);
                state_ = RLEState::PixelGreen;
                break;
            case RLEState::PixelGreen:
                cache_.push_back(input[i]);
                state_ = RLEState::PixelBlue;
                break;
            case RLEState::PixelBlue:
                cache_.push_back(input[i]);
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
    // TODO
}

void HuffmanEncoder::operator()(Buffer const& input, Buffer& output, bool last)
{
    ranges::copy(input, back_inserter(cache_));

    if (last)
        encode(cache_, output, debug_);
}

void HuffmanEncoder::encode(Buffer const& input, Buffer& output, bool debug)
{
    auto const static writeBits = [](vector<bool> const& bits, vector<bool>& writeCache) -> optional<Buffer> {
        writeCache.insert(end(writeCache), begin(bits), end(bits));

        size_t constexpr frameSize = 4096 * 8;

        if (writeCache.size() < frameSize)
            return nullopt;

        auto out = huffman::to_bytes(writeCache, frameSize);
        writeCache.erase(begin(writeCache), next(begin(writeCache), frameSize));
        return {move(out)};
    };

    auto const static flushLastBits = [](vector<bool> const& writeCache) -> Buffer {
        return huffman::to_bytes(writeCache);
    };

    auto const root = huffman::encode(input);
    if (debug)
		clog << huffman::to_dot(root) << endl;

    huffman::CodeTable const codeTable = huffman::encode(root);

    write<uint64_t>(output, input.size());

    for (size_t code = 0; code < 256; ++code)
    {
        auto const& bits = codeTable[code];
        auto const bytes = huffman::to_bytes(bits);
        write<uint16_t>(output, static_cast<uint16_t>(bits.size()));
        for (auto const b : bytes)
            write<uint8_t>(output, b);
    }

    vector<bool> writeCache{};
    for (auto const sym : input)
        if (auto const frame = writeBits(codeTable[sym], writeCache); frame.has_value())
            output.insert(end(output), begin(*frame), end(*frame));
    auto const lastFrame = flushLastBits(writeCache);
    output.insert(end(output), begin(lastFrame), end(lastFrame));
}

}  // namespace pipeline