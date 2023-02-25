# Schedulability Test with Priority Ceiling Protocol
This repository contains a C++ implementation of an algorithm that simulates a real-time system with four periodic tasks scheduled with Rate Monotonic. The goal of this implementation is to test the schedulability of the task set and demonstrate the use of semaphores with Priority Ceiling Protocol.

## Requirements
The implementation requires the following:

- C++ compiler
- pthread library
## Usage
To compile the code, run the following command:

```bash
g++ -lpthread schedulability_test.cpp -o schedulability_test

```
To execute the program, run:
```bash
./schedulability_test
```
## Description
The system has four periodic tasks with periods 80ms, 100ms, 200ms, and 160ms. Task1 has the highest priority, and Task4 has the lowest priority. There are three global variables called T1T2, T1T4, T2T3. The following requirements should be met:

- Task1 writes into T1T2, and Task2 reads from it.
- Task1 writes into T1T4, and Task4 reads from it.
- Task2 writes into T2T3, and Task3 reads from it.
All critical sections are protected by semaphores using Priority Ceiling Protocol.
In each task code, the critical sections are protected by mutexes, and the waste_time() function is added to increase the probability of being pre-empted by another task that uses that semaphore. The utilization factor is computed by executing each task 100 times to have a more reliable value. When the task set results schedulable, the number of missed deadlines should be lower.

## Conclusion
It is recommended to try this code in a real Ubuntu operating system, instead of using an emulator (such as Virtual Machine), to achieve the highest accuracy in task scheduling. Using a virtual machine severely compromises the execution of the code, and drawing accurate conclusions about it may be tricky.

## Author
This implementation was created by Alessandro Perri.
