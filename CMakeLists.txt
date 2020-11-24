cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "RayCasting")
project (${PROJECT_NAME})

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Project and out dir
if(EMSCRIPTEN)
  message("Build [Emscripten]")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/public)
else(EMSCRIPTEN)
  message("Build [Binary]")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/build)
endif(EMSCRIPTEN)

# Source
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Compile options
target_compile_options(${PROJECT_NAME} PUBLIC -Wall -Wextra -pedantic -O2)

# Link
if(EMSCRIPTEN)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  target_compile_options(${PROJECT_NAME} PUBLIC -sWASM=1 -sUSE_SDL=2 -sDEMANGLE_SUPPORT=1)
  target_link_libraries(${PROJECT_NAME} -sUSE_SDL=2)
else(EMSCRIPTEN)
  if(MINGW)
    target_link_libraries(${PROJECT_NAME} -lmingw32)
  endif(MINGW)

  find_library(SDL_LIBRARY NAMES libSDL2.dll.a libSDL2.so libSDL2.dylib)
  find_library(SDL_MAIN_LIBRARY NAMES libSDL2main.a libSDL2main.so libSDL2main.dylib)
  target_link_libraries(${PROJECT_NAME} -lm ${SDL_MAIN_LIBRARY} ${SDL_LIBRARY})
endif(EMSCRIPTEN)
