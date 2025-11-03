cmake_minimum_required(VERSION 3.5)

project(pas_fmt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message("***** CMAKE_CXX_COMPILER_ID: ${CMAKE_CXX_COMPILER_ID}")
message("***** CMAKE_CXX_COMPILER_VERSION: ${CMAKE_CXX_COMPILER_VERSION}")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/x64")

add_library(pas_fmt_${CMAKE_CXX_COMPILER_ID}-${CMAKE_CXX_COMPILER_VERSION}_${CMAKE_BUILD_TYPE}
  pascal_format.h
  pascal_format.cpp
  cpp_unicode.h
  cpp_unicode.cpp
  cpp_unicode_windows.cpp
  dwmutex.h
  dwmutex_windows.cpp
  dwsync.h
  pasfmt_thread_support.h
  pasfmt_thread_support.cpp
)
