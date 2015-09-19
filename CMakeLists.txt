cmake_minimum_required (VERSION 2.8)

project (video)

# build driver library
set(DRIVER_NAME "video")
add_subdirectory(source)

# build tests
# add_subdirectory(test)

# TODO: build app
# add_subdirectory(app)
