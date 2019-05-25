#include <sgfx/color.hpp>
#include <sgfx/image.hpp>
#include <sgfx/ppm.hpp>
#include <sgfx/primitive_types.hpp>
#include <sgfx/primitives.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using namespace std;

#define let auto

namespace sgfx {

canvas load_ppm(const std::string& _path)
{
    filesystem::path path{_path};
    let fileSize = std::filesystem::file_size(path);

    let data = string{};
    data.resize(fileSize);

    ifstream ifs{path, ios::binary};
    if (!ifs.is_open())
        throw runtime_error("Could not open file.");

    ifs.read(data.data(), fileSize);
    if (static_cast<size_t>(ifs.tellg()) < static_cast<size_t>(fileSize))
    {
        throw runtime_error{(ostringstream{} << "Expected to read " << fileSize << " bytes but only read "
                                             << ifs.tellg() << " bytes.")
                                .str()};
    }

    let parser = ppm::Parser{};
    let image = parser.parseString(data);

    return image;
}

void save_ppm(widget const& image, const std::string& filename)
{
    let os = ofstream{filename};

    os << "P3\n"
       << "# Created by Gods of Code\n"
       << image.width() << ' ' << image.height() << '\n'
       << "255\n";

    let const pixelWriter = [&](let pixel) {
        os << pixel.red() << ' ' << pixel.green() << ' ' << pixel.blue() << '\n';
    };

    for_each(cbegin(image.pixels()), cend(image.pixels()), pixelWriter);
}

rle_image load_rle(const std::string& filename)
{
    return {};
}

void save_rle(const rle_image& src, const std::string& filename)
{
}

rle_image rle_encode(widget& src)
{
    return {};
}

void draw(widget& target, const rle_image& source, point top_left)
{
}

void draw(widget& target, const rle_image& source, point top_left, color::rgb_color colorkey)
{
}

}  // namespace sgfx
