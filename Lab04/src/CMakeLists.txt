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
add_project_lib(sig sig.c)
if(BUILD_SOLUTIONS)
  add_project_lib(shell shell_solution.c)
else(BUILD_SOLUTIONS)
  add_project_lib(shell shell.c)
endif(BUILD_SOLUTIONS)

