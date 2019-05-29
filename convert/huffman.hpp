#pragma once

#include <array>
#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace huffman {

struct Branch;
struct Leaf;
using Node = std::variant<Branch, Leaf>;

struct Leaf {
    uint8_t ch; // TODO: rename to symbol
    unsigned frequency;
};

struct Branch {
	// XXX shared_ptr instead of unique_ptr due to issues with priotity_queue otherwise
    std::shared_ptr<Node> left;
    std::shared_ptr<Node> right;
    unsigned frequency;
};

inline unsigned frequency(Node const& n) noexcept
{
    if (std::holds_alternative<Leaf>(n))
        return std::get<Leaf>(n).frequency;
    else
        return std::get<Branch>(n).frequency;
}

using BitVector = std::vector<bool>;
using CodeTable = std::array<BitVector, 256>;

/// Encodes given arbitrary input @p data into a Huffman tree.
Node encode(std::vector<uint8_t> const& data);

/// Translates Huffman tree into a linear coding table.
CodeTable encode(Node const& root);

/// Transforms first @p count elements of vector @p bits into a zero-padded vector of bytes.
std::vector<uint8_t> to_bytes(BitVector const& bits, size_t count);

/// Transforms vector of bits into a zero-padded vector of bytes.
inline std::vector<uint8_t> to_bytes(BitVector const& bits)
{
    return to_bytes(bits, bits.size());
}

/// Retrieves a human readable representational text of given node @p n, suitable for dot graph labeling.
std::string label(Node const& n);

/// Retrieves the online string representation of the Huffman code table @p codes.
std::string to_string(CodeTable const& codes);

/// Retrieves the dot-file format representation (see graphviz) of the Huffman tree @p root.
std::string to_dot(Node const& root);

/// Serializes given Huffman tree @p root into a oneline string.
std::string to_string(Node const& root);

}  // namespace huffman
