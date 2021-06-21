cmake_minimum_required(VERSION 3.16)
project(appfw)

# Includes
include(CTest)
include(FetchContent)
include(cmake/platform_info.cmake)

# Set C++17 in this scope
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Check if project has parent scope
get_directory_property(APPFW_HAS_PARENT_SCOPE PARENT_DIRECTORY)

# Options
if(NOT APPFW_HAS_PARENT_SCOPE)
	option(APPFW_BUILD_EXAMPLES "appfw: Build examples" ON)
	
	if(BUILD_TESTING)
		option(APPFW_BUILD_TESTS "appfw: Build tests" ON)
	endif()
endif()

if(NOT BUILD_TESTING)
	set(APPFW_BUILD_TESTS OFF)
endif()

# Functions and macros

# Makes the target use latest supported C++ standart.
function(appfw_latest_cpp TARGET)
	set_target_properties(${TARGET} PROPERTIES
		CXX_STANDARD 17
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO
	)
endfunction(appfw_latest_cpp)

# Sets VAR_NAME to a list of common standard headers for use in a PCH.
macro(appfw_get_std_pch VAR_NAME)
	set(${VAR_NAME}
		# C headers
		<cassert>
		<cctype>
		<cerrno>
		<cmath>
		<cstddef>
		<cstdint>
		<cstdio>
		<cstdlib>
		<cstring>
		<ctime>
		
		# C++ headers
		<algorithm>
		<array>
		<atomic>
		<chrono>
		<condition_variable>
		<exception>
		<filesystem>
		<fstream>
		<functional>
		<future>
		<initializer_list>
		<limits>
		<list>
		<map>
		<memory>
		<mutex>
		<optional>
		<queue>
		<set>
		<stack>
		<stdexcept>
		<string>
		<string_view>
		<system_error>
		<thread>
		<tuple>
		<unordered_map>
		<unordered_set>
		<utility>
		<vector>
	)
endmacro(appfw_get_std_pch)

# Sets VAR_NAME to a list of appfw headers for use in a PCH.
macro(appfw_get_pch VAR_NAME)
	set(${VAR_NAME}
		<appfw/appfw.h>
	)
endmacro(appfw_get_pch)

# Generates module header file for a target
function(appfw_module TARGET)
	set(includeDir "${CMAKE_CURRENT_BINARY_DIR}/appfw/generated/${TARGET}")
	get_property(headerFile TARGET appfw PROPERTY APPFW_MODULE_HEADER_FILE)
	configure_file(
		"${headerFile}"
		"${includeDir}/this_module_info.h"
	)
	target_include_directories(${TARGET} BEFORE PRIVATE ${includeDir})
endfunction(appfw_module)

# Dependencies
if(NOT APPFW_DONT_DOWNLOAD_DEPS)
	# fmtlib
	FetchContent_Declare(
		fmt_fetch
		GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
		GIT_TAG "7.1.3"
		GIT_SHALLOW
	)
	
	message(STATUS ">>>>>> Dependencies begin")
	FetchContent_MakeAvailable(fmt_fetch)
	message(STATUS ">>>>>> Dependencies end")
endif()

add_subdirectory(external/doctest)

# appfw library
set(SOURCE_FILES
	include/appfw/console/con_item.h
	include/appfw/console/con_msg.h
	include/appfw/console/console_system.h
	include/appfw/console/std_console.h
	include/appfw/console/term_console.h
	include/appfw/appfw.h
	include/appfw/binary_buffer.h
	include/appfw/binary_file.h
	include/appfw/binary_stream.h
	include/appfw/cmd_buffer.h
	include/appfw/cmd_string.h
	include/appfw/command_line.h
	include/appfw/compiler.h
	include/appfw/dbg.h
	include/appfw/init.h
	include/appfw/platform.h
	include/appfw/sha256.h
	include/appfw/span.h
	include/appfw/timer.h
	include/appfw/utils.h
	include/appfw/windows.h
	
	src/console/con_item.cpp
	src/console/console_system.cpp
	src/console/std_console.cpp
	src/appfw.cpp
	src/binary_buffer.cpp
	src/binary_file.cpp
	src/binary_stream.cpp
	src/cmd_buffer.cpp
	src/cmd_string.cpp
	src/command_line.cpp
	src/dbg.cpp
	src/sha256.cpp
	src/utils.cpp
)

add_library(appfw STATIC
	CMakeLists.txt
	${SOURCE_FILES}
)

define_property(
	TARGET PROPERTY APPFW_MODULE_HEADER_FILE
	BRIEF_DOCS "Path to this_module_info.h.in"
	FULL_DOCS "Path to this_module_info.h.in"
)
set_property(TARGET appfw PROPERTY APPFW_MODULE_HEADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/src/this_module_info.h.in)

appfw_module(appfw)

target_include_directories(appfw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(appfw PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_definitions(appfw PUBLIC ${PLATFORM_DEFINES})
target_compile_definitions(appfw PRIVATE ${APPFW_PRIVATE_DEFS})

target_link_libraries(appfw
	fmt
)

appfw_get_std_pch(PCH_STD_HEADERS)
appfw_get_pch(PCH_APPFW_HEADERS)
target_precompile_headers(appfw PRIVATE
	${PCH_STD_HEADERS}
	${PCH_APPFW_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

# appfw tests
if(APPFW_BUILD_TESTS)
	# Add test executable
	add_executable(appfw_test_exec 
		${CMAKE_CURRENT_SOURCE_DIR}/tests/binary_buffer.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/binary_stream.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/cmd_string.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/command_line.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/platform.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/utils.cpp
	)
	
	appfw_module(appfw_test_exec)
	
	target_link_libraries(appfw_test_exec
		appfw
		doctest
	)
	
	# Prevent linker from removing "unused" objects with tests
	if(COMPILER_MSVC)
		target_link_options(appfw_test_exec PRIVATE "/OPT:NOREF")
	endif()
	
	add_test(appfw_test appfw_test_exec)
endif()

# appfw examples
if(APPFW_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
