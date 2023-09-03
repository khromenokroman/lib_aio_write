#include "interface.hpp"
#include <dlfcn.h>
#include <fstream>
#include <gtest/gtest.h>
#include <iostream>

struct File_fixture : public testing::Test {
  /* Создадим тестовый исходный файл*/
  static void SetUpTestSuite() {
    std::cout << "Создаю тестовый файл 1GB\n";
    std::system("dd if=/dev/urandom of=/tmp/in.dat bs=1000000 count=1000");
    std::cout << "Создание файлов завершено!\n";
  }
  void SetUp() override {
    /* ===================================================находим символы*/
    /* загружаем библиотеку*/
    ::dlerror();
    my_lib = ::dlopen(path_lib, RTLD_LAZY);
    if (my_lib == NULL) {
      std::cerr << "***[ОШИБКА]*** Ошибка загрузки библиотеки!\n";
      std::cerr << "***[ОШИБКА]***" << ::dlerror() << "\n";
    }
    ASSERT_FALSE(my_lib == NULL);

    /* Находим функцию создания объекта */
    ::dlerror();
    make_obj = (make_file_t *)::dlsym(my_lib, func_name_first_init);
    if (make_obj == 0) {
      std::cerr << "***[ОШИБКА]*** Не найдета функция create_object\n";
      std::cerr << "***[ОШИБКА]***" << ::dlerror() << "\n";
    }
    ASSERT_FALSE(make_obj == 0);

    /* Находим метод создания объекта */
    ::dlerror();
    copy_obj = (copy_file_t *)::dlsym(my_lib, func_name_copy);
    if (copy_obj == 0) {
      std::cerr << "***[ОШИБКА]*** Ошибка загрузки метода copy_obj\n";
      std::cerr << "***[ОШИБКА]***" << ::dlerror() << "\n";
    }
    ASSERT_FALSE(copy_obj == 0);

    /* Находим функцию разрушающий объект */
    ::dlerror();
    close_obj = (close_file_t *)(::dlsym(my_lib, func_name_close_obj));
    if (close_obj == 0) {
      std::cerr << "***[ОШИБКА]*** Ошибка загрузки метода close_object\n";
      std::cerr << "***[ОШИБКА]***" << ::dlerror() << "\n";
    }
    ASSERT_FALSE(close_obj == 0);
  /* ===================================================*/

  /* ====================================================выполнение*/
    ASSERT_TRUE(obj_file_in == nullptr);
    /* Исходный файл */
    int result_in = make_obj(&obj_file_in, file_name_in, Mode::read, 50);
    if (result_in == -1) {
      std::cerr << "***[ОШИБКА]*** Не удается создать объект класса File\n";
    }
    ASSERT_NE(result_in, -1);

    ASSERT_TRUE(obj_file_out == nullptr);
    /* Целевой файл */
    int result_out = make_obj(&obj_file_out, file_name_out, Mode::write, 50);
    if (result_in == -1) {
      std::cerr << "***[ОШИБКА]*** Не удается создать объект класса File\n";
    }
    ASSERT_NE(result_out, -1);

    ASSERT_FALSE(obj_file_in == nullptr);
    ASSERT_FALSE(obj_file_out == nullptr);

  }
  void TearDown() override {

    /* Разрушаем ранее созданные объекты */
    close_obj(obj_file_in);
    close_obj(obj_file_out);

    /* Выгружаем библиотеку */
    ::dlerror();
    int result_close_lib = ::dlclose(my_lib);
    if (result_close_lib == -1) {
      std::cerr << "***[ОШИБКА]*** Ошибка при выгрузке библиотеки\n";
      std::cerr << "***[ОШИБКА]***" << ::dlerror() << "\n";
    }
    ASSERT_EQ(result_close_lib, 0);
  }
  /* указатель на загруженную библиотеку*/
  pointer my_lib = nullptr;
  /* указатели на 2 файла исходный и целевой */
  void *obj_file_in = nullptr;
  void *obj_file_out = nullptr;
  /* указатель на функцию первой инициализации */
  make_file_t *make_obj;
  /* указатель на функцию копирования */
  copy_file_t *copy_obj;
  /* указатель на функцию сборки мусора :) */
  close_file_t *close_obj;
  /* путь до библиотеки*/
  char const *path_lib = "../lib/lib_prototipe_pcap_on_demand.so";
  /* название функции создания */
  char const *func_name_first_init = "create_object";
  /* название функции копирования */
  char const *func_name_copy = "copy";
  /* название функции закрытия */
  char const *func_name_close_obj = "close_object";
  /* исходный файл*/
  char const *file_name_in = "/tmp/in.dat";
  /* целевой файл*/
  char const *file_name_out = "/tmp/out.dat";
  /* очередь */
  int queue = 50;
};
TEST_F(File_fixture, func_copy) {

  /* выполним копирование */
  int result_copy = copy_obj(obj_file_in, obj_file_out);
  if (result_copy == -1){
    std::cerr << "***[ОШИБКА]*** Копирование выполнено с ошибкой!\n ";
  }
  ASSERT_NE(result_copy, -1);
}

struct Files_identical : public testing::Test {

  static void TearDownTestSuite() {

    std::cout << "Начинаю удаление файлов...\n";
    std::system("rm /tmp/in.dat");
    std::system("rm /tmp/out.dat");
    std::cout << "Удаление файлов завершено!\n";
  }

  /* исходный файл*/
  char const *file_name_in = "/tmp/in.dat";
  /* целевой файл*/
  char const *file_name_out = "/tmp/out.dat";
};
TEST_F(Files_identical, files_identical) {
  /* проверим одинаковые ли файлы */
  std::ifstream fin(file_name_in, std::ios::binary | std::ios::ate);
  std::ifstream fout(file_name_out, std::ios::binary | std::ios::ate);
  if (fin.fail()) {
    std::cerr << "***[ОШИБКА]*** Не доступа к фалу " << file_name_in << "\n";
  } else if (fout.fail()) {
    std::cerr << "***[ОШИБКА]*** Не доступа к фалу " << file_name_out << "\n";
  }
  ASSERT_FALSE(fin.fail());
  ASSERT_FALSE(fout.fail());
  /*файлы равны*/
  if (fin.tellg() != fout.tellg()) {
    std::cerr << "***[ОШИБКА]*** Файлы имеют разный размер!!!\n";
  }
  ASSERT_TRUE(fin.tellg() == fout.tellg());
  /* смешение в начало */
  fin.seekg(0, std::ios::beg);
  fout.seekg(0, std::ios::beg);

  char c1, c2;
  bool equal = true;
  while (fin.get(c1) && fout.get(c2)) {
    if (c1 != c2) {
      equal = false;
    }
  }
  fin.close();
  fout.close();
  if (!equal) {
    std::cerr << "***[ОШИБКА]*** Файлы не равны!!!\n";
  }
  ASSERT_TRUE(equal);
}
