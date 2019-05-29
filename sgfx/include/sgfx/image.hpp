#ifndef SGFX_IMAGE_H
#define SGFX_IMAGE_H

#include <sgfx/canvas.hpp>
#include <sgfx/color.hpp>
#include <sgfx/primitive_types.hpp>
#include <sgfx/primitives.hpp>

#include <string>
#include <vector>

namespace sgfx {

// would be better to use std::filesystem::path, but support seems to be lacking on some platforms(...) and it
// seems like not everbody is willing to use the VM xD
canvas load_ppm(const std::string& path);
void save_ppm(widget const& source, const std::string& path);

class rle_image {
  public:
    struct Run {
        uint8_t length;
        color::rgb_color color;
    };
    using Row = std::vector<Run>;

    static rle_image load(std::string const& data);

    static std::vector<Run> decodeLine(uint8_t const* line, size_t width);
    static void encodeLine(std::vector<uint8_t> const& input, std::vector<uint8_t>& output);

    rle_image(dimension dim, std::vector<Row> rows) : dim_{dim}, rows_{move(rows)} {}
    rle_image() : rle_image{sgfx::dimension{0, 0}, {}} {}

    dimension const& dim() const noexcept { return dim_; }
    size_t row_count() const noexcept { return rows_.size(); }
    Row const& row(size_t i) const noexcept { return rows_[i]; }
    std::vector<Row> const& rows() const noexcept { return rows_; }

  private:
    sgfx::dimension dim_;
    std::vector<Row> rows_;
};

rle_image load_rle(const std::string& path);
void save_rle(const rle_image& source, const std::string& path);
rle_image rle_encode(widget& source);
void draw(widget& target, const rle_image& source, point top_left);
void draw(widget& target, const rle_image& source, point top_left, color::rgb_color colorkey);

}  // namespace sgfx

#endif
