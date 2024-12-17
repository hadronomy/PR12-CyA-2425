project(${CMAKE_PROJECT_NAME})

file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc")
list(REMOVE_ITEM SOURCES "main.cc")
message(STATUS "Found sources: ${SOURCES}")

add_executable("cya")
set_target_properties("cya" PROPERTIES ENABLE_EXPORTS TRUE CXX_STANDARD ${CMAKE_CXX_STANDARD})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  target_compile_options("cya" PRIVATE 
    -fsized-deallocation -D__cpp_concepts=202002 -Wno-builtin-macro-redefined
    -Wno-reorder-init-list
  )
  target_link_libraries("cya" PRIVATE stdc++exp)
else()
  message(FATAL_ERROR "Unsupported compiler use Clang instead")
endif()

target_sources("cya"
    PRIVATE
      "main.cc"
    PUBLIC
      ${SOURCES}
)

target_include_directories(
    "cya"
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)
