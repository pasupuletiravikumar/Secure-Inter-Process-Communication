@echo off
echo Compiling Secure IPC with Privacy Governance Hub for Windows...
g++ -O3 -std=c++17 -o secureIPC src/ipc_channels.cpp src/cryptography.cpp src/governance.cpp src/compliance.cpp main.cpp -lws2_32
if %ERRORLEVEL% EQU 0 (
    echo Compilation Successful! Run secureIPC.exe to start.
) else (
    echo Compilation Failed! Please check if g++ is installed and in PATH.
)
pause
