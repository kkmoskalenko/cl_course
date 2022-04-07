cmake_minimum_required(VERSION 3.19)
project(CL_Course)
set(CMAKE_CXX_STANDARD 17)

cmake_path(APPEND RESOURCES_PATH ${CMAKE_SOURCE_DIR} Resources)
cmake_path(APPEND DICTIONARY_PATH ${RESOURCES_PATH} dict.opcorpora.txt)
cmake_path(APPEND NEWS_PATH ${RESOURCES_PATH} news)

add_compile_definitions(DICTIONARY_PATH="${DICTIONARY_PATH}")
add_compile_definitions(NEWS_PATH="${NEWS_PATH}")

add_subdirectory(Parser)
add_subdirectory(Task_1)
add_subdirectory(Task_2)
