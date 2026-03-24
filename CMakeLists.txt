cmake_minimum_required(VERSION 3.16)

project(my_compiler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 先找系统里的 zstd 库
find_library(ZSTD_LIBRARY NAMES zstd)

# 如果系统里有 libzstd.so，但没有对应的 CMake target，
# 就手动补一个 zstd::libzstd_shared
if(ZSTD_LIBRARY AND NOT TARGET zstd::libzstd_shared)
    add_library(zstd::libzstd_shared UNKNOWN IMPORTED)
    set_target_properties(zstd::libzstd_shared PROPERTIES
        IMPORTED_LOCATION "${ZSTD_LIBRARY}"
    )
endif()

# 查找 LLVM
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

add_executable(my_compiler
    src/main.cpp
    src/lexer.cpp
    src/parser.cpp
    src/semantic.cpp
    src/codegen.cpp
)

target_include_directories(my_compiler PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

llvm_map_components_to_libnames(LLVM_LIBS
    core
    support
)

target_link_libraries(my_compiler PRIVATE ${LLVM_LIBS})