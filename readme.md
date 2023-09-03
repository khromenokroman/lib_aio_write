### Порядок сборки

чтоб заработало aio надо
```
apt install libaio1 && apt install libaio-dev
это если без cmake запускать
g++ test.cpp -laio
```
перед тем как собрать решение, надо установить googleTest для этого на linux надо:
```
sudo apt install libgtest-dev googletest
cd /usr/src/gtest
ls
sudo cmake -Bbuild
sudo cmake --build build/
sudo cp ./build/lib/libgtest* /usr/lib
```

```
cd /tmp && git clone https://github.com/khromenokroman/lib_aio_write && cd lib_aio_write
mkdir build && cd build && cmake .. && cmake --build .
```

### Поумолчанию программа будет копировать файл /tmp/in.dat в /tmp/out.dat
### файл in.dat можно создать например dd