#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <ctype.h>
#include "CPU.h"

using namespace std;

// Define constants for user and kernel modes
int USER = 0;
int KERNEL = 1;

// Variables for program state and control flow
int runningMode; // Stores the current execution mode (USER or KERNEL)
bool isRunning = true; // Indicates whether the program is still running

// Pipes for communication between processes
int wp; // Write pipe
int rp; // Read pipe

// Memory addresses for system and stack
int systemAddress = 1000; // System memory starts at address 1000
int systemStack = 2000;   // System stack starts at address 2000

// CPU registers and control variables
int PC, SP, IR, AC, X, Y; // CPU registers
int interruptTimer;       // Timer for interrupts
int timeout;              // Timer threshold for interrupts

// Read data from a specific memory address
int readMemory(int address){
    return accessMemory(address, 0, 'R'); // Read value from memory
}

// Write data to a specific memory address
void writeMemory(int address, int metadata){
    accessMemory(address, metadata, 'W'); // Write value to memory
}


// Function to access memory for read or write operations
int accessMemory(int address, int metadata, char operation){
    if (address >= systemAddress && runningMode == USER){
        // Memory access violation in user mode, handle and exit with error
        cout << "Memory violation: accessing system address " << address << " in user mode" << endl;
        write(wp, "E", sizeof(char));
        exit(-1);
    }

    int value;

    // Perform read or write operation via pipes
    write(wp, &operation, sizeof(char));
    write(wp, &address, sizeof(int));

    if (operation == 'W'){
        write(wp, &metadata, sizeof(int));
    }
    else if (operation == 'R'){
        read(rp, &value, sizeof(int));
    }

    return (operation == 'R') ? value : 0; 
}

// Initialize CPU with specified timer countdown and communication pipes
void loadCPU(int timerCountdown, int writePipe, int readPipe){
    // Reset CPU registers and set up communication pipes
    PC = AC = X = Y = interruptTimer = IR = 0;
    SP = 1000;

    wp = writePipe;
    rp = readPipe;

    runningMode = USER;
    timeout = timerCountdown;
}

// Execute CPU instructions
void executableCPU(){
    while (isRunning){
        IR = accessMemory(PC++, 0, 'R'); // Fetch instruction from memory

        switch (IR){
        case 1: //Load the value into the AC
            AC = readMemory(PC++);
            break;
        case 2: //Load the value at the address into the AC
            AC = readMemory(readMemory(PC++));
            break;
        case 3: //Load the value from the address found in the given address into the AC
            AC = readMemory(readMemory(readMemory(PC++)));
            break;
        case 4: //Load the value at (address+X) into the AC
            AC = readMemory(readMemory(PC++) + X);
            break;
        case 5: //Load the value at (address+Y) into the AC
            AC = readMemory(readMemory(PC++) + Y);
            break;
        case 6: //Load from (Sp+X) into the AC
            AC = readMemory(SP + X);
            break;
        case 7: //Store the value in the AC into the address
            writeMemory(readMemory(PC++), AC);
            break;
        case 8: //Gets a random int from 1 to 100 into the AC
            AC = rand() % 100 + 1;
            break;
        case 9:{
            int operand = readMemory(PC++);
            if (operand == 1){ //If port=1, writes AC as an int to the screen
                cout << AC;
            }
            else if (operand == 2){ //If port=2, writes AC as a char to the screen
                cout << (char)AC;
            }
            break;
            }
        case 10: //Add the value in X to the AC
            AC += X;
            break;
        case 11: //Add the value in Y to the AC
            AC += Y;
            break;
        case 12: //Subtract the value in X from the AC
            AC -= X;
            break;
        case 13: //Subtract the value in Y from the AC
            AC -= Y;
            break;
        case 14: //Copy the value in the AC to X
            X = AC;
            break;
        case 15: //Copy the value in X to the AC
            AC = X;
            break;
        case 16: //Copy the value in the AC to Y
            Y = AC;
            break;
        case 17: //Copy the value in Y to the AC
            AC = Y;
            break;
        case 18: //Copy the value in AC to the SP
            SP = AC;
            break;
        case 19: //Copy the value in SP to the AC 
            AC = SP;
            break;
        case 20: //Jump to the address
            PC = readMemory(PC++);
            break;
        case 21:{ //Jump to the address only if the value in the AC is zero
            int value = readMemory(PC++);
            if (AC == 0){
                PC = value;
            }
            break;
            }
        case 22:{ //Jump to the address only if the value in the AC is not zero
            int value = readMemory(PC++);
            if (AC != 0){
                PC = value;
            }
            break;
            }
        case 23:{ //Push return address onto stack, jump to the address
            int address = readMemory(PC++);
            SP--;
            writeMemory(SP, PC);
            PC = address;
            break;
            }
        case 24:{ //Pop return address from the stack, jump to the address
            PC = readMemory(SP);
            SP++;
            break;
            }
        case 25: //Increment the value in X
            X++;
            break;
        case 26: //Decrement the value in X
            X--;
            break;
        case 27: //Push AC onto stack
            SP--;
            writeMemory(SP, AC);
            break;
        case 28: //Pop from stack into AC
            AC = readMemory(SP++);
            break;
        case 29: //Perform system call
            if (runningMode == USER){
                runningMode = KERNEL;
                int tempSP = SP;
                SP = systemStack;
                SP--;
                writeMemory(SP, tempSP);
                SP--;
                writeMemory(SP, PC);
                PC = 1500;
            }
            break;
        case 30: //Return from system call
            PC = readMemory(SP++);
            SP = readMemory(SP++);
            runningMode = USER;
            break;
        case 50: //End execution
            isRunning = false;
            write(wp, "E", sizeof(char));
            break;
        default:
            cout << "ERROR: Invalid Instruction Entered." << endl;
            write(wp, "E", sizeof(char));
            exit(-1);
        }

        interruptTimer++;

        
        if (runningMode == USER && interruptTimer >= timeout){
            // The timer is reset, the mode switches to KERNEL, and relevant state information is saved
            interruptTimer -= timeout;
            runningMode = KERNEL;
            int tempSP = SP;
            SP = systemStack;
            SP--;
            writeMemory(SP, tempSP);
            SP--;
            writeMemory(SP, PC);
            PC = 1000;
        }
    }
    write(wp, "E", sizeof(char)); // Signal end of execution
}
