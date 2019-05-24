#ifndef SGFX_PRIMITIVES_H
#define SGFX_PRIMITIVES_H

#include <sgfx/canvas.hpp>
#include <sgfx/primitive_types.hpp>

#include <cstdint>

namespace sgfx
{
	void plot(canvas_view target, point p, color::rgb_color col);
	
	void clear(canvas_view target, color::rgb_color col);
	
	void hline(canvas_view target, point p, std::uint16_t length, color::rgb_color col);
	void vline(canvas_view target, point p, std::uint16_t length, color::rgb_color col);
	
	void fill(canvas_view target, rectangle rect, color::rgb_color col);
	
	void line(canvas_view target, point p0, point p1, color::rgb_color col);
	
}

#endif
