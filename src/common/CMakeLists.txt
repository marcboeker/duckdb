add_subdirectory(enums)
add_subdirectory(operator)
add_subdirectory(statistics_operations)
add_subdirectory(types)
add_subdirectory(value_operations)
add_subdirectory(vector_operations)


add_library_unity(duckdb_common OBJECT buffered_serializer.cpp buffered_deserializer.cpp constants.cpp file_system.cpp gzip_stream.cpp limits.cpp printer.cpp serializer.cpp string_util.cpp symbols.cpp types.cpp fstream_util.cpp)
set(ALL_OBJECT_FILES ${ALL_OBJECT_FILES} $<TARGET_OBJECTS:duckdb_common> PARENT_SCOPE)
