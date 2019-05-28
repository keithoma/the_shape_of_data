#include "pipeline.hpp"
#include "huffman.hpp"

#include <cmath>
#include <iostream>

using namespace std;

namespace pipeline {

void Filter::applyFilters(const list<unique_ptr<Filter>>& filters, const Buffer& input,
                          Buffer* output, bool last)
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

RawSource::RawSource(std::unique_ptr<std::istream> owned) : owned_{move(owned)}, source_{*owned_}
{
	if (!source_.good())
		throw std::runtime_error("Could not open file.");
}

RawSource::RawSource(std::istream& source) : owned_{}, source_{source}
{
	if (!source_.good())
		throw std::runtime_error("Could not open file.");
}

size_t RawSource::read(Buffer& target)
{
    auto const count = abs(static_cast<long>(target.capacity() - target.size()));
    auto const pos0 = source_.tellg();
    source_.read(reinterpret_cast<char*>(target.data() + target.size()), count);
    auto const pos1 = source_.tellg();
    auto const nread = pos1 - pos0;
    target.resize(target.size() + nread);
    return nread;
}

void RawSink::write(uint8_t const* data, size_t count)
{
    target_.write(reinterpret_cast<char const*>(data), count);
}

}  // namespace pipeline