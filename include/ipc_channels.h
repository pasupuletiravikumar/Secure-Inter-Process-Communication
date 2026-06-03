#ifndef IPC_CHANNELS_H
#define IPC_CHANNELS_H

#include <string>

// Channel Configs
#define SHM_NAME "Local\\SecureIPC_SharedMemory"
#define SHM_PATH "/secure_ipc_shm"
#define SHM_SIZE 2048

#define PIPE_NAME "\\\\.\\pipe\\SecureIPC_NamedPipe"
#define PIPE_PATH "/tmp/secure_ipc_pipe"

#define MSG_KEY 5678
#define PORT_NUM 9090

// -------------------------------------------------------------
// Core IPC Channel Routines
// -------------------------------------------------------------

// Shared Memory
bool init_shared_memory();
bool write_shared_memory(const std::string &payload);
std::string read_shared_memory();
void close_shared_memory();

// Named Pipes
bool init_pipe_channel();
bool write_pipe_channel(const std::string &payload);
std::string read_pipe_channel();
void close_pipe_channel();

// Message Queues (System V MQ on Linux, Custom Mailslot/Queue on Windows)
bool init_message_queue();
bool write_message_queue(const std::string &payload);
std::string read_message_queue();
void close_message_queue();

// Sockets (TCP Loopback)
bool init_socket_channel();
bool write_socket_channel(const std::string &payload);
std::string read_socket_channel();
void close_socket_channel();

#endif // IPC_CHANNELS_H
