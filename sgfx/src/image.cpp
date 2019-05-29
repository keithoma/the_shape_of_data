#include <sgfx/color.hpp>
#include <sgfx/image.hpp>
#include <sgfx/ppm.hpp>
#include <sgfx/primitive_types.hpp>
#include <sgfx/primitives.hpp>

//#include "sysconfig.h"

#include <experimental/filesystem>

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include <cassert>

using namespace std;
using namespace std::experimental;

#define let auto /* Pure provocation with respect to my dire love to F# & my hate to C++ auto keyword. */

namespace {

template <typename T>
T read(std::istream& source)
{
    T value;
    source.read((char*) &value, sizeof(value));
    return value;
}

template <typename T>
void write(std::ostream& os, T const& value)
{
    os.write((char*) &value, sizeof(value));
}

}  // namespace

namespace sgfx {


canvas load_ppm(const std::string& _path)
{
    experimental::filesystem::path path{_path};
    let fileSize = experimental::filesystem::file_size(path);

    let data = string{};
    data.resize(fileSize);

    ifstream ifs{path, ios::binary};
    if (!ifs.is_open())
        throw runtime_error("Could not open file.");

    ifs.read(data.data(), fileSize);
    if (static_cast<size_t>(ifs.tellg()) < static_cast<size_t>(fileSize))
	{
        auto msg = ostringstream{};
		msg << "Expected to read " << fileSize << " bytes but only read " << ifs.tellg() << " bytes.";
        throw runtime_error{msg.str()};
	}

    return ppm::Parser{}.parseString(data);
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

void rle_image::encodeLine(std::vector<byte> const& input, std::vector<byte>& output)
{
    size_t i = 0;
    while (i + 2 < input.size())
    {
        auto const red = input[i++];
        auto const green = input[i++];
        auto const blue = input[i++];

        auto count = unsigned{1};
        while (i + 2 < input.size() && count < 255 && input[i] == red && input[i + 1] == green
               && input[i + 2] == blue)
        {
			i += 3;
            ++count;
        }

        output.push_back(static_cast<byte>(count));
        output.push_back(red);
        output.push_back(green);
        output.push_back(blue);
    }
    assert(i == input.size());
}

std::vector<rle_image::Run> rle_image::decodeLine(uint8_t const* line, size_t width)
{
    let runs = vector<Run>(line[0] | (line[1] << 8));
    line += 2;

    for (unsigned i = 0; i < runs.size(); ++i)
    {
        let const length = *line++;
        let const red = *line++;
        let const green = *line++;
        let const blue = *line++;
        runs[i] = Run{length, color::rgb_color{red, green, blue}};
    }

    return runs;
}

rle_image load_rle(const std::string& filename)
{
    let in = ifstream{filename, ios::binary};
    if (!in.is_open())
        throw std::runtime_error{"Could not open file."};

    let const width = read<uint16_t>(in);
    let const height = read<uint16_t>(in);

    let lines = vector<vector<rle_image::Run>>();
    while (in.good())
    {
        let runs = vector<rle_image::Run>(read<uint16_t>(in));

        for (unsigned i = 0; i < runs.size(); ++i)
        {
            let const length = read<uint8_t>(in);
            let const red = read<uint8_t>(in);
            let const green = read<uint8_t>(in);
            let const blue = read<uint8_t>(in);
            runs[i] = rle_image::Run{length, color::rgb_color{red, green, blue}};
        }
        lines.emplace_back(move(runs));
        // TODO: lines.emplace_back(rle_image::decodeLine(line, width));
    }

    return rle_image{dimension{width, height}, move(lines)};
}

void save_rle(const rle_image& image, const std::string& filename)
{
    ofstream os{filename, ios::binary};

    write<uint16_t>(os, image.dim().width);
    write<uint16_t>(os, image.dim().height);

    for (rle_image::Row const& row : image.rows())
    {
        write<uint16_t>(os, static_cast<uint16_t>(row.size()));
        for (rle_image::Run const& run : row)
        {
            write<uint8_t>(os, run.length);
            write<uint8_t>(os, run.color.red());
            write<uint8_t>(os, run.color.green());
            write<uint8_t>(os, run.color.blue());
        }
    }
}

rle_image rle_encode(widget& image)
{
    using Run = rle_image::Run;

    // reads one run, i.e. one color and all up to 255 following colors that match that first color.
    auto readOne = [&image](int x, int y) -> Run {
        let const color = image[point{x, y}];
        let length = uint8_t{1};
        ++x;
        while (x < image.width() && length < 255 && image[point{x, y}] == color)
            ++length, ++x;
        return {length, color};
    };

    vector<rle_image::Row> rows;
    rows.reserve(image.height());

    for (int x = 0, y = 0; y < image.height(); x = 0, ++y)
        rows.emplace_back([&]() {
            rle_image::Row row;
            for (auto run = readOne(x, y); run.length > 0; run = readOne(x, y), x += row.back().length)
                row.emplace_back(move(run));
            return row;
        }());

    return rle_image{dimension{image.width(), image.height()}, move(rows)};
}

void draw(widget& target, const rle_image& source, point top_left)
{
    for (int x = 0, y = 0; y < source.dim().height; x = 0, ++y)
        for (rle_image::Run const& run : source.row(y))
            for (unsigned i = 0; i < run.length; ++i, ++x)
                target[top_left + point{x, y}] = run.color;
}

void draw(widget& target, const rle_image& source, point top_left, color::rgb_color colorkey)
{
    for (int x = 0, y = 0; y < source.dim().height; x = 0, ++y)
        for (rle_image::Run const& run : source.row(y))
            for (unsigned i = 0; i < run.length; ++i, ++x)
                if (run.color != colorkey)
                    target[top_left + point{x, y}] = run.color;
}

}  // namespace sgfx
