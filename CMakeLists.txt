cmake_minimum_required(VERSION 3.8)
project(raytracing)

# define C++
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wl,--enable-stdcall-fixup")
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}   -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
# end C++

# define SDL
set(ENV{SDLDIR} ${PROJECT_SOURCE_DIR}/tools/SDL-1.2.15)
find_package(SDL REQUIRED)

include_directories(${SDL_INCLUDE_DIR})
# end SDL

# define OpenEXR
set(OPENEXR_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/OpenEXR-mingw)
include_directories(${OPENEXR_DIRECTORY}/include/OpenEXR)
file(GLOB OPENEXR_LIBRARY "${OPENEXR_DIRECTORY}/lib/*.a")
# end OpenEXR

#define bitmap_image
set(BITMAP_IMAGE_DIRECTORY ${PROJECT_SOURCE_DIR}/tools/bitmap)
include_directories(${BITMAP_IMAGE_DIRECTORY})
#end bitmap_image

# define raytracing
file(GLOB SOURCE_FILES "src/*.h" "src/*.cpp")
set(SOURCE_FILES ${SOURCE_FILES} ${BITMAP_IMAGE_DIRECTORY}/bitmap_image.hpp)

add_executable(raytracing ${SOURCE_FILES})

target_link_libraries(raytracing ${SDL_LIBRARY})
target_link_libraries(raytracing ${OPENEXR_LIBRARY})
# end raytracing
