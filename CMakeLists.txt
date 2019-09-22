cmake_minimum_required(VERSION 3.12 FATAL_ERROR)
project(clang-compose)

set(WFLAGS "-Werror -Wall -Wextra -Wpedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wenum-compare -Wnon-virtual-dtor -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wsign-promo")

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WFLAGS} -Wno-c++98-compat")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=native -mtune=native -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-sanitize-recover=all -g -O0")
set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS} -Wodr -fsanitize=address,undefined -fno-sanitize=vptr -fno-sanitize-recover=all -g -O0")
set(CMAKE_CXX_FLAGS_RELASAN "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-sanitize=vptr -fno-sanitize-recover=all -g -O3 -march=native -mtune=native -fno-omit-frame-pointer")

find_package(LLVM REQUIRED)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
message(STATUS "LLVM include dir is ${LLVM_INCLUDE_DIR}")

set(SRC "src/compose.cpp")
include_directories("include")

add_executable(compose ${SRC})
set_target_properties(compose PROPERTIES COMPILE_FLAGS "-Wno-unused-parameter -Wno-unused-variable -fno-rtti")

set(clang_libs clangFrontend clangSerialization clangDriver)

if (${LLVM_PACKAGE_VERSION} VERSION_EQUAL 8 OR ${LLVM_PACKAGE_VERSION} VERSION_GREATER 8)
  list(APPEND clang_libs clangASTMatchers)
endif()

list(APPEND clang_libs clangParse clangSema clangCodeGen clangAnalysis clangRewrite clangEdit
     clangAST clangLex clangTooling clangBasic)
   
target_link_libraries(compose LLVMOption LLVM ${clang_libs})
