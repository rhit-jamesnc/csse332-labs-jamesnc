# top level testing

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

add_test_executable(shell shell sig readline)
add_debug_executable(shell shell sig readline)

# Add the simpleshell itself for student testing.
add_executable(simpleshell.run simpleshell.c)
target_link_libraries(simpleshell.run shell sig readline)
target_include_directories(simpleshell.run
    PUBLIC ${PROJECT_SOURCE_DIR}/include
    PRIVATE ${PROJECT_SOURCE_DIR}/src)

# add donothing
add_executable(donothing.run donothing.c)
