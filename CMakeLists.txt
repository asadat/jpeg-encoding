cmake_minimum_required(VERSION 2.4.6 FATAL_ERROR)

SET(ProjName jpeg)

include_directories("include")
include_directories("/usr/local/include/TooN")
include_directories("/usr/include")
include_directories("/opt/X11/include")
include_directories("/opt/local/include")
include_directories("/usr/local/include")

link_directories("/opt/X11/lib")
link_directories("/usr/lib/x86_64-linux-gnu")
link_directories("/usr/local/lib/TooN")
link_directories("/opt/local/lib")
link_directories("/usr/local/lib")

set(jpeg_LIBRARIES GL GLU glut cvd)

#SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_CGAL_COMPILE_FLAGS}" )

add_executable(${ProjName} src/jpeg.cpp)
target_link_libraries(${ProjName} ${jpeg_LIBRARIES})

add_executable(h264 src/h264.cpp)
target_link_libraries(h264 ${jpeg_LIBRARIES})
