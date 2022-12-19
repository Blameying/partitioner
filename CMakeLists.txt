CMAKE_MINIMUM_REQUIRED(VERSION 3.24)

project(partitioner)

add_subdirectory(src/external_libs/EWAHBoolArray)

if(DEBUG)
  add_definitions(-DDEBUG=1)
endif()

add_executable(${PROJECT_NAME} 
  src/definition.h
  src/main.cpp
  src/parser_input.cpp
  src/parser_input.h
  src/fm_partition.h
  src/fm_partition.cpp
  )

target_link_libraries(${PROJECT_NAME} ewah)
target_include_directories(${PROJECT_NAME} PUBLIC src)
