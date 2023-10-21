cmake_minimum_required(VERSION 3.5)

# settings
project(ringbuffer)
set(DESCRIPTION "a lock free, multi-reader ringbuffer lib")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "9")
SET(VERSION_PATCH "0")

# options
option(WANT_MLOCK "provide the mlock system call" ON)
option(RINGBUFFER_DO_CPACK "execute cpack" OFF)

# custom targets
add_custom_target(ringbuffer_stoat stoat -c -w /usr/local/share/stoat/whitelist.txt -b /usr/local/share/stoat/blacklist.txt -w ${CMAKE_CURRENT_SOURCE_DIR}/data/stoat_suppression_list.txt -r ${CMAKE_CURRENT_BINARY_DIR}/src/)

# processing
include(cmake/process_project.txt)
print_summary_base()

