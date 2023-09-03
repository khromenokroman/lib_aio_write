#pragma once
#include "interface.hpp"

#include <cstddef>
#include <fcntl.h>
#include <libaio.h>
#include <stdio.h>
#include <string>
#include <unistd.h>

#include <iostream>

#include <memory>
#include <vector>

class Buffer;
class CircularBuffer;
class File final
{
public:
  File(char const *path, Mode mode, int queue) noexcept;
  ~File();

  /**
   * @brief Запускает процесс копирования
   * @param[in] obj_out целефой файл
   * @return 0-все хорошо, -1-ошибка
   */
  int copy(void *const obj_out);

private:
  int queue;                          ///< Глубина очереди
  int fd;                             ///< Файловый дескриптор
  bool fail = false;                  ///< Признак наличия ошибки, прервать выполнение
  bool eof = false;                   ///< Признак конца файла
  int answer;                         ///< ответ io_submit
  off_t offset_read = 0;              ///< Смещение чтение
  off_t offset_write = 0;             ///< Смещение записи
  io_context_t ctx = {0};             ///< Контекст
  std::unique_ptr<iocb[]> task;       ///< Задание
  std::unique_ptr<iocb *[]> tasks;    ///< Задания
  std::unique_ptr<io_event[]> events; ///< Событие

  /**
   * @brief открывает файл либо на чтение либо на запись
   * @param[in] path путь до файла
   * @param[in] mode режим открытия
   * @return 0-все хорошо, -1-ошибка
   */
  int open_file(char const *path, Mode mode) noexcept;

  /**
   * @brief читает из дескриптора
   * @param[in] буфер куда записывает прочитанное
   * @param[in] сколько надо прочитать
   * @return количество считанных байт
   */
  size_t read_file(char *buf, size_t bytes_to_read) noexcept;

  /**
   * @brief Копирование
   * @param b объект буферов
   * @param cb объект кольцевых буферов
   * @param obj_out выходной файл
   * @return true-продолжаем запись, false-запись закончена
   */
  bool start(Buffer &b, CircularBuffer &cb, void *const obj_out) noexcept;

  /**
   * @brief Проверка событий записи
   * @param[in] buffers объект буферов
   * @param circ_buf объект кольцевых буферов
   */
  void events_write(Buffer &buffers, CircularBuffer &circ_buf) noexcept;
};

class Buffer final
{
public:
  /**
   * @brief Создает буфера нужного количество
   * @param[in] queue количество буферов которые нужно создать
   */
  Buffer(int queue);
  ~Buffer();

private:
  int quantity_buffers;            ///< количество буферов
  bool fail = false;               ///< флаг ошибки
  std::vector<char *> buffers;     ///< Массив буферов = глубине очереди
  size_t const BUFFER_SIZE = 4096; ///< Размер буферов
  size_t alignment = 0;            ///< Смещение

  /**
   * @brief Получает размер кеш линии процессова, не все компиляторы поддерживаю
   * @return размер кеш линии в байтах
   */
  size_t get_cpu_1_cash() const noexcept;

  /**
   * @brief выделение выровненного буфера
   * @param[in] align выравнивание
   * @param[in] n какой размер выделить
   * @return указатель на выделенный буфер
   */
  void *allocateArray(size_t align, size_t n);

  friend File;
};

class CircularBuffer final
{
public:
  CircularBuffer(int size);
  ~CircularBuffer();

  /**
   * @brief добавления элемента в буффер (двигаем голову).
   * @param[in] ref указктель на буфер который нужно добавить
   * @return true если есть место false если нет места
   */
  bool push(void *ref) noexcept;

  /**
   * @brief получаtv элемент (двигаем хвост)
   */
  void *pull() noexcept;

  /**
   * @brief Состояние флага ошибки
   * @return состояние переменной fail
   */
  bool get_status() const noexcept { return fail; }

private:
  std::unique_ptr<void *[]> data; ///< Ссылки на массивы
  int head = 0;                   ///< Начальная позиция головы
  int tail = 0;                   ///< Начальная позиция хвоста
  int max_size = 0;               ///< Размер циклического буффера
  bool fail = false;              ///<
};