CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/ringbuffer-config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-config.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/ringbuffer-version.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-version.h)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ringbuffer.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/ringbuffer.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-version.h"
		"${CMAKE_CURRENT_BINARY_DIR}/ringbuffer-config.h"
		DESTINATION "${INSTALL_INC_DIR}/ringbuffer")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer.pc"
	DESTINATION "${INSTALL_LIB_DIR}/pkgconfig")
# ^ TODO: into common files

add_subdirectory(lib)
if(CAN_TEST)
    add_subdirectory(test)
endif()

