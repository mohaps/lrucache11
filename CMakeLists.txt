cmake_minimum_required(VERSION 3.2)

project(LRUCache11)

SET(${PROJECT_NAME}_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/LRUCache11.hpp
)

add_library(${PROJECT_NAME} INTERFACE)

#target_link_libraries(${PROJECT_NAME} INTERFACE
#)

target_include_directories(${PROJECT_NAME}
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(${PROJECT_NAME} INTERFACE
  cxx_nonstatic_member_init cxx_auto_type
)

target_sources(${PROJECT_NAME} INTERFACE ${${PROJECT_NAME}_SOURCES})
