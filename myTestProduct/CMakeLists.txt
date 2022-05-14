cmake_minimum_required(VERSION 3.21)
project(myTestProduct)

set(CMAKE_CXX_STANDARD 17)

include_directories(.)

add_executable(myTestProduct
        plant.cpp
        plant.h
        Source.cpp
        twoWorker.cpp bufferState.cpp bufferState.h)
