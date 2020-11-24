cmake_minimum_required(VERSION 3.0)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)

project (RayCasting)

find_library(SDL_LIBRARY NAMES libSDL2.dll.a libSDL2.so libSDL2.dylib)

# SOURCE
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

add_executable (RayCasting ${SOURCES} ${HEADERS})

target_compile_options(RayCasting PRIVATE -Wall -Wextra -pedantic)

target_link_libraries(RayCasting -lm)
target_link_libraries(RayCasting ${SDL_LIBRARY})
