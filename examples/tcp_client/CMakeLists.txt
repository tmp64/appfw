add_executable(tcp_client
	CMakeLists.txt
	main.cpp
)

appfw_module(tcp_client)
target_link_libraries(tcp_client appfw)

appfw_get_std_pch(PCH_STD_HEADERS)
appfw_get_pch(PCH_APPFW_HEADERS)
target_precompile_headers(tcp_client PRIVATE
	${PCH_STD_HEADERS}
	${PCH_APPFW_HEADERS}
)
