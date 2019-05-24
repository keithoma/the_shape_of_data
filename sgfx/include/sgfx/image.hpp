#ifndef SGFX_IMAGE_H
#define SGFX_IMAGE_H

#include <sgfx/canvas.hpp>
#include <sgfx/color.hpp>
#include <sgfx/primitives.hpp>

#include <string>

namespace sgfx
{
	//would be better to use std::filesystem::path, but support seems to be lacking on some platforms(...) and it seems like not everbody is willing to use the VM xD
	canvas load_ppm(const std::string& path);
	void save_ppm(widget& source, const std::string& path);
	
	class rle_image{};
	
	rle_image load_rle(const std::string& path);
	void save_rle(const rle_image& source, const std::string& path);
	rle_image rle_encode(widget& source);
	void draw(widget& target, const rle_image& source, point top_left);
	void draw(widget& target, const rle_image& source, point top_left, color::rgb_color colorkey);
}

#endif
