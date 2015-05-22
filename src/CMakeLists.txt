configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ringbuffer.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/ringbuffer.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer.pc"
	DESTINATION "${INSTALL_LIB_DIR}/pkgconfig")

add_subdirectory(lib)
add_subdirectory(test)

