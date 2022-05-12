add_executable(Task_4 main.cpp Model.h json.hpp)
target_link_libraries(Task_4 PUBLIC Parser)

cmake_path(APPEND MODEL_PATH ${CMAKE_CURRENT_SOURCE_DIR} model.json)
add_compile_definitions(MODEL_PATH="${MODEL_PATH}")