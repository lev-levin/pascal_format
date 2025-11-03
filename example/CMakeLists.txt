cmake_minimum_required(VERSION 3.5)

project(pasfmt_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(pasfmt_test
  main.cpp
)

target_link_directories(pasfmt_test PUBLIC
  pasfmt_lib
)

target_include_directories(pasfmt_test PUBLIC
  pasfmt_lib/include
)

target_link_libraries(pasfmt_test
  "pas_fmt_GNU-7.3.0_Release"
)
