set(target run_main)
set(CMAKE_CXX_STANDARD 17)
file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB code "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_executable(${target} ${headers} ${code})
target_include_directories(${target}
    INTERFACE "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(${target}
  dual_simplex
  utils
  logical_solver
  primal_bland
  core
  deprecated
)
