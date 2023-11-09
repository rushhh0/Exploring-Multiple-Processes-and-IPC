void loadCPU(int timerCountdown, int writePipe, int readPipe);
void executableCPU();
int accessMemory(int address, int metadata, char operation);
int readMemory(int address);
void writeMemory(int address, int metadata);
void execute();