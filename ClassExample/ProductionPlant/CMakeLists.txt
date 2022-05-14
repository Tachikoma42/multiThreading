cmake_minimum_required(VERSION 3.21)
project(ProductionPlant)

set(CMAKE_CXX_STANDARD 14)

add_executable(ProductionPlant
        Source.cpp plant.cpp plant.h)
