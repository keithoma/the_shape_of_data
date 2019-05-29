#pragma once

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

struct Chunk {
    Buffer buffer;
    std::optional<std::pair<unsigned, unsigned>> widthAndHeight;
};

class Filter {
  public:
    virtual ~Filter() = default;

    /**
     * Applies this filter to the given @p input.
     *
     * @param input the input data this filter to apply to.
     * @param output the output to store the filtered data to.
     * @param last indicator whether or not this is the last data chunk in the
     *             stream.
     */
    virtual void filter(const Buffer& input, Buffer* output, bool last) = 0;

    /// Retrieves {with, height} if available.
    virtual std::optional<std::pair<unsigned, unsigned>> widthAndHeight() const { return std::nullopt; }

    /// Applies a set of filters to @p input and stores result in @p output.
    static void applyFilters(const std::list<std::unique_ptr<Filter>>& filters, const Buffer& input,
                             Buffer* output, bool last);
};

/**
 * Decodes a single PPM image file stream chunk-wise.
 */
class PPMDecoder : public Filter {
  public:
    void filter(const Buffer& input, Buffer* output, bool last) override;

  private:
    std::string cache_;
};

class PPMEncoder : public Filter {
  public:
    void filter(Buffer const& input, Buffer* output, bool last) override;

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

class RLEDecoder : public Filter {
  public:
    void filter(const Buffer& input, Buffer* output, bool last) override;

  private:
    RLEState state_ = RLEState::Width1; // TODO: make use of it (performance increase)!

    std::string cache_;
};

class RLEEncoder : public Filter {
  public:
    void filter(const Buffer& input, Buffer* output, bool last) override;

  private:
	RLEState state_ = RLEState::Width1;
    Buffer cache_{};
    unsigned width_ = 0;
    unsigned height_ = 0;
    unsigned currentLine_ = 0;
    unsigned currentColumn_ = 0;
};

class HuffmanEncoder : public Filter {
  public:
    void filter(const Buffer& input, Buffer* output, bool last) override;
};

class HuffmanDecoder : public Filter {
  public:
    void filter(const Buffer& input, Buffer* output, bool last) override;
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

// Reads from huffman encoded source, for retrieving decoded data.
class HuffmanSource : public Source {
  public:
    std::size_t read(Buffer& target) override;
};

class ImageSource : public Source {
  public:
    virtual unsigned width() const noexcept = 0;
    virtual unsigned height() const noexcept = 0;
};

class PPMSource : public ImageSource {
  public:
    PPMSource(std::istream& source) : source_{source} {}
    unsigned width() const noexcept override;
    unsigned height() const noexcept override;
    std::size_t read(Buffer& target) override;

  private:
    std::istream& source_;
};

class RLESource : public ImageSource {
  public:
    unsigned width() const noexcept override;
    unsigned height() const noexcept override;
    std::size_t read(Buffer& target) override;
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

class PPMSink : public Sink {
  public:
    PPMSink(unsigned width, unsigned height, std::ostream& target);
    void write(uint8_t const* data, size_t count) override;

  private:
    std::ostream& target_;
};

class RLESink : public Sink {
  public:
    RLESink(unsigned width, unsigned height, std::ostream& target);
    void write(uint8_t const* data, size_t count) override;

  private:
    std::ostream& target_;
};

}  // namespace pipeline
