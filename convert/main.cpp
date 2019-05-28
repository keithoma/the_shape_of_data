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
    return 110;  // default (I intentionally did not chose 80 here), as it's aged.
#endif
}

unique_ptr<pipeline::Source> createSource(string const& format, string const& filename)
{
    if (format == "raw")
        return make_unique<pipeline::RawSource>(make_unique<ifstream>(filename));

    return nullptr;
}

unique_ptr<pipeline::Sink> createSink(string const& format, string const& filename)
{
    if (format == "raw")
        return make_unique<pipeline::RawSink>(make_unique<ofstream>(filename));

    return nullptr;
}

int main(int argc, const char* argv[])
{
    flags::Flags cli;
    cli.defineString("input-format", 'I', "FORMAT", "Specifies which format the input stream has.");
    cli.defineString("input-file", 'i', "PATH", "Specifies the path to the input file to read from.");
    cli.defineString("output-format", 'O', "FORMAT", "Specifies which format the output stream will be.");
    cli.defineString("output-file", 'o', "PATH", "Specifies the path to the output file to write to.");
    cli.defineBool("help", 'h', "Shows this help.");

    if (error_code ec = cli.tryParse(argc, argv); ec)
    {
        cerr << "Failed to parse command line arguments. " << ec.message() << endl;
        return EXIT_FAILURE;
    }
    else if (cli.isSet("help"))
    {
        string const static header =
            "convert - command line tool for converting some file formats.\n"
            "Copyright (c) 2019 by Christian Parpart and Kei Thoma.\n\n";
        cout << cli.helpText(header, getTerminalWidth(STDOUT_FILENO), 32);
        return EXIT_SUCCESS;
    }
    else
    {
        auto const inputFile = cli.getString("input-file");
        auto const inputFormat = cli.getString("input-format");
        auto const outputFile = cli.getString("output-file");
        auto const outputFormat = cli.getString("output-format");

        auto source = createSource(inputFormat, inputFile);
        auto sink = createSink(outputFormat, outputFile);

        auto buffer = pipeline::Buffer{};
        buffer.reserve(4096);

        while (source->read(buffer))
        {
			sink->write(buffer.data(), buffer.size());
            buffer.clear();
        }

        // ---
#if 0
        huffman::Node const root = huffman::encode("aaaabcadaaabbaaaacaabaadc");

        cout << to_string(root) << "\n\n";
        cout << huffman::to_dot(root) << "\n\n";

        huffman::CodeTable const codes = huffman::encode(root);
        cout << huffman::to_string(codes) << "\n";
#endif
    }

    return EXIT_SUCCESS;
}
