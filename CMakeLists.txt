 cmake_minimum_required(VERSION 2.8.7)
project(DuckDB)

find_package(Threads REQUIRED)


SET(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

SET(CMAKE_VERBOSE_MAKEFILE OFF)
SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
SET(CMAKE_MACOSX_RPATH 1)

SET(M32_FLAG "")
IF (FORCE_32_BIT)
SET(M32_FLAG " -m32 ")
ENDIF()

IF(NOT MSVC)
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0 -DDEBUG -Wall ${M32_FLAG}")

	SET(CXX_EXTRA_DEBUG "-fsanitize=address -Wunused-variable -Wunused-const-variable -Werror=vla -Wnarrowing")

	IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 8.0)
		SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_EXTRA_DEBUG}")
	ELSEIF ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
		SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_EXTRA_DEBUG}")
	ELSE()
	    message(WARNING "Please use a recent compiler for debug builds")
	ENDIF()
ELSE()
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4244") # equivalent to -Wnarrowing
ENDIF()


# todo use CHECK_CXX_COMPILER_FLAG(-fsanitize=address SUPPORTS_SANITIZER) etc.

SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3 ${M32_FLAG}")
SET(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE}")



SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
SET(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")



IF (NOT CMAKE_BUILD_TYPE)
    message(FATAL_ERROR "Choose the type of build by setting -DCMAKE_BUILD_TYPE=<type>, options are: Debug Release")
ENDIF()

include_directories(src/include)
include_directories(third_party/hyperloglog)
include_directories(third_party/re2)
include_directories(third_party/miniz)

# todo only regenerate ub file if one of the input files changed
# hack alert
function(enable_unity_build UB_SUFFIX SOURCE_VARIABLE_NAME)
  set(files ${${SOURCE_VARIABLE_NAME}})
  
  # Generate a unique filename for the unity build translation unit
  set(unit_build_file ${CMAKE_CURRENT_BINARY_DIR}/ub_${UB_SUFFIX}.cpp)
  # Exclude all translation units from compilation
  set_source_files_properties(${files} PROPERTIES HEADER_FILE_ONLY true)
  
  SET(rebuild FALSE)
  foreach(source_file ${files} )
  	if(${CMAKE_CURRENT_SOURCE_DIR}/${source_file} IS_NEWER_THAN ${unit_build_file})
  		SET(rebuild TRUE)
  	endif()
  endforeach(source_file)
  
  if (${rebuild})
	  FILE(WRITE ${unit_build_file} "// Unity Build generated by CMake\n")
	  foreach(source_file ${files} )
	    FILE( APPEND ${unit_build_file} "#line 0 \"${source_file}\"\n#include <${CMAKE_CURRENT_SOURCE_DIR}/${source_file}>\n")
	  endforeach(source_file)
  endif()
  
  # Complement list of translation units with the name of ub
  set(${SOURCE_VARIABLE_NAME} ${${SOURCE_VARIABLE_NAME}} ${unit_build_file} PARENT_SCOPE)  
endfunction(enable_unity_build)

function(add_library_unity NAME MODE)
  SET(SRCS ${ARGN})
 if (NOT DISABLE_UNITY)
  enable_unity_build(${NAME} SRCS)
 endif()
  add_library(${NAME} OBJECT ${SRCS})
endfunction()


add_subdirectory(src)
add_subdirectory(third_party)

IF (NOT LEAN)
	add_subdirectory(test)
	IF (NOT WIN32)
	  add_subdirectory(tools)
	  add_subdirectory(benchmark)
	ENDIF()
ENDIF()
