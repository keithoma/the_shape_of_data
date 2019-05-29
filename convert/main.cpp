#include "flags.hpp"
#include "huffman.hpp"
#include "pipeline.hpp"
#include "utils.hpp"

#include "sysconfig.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>

#include <direct.h>  // WINDOWS

#if defined(HAVE_IOCTL_H)
#    include <ioctl.h>
#endif

#if defined(HAVE_UNISTD_H)
#    include <unistd.h>
#endif

#if !defined(STDOUT_FILENO)
#    define STDOUT_FILENO 1
#endif

using namespace std;

static unsigned getTerminalWidth(int terminalFd)
{
#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    return ts.ts_cols;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    return ts.ws_col;
#else
    // default (I intentionally did not chose 80 here), as it's aged.
    return 110;
#endif
}

static list<pipeline::Filter> populateFilters(string const& input, string const& output, bool debug)
{
    list<pipeline::Filter> filters;

    if (input == "ppm")
        filters.emplace_back(pipeline::PPMDecoder{});
    else if (input == "rle")
        filters.emplace_back(pipeline::RLEDecoder{});
    else if (input == "huffman")
        filters.emplace_back(pipeline::HuffmanDecoder{});
    else if (input == "rle+huffman")
    {
        filters.emplace_back(pipeline::HuffmanDecoder{});
        filters.emplace_back(pipeline::RLEDecoder{});
    }
    else
        throw std::runtime_error{"Invalid input format specified: " + input};

    if (output == "ppm")
        filters.emplace_back(pipeline::PPMEncoder{});
    else if (output == "rle")
        filters.emplace_back(pipeline::RLEEncoder{});
    else if (output == "huffman")
        filters.emplace_back(pipeline::HuffmanEncoder{debug});
    else if (output == "rle+huffman")
    {
        filters.emplace_back(pipeline::RLEEncoder{});
        filters.emplace_back(pipeline::HuffmanEncoder{debug});
    }
    else
        throw std::runtime_error{"Invalid output format specified: " + input};

    if (input == output)
        // we intentionally populate/destruct so we also have know that file formats were valid.
        filters.clear();

    return filters;
}

static void write(ostream& sink, pipeline::Buffer const& source)
{
    sink.write(reinterpret_cast<char const*>(source.data()), source.size());
}

static pipeline::Buffer& read(istream& source, pipeline::Buffer& target)
{
    auto const start = target.size();
    auto const count = static_cast<size_t>(target.capacity() - target.size());

    // There seems to be an optimization that the reserve() call's underlying malloc
    // is deferred, so we have to force it here.
    target.resize(start + count);

    source.read(reinterpret_cast<char*>(target.data() + start), count);
    auto const nread = source.gcount();

    target.resize(start + nread);

    return target;
}

int main(int argc, const char* argv[])
{
    flags::Flags cli;
    cli.defineString("input-format", 'I', "FORMAT", "Specifies which format the input stream has.", "raw");
    cli.defineString("input-file", 'i', "PATH", "Specifies the path to the input file to read from.");
    cli.defineString("output-format", 'O', "FORMAT", "Specifies which format the output stream will be.",
                     "raw");
    cli.defineString("output-file", 'o', "PATH", "Specifies the path to the output file to write to.");
    cli.defineBool("debug", 'D', "Enable some debug prints.");
    cli.defineBool("help", 'h', "Shows this help.");

    if (error_code ec = cli.tryParse(argc, argv); ec)
    {
        cerr << "Failed to parse command line arguments. " << ec.message() << endl;
        return EXIT_FAILURE;
    }
    else if (cli.getBool("help"))
    {
        string_view constexpr static header =
            "convert - command line tool for converting some file formats.\n"
            "Copyright (c) 2019 by Christian Parpart and Kei Thoma.\n\n";

        string_view constexpr static footer =
            "Supported file formats are:\n"
            "\n"
            " * raw: no encoding or decoding is happening\n"
            " * ppm: PPM image file encoding/decoding\n"
            " * rle: RLE image file encoding/decoding\n"
            " * huffman: Huffman file encoding/decoding\n";

        cout << cli.helpText(header, footer, getTerminalWidth(STDOUT_FILENO), 32);
        return EXIT_SUCCESS;
    }
    else
    {
        auto const inputFile = cli.getString("input-file");
        auto const inputFormat = cli.getString("input-format");
        auto const outputFile = cli.getString("output-file");
        auto const outputFormat = cli.getString("output-format");
        auto const debug = cli.getBool("debug");

        auto source = ifstream{inputFile};
        if (!source.is_open())
            throw std::runtime_error("Could not open file.");
        auto sink = ofstream{outputFile, ios::binary | ios::trunc};

        auto filters = populateFilters(inputFormat, outputFormat, debug);
        auto input = pipeline::Buffer{};
        auto output = pipeline::Buffer{};

        input.reserve(4096);  // page-size

        for (; !read(source, input).empty(); input.clear())
            write(sink, pipeline::apply(filters, input, output, false));

        // mark end in filter pipeline, in case some filter eventually still has to flush something.
        write(sink, pipeline::apply(filters, {}, output, true));
    }

    return EXIT_SUCCESS;
}
