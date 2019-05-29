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
using Filter = std::function<void(Buffer const& /*input*/, Buffer* /*output*/, bool /*last*/)>;

/**
  * Applies a set of filters to @p input and stores result in @p output.
  *
  * @param filters list of filters to apply in order
  * @param input input buffer to pass to first filter
  * @param output resulting output buffer to store the output of the last filter
  */
void apply(const std::list<Filter>& filters, const Buffer& input, Buffer* output, bool last);

/**
 * Decodes a single PPM image file stream chunk-wise.
 */
class PPMDecoder {
  public:
    void operator()(const Buffer& input, Buffer* output, bool last);

  private:
    std::string cache_;
};

class PPMEncoder {
  public:
    void operator()(Buffer const& input, Buffer* output, bool last);

	static std::string encode(Buffer const& input); 

  private:
    Buffer cache_;
};

enum class RLEState {
	Width1,
	Width2,
	Height1,
	Height2,
	PixelRed,
	PixelGreen,
	PixelBlue,
};

class RLEDecoder {
  public:
    void operator()(const Buffer& input, Buffer* output, bool last);

  private:
    RLEState state_ = RLEState::Width1; // TODO: make use of it (performance increase)!

    std::string cache_;
};

class RLEEncoder {
  public:
    void operator()(const Buffer& input, Buffer* output, bool last);

  private:
	RLEState state_ = RLEState::Width1;
    Buffer cache_{};
    unsigned width_ = 0;
    unsigned height_ = 0;
    unsigned currentLine_ = 0;
    unsigned currentColumn_ = 0;
};

class HuffmanEncoder {
  public:
    void operator()(const Buffer& input, Buffer* output, bool last);
	// TODO
};

class HuffmanDecoder : public Filter {
  public:
    void operator()(const Buffer& input, Buffer* output, bool last);
	// TODO
};

// -----------------------------------------------------------------------------
// Source API

class Source {
  public:
    virtual ~Source() = default;

    virtual std::size_t read(Buffer& target) = 0;
};

/// Reads from source without interpreting (identity).
class RawSource : public Source {
  public:
    explicit RawSource(std::unique_ptr<std::istream> owned);
    explicit RawSource(std::istream& source);

    std::size_t read(Buffer& target) override;

  private:
    std::unique_ptr<std::istream> owned_;
    std::istream& source_;
};

// -----------------------------------------------------------------------------
// Sink API

class Sink {
  public:
    virtual ~Sink() = default;

    virtual void write(uint8_t const* data, size_t count) = 0;
};

/// Writes data as-is into the target stream.
class RawSink : public Sink {
  public:
    explicit RawSink(std::unique_ptr<std::ostream> owned) : owned_{move(owned)}, target_{*owned_} {}
    explicit RawSink(std::ostream& target) : owned_{}, target_{target} {}

    void write(uint8_t const* data, size_t count) override;

  private:
    std::unique_ptr<std::ostream> owned_;
    std::ostream& target_;
};

}  // namespace pipeline
