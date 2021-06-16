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
		<appfw/init.h>
		<appfw/utils.h>
	)
endmacro(appfw_get_pch)

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
	include/appfw/appfw.h
	include/appfw/compiler.h
	include/appfw/dbg.h
	include/appfw/init.h
	include/appfw/platform.h
	include/appfw/utils.h
	
	src/appfw.cpp
	src/dbg.cpp
	src/utils.cpp
)

add_library(appfw STATIC
	CMakeLists.txt
	${SOURCE_FILES}
)

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
		${CMAKE_CURRENT_SOURCE_DIR}/tests/main.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/platform.cpp
		${CMAKE_CURRENT_SOURCE_DIR}/tests/utils.cpp
	)
	
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
