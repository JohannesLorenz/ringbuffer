set(src_dir "${CMAKE_CURRENT_SOURCE_DIR}")
set(incl_dir "${src_dir}/../lib")

include_directories("${incl_dir}")

# TODO: comfortable function for that
add_executable(test_seq test_seq.cpp)
target_link_libraries(test_seq ringbuffer)

find_package(Threads)

add_executable(test_par test_par.cpp)
target_link_libraries(test_par ${CMAKE_THREAD_LIBS_INIT} ringbuffer)

add_test(sequential test_seq)
add_test(parallel test_par)

