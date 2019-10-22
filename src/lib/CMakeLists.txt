set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")

set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")
file(GLOB ringbuffer_lib_src ${src_dir}/*.cpp)
file(GLOB ringbuffer_lib_hdr ${src_dir}../../include/ringbuffer/*.h)

add_library(ringbuffer SHARED ${ringbuffer_lib_src} ${ringbuffer_lib_hdr})

include(GenerateExportHeader)
generate_export_header(ringbuffer)
target_include_directories(ringbuffer PUBLIC
		# src, for config.h
		"${CMAKE_CURRENT_BINARY_DIR}/../"
		# root/include for lib headers
		"${CMAKE_CURRENT_SOURCE_DIR}/../../include"
		# this bin dir for the export header.
		"${CMAKE_CURRENT_BINARY_DIR}"
)

install(TARGETS ringbuffer
	LIBRARY DESTINATION ${INSTALL_LIB_DIR}
	ARCHIVE DESTINATION ${INSTALL_LIB_DIR}
	)
#install(FILES ${lib_hdr}
#	DESTINATION "${INSTALL_INC_DIR}/ringbuffer")
