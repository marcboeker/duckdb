//===----------------------------------------------------------------------===//
//                         DuckDB
//
// storage/data_table.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "common/enums/index_type.hpp"
#include "common/types/data_chunk.hpp"
#include "common/types/tuple.hpp"
#include "storage/column_statistics.hpp"
#include "storage/index.hpp"
#include "storage/storage_chunk.hpp"
#include "storage/table_statistics.hpp"
#include "storage/unique_index.hpp"

#include <atomic>
#include <mutex>
#include <vector>

namespace duckdb {
class ClientContext;
class ColumnDefinition;
class StorageManager;
class TableCatalogEntry;
class Transaction;

struct VersionInformation;

struct ScanStructure {
	StorageChunk *chunk;
	index_t offset;
	VersionInformation *version_chain;
	vector<unique_ptr<StorageLock>> locks;
};

//! DataTable represents a physical table on disk
class DataTable {
	friend class UniqueIndex;

public:
	DataTable(StorageManager &storage, string schema, string table, vector<TypeId> types);

	void InitializeScan(ScanStructure &structure);
	//! Scans up to STANDARD_VECTOR_SIZE elements from the table starting
	// from offset and store them in result. Offset is incremented with how many
	// elements were returned.
	void Scan(Transaction &transaction, DataChunk &result, const vector<column_t> &column_ids,
	          ScanStructure &structure);
	//! Fetch data from the specific row identifiers from the base table
	void Fetch(Transaction &transaction, DataChunk &result, vector<column_t> &column_ids, Vector &row_ids);
	//! Append a DataChunk to the table. Throws an exception if the columns
	// don't match the tables' columns.
	void Append(TableCatalogEntry &table, ClientContext &context, DataChunk &chunk);
	//! Delete the entries with the specified row identifier from the table
	void Delete(TableCatalogEntry &table, ClientContext &context, Vector &row_ids);
	//! Update the entries with the specified row identifier from the table
	void Update(TableCatalogEntry &table, ClientContext &context, Vector &row_ids, vector<column_t> &column_ids,
	            DataChunk &data);

	//! Scan used for creating an index, incrementally locks all storage chunks
	void CreateIndexScan(ScanStructure &structure, vector<column_t> &column_ids, DataChunk &result);

	//! Get statistics of the specified column
	ColumnStatistics &GetStatistics(column_t oid) {
		if (oid == COLUMN_IDENTIFIER_ROW_ID) {
			return rowid_statistics;
		}
		return statistics[oid];
	}

	//! The amount of elements in the table. Note that this number signifies the amount of COMMITTED entries in the
	//! table. It can be inaccurate inside of transactions. More work is needed to properly support that.
	std::atomic<index_t> cardinality;
	//! Total per-tuple size of the table
	index_t tuple_size;
	//! Accumulative per-tuple size
	vector<index_t> accumulative_tuple_size;

	// schema of the table
	string schema;
	// name of the table
	string table;
	//! Types managed by data table
	vector<TypeId> types;

	//! Tuple serializer for this table
	TupleSerializer serializer;
	//! A reference to the base storage manager
	StorageManager &storage;

	StorageChunk *GetChunk(index_t row_number);

	//! Unique indexes
	vector<unique_ptr<UniqueIndex>> unique_indexes;

	//! Indexes
	vector<unique_ptr<Index>> indexes;

	//! Retrieves versioned data from a set of pointers to tuples inside an
	//! UndoBuffer and stores them inside the result chunk; used for scanning of
	//! versioned tuples
	void RetrieveVersionedData(DataChunk &result, const vector<column_t> &column_ids,
	                           data_ptr_t alternate_version_pointers[], index_t alternate_version_index[],
	                           index_t alternate_version_count);
	//! Retrieves data from the base table for use in scans
	void RetrieveBaseTableData(DataChunk &result, const vector<column_t> &column_ids, sel_t regular_entries[],
	                           index_t regular_count, StorageChunk *current_chunk, index_t current_offset = 0);

private:
	//! Verify whether or not a new chunk violates any constraints
	void VerifyConstraints(TableCatalogEntry &table, ClientContext &context, DataChunk &new_chunk);
	//! The stored data of the table
	unique_ptr<StorageChunk> chunk_list;
	//! A reference to the last entry in the chunk list
	StorageChunk *tail_chunk;
	//! The table statistics
	TableStatistics table_statistics;
	//! Row ID statistics
	ColumnStatistics rowid_statistics;
	//! The statistics of each of the columns
	unique_ptr<ColumnStatistics[]> statistics;
};
} // namespace duckdb
