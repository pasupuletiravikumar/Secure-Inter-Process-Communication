#include "../include/ipc_channels.h"
#include <iostream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

// Handle buffers
#ifdef _WIN32
static HANDLE hShmFile = NULL;
static HANDLE hNamedPipe = INVALID_HANDLE_VALUE;
static SOCKET listenSocket = INVALID_SOCKET;
static SOCKET clientSocket = INVALID_SOCKET;
#else
static int shm_fd = -1;
static int fifo_fd = -1;
static int msg_qid = -1;
static int server_socket = -1;
static int client_socket = -1;
#endif

// -------------------------------------------------------------
// SHARED MEMORY IMPLEMENTATION
// -------------------------------------------------------------

bool init_shared_memory() {
#ifdef _WIN32
    hShmFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, SHM_SIZE, SHM_NAME);
    return hShmFile != NULL;
#else
    shm_fd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) return false;
    return ftruncate(shm_fd, SHM_SIZE) != -1;
#endif
}

bool write_shared_memory(const std::string &payload) {
    if (payload.size() >= SHM_SIZE) return false;
#ifdef _WIN32
    HANDLE hMap = OpenFileMappingA(FILE_MAP_WRITE, FALSE, SHM_NAME);
    if (!hMap) hMap = hShmFile;
    if (!hMap) return false;

    void* pBuf = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, SHM_SIZE);
    if (!pBuf) return false;

    std::memcpy(pBuf, payload.c_str(), payload.size() + 1);
    UnmapViewOfFile(pBuf);
    if (hMap != hShmFile) CloseHandle(hMap);
    return true;
#else
    int fd = shm_open(SHM_PATH, O_RDWR, 0666);
    if (fd == -1) fd = shm_fd;
    if (fd == -1) return false;

    void* pBuf = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pBuf == MAP_FAILED) return false;

    std::memcpy(pBuf, payload.c_str(), payload.size() + 1);
    munmap(pBuf, SHM_SIZE);
    if (fd != shm_fd) close(fd);
    return true;
#endif
}

std::string read_shared_memory() {
#ifdef _WIN32
    HANDLE hMap = OpenFileMappingA(FILE_MAP_READ, FALSE, SHM_NAME);
    if (!hMap) hMap = hShmFile;
    if (!hMap) return "";

    void* pBuf = MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, SHM_SIZE);
    if (!pBuf) return "";

    std::string result((char*)pBuf);
    UnmapViewOfFile(pBuf);
    if (hMap != hShmFile) CloseHandle(hMap);
    return result;
#else
    int fd = shm_open(SHM_PATH, O_RDONLY, 0666);
    if (fd == -1) fd = shm_fd;
    if (fd == -1) return "";

    void* pBuf = mmap(NULL, SHM_SIZE, PROT_READ, MAP_SHARED, fd, 0);
    if (pBuf == MAP_FAILED) return "";

    std::string result((char*)pBuf);
    munmap(pBuf, SHM_SIZE);
    if (fd != shm_fd) close(fd);
    return result;
#endif
}

void close_shared_memory() {
#ifdef _WIN32
    if (hShmFile) {
        CloseHandle(hShmFile);
        hShmFile = NULL;
    }
#else
    if (shm_fd != -1) {
        close(shm_fd);
        shm_fd = -1;
    }
    shm_unlink(SHM_PATH);
#endif
}

// -------------------------------------------------------------
// NAMED PIPES IMPLEMENTATION
// -------------------------------------------------------------

bool init_pipe_channel() {
#ifdef _WIN32
    hNamedPipe = CreateNamedPipeA(
        PIPE_NAME, PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, SHM_SIZE, SHM_SIZE, 0, NULL
    );
    return hNamedPipe != INVALID_HANDLE_VALUE;
#else
    unlink(PIPE_PATH);
    return mkfifo(PIPE_PATH, 0666) != -1;
#endif
}

bool write_pipe_channel(const std::string &payload) {
#ifdef _WIN32
    HANDLE pipe = hNamedPipe;
    bool isServer = true;
    if (pipe == INVALID_HANDLE_VALUE) {
        pipe = CreateFileA(PIPE_NAME, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        isServer = false;
    }
    if (pipe == INVALID_HANDLE_VALUE) return false;

    if (isServer) {
        ConnectNamedPipe(pipe, NULL);
    }

    DWORD written;
    BOOL ok = WriteFile(pipe, payload.c_str(), payload.size() + 1, &written, NULL);
    
    if (isServer) DisconnectNamedPipe(pipe);
    else CloseHandle(pipe);
    return ok == TRUE;
#else
    int fd = open(PIPE_PATH, O_WRONLY);
    if (fd == -1) return false;
    ssize_t bytes = write(fd, payload.c_str(), payload.size() + 1);
    close(fd);
    return bytes != -1;
#endif
}

std::string read_pipe_channel() {
#ifdef _WIN32
    HANDLE pipe = hNamedPipe;
    bool isServer = true;
    if (pipe == INVALID_HANDLE_VALUE) {
        pipe = CreateFileA(PIPE_NAME, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
        isServer = false;
    }
    if (pipe == INVALID_HANDLE_VALUE) return "";

    if (isServer) {
        ConnectNamedPipe(pipe, NULL);
    }

    char buffer[SHM_SIZE];
    DWORD bytesRead;
    BOOL ok = ReadFile(pipe, buffer, SHM_SIZE - 1, &bytesRead, NULL);
    if (ok) buffer[bytesRead] = '\0';

    if (isServer) DisconnectNamedPipe(pipe);
    else CloseHandle(pipe);
    return ok ? std::string(buffer) : "";
#else
    int fd = open(PIPE_PATH, O_RDONLY);
    if (fd == -1) return "";
    char buffer[SHM_SIZE];
    ssize_t bytes = read(fd, buffer, SHM_SIZE - 1);
    close(fd);
    if (bytes > 0) {
        buffer[bytes] = '\0';
        return std::string(buffer);
    }
    return "";
#endif
}

void close_pipe_channel() {
#ifdef _WIN32
    if (hNamedPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hNamedPipe);
        hNamedPipe = INVALID_HANDLE_VALUE;
    }
#else
    unlink(PIPE_PATH);
#endif
}

// -------------------------------------------------------------
// MESSAGE QUEUES IMPLEMENTATION
// -------------------------------------------------------------
// (Simulated using Named Pipes on Windows, native System V MQ on Linux)

bool init_message_queue() {
#ifdef _WIN32
    // Windows maps Message Queue to an alternate Named Pipe instance
    hNamedPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\SecureIPC_MsgQueue", PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, SHM_SIZE, SHM_SIZE, 0, NULL
    );
    return hNamedPipe != INVALID_HANDLE_VALUE;
#else
    msg_qid = msgget(MSG_KEY, 0666 | IPC_CREAT);
    return msg_qid != -1;
#endif
}

struct posix_msg {
    long type;
    char text[SHM_SIZE];
};

bool write_message_queue(const std::string &payload) {
    if (payload.size() >= SHM_SIZE) return false;
#ifdef _WIN32
    HANDLE pipe = CreateFileA("\\\\.\\pipe\\SecureIPC_MsgQueue", GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (pipe == INVALID_HANDLE_VALUE) return false;
    DWORD written;
    BOOL ok = WriteFile(pipe, payload.c_str(), payload.size() + 1, &written, NULL);
    CloseHandle(pipe);
    return ok == TRUE;
#else
    if (msg_qid == -1) return false;
    posix_msg msg;
    msg.type = 1;
    std::memcpy(msg.text, payload.c_str(), payload.size() + 1);
    return msgsnd(msg_qid, &msg, payload.size() + 1, 0) != -1;
#endif
}

std::string read_message_queue() {
#ifdef _WIN32
    if (hNamedPipe == INVALID_HANDLE_VALUE) return "";
    ConnectNamedPipe(hNamedPipe, NULL);
    char buffer[SHM_SIZE];
    DWORD readBytes;
    BOOL ok = ReadFile(hNamedPipe, buffer, SHM_SIZE - 1, &readBytes, NULL);
    DisconnectNamedPipe(hNamedPipe);
    if (ok) {
        buffer[readBytes] = '\0';
        return std::string(buffer);
    }
    return "";
#else
    if (msg_qid == -1) return "";
    posix_msg msg;
    if (msgrcv(msg_qid, &msg, SHM_SIZE, 1, 0) == -1) return "";
    return std::string(msg.text);
#endif
}

void close_message_queue() {
#ifdef _WIN32
    if (hNamedPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hNamedPipe);
        hNamedPipe = INVALID_HANDLE_VALUE;
    }
#else
    if (msg_qid != -1) {
        msgctl(msg_qid, IPC_RMID, NULL);
        msg_qid = -1;
    }
#endif
}

// -------------------------------------------------------------
// SOCKETS IMPLEMENTATION (TCP Loopback)
// -------------------------------------------------------------

bool init_socket_channel() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;
    
    listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        WSACleanup();
        return false;
    }
    
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT_NUM);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(listenSocket);
        WSACleanup();
        return false;
    }
    return listen(listenSocket, 1) != SOCKET_ERROR;
#else
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) return false;
    
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT_NUM);

    if (bind(server_socket, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(server_socket);
        return false;
    }
    return listen(server_socket, 1) != -1;
#endif
}

bool write_socket_channel(const std::string &payload) {
#ifdef _WIN32
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) return false;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(PORT_NUM);

    if (connect(s, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        closesocket(s);
        return false;
    }

    int bytes = send(s, payload.c_str(), payload.size() + 1, 0);
    closesocket(s);
    return bytes != SOCKET_ERROR;
#else
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) return false;

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT_NUM);
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(s);
        return false;
    }

    ssize_t bytes = send(s, payload.c_str(), payload.size() + 1, 0);
    close(s);
    return bytes != -1;
#endif
}

std::string read_socket_channel() {
#ifdef _WIN32
    if (listenSocket == INVALID_SOCKET) return "";
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) return "";

    char buffer[SHM_SIZE];
    int bytes = recv(clientSocket, buffer, SHM_SIZE - 1, 0);
    closesocket(clientSocket);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        return std::string(buffer);
    }
    return "";
#else
    if (server_socket == -1) return "";
    client_socket = accept(server_socket, NULL, NULL);
    if (client_socket == -1) return "";

    char buffer[SHM_SIZE];
    ssize_t bytes = recv(client_socket, buffer, SHM_SIZE - 1, 0);
    close(client_socket);
    
    if (bytes > 0) {
        buffer[bytes] = '\0';
        return std::string(buffer);
    }
    return "";
#endif
}

void close_socket_channel() {
#ifdef _WIN32
    if (listenSocket != INVALID_SOCKET) {
        closesocket(listenSocket);
        listenSocket = INVALID_SOCKET;
    }
    WSACleanup();
#else
    if (server_socket != -1) {
        close(server_socket);
        server_socket = -1;
    }
#endif
}
