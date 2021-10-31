cmake_minimum_required(VERSION 3.0)

# Define the project and executable
project(EasyCodeIt C)
add_executable(eci)

# Enable warnings
if(CMAKE_COMPILER_IS_GNUCC)
	target_compile_options(eci PRIVATE -Wall -Wpedantic -Wextra -Wshadow -Wno-maybe-uninitialized -Wno-parentheses)
	#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	#	message(STATUS "Adding address and undefined sanitzers")
	#	target_compile_options(eci PRIVATE -fsanitize=undefined,address)
	#endif()
endif()

# Add dynarr (dynamic array) library
add_subdirectory(dynarr EXCLUDE_FROM_ALL)

# Add jansson library for JSON support
set(JANSSON_INSTALL OFF)
set(JANSSON_BUILD_DOCS OFF)
add_subdirectory(jansson EXCLUDE_FROM_ALL)

# Generate the parser
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(lexer.c ${CMAKE_BINARY_DIR}/lexer.c)
FLEX_TARGET(Lexer parser/lexer.l ${lexer.c})
#add_library(lexer OBJECT ${lexer.c})
#target_include_directories(lexer PRIVATE ${CMAKE_SOURCE_DIR}/parser ${CMAKE_SOURCE_DIR})

set(parser.c ${CMAKE_BINARY_DIR}/parser.c)
BISON_TARGET(Parser parser/parser.y ${parser.c}
	DEFINES_FILE ${CMAKE_BINARY_DIR}/parser.tab.h
	COMPILE_FLAGS -Wcounterexamples
)
#add_library(parser OBJECT ${parser.c})

# Add sources to main executable
target_include_directories(eci PRIVATE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR}/jansson/include) # IDEA: Convert lexer into an OBJECT library with its own include directory
target_link_libraries(eci PRIVATE jansson)
target_sources(eci PRIVATE utils.c alloc/alloc.c cease/cease.c ${lexer.c} ${parser.c} eci.c)
