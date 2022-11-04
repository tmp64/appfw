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
	option(APPFW_ENABLE_NETWORK "appfw: Enable network" ON)
	option(APPFW_ENABLE_GLM "appfw: Enable GLM support" OFF)
	
	if(APPFW_ENABLE_NETWORK)
		option(APPFW_ENABLE_EXTCON "appfw: Enable External Console support (requires networking)" OFF)
	endif()
	
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
		<type_traits>
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
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(NOT APPFW_DONT_DOWNLOAD_DEPS)
	# fmtlib
	FetchContent_Declare(
		fmt_fetch
		GIT_REPOSITORY "https://github.com/fmtlib/fmt.git"
		GIT_TAG "8.0.0"
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
	include/appfw/filesystem.h
	include/appfw/init.h
	include/appfw/platform.h
	include/appfw/prof.h
	include/appfw/sha256.h
	include/appfw/span.h
	include/appfw/str_utils.h
	include/appfw/timer.h
	include/appfw/unique_function.h
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
	src/filesystem.cpp
	src/prof.cpp
	src/sha256.cpp
	src/span.natvis
	src/utils.cpp
)

if(APPFW_ENABLE_NETWORK)
	set(SOURCE_FILES
		${SOURCE_FILES}
		include/appfw/network/datagram_parser.h
		include/appfw/network/ip_address.h
		include/appfw/network/sock_addr.h
		include/appfw/network/socket.h
		include/appfw/network/tcp_client4.h
		include/appfw/network/tcp_server4.h
		
		src/network/datagram_parser.cpp
		src/network/plat_sockets.cpp
		src/network/plat_sockets.h
		src/network/sock_addr.cpp
		src/network/socket.cpp
		src/network/tcp_client4.cpp
		src/network/tcp_server4.cpp
	)
	
	if(APPFW_ENABLE_EXTCON)
		set(SOURCE_FILES
			${SOURCE_FILES}
			include/appfw/extcon/consts.h
			include/appfw/extcon/extcon_host.h

			src/extcon/extcon_host.cpp
		)
	endif()
	
	if(PLATFORM_WINDOWS)
		set(NETOWRK_LIBS
			Ws2_32.lib
		)
	endif()
endif()

set(GLM_LIBS)
set(GLM_DEFS APPFW_GLM=0)
if (APPFW_ENABLE_GLM)
	set(GLM_LIBS glm::glm)
	set(GLM_DEFS APPFW_GLM=1 GLM_FORCE_SILENT_WARNINGS)
endif()

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

target_compile_definitions(appfw PUBLIC ${PLATFORM_DEFINES} ${GLM_DEFS})
target_compile_definitions(appfw PRIVATE ${APPFW_PRIVATE_DEFS})

target_link_libraries(appfw
	fmt
	${NETOWRK_LIBS}
	${GLM_LIBS}
	Threads::Threads
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
		tests/src/binary_buffer.cpp
		tests/src/binary_stream.cpp
		tests/src/cmd_string.cpp
		tests/src/command_line.cpp
		tests/src/filesystem.cpp
		tests/src/main.cpp
		tests/src/platform.cpp
		tests/src/utils.cpp
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
	
	add_test(
		NAME appfw_test
		COMMAND appfw_test_exec
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests/workdir"
	)
endif()

# appfw examples
if(APPFW_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
