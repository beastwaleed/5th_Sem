NU-Information Exchange System 📡
=================================

A robust, multi-client network communication system designed to model the information exchange between various FAST-NUCES campuses. Implemented in **C** using **Raw Socket Programming**, this project demonstrates advanced concepts such as I/O Multiplexing, Multi-threading, and Hybrid Protocols.

🚀 Features
-----------

### Core Functionality

*   **Centralized Server Hub:** Manages connections, authentication, and message routing for all campuses.
    
*   **Inter-Campus Messaging:** Reliable TCP-based chat system allowing direct communication between campuses (e.g., Lahore to Karachi).
    
*   **Live Status Monitoring:** Real-time online/offline tracking using a background UDP heartbeat mechanism.
    

### Technical Highlights

*   **Hybrid Protocol Architecture:**
    
    *   **TCP (Port 8080):** Used for critical tasks (Authentication, Message Routing).
        
    *   **UDP (Port 8081):** Used for low-overhead tasks (Heartbeats, System Broadcasts).
        
*   **Server Concurrency:** Implemented using **I/O Multiplexing (select())** to handle multiple clients and Admin input in a single thread without blocking.
    
*   **Client Concurrency:** Implemented using **Multi-threading (pthread)** to handle UI input, heartbeat transmission, and message reception simultaneously.
    
*   **Admin Console:** Integrated non-blocking admin interface to monitor status and broadcast system-wide alerts.
    

🛠️ System Architecture
-----------------------

### 1\. The Central Server (server.c)

The server acts as the router. It maintains a list of connected clients and their sockets.

*   **Input Handling:** Uses select() to monitor:
    
    *   The Master TCP Socket (New Connections).
        
    *   The UDP Socket (Incoming Heartbeats).
        
    *   Connected Client Sockets (Incoming Chat Messages).
        
    *   STDIN (Admin Keyboard Commands).
        
*   **Routing Logic:** Parses incoming SEND:Target:Message packets and forwards them to the correct destination socket.
    

### 2\. The Campus Client (client.c)

The client represents a specific campus node. It utilizes three concurrent execution flows:

1.  **Main Thread:** Handles Authentication and User Interface (Sending messages).
    
2.  **Heartbeat Thread:** Wakes up every 5 seconds to send a UDP "Alive" packet to the server.
    
3.  **Receiver Thread:** Listens for incoming TCP messages (Chat) and UDP packets (Admin Broadcasts).
    

📦 Installation & Compilation
-----------------------------

### Prerequisites

*   **OS:** Ubuntu Linux (or any POSIX-compliant system).
    
*   **Compiler:** GCC (GNU Compiler Collection).
    

### Compilation Commands

1.  gcc server.c -o server
    
2.  Note: The -pthread flag is required for threading support.gcc client.c -o client -pthread
    

💻 Usage Instructions
---------------------

### Step 1: Start the Server

Open a terminal and run the server. It will start listening on TCP:8080 and UDP:8081.

```bash
./server
```

**Admin Commands:**

*   /status: Displays a live table of connected campuses and their heartbeat status.
    
*   /broadcast : Sends a UDP alert to all active clients.
    

### Step 2: Start a Campus Client

Open a new terminal (or multiple terminals for multiple campuses) and run the client.

```bash
./client
```

1.  Enter the Campus Name (e.g., Lahore).
    
2.  Use the menu to send messages to other connected campuses.
    

### Example Workflow

1.  **Terminal A:** Run ./server.
    
2.  **Terminal B:** Run ./client, login as **Lahore**.
    
3.  **Terminal C:** Run ./client, login as **Karachi**.
    
4.  **Action:** In Terminal B, select "Send Message", enter Target: Karachi, Message: Hello!.
    
5.  **Result:** Terminal C receives the message immediately. Server logs the routing event.
    

📂 Project Structure
--------------------

```bash   
.
├── server.c       # Source code for Central Server & Admin Console
├── client.c       # Source code for Campus Client (Multi-threaded)
├── Makefile       # (Optional) Automated build script
└── README.md      # Project Documentation
```

👥 Contributors
---------------

**Computer Networks Lab (Fall 2025)**

*   **Shawaiz Shahid:** (Server Architecture, Routing Logic)
    
*   **Rana Hamad Iftikhar:** (Client Core, Threading, Hybrid Protocol)
    
*   **Waleed Afzal:** (Admin Console, UI Design, Logging)
    

📜 License
----------

This project is for educational purposes as part of the Computer Networks curriculum at FAST-NUCES.
