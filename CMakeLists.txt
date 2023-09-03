cmake_minimum_required(VERSION 3.5)

project(lib_aio)

# библиотека
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# пример, как будет рабоать пользователь
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

# тесты
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/test)
