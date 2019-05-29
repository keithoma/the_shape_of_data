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

unsigned getTerminalWidth(int terminalFd)
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

unique_ptr<pipeline::Source> createSource(string const& filename)
{
    if (auto f = make_unique<ifstream>(filename, ios::binary); f->is_open())
        return make_unique<pipeline::RawSource>(move(f));
    else
        throw std::runtime_error("Could not open file for reading.");
}

unique_ptr<pipeline::Sink> createSink(string const& filename)
{
    if (auto f = make_unique<ofstream>(filename, ios::binary | ios::trunc); f->is_open())
        return make_unique<pipeline::RawSink>(move(f));
    else
        throw std::runtime_error("Could not open file for writing.");
}

list<pipeline::Filter> populateFilters(string const& input, string const& output)
{
    list<pipeline::Filter> filters;

    if (input == "ppm")
        filters.emplace_back(pipeline::PPMDecoder{});
    else if (input == "rle")
        filters.emplace_back(pipeline::RLEDecoder{});
    else if (input == "huffman")
        ;  // TODO
    else
        ;  // TODO

	if (output == "ppm")
        filters.emplace_back(pipeline::PPMEncoder{});
    else if (output == "rle")
        ;  // TODO
    else if (output == "huffman")
        ;  // TODO
    else
        ;  // TODO

	return filters;
}

int main(int argc, const char* argv[])
{
    flags::Flags cli;
    cli.defineString("input-format", 'I', "FORMAT", "Specifies which format the input stream has.", "raw");
    cli.defineString("input-file", 'i', "PATH", "Specifies the path to the input file to read from.");
    cli.defineString("output-format", 'O', "FORMAT", "Specifies which format the output stream will be.",
                     "raw");
    cli.defineString("output-file", 'o', "PATH", "Specifies the path to the output file to write to.");
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

        auto source = createSource(inputFile);
        auto sink = createSink(outputFile);

        auto filters = populateFilters(inputFormat, outputFormat);
        auto input = pipeline::Buffer{};
        auto output = pipeline::Buffer{};

        input.reserve(4096);  // page-size

        while (source->read(input))
        {
            pipeline::apply(filters, input, &output, false);
            sink->write(output.data(), output.size());
            input.clear();
            // TODO: make this loop body a one-liner (refactor APIs)
        }

        // mark end in filter pipeline, in case some filter eventually still has to flush something.
        pipeline::apply(filters, {}, &output, true);
        sink->write(output.data(), output.size());
    }

    return EXIT_SUCCESS;
}
