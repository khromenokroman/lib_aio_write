cmake_minimum_required(VERSION 3.5)

project(google_test)

# Ищем предустановленный гугОл тест
find_package(GTest REQUIRED)
    
# Задача на сборку файлика с тестами
add_executable(google_test
    test.cpp)

# подключенные директории
target_include_directories(google_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib)

# библиотеки
target_link_libraries(google_test PRIVATE
    ${GTEST_BOTH_LIBRARIES}
    GTest::GTest)
