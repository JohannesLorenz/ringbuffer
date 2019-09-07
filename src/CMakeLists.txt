configure_file("${CMAKE_CURRENT_SOURCE_DIR}/ringbuffer.pc.in"
	"${CMAKE_CURRENT_BINARY_DIR}/ringbuffer.pc" @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/ringbuffer.pc"
	DESTINATION "${INSTALL_LIB_DIR}/pkgconfig")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/version.h"
		"${CMAKE_CURRENT_BINARY_DIR}/config.h"
		DESTINATION "${INSTALL_INC_DIR}/ringbuffer")
# ^ TODO: into common files

add_subdirectory(lib)
if(CAN_TEST)
    add_subdirectory(test)
endif()

