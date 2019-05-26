#include <sgfx/color.hpp>
#include <sgfx/image.hpp>
#include <sgfx/ppm.hpp>
#include <sgfx/primitive_types.hpp>
#include <sgfx/primitives.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

using namespace std;

#define let auto /* Pure provocation with respect to my dire love to F# & my hate to C++ auto keyword. */

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
        throw runtime_error{(ostringstream{} << "Expected to read " << fileSize << " bytes but only read "
                                             << ifs.tellg() << " bytes.")
                                .str()};

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

class rle_parser {
  public:
    static rle_image parse(std::istream& source);

  private:
    explicit rle_parser(std::istream& source) : source_{source} {}
    rle_image parse();

    template <typename T>
    T read()
    {
        T value;
        source_.read((char*) &value, sizeof(value));
        return value;
    }

  private:
    std::istream& source_;
};

rle_image rle_parser::parse(std::istream& source)
{
    return rle_parser{source}.parse();
}

rle_image rle_parser::parse()
{
    let const width = read<uint16_t>();
    let const height = read<uint16_t>();

    let lines = vector<vector<rle_image::Run>>();
    while (source_.good())
    {
        let runs = vector<rle_image::Run>(read<uint16_t>());

        for (unsigned i = 0; i < runs.size(); ++i)
        {
            let const length = read<uint8_t>();
            let const red = read<uint8_t>();
            let const green = read<uint8_t>();
            let const blue = read<uint8_t>();
            runs[i] = rle_image::Run{length, color::rgb_color{red, green, blue}};
        }
        lines.emplace_back(move(runs));
    }

    return rle_image{dimension{width, height}, move(lines)};
}

rle_image load_rle(const std::string& filename)
{
    let ifs = ifstream{filename, ios::binary};
    if (!ifs.is_open())
        throw std::runtime_error{"Could not open file."};

    return rle_parser::parse(ifs);
}

template <typename T>
void write(std::ostream& os, T const& value)
{
    os.write((char*) &value, sizeof(value));
}

void save_rle(const rle_image& image, const std::string& filename)
{
    ofstream os{filename, ios::binary};

    write<uint16_t>(os, image.dimension().width);
    write<uint16_t>(os, image.dimension().height);

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
    // reads one run, i.e. one color and all up to 255 following colors that match that first color.
    auto readOne = [&](int x, int y) {
        auto const color = image[point{x, y}];
        ++x;
        int length = 1;
        while (x < image.width() && length <= 255 && image[point{x, y}] == color)
        {
            ++length;
            ++x;
        }
        return rle_image::Run{static_cast<uint8_t>(length), color};
    };

	vector<rle_image::Row> rows;
    rows.reserve(image.height());

    for (int x = 0, y = 0; y < image.height(); x = 0, ++y)
    {
        rle_image::Row row;
        for (auto run = readOne(x, y); run.length > 0; run = readOne(x, y))
        {
            x += run.length;
			row.emplace_back(move(run));
        }
        rows.emplace_back(move(row));
    }
    return rle_image{dimension{image.width(), image.height()}, move(rows)};
}

void draw(widget& target, const rle_image& source, point top_left)
{
    for (int x = 0, y = 0; y < source.dimension().height; x = 0, ++y)
        for (rle_image::Run const& run : source.row(y))
            for (unsigned i = 0; i < run.length; ++i, ++x)
                target[top_left + point{x, y}] = run.color;
}

void draw(widget& target, const rle_image& source, point top_left, color::rgb_color colorkey)
{
    for (int x = 0, y = 0; y < source.dimension().height; x = 0, ++y)
        for (rle_image::Run const& run : source.row(y))
            for (unsigned i = 0; i < run.length; ++i, ++x)
                if (run.color != colorkey)
                    target[top_left + point{x, y}] = run.color;
}

}  // namespace sgfx
