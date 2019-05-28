#include "pipeline.hpp"
#include "huffman.hpp"

#include <cmath>
#include <iostream>

using namespace std;

namespace pipeline {

size_t RawSource::read(Buffer& target)
{
    size_t const count = abs(static_cast<long>(target.capacity() - target.size()));
    source_.read(reinterpret_cast<char*>(target.data() + target.size()), count);
    return count;
}

void RawSink::write(uint8_t const* data, size_t count)
{
    target_.write(reinterpret_cast<char const*>(data), count);
}

}  // namespace pipeline