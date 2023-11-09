#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdlib>
#include <string.h>
#include <fstream>
#include <ctype.h>
#include "Memory.h"
#include "CPU.h"

using namespace std;

const int SIZE = 2000;
int memory[SIZE]; // Define an array to simulate memory

// Function to load memory from a file
void loadMemory(const char *fileName) { // Added const for file name
    int counter = 0;
    FILE *name = fopen(fileName, "r");
    char buffer[100];

    while (fgets(buffer, sizeof(buffer), name)) {
        if (isdigit(buffer[0])) {
            memory[counter++] = atoi(buffer); // Convert string to integer and store in memory
        } else if (buffer[0] == '.') {
            counter = atoi(&buffer[1]); // Set the counter to a specific value
        }
    }

    fclose(name);
}

void executableMemory(int writePipe, int readPipe) {
    bool isActive = true; // Variable to control the loop, indicates if the process is active.

    while (isActive) { // Loop until isActive is false.
        char operation; // Variable to store the operation type (R for read, W for write, E for exit)
        int memoryLocation, metadata; // Variables to store memory location and metadata.

        // Read the operation type from the pipe.
        read(readPipe, &operation, sizeof(operation));

        switch (operation) { // Use a switch statement to handle different operations.
            case 'R': { // Case for read operation.
                // Read the memory location from the pipe.
                read(readPipe, &memoryLocation, sizeof(memoryLocation));
                // Read data from memory and write it to the pipe.
                write(writePipe, &memory[memoryLocation], sizeof(memory[memoryLocation]));
                break; // Exit the case.
            }
            case 'W': { // Case for write operation.
                // Read the memory location and metadata from the pipe.
                read(readPipe, &memoryLocation, sizeof(memoryLocation));
                read(readPipe, &metadata, sizeof(metadata));
                // Write metadata to the specified memory location.
                memory[memoryLocation] = metadata;
                break; // Exit the case.
            }
            case 'E': { // Case for exit operation.
                isActive = false; // Set isActive to false, exiting the loop.
                break; // Exit the case.
            }
            default: { // Default case if the operation type is not recognized.
                cout << "Command is not valid " << operation; // Print an error message.
                exit(-1); // Exit the process with an error code.
            }
        }
    }

    exit(0); // Exit the process normally.
}

// Function to execute operations in the child process
void executeChild(const char* file, int memoryToCPU[2], int cpuToMemory[2]) {
    close(memoryToCPU[0]); // Close unused read end of pipe
    close(cpuToMemory[1]); // Close unused write end of pipe

    // Load memory from the input file and execute memory operations
    loadMemory(file); // Load memory from the input file
    executableMemory(memoryToCPU[1], cpuToMemory[0]); // Execute memory operations

    close(memoryToCPU[1]); // Close write end of pipe after execution
    close(cpuToMemory[0]); // Close read end of pipe after execution
    exit(0); // Exit the child process
}

// Function to execute operations in the parent process
void executeParent(int timer, int cpuToMemory[2], int memoryToCPU[2]) {
    close(memoryToCPU[1]); // Close unused write end of pipe
    close(cpuToMemory[0]); // Close unused read end of pipe

    sleep(1);

    // Load CPU with timer and execute CPU operations
    loadCPU(timer, cpuToMemory[1], memoryToCPU[0]); // Load CPU with timer
    executableCPU(); // Execute CPU operations

    close(memoryToCPU[0]); // Close read end of pipe after execution
    close(cpuToMemory[1]); // Close write end of pipe after execution
    exit(0); // Exit the parent process
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <input_file> <timer>" << endl;
        return -1; // Incorrect number of arguments, exit with an error code
    }

    const char* file = argv[1]; // Get the input file name
    int timer = atoi(argv[2]); // Get the timer value

    int MemoryToCPU[2], CPUToMemory[2]; // Create pipes for communication

    // Check if pipe creation was successful
    if (pipe(MemoryToCPU) == -1 || pipe(CPUToMemory) == -1) {
        cout << "Failed to open pipes" << endl;
        return -1; // Pipe creation failed, exit with an error code
    }

    pid_t pid = fork(); // Create a child process

    // Check if forking the child process was successful
    if (pid == -1) {
        cout << "Fork failed" << endl;
        return -1; // Forking a child process failed, exit with an error code
    }

    // Execute different operations in child and parent processes
    if (pid == 0) {
        executeChild(file, MemoryToCPU, CPUToMemory); // Child process
    } else {
        executeParent(timer, CPUToMemory, MemoryToCPU); // Parent process
    }
}
