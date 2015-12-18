# std threads
find_package(Threads REQUIRED)

# find ffmpeg libraries
find_package(FFMPEG REQUIRED)
find_package(LibSWScale REQUIRED)

# find sdl
find_package(SDL2 REQUIRED)

# epoxy gl bindings
find_package(LibEpoxy REQUIRED)

add_definitions(-std=c++14)

# main library cpp files
file(GLOB SRC_FILES
	"*.cpp"
	"audio/*.cpp"
	"common/*.cpp"
	"os/*.cpp"
	"video/*.cpp"
)

file(GLOB INCLUDE_FILES
	"*.h"
	"audio/*.h"
	"common/*.h"
	"os/*.h"
	"video/*.h"
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
	${FFMPEG_LIBAVCODEC}
	${FFMPEG_LIBAVFORMAT}
	${FFMPEG_LIBAVUTIL}
	${LIBSWSCALE_LIBRARIES}
	${SDL2_LIBRARY}
	${EPOXY_LIBRARY}
)
