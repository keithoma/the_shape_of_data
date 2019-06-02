// This file is part of the "convert" project, http://github.com/keithoma>
//   (c) 2019 Kei Thoma <thomakei@gmail.com>
//   (c) 2019 Christian Parpart <christian@parpart.family>
//
// Licensed under the MIT License (the "License"); you may not use this
// file except in compliance with the License. You may obtain a copy of
// the License at: http://opensource.org/licenses/MIT

#include "huffman.hpp"
#include "bitstream.hpp"
#include "utils.hpp"

#include <algorithm>
#include <map>
#include <queue>
#include <sstream>
#include <utility>
#include <variant>
#include <vector>

#include <cassert>
#include <cctype>

using namespace std;

namespace huffman {

string to_string(CodeTable const& codes)
{
    stringstream os;
    for (unsigned i = 0; i < 256; ++i)
    {
        if (i++)
            os << ", ";

        if (isprint(i))
            os << '{' << static_cast<char>(i) << ": ";
        else
            os << '{' << static_cast<unsigned>(i) << ": ";

        for (bool bit : codes[i])
            os << (bit ? '1' : '0');

        os << '}';
    }
    return os.str();
}

vector<uint8_t> to_bytes(BitVector const& bits, size_t count)
{
    assert(count <= bits.size());

    vector<uint8_t> out;

    size_t i = 0;
    for (; i + 7 < count; i += 8)
        out.push_back((bits[i + 0] << 7) | (bits[i + 1] << 6) | (bits[i + 2] << 5) | (bits[i + 3] << 4)
                      | (bits[i + 4] << 3) | (bits[i + 5] << 2) | (bits[i + 6] << 1) | (bits[i + 7] << 0));

    if (i < count)
    {
        auto path = uint8_t{0};

        if (i < bits.size())
            path |= bits[i] << 7;

        if (++i < bits.size())
            path |= bits[i] << 6;

        if (++i < bits.size())
            path |= bits[i] << 5;

        if (++i < bits.size())
            path |= bits[i] << 4;

        if (++i < bits.size())
            path |= bits[i] << 3;

        if (++i < bits.size())
            path |= bits[i] << 2;

        if (++i < bits.size())
            path |= bits[i] << 1;

        // no `<< 0`, as this case was already covered in the while-loop

        out.push_back(path);
    }

    return out;
}

void encode(Node const& root, CodeTable& result, BitVector state)
{
    if (holds_alternative<Leaf>(root))
        result[get<Leaf>(root).ch] = state;
    else
    {
        state.push_back(false);
        encode(*get<Branch>(root).left, result, state);

        state.back() = true;
        encode(*get<Branch>(root).right, result, state);
    }
}

CodeTable encode(Node const& root)
{
    CodeTable result;
    encode(root, result, {});
    return result;
}

string label(Node const& n)
{
    char buf[128];
    if (holds_alternative<Leaf>(n))
    {
        Leaf const& leaf = get<Leaf>(n);
        if (isprint(leaf.ch) && leaf.ch != '"')
            snprintf(buf, sizeof(buf), "%c(%u)", leaf.ch, leaf.frequency);
        else
            snprintf(buf, sizeof(buf), "0x%02x(%u)", leaf.ch, leaf.frequency);
    }
    else
    {
        Branch const& br = get<Branch>(n);
        snprintf(buf, sizeof(buf), "-(%u)", br.frequency);
    }
    return string{buf};
}

// helper method for streaming Huffman tree in dot-file format (see graphviz) into an output stream.
static void write_dot(Node const& n, ostream& out, unsigned& nextId)
{
    auto const id = nextId;
    out << "    n" << id << " [label=\"" << label(n) << "\"];\n";

    if (holds_alternative<Branch>(n))
    {
        auto const& br = get<Branch>(n);

        auto const lhsId = ++nextId;
        write_dot(*br.left, out, nextId);

        auto const rhsId = ++nextId;
        write_dot(*br.right, out, nextId);

        out << "    n" << id << " -> n" << lhsId << " [label=\"0\"];\n";
        out << "    n" << id << " -> n" << rhsId << " [label=\"1\"];\n";
    }
}

string to_dot(Node const& root)
{
    std::stringstream out;

    out << "digraph D {\n";
    unsigned nextId = 0;
    write_dot(root, out, nextId);
    out << "}\n";

    return out.str();
}

string to_string(Node const& node)
{
    stringstream os;
    if (holds_alternative<Leaf>(node))
    {
        auto const& leaf = get<Leaf>(node);
        os << "{" << leaf.frequency << ": " << leaf.ch << "}";
    }
    else
    {
        auto const& br = get<Branch>(node);
        os << "{" << br.frequency << ": " << to_string(*br.left) << ", " << to_string(*br.right) << "}";
    }
    return os.str();
}

Node encode(vector<uint8_t> const& data)
{
	struct NodeGreater {
		bool operator()(Node const& a, Node const& b) const noexcept { return frequency(a) > frequency(b); }
	};

    if (data.empty())
        return {};

    // collect frequencies
    auto freqs = map<uint8_t, unsigned>{};
    for_each(begin(data), end(data), [&](uint8_t c) { freqs[c]++; });

	// feed queue
    priority_queue<Node, vector<Node>, NodeGreater> queue;
    for (auto&& [symbol, freq] : freqs)
        queue.push(Leaf{symbol, freq});

	// reduce queue
    while (queue.size() != 1)
    {
        auto left = make_shared<Node>(queue.top());
        queue.pop();

        auto right = make_shared<Node>(queue.top());
        queue.pop();

        queue.push(Branch{left, right, frequency(*left) + frequency(*right)});
    }

	return queue.top();
}

}  // namespace huffman
