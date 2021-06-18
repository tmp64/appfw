add_executable(hello_world
	CMakeLists.txt
	main.cpp
)

appfw_module(hello_world)
target_link_libraries(hello_world appfw)

appfw_get_std_pch(PCH_STD_HEADERS)
appfw_get_pch(PCH_APPFW_HEADERS)
target_precompile_headers(hello_world PRIVATE
	${PCH_STD_HEADERS}
	${PCH_APPFW_HEADERS}
)
