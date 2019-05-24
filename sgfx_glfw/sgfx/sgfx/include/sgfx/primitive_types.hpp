#ifndef SGFX_PRIMITIVE_TYPES_H
#define SGFX_PRIMITIVE_TYPES_H

#include <cstdint>

namespace sgfx
{
	struct vec
	{
		int x, y;
	};

	struct point
	{
		int x, y;
	};
	
	struct dimension
	{
		std::uint16_t w, h;
	};
	
	struct rectangle
	{
		point top_left;
		dimension size;
	};
}

#endif
