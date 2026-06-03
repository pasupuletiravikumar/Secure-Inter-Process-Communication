# **Secure Inter-Process Communication (IPC) Framework & Privacy Governance Hub**

A cybersecurity-focused platform designed to secure local communication between system processes using **AES-256-CBC encryption, RSA-2048 client authentication, SHA-256 data integrity verification, Sockets, Named Pipes, Shared Memory, and Message Queues**. 

This repository has been extended to integrate a **Privacy Governance Layer** tailored for B2B data flow audit operations (such as in Account-Based Marketing at Demandbase). The governance layer enforces runtime compliance with GDPR and DPDPA frameworks directly at the process serialization layer using Role-Based Access Control (RBAC), a central Consent Management Registry, and dynamic Data Minimization filters.

---

## **1. Architecture Overview**

The system operates across three conceptual layers:

```
┌────────────────────────────────────────────────────────┐
│               Client Application / CLI                 │
└──────────────────────────┬─────────────────────────────┘
                           │ Authenticates using RSA-2048
                           ▼
┌────────────────────────────────────────────────────────┐
│               Privacy Governance Layer                 │
│  - Consent Verification (Opt-in validation)            │
│  - Role-Based Access Control (Admin / Auditor / User)  │
│  - Data Classification Engine (Restricted / Public)    │
│  - Data Minimization (PII masking & Pseudonymization)  │
└──────────────────────────┬─────────────────────────────┘
                           │ Filters and sanitizes payload
                           ▼
┌────────────────────────────────────────────────────────┐
│               Cryptographic Pipeline                  │
│  - Hashing: SHA-256 payload integrity signature        │
│  - Cipher: AES-256-CBC Encryption (scrambling)          │
└──────────────────────────┬─────────────────────────────┘
                           │ Serializes ciphertext
                           ▼
┌────────────────────────────────────────────────────────┐
│                 Native IPC Transport                   │
│  - Shared Memory (Win32 File Mapping vs Linux shm)     │
│  - Named Pipes (Win32 NamedPipe vs Linux FIFO)         │
│  - Sockets (Loopback TCP Ports)                        │
│  - Message Queues (Win32 Pipe-FIFO vs Linux SysV MQ)   │
└────────────────────────────────────────────────────────┘
```

---

## **2. Directory Structure**

```
secure_ipc/
├── include/
│   ├── ipc_channels.h   # Named Pipes, Message Queues, Shared Memory, Sockets
│   ├── cryptography.h   # AES-256-CBC, RSA-2048, SHA-256 signatures
│   ├── governance.h     # Consent Management, RBAC, Data Classification
│   └── compliance.h     # GDPR & DPDPA reports, Data Subject Rights (DSR)
├── src/
│   ├── ipc_channels.cpp # Platform-specific IPC calls (Windows/POSIX)
│   ├── cryptography.cpp # Cipher and hashing mathematical engines
│   ├── governance.cpp   # RBAC maps, consent sets, and PII filters
│   └── compliance.cpp   # Audit logs, data retention, DSR eraser
├── web/
│   ├── index.html       # Cybersecurity-themed interactive dashboard
│   ├── index.css        # Vanilla CSS design system (glassmorphism dark mode)
│   └── index.js         # Simulation engine, metric registers, and DSR
├── db/
│   └── schema.sql       # SQL script modeling SQLite logging structures
├── main.cpp             # Interactive C++ CLI testing driver
├── compile.bat          # g++ compile script for Windows CMD/PowerShell
├── compile.sh           # g++ compile script for Linux/macOS
├── resume_interview.md  # Resume bullet points & interview prep guide
└── README.md            # Extensive installation & Technical manual
```

---

## **3. Database Schema (`db/schema.sql`)**

The system models log data and consent records using three core tables:

1. **`consent_registry`**: Registers consent status per Subject ID.
2. **`privacy_audit_log`**: Records all IPC transactions, classification levels, integrity signatures, and access statuses.
3. **`dsr_requests`**: Logs requested, pending, and completed Data Subject Rights operations.

Refer to [schema.sql](file:///C:/Users/Lenovo/.gemini/antigravity/scratch/secure_ipc/db/schema.sql) for full SQL definitions.

---

## **4. Technical Specification & Logical API Endpoints**

Although the system operates locally via C++ native binaries and a static web console, it exposes clear logical interfaces (APIs) for process control:

### **IPC Controllers (`include/ipc_channels.h`)**
- `bool init_shared_memory()`: Allocates memory segments using platform-native APIs.
- `bool write_shared_memory(const string &payload)`: Serializes payload into mapped buffers.
- `string read_shared_memory()`: Reads content from segments.
- `bool init_pipe_channel()`: Configures native full-duplex named pipe channels.
- `bool init_socket_channel()`: Opens TCP loopback sockets on port `9090`.

### **Governance & Consent Filters (`include/governance.h`)**
- `bool check_consent(const string &subject_id)`: Checks if consent is granted in the registry. Defaults to `false` (Opt-in security default).
- `string apply_data_minimization(const string &payload, DataClassification classification, UserRole role)`: Checks user role against classification. Automatically masks e-mails/PII to `[PII_MASKED]` for `User` roles or pseudonymizes values to SHA-256 for `Auditor` roles.

### **Compliance & Data Subject Rights (`include/compliance.h`)**
- `string execute_dsr_access(const string &subject_id)`: Implements GDPR Article 15 (Access), returns all transaction references matching subject.
- `string execute_dsr_export(const string &subject_id)`: Generates portable JSON containing subject's transaction history.
- `bool execute_dsr_deletion(const string &subject_id)`: Implements GDPR Article 17 (Right to be Forgotten), fully purging logs and disabling consent keys.

---

## **5. Compilation and Execution**

### **C++ Interactive CLI**

#### **On Windows (using MinGW g++)**:
Open CMD or PowerShell in `secure_ipc/` and run:
```cmd
compile.bat
secureIPC.exe
```

#### **On Linux / macOS**:
Open bash terminal in `secure_ipc/` and run:
```bash
chmod +x compile.sh
./compile.sh
./secureIPC
```

**CLI Walkthrough**:
1. Log in as `Process_Admin` with passphrase `admin@nexus`, or `Process_User` with passphrase `user@001`.
2. Enter your commands to test classified transfers, view the audit trail, run DSR routines, or generate GDPR compliance logs.

### **Web Visualizer Dashboard**
Double-click `web/index.html` to load the premium glassmorphism interface in any modern browser. Log in with the same credentials listed in the login dialog to test the interactive simulation and visual audit logs.
