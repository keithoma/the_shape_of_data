#include "huffman.hpp"
#include "utils.hpp"

#include <algorithm>
#include <map>
#include <sstream>
#include <vector>
#include <utility>
#include <variant>

using namespace std;

namespace huffman {

string to_string(CodeTable const& codes)
{
    stringstream os;
    int i = 0;
    for (auto const& code : codes)
    {
        if (i++)
            os << ", ";
        os << '{' << code.first << ": ";
        for (bool bit : code.second)
            os << (bit ? '1' : '0');
        os << '}';
    }
    return os.str();
}

CodeTable encode(Node const& root, vector<bool> state)
{
    vector<pair<char, vector<bool>>> result;

    if (holds_alternative<Leaf>(root))
        result.emplace_back(make_pair(get<Leaf>(root).ch, state));
    else
    {
        auto const& br = get<Branch>(root);

        state.push_back(false);
        auto x = encode(*br.left, state);
        result.insert(end(result), begin(x), end(x));

        state.back() = true;
        auto y = encode(*br.right, state);
        result.insert(end(result), begin(y), end(y));
    }

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

Node encode(string const& data)
{
    if (data.empty())
        return {};

    auto static const smallestFreq = [](auto const& a, auto const& b) { return a.second < b.second; };

    // collect frequencies
    map<char, unsigned> freqs;
    for_each(begin(data), end(data), [&](char c) { freqs[c]++; });

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
