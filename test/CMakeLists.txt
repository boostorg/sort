set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # warning level 4
  add_compile_options(/W4)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT TARGET tests)
  add_custom_target(tests)
endif()

set(PREFIX "boost_sort_")
set(LINK_LIBRARIES Boost::sort Boost::included_test_exec_monitor)

function(boost_sort_add_test name source)
  set(pname "${PREFIX}${name}")
  add_executable(${pname} ${source})
  target_link_libraries(${pname} ${LINK_LIBRARIES})
  add_test(NAME ${pname} COMMAND ${pname})
  add_dependencies(tests ${pname})
endfunction()

boost_sort_add_test(float_sort_test float_sort_test.cpp)
boost_sort_add_test(integer_sort_test integer_sort_test.cpp)
boost_sort_add_test(sort_detail_test sort_detail_test.cpp)
boost_sort_add_test(string_sort_test string_sort_test.cpp)
boost_sort_add_test(test_block_indirect_sort test_block_indirect_sort.cpp)
boost_sort_add_test(test_flat_stable_sort test_flat_stable_sort.cpp)
boost_sort_add_test(test_insert_sort test_insert_sort.cpp)
boost_sort_add_test(test_parallel_stable_sort test_parallel_stable_sort.cpp)
boost_sort_add_test(test_pdqsort test_pdqsort.cpp)
boost_sort_add_test(test_sample_sort test_sample_sort.cpp)
boost_sort_add_test(test_spinsort test_spinsort.cpp)
