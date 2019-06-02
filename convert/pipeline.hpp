// This file is part of the "convert" project, https://github.com/keithoma>
//   (c) 2019 Kei Thoma <thomakei@gmail.com>
//   (c) 2019 Christian Parpart <christian@parpart.family>
//
// Design idea originated from the x0 project, https://github.com/chrisitanparpart/x0.
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#pragma once

#include <functional>
#include <iosfwd>
#include <list>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <cstddef>
#include <cstdint>

namespace pipeline {

using Buffer = std::vector<uint8_t>;

// -----------------------------------------------------------------------------
// Filter API

/**
 * Applies this filter to the given @p input.
 *
 * @param input the input data this filter to apply to.
 * @param output the output to store the filtered data to.
 * @param last indicator whether or not this is the last data chunk in the
 *             stream.
 */
using Filter = std::function<void(Buffer const& /*input*/, Buffer& /*output*/, bool /*last*/)>;

/**
 * Applies a set of filters to @p input and stores result in @p output.
 *
 * @param filters list of filters to apply in order
 * @param input input buffer to pass to first filter
 * @param output resulting output buffer to store the output of the last filter
 *
 * @returns reference to output buffer
 *
 * @note Any previous data in output will be lost.
 */
Buffer& apply(const std::list<Filter>& filters, const Buffer& input, Buffer& output, bool last);

/**
 * Decodes a single PPM image file stream chunk-wise.
 */
class PPMDecoder {
  public:
    void operator()(const Buffer& input, Buffer& output, bool last);

  private:
    std::string cache_;
};

class PPMEncoder {
  public:
    void operator()(Buffer const& input, Buffer& output, bool last);

  private:
    void write(Buffer& output, char const* text);
    void write(Buffer& output, unsigned value);
    void write(Buffer& output, char ch);

  private:
    Buffer cache_;
};

class RLEDecoder {
  public:
    void operator()(const Buffer& input, Buffer& output, bool last);

  private:
    std::string cache_;
};

class RLEEncoder {
  public:
    void operator()(const Buffer& input, Buffer& output, bool last);

  private:
    enum class RLEState {
        Width1,
        Width2,
        Height1,
        Height2,
        PixelRed,
        PixelGreen,
        PixelBlue,
    };

    Buffer cache_{};
    RLEState state_ = RLEState::Width1;
    unsigned width_ = 0;
    unsigned height_ = 0;
    unsigned currentLine_ = 0;
    unsigned currentColumn_ = 0;
};

class HuffmanEncoder {
  public:
    HuffmanEncoder(std::string dotfile, bool debug) : dotfile_{move(dotfile)}, debug_{debug} {}
    HuffmanEncoder() : HuffmanEncoder{{}, false} {}

    void operator()(const Buffer& input, Buffer& output, bool last);

    static void encode(Buffer const& input, Buffer& output, std::string const& dotfile, bool debug);

  private:
    std::string dotfile_;              // optional dotfile name to dump huffman tree graph to
    bool debug_;                       // optional debug printing to stderr
    Buffer cache_{};                   // population cache
    std::vector<bool> pendingBits_{};  // write-out cache
};

class HuffmanDecoder : public Filter {
  public:
    void operator()(const Buffer& input, Buffer& output, bool last);
    // TODO
};

}  // namespace pipeline
