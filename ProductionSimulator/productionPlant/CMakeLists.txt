cmake_minimum_required(VERSION 3.21)
project(productionPlant)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(productionPlant
        main.cpp
        plant.cpp
        plant.h twoWorker.cpp)
