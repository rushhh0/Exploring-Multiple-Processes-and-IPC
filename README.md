# Exploring-Multiple-Processes-and-IPC

Rushi Trivedi
4348.501

Project #1 Exploring Multiple Processes and IPC

Project Description:

This project simulates a simple computer system with a CPU and Memory. The CPU and Memory are implemented as separate processes that communicate using pipes. The CPU executes a user program stored in memory. The user program is written in a custom assembly-like language and consists of a series of instructions. The CPU has registers (PC, SP, IR, AC, X, Y) and supports various instructions for arithmetic, memory operations, I/O, and control flow.

My File Description:

1. CPU.cpp - Contains the implementation of the CPU functionality, including instruction execution and interrupt handling.
2. CPU.h - Contains function declarations and constants used in CPU operations.
3. Memory.cpp - Implements the memory functionality, including loading a program from a file and handling memory operations. Coordinates the execution of the CPU and Memory processes, sets up pipes for communication, and manages the overall program flow.
4. Memory.h - Contains function declarations for memory operations.

How to Compile:

g++ CPU.cpp Memory.cpp -o p1

Note: p1 is the name of my executable file.

How to Run:

./p1 sample1.txt 30
./p1 sample2.txt 30
./p1 sample3.txt 30
./p1 sample4.txt 30
./p1 sample5.txt 30


sample1.txt:
ABCDEFGHIJKLMNOPQRSTUVWXYZ12345678910

sample2.txt:
    ------
 /         \
/   -*  -*  \
|           |
\   \____/  /
 \         /
    ------

sample3.txt:
A
0
A
0
A
2
A
4
A
5
A
7
A
9
A
10
A
12
A
14

sample4.txt: (Note: I am using a decrement then write approach for the stack) 
1000
999
1000
1998
1997
1998
Memory violation: accessing system address 1000 in user mode 

sample5.txt:
Expected Output: I_want_these_6_points
