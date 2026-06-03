#!/bin/bash
echo "Compiling Secure IPC with Privacy Governance Hub for Linux..."
g++ -O3 -std=c++17 -o secureIPC src/ipc_channels.cpp src/cryptography.cpp src/governance.cpp src/compliance.cpp main.cpp -lrt -pthread
if [ $? -eq 0 ]; then
    echo "Compilation Successful! Run ./secureIPC to start."
    chmod +x secureIPC
else
    echo "Compilation Failed!"
fi
