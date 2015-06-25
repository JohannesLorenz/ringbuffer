set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB lib_hdr ${src_dir}/*.h)

install(FILES ${lib_hdr}
	DESTINATION "${INSTALL_INC_DIR}/ringbuffer")
