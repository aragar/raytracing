cmake_minimum_required(VERSION 3.8)
project(raytracing)

# define C++
set(CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
# end C++

# define SDL
set(ENV{SDLDIR} ${PROJECT_SOURCE_DIR}/tools/SDL-1.2.15)
find_package(SDL REQUIRED)

include_directories(${SDL_INCLUDE_DIR})
# end SDL

# define OpenEXR
include_directories(${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/include/OpenEXR)
set(OPENEXR_LIBRARY
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libIlmImf.a
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libHalf.a
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libIex.a
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libIexMath.a
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libImath.a
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libzlibstatic.a
		${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw/lib/libIlmThread.a)
# end OpenEXR

# define raytracing
set(SOURCE_FILES 
		src/main.cpp
		src/color.h
		src/color.cpp
		src/utils.h
		src/constants.h
		src/vector.cpp
		src/vector.h
		src/matrix.cpp
		src/matrix.h
		src/camera.cpp
		src/camera.h
		src/geometry.cpp
		src/geometry.h
		src/sdl.cpp
		src/sdl.h
		src/shading.cpp
		src/shading.h
		src/ray.h
		src/bitmap.cpp
		src/bitmap.h
		src/texture.cpp
		src/texture.h
		src/shadinghelper.cpp
		src/shadinghelper.h
		src/utils.cpp)
	
add_executable(raytracing ${SOURCE_FILES})

target_link_libraries(raytracing ${SDL_LIBRARY})
target_link_libraries(raytracing ${OPENEXR_LIBRARY})
# end raytracing