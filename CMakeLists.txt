cmake_minimum_required(VERSION 3.8)
project(raytracing)

# define C++
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wl,--enable-stdcall-fixup")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}   -O2")
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

#define bitmap_image
set(BITMAP_IMAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/bitmap)
include_directories(${BITMAP_IMAGE_DIRECTORY})
#end bitmap_image

# define raytracing
set(SOURCE_FILES
        ${BITMAP_IMAGE_DIRECTORY}/bitmap_image.hpp
        src/main.cpp
        src/color.h 				src/color.cpp
        src/utils.h 				src/utils.cpp
        src/constants.h
        src/vector.h 				src/vector.cpp
        src/matrix.h 				src/matrix.cpp
        src/camera.h 				src/camera.cpp
        src/geometry.h 				src/geometry.cpp
        src/sdl.h 					src/sdl.cpp
        src/shading.h 				src/shading.cpp
        src/ray.h
        src/bitmap.h 				src/bitmap.cpp
        src/texture.h 				src/texture.cpp
        src/shadinghelper.h 		src/shadinghelper.cpp
        src/environment.h 			src/environment.cpp
        src/colors.h 				src/colors.cpp
        src/mesh.h 					src/mesh.cpp
        src/transform.h 			src/transform.cpp
        src/scene.h 				src/scene.cpp
        src/random_generator.h 		src/random_generator.cpp
        src/light.h 				src/light.cpp
        src/bbox.h 					src/bbox.cpp
        src/heightfield.h 			src/heightfield.cpp
        src/KDTree.h 				src/KDTree.cpp)

add_executable(raytracing ${SOURCE_FILES})

target_link_libraries(raytracing ${SDL_LIBRARY})
target_link_libraries(raytracing ${OPENEXR_LIBRARY})
# end raytracing
