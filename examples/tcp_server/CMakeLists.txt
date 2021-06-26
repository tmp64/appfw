add_executable(tcp_server
	CMakeLists.txt
	main.cpp
)

appfw_module(tcp_server)
target_link_libraries(tcp_server appfw)

appfw_get_std_pch(PCH_STD_HEADERS)
appfw_get_pch(PCH_APPFW_HEADERS)
target_precompile_headers(tcp_server PRIVATE
	${PCH_STD_HEADERS}
	${PCH_APPFW_HEADERS}
)
