cmake_minimum_required(VERSION 3.16)
project(mandelbrot VERSION 0.1.0)

# richiedi l'uso di C++17, senza estensioni non-standard offerte dal compilatore usato 
set(CMAKE_CXX_STANDARD 20)		#modificato, qui c'era 17
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# compila sempre con i warning abilitati
string(APPEND CMAKE_CXX_FLAGS " -Wall -Wextra")

# abilita l'address sanitizer in debug mode
string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address -fno-omit-frame-pointer")
string(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG " -fsanitize=address -fno-omit-frame-pointer")

find_package(SFML 2.5 COMPONENTS graphics REQUIRED)

add_executable(mandelbrot main.cpp)
target_link_libraries(mandelbrot PRIVATE sfml-graphics)
target_link_libraries(mandelbrot PRIVATE -ltbb)

#find_package(PythonLibs 3.6)
#target_include_directories(mandelbrot PRIVATE ${PYTHON_INCLUDE_DIRS})
#target_link_libraries(mandelbrot PRIVATE sfml-graphics tbb ${PYTHON_LIBRARIES})


#target_link_libraries(mandelbrot ${PYTHON_LIBRARIES})
