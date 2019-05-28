#pragma once

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
    char ch;
    unsigned frequency;
};

struct Branch {
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
    unsigned frequency;
};

using CodeTable = std::vector<std::pair<char, std::vector<bool>>>;

/**
  * @param root tree root of our huffman tree
  * @param state remembered bits until this root
  */
CodeTable encode(Node const& root, std::vector<bool> state = {});

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
Node encode(std::string const& data);

}
