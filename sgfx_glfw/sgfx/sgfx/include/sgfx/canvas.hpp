#ifndef SGFX_CANVAS_H
#define SGFX_CANVAS_H

#include <sgfx/color.hpp>
#include <sgfx/primitive_types.hpp>

#include <vector>

namespace sgfx
{
	class canvas
	{
		public:
		explicit canvas(dimension size):
			pixels_(size.w*size.h),
			size_{size}
		{}
		
		std::uint16_t width() const { return size_.w; }
		std::uint16_t height() const { return size_.h; }
		
		auto pixels() { return pixels_.data(); }
		auto pixels() const { return pixels_.data(); }
		
		private:
		std::vector<color::rgb_color> pixels_;
		dimension size_;
	};
	
	class canvas_view
	{
		public:
		template <typename T>
		canvas_view(T& target):
			pixels_{target.pixels()},
			size_{target.width(),target.height()}
		{}
		
		std::uint16_t width() const { return size_.w; }
		std::uint16_t height() const { return size_.h; }
		
		auto pixels() { return pixels_; }
		auto pixels() const { return pixels_; }
		
		private:
		color::rgb_color* pixels_;
		dimension size_;
	};
	
	void draw(canvas_view target, const canvas& img, point top_left);
	void draw(canvas_view target, const canvas& img, point top_left, color::rgb_color color_key);
}

#endif
