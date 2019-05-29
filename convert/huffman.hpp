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
    uint8_t ch;
    unsigned frequency;
};

struct Branch {
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    unsigned frequency;
};

using CodeTable = std::array<std::vector<bool>, 256>;

std::vector<uint8_t> to_bytes(std::vector<bool> const& bits, size_t count);

inline std::vector<uint8_t> to_bytes(std::vector<bool> const& bits)
{
    return to_bytes(bits, bits.size());
}

/**
  * Translates Huffman tree into a linear coding table.
  */
CodeTable encode(Node const& root);

/**
 * Retrieves the online string representation of the Huffman code table @p codes.
 */
std::string to_string(CodeTable const& codes);

/**
 * Retrieves the dot-file format representation (see graphviz) of the Huffman tree @p root.
 */
std::string to_dot(Node const& root);

/**
 * Serializes given Huffman tree @p root into a oneline string.
 */
std::string to_string(Node const& root);

/**
 * Encodes given arbitrary input @p data into a Huffman tree.
 */
Node encode(std::vector<uint8_t> const& data);

}  // namespace huffman
