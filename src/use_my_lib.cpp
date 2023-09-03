/* пример использования библиотеки */

#include "interface.hpp"
#include <dlfcn.h>
#include <iostream>
#include <stdio.h>

int main(int argc, char const *argv[]) {
  /* указатель на загруженную библиотеку*/
  pointer my_lib = nullptr;
  void *obj_file_in = nullptr;
  void *obj_file_out = nullptr;

  // загружаем библиотеку
  my_lib = dlopen("../lib/lib_prototipe_pcap_on_demand.so", RTLD_LAZY);
  if (my_lib == NULL) {
    std::cerr << "Ошибка загрузки библиотеки!\n";
    return -1;
  }

  /* Находим метод создания объекта */
  make_file_t *make_obj = (make_file_t *)::dlsym(my_lib, "create_object");
  /*проверка загрузки метода*/
  if (make_obj == 0) {
    std::cerr << "Ошибка загрузки метода create_object\n";
    return -1;
  }
 
  /* Исходный файл */
  int result_in = make_obj(&obj_file_in, "/tmp/in.dat", Mode::read, 50);
  if (result_in == -1) {
    std::cerr << "Ошибка создания объета File in!\n";
    return -1;
  }

  /* Целевой фалй */
  int result_out = make_obj(&obj_file_out, "/tmp/out.dat", Mode::write, 50);
  if (result_out == -1) {
    std::cerr << "Ошибка создания объета File out!\n";
    return -1;
  }
  /* Находим метод создания объекта */
  copy_file_t *copy_obj = (copy_file_t *)::dlsym(my_lib, "copy");
  /*проверка загрузки метода*/
  if (copy_obj == 0) {
    std::cerr << "Ошибка загрузки метода copy_obj\n";
    return -1;
  }

  /* выполним копирование */
  int result_copy = copy_obj(obj_file_in,obj_file_out);
  if(result_copy == -1)
  {
    std::cerr << "Ошибка при копировании!!!\n";
    return -1;

  }
  /* Находим метод разрушающий объект */
  close_file_t *close_obj = (close_file_t *)(::dlsym(my_lib, "close_object"));
  /*проверка загрузки метода*/
  if (close_obj == 0) {
    std::cerr << "Ошибка загрузки метода close_object\n";
    return -1;
  }
  /* Разрушаем ранее созданные объекты */
  close_obj(obj_file_in);
  close_obj(obj_file_out);

  /* Выгрузаем библиотеку */
  ::dlclose(my_lib);
}