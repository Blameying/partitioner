CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(partitioner)

add_subdirectory(src/external_libs/EWAHBoolArray)

add_executable(${PROJECT_NAME} 
  src/definition.h
  src/main.cpp
  src/parser_input.cpp
  src/parser_input.h
  )

target_link_libraries(${PROJECT_NAME} private ewah)
target_include_directories(${PROJECT_NAME} PUBLIC src)
