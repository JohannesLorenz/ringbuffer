#
# General
#

#Specify the version being used aswell as the language
cmake_minimum_required(VERSION 2.8)

#Compiler
set(WARN_EXC "")

if(NOT COMPILER_SET STREQUAL "1")
if(COMPILER STREQUAL "gcc")
	set(CMAKE_CXX_FLAGS_RELEASE "-flto")
	set(FLTO_FLAGS "-flto")

	SET( CMAKE_CXX_COMPILER "/usr/bin/g++" ) # the user bin is a dumb fix for a cmake bug
	SET( CMAKE_CC_COMPILER "/usr/bin/gcc" )
elseif(COMPILER STREQUAL "clang")
	set(FLTO_FLAGS "") # not supported by clang on most platforms

	SET( CMAKE_CXX_COMPILER "/usr/bin/clang++" )
	SET( CMAKE_CC_COMPILER "/usr/bin/clang" )
elseif(COMPILER STREQUAL "stoat")
	set(FLTO_FLAGS "") # not supported

	SET( CMAKE_CXX_COMPILER "/usr/local/bin/stoat-compile++" )
	SET( CMAKE_CC_COMPILER "/usr/local/bin/stoat-compile" )
else()
	message(FATAL_ERROR "Please specify -DCOMPILER=[gcc|clang|stoat]")
endif()
endif()

#Name your project here
project(ringbuffer)

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "0")
SET(VERSION_PATCH "0")
SET(VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

MESSAGE("")
MESSAGE("WELCOME")
MESSAGE("This is the Makefile for `${PROJECT_NAME} ${VERSION}'")
MESSAGE("")
MESSAGE("READING CONFIGURATION...")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# help
# TODO: $myproject from project()
# MESSAGE("This is the Makefile for ${MYPROJECT} (version ${VERSION}).")
# MESSAGE("Options: (-D...=..., cmake -L for details)")
# MESSAGE(" * COMPILER: ${COMPILER} (allowed: [gcc|clang])")

SET(COMPILER "" CACHE STRING "Compiler to use (allowed: [gcc|clang|stoat])")

# testing
enable_testing ()

#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/res/")
SET(src_base "${CMAKE_CURRENT_SOURCE_DIR}/src/")
#SET(res_dir "${src_base}res/")

function(cp_script script_name)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${script_name}
		${CMAKE_CURRENT_BINARY_DIR}/${script_name} COPYONLY)
endfunction()

include_directories("${CMAKE_CURRENT_BINARY_DIR}/src/") # for config.h
#include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/include")

#
# options
#

option(USE_MLOCK "provide the mlock system call" ON)

#
# special headers
#

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/config.h.in
	${CMAKE_CURRENT_BINARY_DIR}/src/config.h)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/version.h.in
	${CMAKE_CURRENT_BINARY_DIR}/src/version.h)

MESSAGE("CHECKING FOR LIBRARIES...")

#TODO!!!
#find_package(PkgConfig REQUIRED)

#
# Compile
#

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
	message(FATAL_ERROR "Please set -DCMAKE_BUILD_TYPE=[Debug|Release]")
endif()
endif()

# FLAGS
# the order of -Wall -Wno-unknown-pragmas is important
# TODO: -Werror
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Werror ${WARN_EXC} -std=c++11 -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 ${FLTO_FLAGS}")

MESSAGE("READING SUB-DIRECTORIES...")
#add_subdirectory(rtosc)
add_subdirectory(src)
add_subdirectory(doc)

add_custom_target(ringbuffer_stoat stoat -c -w /usr/local/share/stoat/whitelist.txt -b /usr/local/share/stoat/blacklist.txt -w ${CMAKE_CURRENT_SOURCE_DIR}/data/stoat_suppression_list.txt -r ${CMAKE_CURRENT_BINARY_DIR}/src/)

#
# CPack Stuff
#

SET(MY_NAME_CAP ${PROJECT_NAME})
#SET(MY_NAME_LOW ${PROJECT_NAME}) # (TODO: tolower)
STRING(TOLOWER ${MY_NAME_CAP} MY_NAME_LOW)
#SET(MY_PACKAGE_FILE_NAME "${MY_NAME_LOW}-${VERSION}-${OS}-${MY_HOST_STR}")
SET(MY_PACKAGE_FILE_NAME "${MY_NAME_LOW}-${VERSION}")
SET(MY_SRC_PACKAGE_FILE_NAME "${MY_NAME_LOW}-${VERSION}")

SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${MY_NAME_CAP} - a lock free, multi-reader ringbuffer lib")
IF(WIN32)
SET(CPACK_PACKAGE_EXECUTABLES "${MY_NAME_LOW}.exe")
ELSE(WIN32)
SET(CPACK_PACKAGE_EXECUTABLES "${MY_NAME_LOW}")
SET(CPACK_STRIP_FILES "bin/${MY_NAME_LOW}") # TODO: bin?
ENDIF(WIN32)
SET(CPACK_PACKAGE_FILE_NAME ${MY_PACKAGE_FILE_NAME})
SET(CPACK_PACKAGE_INSTALL_DIRECTORY "${MY_SRC_PACKAGE_FILE_NAME}")
SET(CPACK_PACKAGE_VENDOR "${MY_NAME_CAP} Developers")
SET(CPACK_PACKAGE_VERSION ${VERSION})
SET(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
SET(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
SET(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.txt")
SET(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.txt")
SET(CPACK_SOURCE_GENERATOR "TBZ2")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${MY_SRC_PACKAGE_FILE_NAME}")
INCLUDE(CPack)

#
# Summary
#

MESSAGE("")
MESSAGE("SUMMARY:")
MESSAGE(" * Project: ${PROJECT_NAME} ${VERSION}")
MESSAGE(" * Compilers Used: ${COMPILER} (${CMAKE_CXX_COMPILER}, ${CMAKE_CC_COMPILER})")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(MSG_BUILD_TYPE_FLAG ${CMAKE_CXX_FLAGS_DEBUG})
else()
	set(MSG_BUILD_TYPE_FLAG ${CMAKE_CXX_FLAGS_RELEASE})
endif()
MESSAGE(" * Build Type: ${CMAKE_BUILD_TYPE} (${MSG_BUILD_TYPE_FLAG})")
MESSAGE(" * Building Doc: No - Type make doc if you want")
MESSAGE(" * Executing Tests: No - Type make test if you want")
MESSAGE("")
MESSAGE("Now, type make and pray...")
MESSAGE("")

