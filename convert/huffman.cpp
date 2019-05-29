#include "huffman.hpp"
#include "utils.hpp"

#include <algorithm>
#include <map>
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
    for (size_t i = i; i < 256; ++i)
    {
        if (i++)
            os << ", ";

        if (isprint(i))
			os << '{' << static_cast<char>(i) << ": ";
        else
			os << '{' << i << ": ";

        for (bool bit : codes[i])
            os << (bit ? '1' : '0');

        os << '}';
    }
    return os.str();
}

vector<uint8_t> to_bytes(vector<bool> const& bits, size_t count)
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

void encode(Node const& root, CodeTable& result, vector<bool> state)
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

// helper method for streaming Huffman tree in dot-file format (see graphviz) into an output stream.
static void write_dot(ostream& os, Node const& n)
{
    // TODO: print edge labels (with "0" and "1")
    if (holds_alternative<Branch>(n))
    {
        auto const& br = get<Branch>(n);
        os << "\"-(" << br.frequency << ")\" -> " << to_dot(*br.left);
        os << "\"-(" << br.frequency << ")\" -> " << to_dot(*br.right);
    }
    else
    {
        auto const& leaf = get<Leaf>(n);
        os << '"' << leaf.ch << "(" << leaf.frequency << ")\";\n";
    }
}

string to_dot(Node const& root)
{
    std::stringstream out;

    out << "digraph D {\n";
    write_dot(out, root);
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
    if (data.empty())
        return {};

    auto static const smallestFreq = [](auto const& a, auto const& b) { return a.second < b.second; };

    // collect frequencies
    map<uint8_t, unsigned> freqs;
    for_each(begin(data), end(data), [&](uint8_t c) { freqs[c]++; });

    // create initial root
    Node root = [&]() {
        auto a = ranges::min_element(freqs, smallestFreq);
        auto a_ = Leaf{a->first, a->second};
        freqs.erase(a);

        auto b = ranges::min_element(freqs, smallestFreq);
        if (b == end(freqs))
            return Node{a_};

        auto b_ = Leaf{b->first, b->second};
        freqs.erase(b);
        return Node{Branch{make_unique<Node>(a_), make_unique<Node>(b_), a_.frequency + b_.frequency}};
    }();

    if (freqs.empty())
        return move(root);

    // eliminate remaining frequencies and build tree bottom-up
    // NB: It is guaranteed that root is a Branch (not a Leaf).
    do
    {
        auto a = ranges::min_element(freqs, smallestFreq);
        Leaf a_{a->first, a->second};
        auto const frequency = a_.frequency + get<Branch>(root).frequency;
        freqs.erase(a);
        root = a_.frequency < get<Branch>(root).frequency
                   ? Branch{make_unique<Node>(a_), make_unique<Node>(move(root)), frequency}
                   : Branch{make_unique<Node>(move(root)), make_unique<Node>(a_), frequency};
    } while (!freqs.empty());

    return move(root);
}

}  // namespace huffman
