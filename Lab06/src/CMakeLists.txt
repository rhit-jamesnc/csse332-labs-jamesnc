# top file of src/ directory

macro(add_test_executable prefix)
  add_executable(${prefix}.run ${ARGN})
  target_link_libraries(${prefix}.run pthread)
  target_include_directories(${prefix}.run
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src)
  add_test(NAME ${prefix} COMMAND $<TARGET_FILE:${prefix}.run>)
endmacro(add_test_executable)

# each problem will be compiled into a library that the tests use
if(BUILD_SOLUTIONS)
  add_test_executable(factoring factoring_solution.c)
  add_test_executable(inorder inorder_solution.c)
  add_test_executable(max max_solution.c)
  add_test_executable(barrier barrier_solution.c)
  add_test_executable(thread_abc thread_abc_solution.c)
else(BUILD_SOLUTIONS)
  add_test_executable(factoring factoring.c)
  add_test_executable(inorder inorder.c)
  add_test_executable(max max.c)
  add_test_executable(barrier barrier.c)
  add_test_executable(thread_abc thread_abc.c)
endif(BUILD_SOLUTIONS)

