set(target basic_test)
file(GLOB code "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${target} ${code})
target_include_directories(${target}
    INTERFACE "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${target}
  dual_simplex
  primal_simplex
  utils
  logical_solver
)