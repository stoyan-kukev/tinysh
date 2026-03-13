cmake_minimum_required(VERSION 3.10)

project(tinysh VERSION 1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(tinysh
  src/main.c
  src/parser.c
  src/executor.c
)

target_include_directories(tinysh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
