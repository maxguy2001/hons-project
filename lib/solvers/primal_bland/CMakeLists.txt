set(target "primal_bland")
file(GLOB headers "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
file(GLOB code "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
add_library(${target} STATIC ${headers} ${code})
include_directories(${target}
    PRIVATE "${CMAKE_SOURCE_DIR}/lib"
    "${CMAKE_CURRENT_SOURCE_DIR}"
)