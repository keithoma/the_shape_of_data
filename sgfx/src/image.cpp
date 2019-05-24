#include <sgfx/image.hpp>

#include <fstream>
#include <string>
#include <stdexcept>

using namespace sgfx;

canvas sgfx::load_ppm(const std::string& path)
{
	return canvas{{10,10}};
}


void sgfx::save_ppm(canvas_view img, const std::string& filename)
{
}

rle_image sgfx::load_rle(const std::string& filename)
{
	return {};
}

void sgfx::save_rle(const rle_image& src, const std::string& filename)
{
}

rle_image sgfx::rle_encode(canvas_view src)
{
	return {};
}

void sgfx::draw(canvas_view target, const rle_image& source, point top_left)
{
}

void sgfx::draw(canvas_view target, const rle_image& source, point top_left, color::rgb_color colorkey)
{
}
