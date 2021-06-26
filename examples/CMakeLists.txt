add_subdirectory(hello_world)

if(APPFW_ENABLE_NETWORK)
	add_subdirectory(tcp_client)
	add_subdirectory(tcp_server)
endif()
