// This file is part of the "convert" project, https://github.com/keithoma>
//   (c) 2019 Kei Thoma <thomakei@gmail.com>
//   (c) 2019 Christian Parpart <christian@parpart.family>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

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

#if defined(HAVE_DIRECT_H)
#    include <direct.h>
#endif

#if defined(HAVE_IOCTL_H)
#    include <ioctl.h>
#endif

#if defined(HAVE_UNISTD_H)
#    include <unistd.h>
#endif

#if !defined(STDOUT_FILENO)
#    define STDOUT_FILENO (1)
#endif

using namespace std;

static unsigned getTerminalWidth(int terminalFd)
{
#if defined(TIOCGSIZE)
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

static list<pipeline::Filter> populateFilters(string const& input, string const& output,
                                              string const& huffmanDotOutput, bool debug)
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
    else if (input != "raw")
        throw std::runtime_error{"Invalid input format specified: " + input};

    if (output == "ppm")
        filters.emplace_back(pipeline::PPMEncoder{});
    else if (output == "rle")
        filters.emplace_back(pipeline::RLEEncoder{});
    else if (output == "huffman")
        filters.emplace_back(pipeline::HuffmanEncoder{huffmanDotOutput, debug});
    else if (output == "rle+huffman")
    {
        filters.emplace_back(pipeline::RLEEncoder{});
        filters.emplace_back(pipeline::HuffmanEncoder{huffmanDotOutput, debug});
    }
    else if (output != "raw")
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
    cli.defineBool("help", 'h', "Shows this help.");
    cli.defineBool("debug", 'D', "Enables optional debug printing to stderr.");
    cli.defineString("input-format", 'I', "FORMAT", "Specifies which format the input stream has.", "raw");
    cli.defineString("input-file", 'i', "PATH", "Specifies the path to the input file to read from.");
    cli.defineString("output-format", 'O', "FORMAT", "Specifies which format the output stream will be.",
                     "raw");
    cli.defineString("output-file", 'o', "PATH", "Specifies the path to the output file to write to.");
    cli.defineString(
        "output-dot-huffman", 0, "PATH",
        "When Huffman encoding is chosen, the tree graph in dot file format is stored at this file location.",
        "");

    if (error_code ec = cli.tryParse(argc, argv); ec)
    {
        cerr << "Failed to parse command line arguments. " << ec.message() << endl;
        return EXIT_FAILURE;
    }
    else if (cli.getBool("help"))
    {
        string_view constexpr static header =
            "convert - command line tool for converting some file formats.\n"
            "Copyright (c) 2019 by Kei Thoma & Christian Parpart.\n\n";

        string_view constexpr static footer =
            "Supported file formats are:\n"
            "\n"
            " * raw: no encoding or decoding is happening\n"
            " * ppm: PPM image file\n"
            " * rle: RLE image file\n"
            " * huffman: Huffman (arbitrary file)\n"
            " * rle+huffman: RLE embedded inside Huffman (image file)\n\n";

        cout << cli.helpText(header, footer, getTerminalWidth(STDOUT_FILENO), 32);
        return EXIT_SUCCESS;
    }
    else
    {
        try
        {
            auto const inputFile = cli.getString("input-file");
            auto const inputFormat = cli.getString("input-format");
            auto const outputFile = cli.getString("output-file");
            auto const outputFormat = cli.getString("output-format");
            auto const huffmanDotOutput = cli.getString("output-dot-huffman");
            auto const debug = cli.getBool("debug");

            auto source = ifstream{inputFile, ios::binary};
            if (!source.is_open())
                throw std::runtime_error("Could not open file.");
            auto sink = ofstream{outputFile, ios::binary | ios::trunc};

            auto filters = populateFilters(inputFormat, outputFormat, huffmanDotOutput, debug);
            auto input = pipeline::Buffer{};
            auto output = pipeline::Buffer{};

            input.reserve(4096);  // page-size

            for (; !read(source, input).empty(); input.clear())
                write(sink, pipeline::apply(filters, input, output, false));

            // mark end in filter pipeline, in case some filter eventually still has to flush something.
            write(sink, pipeline::apply(filters, {}, output, true));
        }
        catch (flags::FlagError const& flagError)
        {
            cerr << make_error_code(flagError.code()).message() + ": " + flagError.arg() << '\n'
                 << "Try --help instead.\n";
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
