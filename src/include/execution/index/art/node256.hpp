//===----------------------------------------------------------------------===//
//                         DuckDB
//
// execution/index/art/node256.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "node.hpp"

namespace duckdb {

class Node256 : public Node {
public:
	unique_ptr<Node> child[256];
	Node256(uint8_t maxPrefixLength) : Node(NodeType::N256, maxPrefixLength) {
	}

	//! Get Node256 Child
	unique_ptr<Node> *getChild(const uint8_t k);

	//! Get position of a byte, returns -1 if not exists
	int getPos(const uint8_t k);

	//! Get min value
	unique_ptr<Node> *getMin();

	//! Insert node From Node256
	static void insert(unique_ptr<Node> &node, uint8_t keyByte, unique_ptr<Node> &child);

	//! Shrink to node 48
	static void erase(unique_ptr<Node> &node, int pos);
};
} // namespace duckdb
