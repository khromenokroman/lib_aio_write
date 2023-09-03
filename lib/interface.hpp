#pragma once
#include <cstddef>
#include <vector>

/* Режимы открытия файла*/
enum class Mode { read, write };

/* Тип pointer = void* */
typedef void *pointer;

/* Сигнатура функции
принимает pointer -> void* и char const * и Mode и int, а возвращает int */
typedef int make_file_t(pointer, char const *, Mode, int);

/* Сигнатура функции
принимает pointer -> void* и pointer -> void*  а возвращает int */
typedef int copy_file_t(pointer, pointer);

/* Сигнатура функции
принимает pointer -> void* а возвращает int */
typedef void close_file_t(pointer);

extern "C" {
/* Создает объет класса File*/
int create_object(pointer *, char const *path, Mode mode, int queue) noexcept;
int copy(pointer in, pointer out) noexcept;
/* Разрушает объект класса File*/
void close_object(pointer) noexcept;
}