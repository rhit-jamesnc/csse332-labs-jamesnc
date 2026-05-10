# top level testing

if(BUILD_BINUTILS)
  link_libraries(bfd opcodes)
endif(BUILD_BINUTILS)

macro(add_test_executable prefix)
  add_executable(${prefix}_tests.run ${prefix}_tests.c)
  target_link_libraries(${prefix}_tests.run ${prefix} cgrade ${ARGN})
  target_include_directories(${prefix}_tests.run
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src)
  add_test(NAME ${prefix} COMMAND $<TARGET_FILE:${prefix}_tests.run>)
endmacro(add_test_executable)

macro(add_debug_executable prefix)
  add_executable(${prefix}_debug.run ${prefix}_debug.c)
  target_link_libraries(${prefix}_debug.run ${prefix} ${ARGN})
  target_include_directories(${prefix}_debug.run
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src)
endmacro(add_debug_executable)

# add_test_executable(fib)
add_debug_executable(rf_parse)
add_debug_executable(rf_load rf_parse)
add_debug_executable(rf_sched rf_proc rf_load rf_parse)

# Graded test suites
add_test_executable(rf_load rf_parse)
add_test_executable(rf_sched rf_proc rf_load rf_parse)

# mmap example
add_executable(mmap_example.run mmap_example.c)
add_executable(context_sample.run context_sample.c)

# Add custom target for running sched with LD_PRELOAD
add_custom_target(run_sched_tests
    COMMAND ${CMAKE_COMMAND} -E env
        LD_PRELOAD=$<TARGET_FILE:rf_context>
        $<TARGET_FILE:rf_sched_tests.run>
    DEPENDS rf_sched_tests.run rf_context
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
