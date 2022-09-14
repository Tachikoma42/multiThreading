# Multithreading
This repository contains varias project programmed using C++ thread for acceleration.

## Introduction

This repository contains various projects programmed using C++ thread library. The projects are: Matrix Multiplication, Fast Fourier transform, bitonic sort, and Producer-Consumer Problem.

## Environment
- Windows 10
- Clion 2021.2.3
- Visual Studio 2022 as compiler
- C++ 14 (and some features of C++ 17)

## Projects
- [x] [Matrix Multiplication](https://github.com/Tachikoma42/multiThreading/tree/main/Matrix%20Multiplication)
    - This project calculate the multiplication of two matrix using C++ thread library.
- [x] [Bitonic Sort](https://github.com/Tachikoma42/multiThreading/tree/main/bitonicSort)
    - This project sort an array using bitonic sort algorithm.
    - Algorithm: [Bitonic Sort](https://en.wikipedia.org/wiki/Bitonic_sorter)
    - This algorithm is easy to parallelize.
- [x] [Fast Fourier transform](https://github.com/Tachikoma42/multiThreading/tree/main/FFT)
    - This project calculate the discrete fast fourier transform of an array.
    - Algorithm: [Fast Fourier transform](https://en.wikipedia.org/wiki/Fast_Fourier_transform) and [Butterfly diagram](https://en.wikipedia.org/wiki/Butterfly_diagram)
- [x] [Producer-Consumer Problem](https://github.com/Tachikoma42/multiThreading/tree/main/ProductionSimulator/productionPlant)
    - This project simulates producer, consumer and product buffer using multithreading.
    - Problem: [Producer-Consumer Problem](https://en.wikipedia.org/wiki/Producer%E2%80%93consumer_problem)
    - Giving the speed of producer and consumer, and the size of buffer, the program will calculate how many product will be produced and consumed.
    - This program uses wait and notify to synchronize the producer and consumer.

## Usage
- Open the solution file in Clion
- Build the project
- Run the project

## Usefull links
- [C++11/14/17/20 std::thread - cppreference.com](https://en.cppreference.com/w/cpp/thread/thread)
- [C++11/14/17/20 std::mutex - cppreference.com](https://en.cppreference.com/w/cpp/thread/mutex)
- [C++11/14/17/20 std::lock_guard - cppreference.com](https://en.cppreference.com/w/cpp/thread/lock_guard)
- [C++11/14/17/20 std::unique_lock - cppreference.com](https://en.cppreference.com/w/cpp/thread/unique_lock)
- [C++11/14/17/20 std::condition_variable - cppreference.com](https://en.cppreference.com/w/cpp/thread/condition_variable)
- [C++11/14/17/20 std::async - cppreference.com](https://en.cppreference.com/w/cpp/thread/async)
- [C++11/14/17/20 std::future - cppreference.com](https://en.cppreference.com/w/cpp/thread/future)
- [C++11/14/17/20 std::promise - cppreference.com](https://en.cppreference.com/w/cpp/thread/promise)
- [C++11/14/17/20 std::packaged_task - cppreference.com](https://en.cppreference.com/w/cpp/thread/packaged_task)
- [C++11/14/17/20 std::this_thread::sleep_for - cppreference.com](https://en.cppreference.com/w/cpp/thread/sleep_for)
- [C++11/14/17/20 std::this_thread::sleep_until - cppreference.com](https://en.cppreference.com/w/cpp/thread/sleep_until)

## Books
- [C++ Concurrency in Action, Second Edition](https://www.amazon.com/Concurrency-Action-Practical-Multithreading-2nd/dp/1617294691)
- [Effective Modern C++: 42 Specific Ways to Improve Your Use of C++11 and C++14](https://www.amazon.com/Effective-Modern-Specific-Ways-Improve/dp/1491903996)
- [CUDA C++ Programming Guide](https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html)
