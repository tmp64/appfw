appfw
==============
C++ framework for console applications


Features
--------------
- Console system with console variables and commands
- Command line arguments parsing
- Virtual file system with search in multiple directories
- Debugging and development utilities (`AFW_ASSERT`, `appfw::Timer`, `appfw/utils.h`, etc)
- TCP networking (optional)


Dependencies
--------------
- C++17
- [fmt](https://github.com/fmtlib/fmt) (auto-downloaded)
- [doctest](https://github.com/onqtam/doctest) (included in repo, only needed for tests)


Usage (CMake-only)
--------------
1. Add the library as a submodule
2. Set the options
   - `APPFW_BUILD_EXAMPLES`: whether or not build examples (default: off)
   - `APPFW_ENABLE_NETWORK`: whether or not enable networking (default: off)
   - `APPFW_BUILD_TESTS`: whether or not build tests (requires BUILD_TESTING, default: off)
3. CMakeLists:
   ```cmake
   add_subdirectory(appfw)
   ...
   add_executable(my_app ...)
   appfw_module(my_app) # This generates this_module_info.h with moudle's name. Used for logging
   target_link_libraries(my_app appfw) # Links with appfw and its dependencies
   
   # If you want to use a PCH (recommended)
   appfw_get_std_pch(PCH_STD_HEADERS)
   appfw_get_pch(PCH_APPFW_HEADERS)
   target_precompile_headers(hello_world PRIVATE
       ${PCH_STD_HEADERS}
       ${PCH_APPFW_HEADERS}
   )
   ```
4. See `examples/hello_world/main.cpp` on how to initialize and use the library.<br>
   See `<appfw/appfw.h>` for common features.
