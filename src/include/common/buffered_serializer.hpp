//===----------------------------------------------------------------------===//
//                         DuckDB
//
// common/buffered_serializer.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "common/serializer.hpp"

namespace duckdb {

#define SERIALIZER_DEFAULT_SIZE 1024

struct BinaryData {
	unique_ptr<data_t[]> data;
	index_t size;
};

class BufferedSerializer : public Serializer {
public:
	//! Serializes to a buffer allocated by the serializer, will expand when
	//! writing past the initial threshold
	BufferedSerializer(index_t maximum_size = SERIALIZER_DEFAULT_SIZE);
	//! Serializes to a provided (owned) data pointer
	BufferedSerializer(unique_ptr<data_t[]> data, index_t size);
	// //! Serializes to a provided non-owned data pointer, bounds on writing are
	// //! not checked
	void Write(const_data_ptr_t buffer, index_t write_size) override;

	//! Retrieves the data after the writing has been completed
	BinaryData GetData() {
		return std::move(blob);
	}

public:
	index_t maximum_size;
	data_ptr_t data;

	BinaryData blob;
};

} // namespace duckdb
