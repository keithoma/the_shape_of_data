#include <sgfx/image.hpp>
#include <sgfx/primitives.hpp>

#include <fstream>
#include <string>
#include <stdexcept>

using namespace std;

using namespace sgfx;

canvas sgfx::load_ppm(const std::string& path)
{
	// open the file
	ifstream ppm_file;
	ppm_file.open(path);

	// create a canvas
	canvas target{{640, 480}};
	string line;
	while(getline(ppm_file, line)) {
    	plot(taget, line, getline(ppm_file, line));
	}
	ppm_file.close();

	return target;
}


void sgfx::save_ppm(widget& img, const std::string& filename)
{
}

rle_image sgfx::load_rle(const std::string& filename)
{
	return {};
}

void sgfx::save_rle(const rle_image& src, const std::string& filename)
{
}

rle_image sgfx::rle_encode(widget& src)
{
	return {};
}

void sgfx::draw(widget& target, const rle_image& source, point top_left)
{
}

void sgfx::draw(widget& target, const rle_image& source, point top_left, color::rgb_color colorkey)
{
}
