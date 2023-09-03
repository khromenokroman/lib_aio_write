#include "prototipe_pcap_on_demand.hpp"
#include "interface.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>

File::File(char const *path, Mode mode, int queue) noexcept {
  this->queue = queue;
  int result_open_file = open_file(path, mode);
  if (result_open_file == -1) {
    fail = true;
  }
};

int File::open_file(char const *path, Mode mode) noexcept {
  int flags, result_open_file;
  switch (mode) {
  case Mode::read:
    flags = O_RDONLY;
    result_open_file = ::open(path, flags, 0660);
    if (result_open_file == -1) {
      ::fprintf(stderr, "Ошибка открытия файла %s\nКод ошибки: %d ==> %s\n",
                path, errno, ::strerror(errno));
      fd = result_open_file;
      return result_open_file;
    } else {
      fd = result_open_file;
      return result_open_file;
    }
    break;
  case Mode::write:
    flags = O_WRONLY | O_CREAT;
    result_open_file = ::open(path, flags, 0660);
    if (result_open_file == -1) {
      ::fprintf(stderr, "Ошибка открытия файла %s\nКод ошибки: %d ==> %s\n",
                path, errno, ::strerror(errno));
      fd = result_open_file;
      return result_open_file;
    } else {
      fd = result_open_file;
      return result_open_file;
    }
    break;
  }
  return result_open_file;
};

size_t File::read_file(char *buf, size_t bytes_to_read) noexcept {
  size_t bytes_read = 0;             // начальное значение
  while (bytes_read < bytes_to_read) // читаем пока не прочитаем
  {
    int currently_read = ::pread(this->fd, buf + bytes_read,
                                 bytes_to_read - bytes_read, offset_read);
    if (currently_read == -1) // вдруг ошибка
    {
      ::fprintf(stderr, "Ошибка чтения из файла!\n");
      fail = true;
    }
    if (currently_read == 0) {
      break;
    }
    bytes_read += currently_read;  // добавляем байт
    offset_read += currently_read; // итерируем смещение
  }
  return bytes_read;
}

int File::copy(void *const obj_out) {
  try {
    if (queue <= 0) {
      ::fprintf(stderr, "Не установлен размер очереди, текущий размер %d\n",
                queue);
    } else if (fd == -1) {
      ::fprintf(stderr,
                "Не открыт файловый дескриптор у исходного файла, текущий "
                "файловый дескриптор %d\n",
                fd);
    } else if (static_cast<File *>(obj_out)->fd == -1) {
      ::fprintf(stderr,
                "Не открыт файловый дескриптор у целевого файла, текущий "
                "файловый дескриптор %d\n",
                static_cast<File *>(obj_out)->fd);
    } else if (fail) {
      ::fprintf(stderr,
                "Не правильно выполнена последовательность инициализации\n");
    } else {
      Buffer buf(queue);
      if (buf.fail != true) {
        CircularBuffer circ_buf(queue);
        if (circ_buf.get_status() != true) {
          task = std::make_unique<iocb[]>(queue);
          tasks = std::make_unique<iocb *[]>(queue);
          events = std::make_unique<io_event[]>(queue);

          /* инициализация структуры */
          int result_io_setup = ::io_setup(queue, &ctx);
          if (result_io_setup != 0) {
            ::fprintf(stderr, "Ошибка инициализации io_setup!\n%s\n",
                      ::strerror(-result_io_setup));
          } else {
            for (int i = 0; i < queue; i++) {
              tasks[i] =
                  &task[i]; // заполним массив всех заданий пустыми заданиями
              circ_buf.push(buf.buffers[i]); // заполним кольцевой буфера
                                             // ссылками на буфера данных
            }
            bool begin = true;
            /* Крутимся пока не прочтем */
            while (begin) {
              begin = start(buf, circ_buf, obj_out);
              events_write(buf, circ_buf);
            }
          }
        }
      }
    }
  } catch (std::exception const &ex) {
    ::fprintf(stderr, "%s\n", ex.what());
    return -1;
  } catch (...) {
    ::fprintf(stderr,
              "Неожиданное исключение!\nПрограмма завершена аварийно!\n");
    return -1;
  }
  return 0;
};

bool File::start(Buffer &b, CircularBuffer &cb, void *const obj_out) noexcept {
  void *ref = cb.pull();
  if (ref == nullptr) {
    return true;
  }

  int count = 0;
  while (true) {
    /* Читаем из файла */
    size_t data_read = read_file(static_cast<char *>(ref), b.BUFFER_SIZE);
    if (data_read == 0) {
      eof = true;
      break;
    } else {
      /* Создаем задание на отправку */
      ::io_prep_pwrite(&task[count], static_cast<File *>(obj_out)->fd, ref,
                       data_read, offset_write);
      task[count].data = ref;
      offset_write += data_read;
      count++;
      if (count == queue)
        break;
      ref = cb.pull();
      if (ref == nullptr) {
        break;
      }
    }
  }
  if (count > 0) {
    /* Ставим задание в очередь на выполнение */
    answer = ::io_submit(ctx, count, tasks.get());
    if (answer < 0) {
      ::fprintf(stderr, "Ошибка работы с io_submit, результат: %d\n%s\n",
                answer, ::strerror(-answer));
    }
  }

  if (eof) {
    return false;
  } else {
    return true;
  }
}

void File::events_write(Buffer &buffers, CircularBuffer &circ_buf) noexcept {
  int num_events = ::io_getevents(ctx, 0, queue, events.get(), NULL);
  if (num_events < 0) {
    ::fprintf(stderr, "Ошибка получении очереди заданий!\n");
  }
  for (int i = 0; i < num_events; i++) {
    circ_buf.push(events[i].data);
  }
}

File::~File() {
  /* Удалим контекст aio*/
  ::io_destroy(ctx);
  /* Закроем дескриптор */
  if (fd > 2) {
    if (::close(fd) == -1) {
      ::fprintf(stderr, "Ошибка при закрытии файлового дескриптора %d\n", fd);
    }
  }
};

Buffer::Buffer(int queue) {
  quantity_buffers = queue;
  /* Значение кеша */
  alignment = get_cpu_1_cash();
  try {
    buffers.reserve(quantity_buffers);
  } catch (std::exception const &ex) {
    ::fprintf(stderr, "Ошибка при резервировании памяти!\n");
    fail = true;
  }
  for (int i = 0; i < quantity_buffers; i++) {
    /* Выделяем выровненую память */
    void *mem = allocateArray(alignment, BUFFER_SIZE);
    if (mem == nullptr) {
      fail = true;
      buffers.push_back(nullptr);
    } else {
      buffers.push_back(static_cast<char *>(mem));
    }
  }
};

/* освободим выделенную память */
Buffer::~Buffer() {
  for (int i = 0; i < quantity_buffers; i++) {
    free(static_cast<void *>(buffers[i]));
  }
};

void *Buffer::allocateArray(size_t align, size_t n) {
  void *mem = std::aligned_alloc(align, n);
  if (mem == nullptr) {
    std::cerr << "Ошибка выделения памяти!\n";
    return nullptr;
  } else {
    return mem;
  }
}

size_t Buffer::get_cpu_1_cash() const noexcept {
  size_t l1dcls = std::hardware_destructive_interference_size;
  return l1dcls;
}

CircularBuffer::CircularBuffer(int size) {
  max_size = size + 1;
  try {
    data = std::make_unique<void *[]>(max_size);
  } catch (std::exception const &ex) {
    ::fprintf(stderr, "%s\n", ex.what());
    fail = true;
  } catch (...) {
    ::fprintf(stderr,
              "Неожиданное исключение!\nПрограмма завершена аварийно!\n");
    fail = true;
  }
}
CircularBuffer::~CircularBuffer() {}

bool CircularBuffer::push(void *ref) noexcept {
  int next = 0;
  next = head + 1;
  if (next >= max_size)
    next = 0;
  if (next == tail) {
    return false;
  }

  data[head] = ref;
  head = next;
  return true;
}

void *CircularBuffer::pull() noexcept {
  int next = 0;
  void *ref;

  if (head == tail)
    return nullptr;

  next = tail + 1;
  if (next >= max_size)
    next = 0;
  ref = data[tail];
  tail = next;
  return ref;
}

int create_object(pointer *obj, char const *path, Mode mode,
                  int queue) noexcept {
  *obj = new File(path, mode, queue);
  if (obj == NULL) {
    ::fprintf(stderr, "Не удалось выделить память под объект класса\n");
    return -1;
  };
  return 0;
}
int copy(pointer in, pointer out) noexcept {
  int result_copy = static_cast<File *>(in)->copy(out);
  return result_copy;
}
void close_object(pointer obj) noexcept { delete static_cast<File *>(obj); }