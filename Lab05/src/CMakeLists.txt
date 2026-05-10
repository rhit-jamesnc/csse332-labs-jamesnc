# top file of src/ directory

macro(add_project_lib library)
  if(${ARGC} GREATER 1)
    add_library(${library} SHARED ${ARGN})
    target_include_directories(${library}
      PUBLIC ${PROJECT_SOURCE_DIR}/include)
    target_compile_options(${library} PUBLIC -Wall -ggdb -Werror)
  else()
    message(FATAL_ERROR "macro add_project_lib called incorrectly")
  endif()
endmacro(add_project_lib)

# each problem will be compiled into a library that the tests use
add_project_lib(rf_parse rf_parse.c)
if(BUILD_SOLUTIONS)
  add_project_lib(rf_load rf_load_solution.c)
  add_project_lib(rf_proc rf_proc_solution.c)
  add_project_lib(rf_sched rf_sched_solution.c)
else(BUILD_SOLUTIONS)
  add_project_lib(rf_load rf_load.c)
  add_project_lib(rf_proc rf_proc.c)
  add_project_lib(rf_sched rf_sched.c)
endif(BUILD_SOLUTIONS)

# add wrapper library
add_library(rf_context SHARED rf_context.c)
target_link_libraries(rf_context dl)
target_compile_options(rf_context PRIVATE -Wall -ggdb -Werror -fPIC)
target_include_directories(rf_context PUBLIC ${PROJECT_SOURCE_DIR}/include)

