//===----------------------------------------------------------------------===//
//                         DuckDB
//
// execution/operator/scan/physical_empty_result.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "execution/physical_operator.hpp"

namespace duckdb {

class PhysicalEmptyResult : public PhysicalOperator {
public:
	PhysicalEmptyResult(vector<TypeId> types) : PhysicalOperator(PhysicalOperatorType::EMPTY_RESULT, types) {
	}

	void GetChunkInternal(ClientContext &context, DataChunk &chunk, PhysicalOperatorState *state) override;
};
} // namespace duckdb
