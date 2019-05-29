#include "pipeline.hpp"
#include "huffman.hpp"

#include <sgfx/image.hpp>
#include <sgfx/ppm.hpp>

#include <cmath>
#include <iostream>

using namespace std;

namespace pipeline {

void Filter::applyFilters(const list<unique_ptr<Filter>>& filters, const Buffer& input, Buffer* output,
                          bool last)
{
    auto i = filters.begin();
    auto e = filters.end();

    if (i == e)
    {
        *output = input;
        return;
    }

    (*i)->filter(input, output, last);
    i++;
    Buffer tmp;
    while (i != e)
    {
        tmp.swap(*output);
        (*i)->filter(tmp, output, last);
        i++;
    }
}

// -------------------------------------------------------------------------
// XXX maybe going to be deleted

RawSource::RawSource(std::unique_ptr<istream> owned) : owned_{move(owned)}, source_{*owned_}
{
    if (!source_.good())
        throw std::runtime_error("Could not open stream.");
}

RawSource::RawSource(std::istream& source) : owned_{}, source_{source}
{
    if (!source_.good())
        throw std::runtime_error("Could not open file.");
}

size_t RawSource::read(Buffer& target)
{
	auto const start = target.size();
    auto const count = static_cast<size_t>(target.capacity() - target.size());

	// There seems to be an optimization that the reserve() call's underlying malloc
	// is deferred, so we have to force it here.
    target.resize(start + count);

    source_.read(reinterpret_cast<char*>(target.data() + start), count);
    auto const nread = source_.gcount();

    target.resize(start + nread);

    return nread;
}

void RawSink::write(uint8_t const* data, size_t count)
{
    target_.write(reinterpret_cast<char const*>(data), count);
}

void PPMDecoder::filter(const Buffer& input, Buffer* output, bool last)
{
}

}  // namespace pipeline