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
cd /tmp && git clone https://gitlab.t-argos.ru/rkhromenok/posix_memalign && cd posix_memalign
mkdir build && cd build && cmake .. && cmake --build .
```