cmake_minimum_required (VERSION 2.8)

project (video)

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# build driver library
set(DRIVER_NAME "video")
add_subdirectory(source)
include_directories("${PROJECT_SOURCE_DIR}/source")

# build tests
add_subdirectory(test)

# TODO: build app
# add_subdirectory(app)

# write src path to binary dir
configure_file (
  "${PROJECT_SOURCE_DIR}/cmake/src_path.in"
  "${PROJECT_BINARY_DIR}/src_path"
)