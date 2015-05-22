set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB lib_src ${src_dir}/*.cpp)
file(GLOB lib_hdr ${src_dir}/*.h)

add_library(ringbuffer SHARED ${lib_src} ${lib_hdr})

install(TARGETS ringbuffer
	LIBRARY DESTINATION ${INSTALL_LIB_DIR})
install(FILES ${lib_hdr}
	DESTINATION "${INSTALL_INC_DIR}/ringbuffer")
