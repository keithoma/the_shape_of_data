#pragma once

#include <sgfx/color.hpp>
#include <sgfx/primitive_types.hpp>
#include <sgfx/widget.hpp>
#include <sgfx/window.hpp>
#include <stdexcept>

namespace sgfx {

class canvas : public widget {
  public:
    using Color = color::rgb_color;
    using Data = std::vector<Color>;

    canvas(dimension dim, Data data) : size_{dim}, pixels_{std::move(data)} {
        if (static_cast<std::size_t>(dim.width * dim.height) != pixels_.size())
            throw std::invalid_argument("Pixel dimensions don't match image data.");
	}

    explicit canvas(dimension size) : size_{size}, pixels_{static_cast<unsigned>(size.width * size.height)} {}

    std::uint16_t width() const noexcept override { return size_.width; }
    std::uint16_t height() const noexcept override { return size_.height; }

    std::vector<color::rgb_color>& pixels() noexcept override { return pixels_; }
    const std::vector<color::rgb_color>& pixels() const noexcept override { return pixels_; }

    static canvas colored(dimension size, color::rgb_color col);

  private:
    const dimension size_;
    std::vector<color::rgb_color> pixels_;
};

void draw(widget& target, const canvas& img, point top_left);

}  // namespace sgfx
