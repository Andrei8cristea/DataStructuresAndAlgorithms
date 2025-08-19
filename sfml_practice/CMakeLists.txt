cmake_minimum_required(VERSION 4.0)
project(sfml_practice)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 2.5 COMPONENTS window REQUIRED)

add_executable(sfml_practice main.cpp)

target_link_libraries(sfml_practice sfml-window sfml-graphics sfml-system)
