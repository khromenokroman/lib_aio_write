cmake_minimum_required(VERSION 3.5)

project(test_my_lib)

# Задача на сборку файлика с тестами
add_executable(test_my_lib
    use_my_lib.cpp)

# подключим директорию с библиотекой
target_include_directories(test_my_lib PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

    #поставим что минимум 17 стандарт
set_target_properties(test_my_lib PROPERTIES 
    CXX_STANDARD 17 
    CXX_STANDARD_REQUIRED ON)