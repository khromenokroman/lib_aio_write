cmake_minimum_required(VERSION 3.5)

project(prototipe_pcap_on_demand)

# создать библиотеку
add_library(_prototipe_pcap_on_demand SHARED 
    prototipe_pcap_on_demand.hpp 
    prototipe_pcap_on_demand.cpp
    interface.hpp
    )

# добавить к ней библиотеку laio
target_link_libraries(_prototipe_pcap_on_demand PRIVATE aio)