execute_process (
  COMMAND ${PYTHON_EXECUTABLE} ./utils/detect_revision.py
  OUTPUT_VARIABLE WL_VERSION
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(REGEX REPLACE "\n|\r$" "" WL_VERSION "${WL_VERSION}")
string(STRIP WL_VERSION "${WL_VERSION}")
file (WRITE ${CMAKE_CURRENT_BINARY_DIR}/VERSION "${WL_VERSION}")

configure_file (${CMAKE_CURRENT_SOURCE_DIR}/src/build_info.cc.cmake ${CMAKE_CURRENT_BINARY_DIR}/src/build_info.cc)

message (STATUS "Version of Widelands Build is ${WL_VERSION}(${CMAKE_BUILD_TYPE})")