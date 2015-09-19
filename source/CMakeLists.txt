# std threads
find_package(Threads REQUIRED)

# find boost libraries
set(Boost_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost COMPONENTS system date_time filesystem REQUIRED)

# additional includes
include_directories(${Boost_INCLUDE_DIRS})

add_definitions(-std=c++14)

# main library cpp files
file(GLOB SRC_FILES
	"*.cpp"
)

file(GLOB INCLUDE_FILES
	"*.h"
)

add_library(
	${DRIVER_NAME}
	SHARED
	${SRC_FILES}
	${INCLUDE_FILES}
)

target_link_libraries(
	${DRIVER_NAME}
	${CMAKE_THREAD_LIBS_INIT}
	${Boost_LIBRARIES}
)