cmake_minimum_required(VERSION 3.8)
project(raytracing)

# define C++
set(CMAKE_CXX_STANDARD 11)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O2")
# end C++

# define STL
set(ENV{SDLDIR} ${PROJECT_SOURCE_DIR}/tools/SDL-1.2.15)
find_package(SDL REQUIRED)

include_directories(${SDL_INCLUDE_DIR})
# end STL

# define raytracing
set(SOURCE_FILES src/main.cpp src/color.h src/color.cpp src/utils.h src/constants.h src/vector.cpp src/vector.h src/matrix.cpp src/matrix.h src/camera.cpp src/camera.h src/geometry.cpp src/geometry.h src/sdl.cpp src/sdl.h src/shading.cpp src/shading.h src/ray.h src/bitmap.cpp src/bitmap.h)
add_executable(raytracing ${SOURCE_FILES})

target_link_libraries(raytracing ${SDL_LIBRARY})
# end raytracing