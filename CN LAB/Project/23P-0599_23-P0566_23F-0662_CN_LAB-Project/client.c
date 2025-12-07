// client.c - Campus Client for Semester Project
// Satisfies Checklist Items:
// [x] Connection Setup (TCP to Server)
// [x] Hybrid Protocol (TCP + Background UDP Heartbeat)
// [x] User Interface (Console Menu)
// [x] Message Reception (TCP)
// [x] Broadcast Listener (UDP)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>       // For close() and sleep()
#include <arpa/inet.h>    // For IP address conversion
#include <sys/socket.h>   // For socket functions
#include <pthread.h>      // For threading
#include <sys/select.h>   // For handling multiple inputs at once

// --- CONFIGURATION ---
#define SERVER_IP "127.0.0.1" // The IP of the server (Localhost)
#define TCP_PORT 8080         // Port for Chat messages
#define UDP_PORT 8081         // Port for Heartbeats

// --- GLOBAL VARIABLES ---
// We make these global so our threads can access them easily
int tcp_socket = 0;
int udp_socket = 0;
struct sockaddr_in server_udp_address; // To know where to send heartbeats
char campus_name[50];                  // To store "Lahore", "Karachi", etc.
int is_running = 1;                    // Controls the main loop

// --- THREAD 1: HEARTBEAT SENDER (UDP) ---
// This function runs in the background. 
// It wakes up every 5 seconds to tell the server "I am alive".
void *send_heartbeats(void *arg) {
    char heartbeat_msg[100];
    
    while(is_running) {
        // Step 1: Format the message (e.g., "HEARTBEAT:Lahore")
        snprintf(heartbeat_msg, sizeof(heartbeat_msg), "HEARTBEAT:%s", campus_name);
        
        // Step 2: Send the UDP packet to the Server
        sendto(udp_socket, heartbeat_msg, strlen(heartbeat_msg), 0, 
              (struct sockaddr*)&server_udp_address, sizeof(server_udp_address));
        
        // Step 3: Wait for 5 seconds before sending the next one
        sleep(5);
    }
    return NULL;
}

// --- THREAD 2: MESSAGE RECEIVER (TCP & UDP) ---
// This function listens for Chat Messages (TCP) and Admin Broadcasts (UDP)
void *receive_messages(void *arg) {
    fd_set readfds;       // A set of file descriptors to monitor
    int max_sd;           // The highest socket number (needed for select function)
    char buffer[1024];    // Temporary storage for incoming messages

    while(is_running) {
        // Step 1: Clear the list and add our two sockets
        FD_ZERO(&readfds);
        FD_SET(tcp_socket, &readfds);
        FD_SET(udp_socket, &readfds);

        // Calculate which socket number is higher
        if (tcp_socket > udp_socket) {
            max_sd = tcp_socket;
        } else {
            max_sd = udp_socket;
        }

        // Step 2: Wait for activity on EITHER socket (Wait forever until something happens)
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        // If there was an error with select, just skip this loop iteration
        if ((activity < 0)) continue;

        // --- CHECK 1: Is there a new Chat Message? (TCP) ---
        if (FD_ISSET(tcp_socket, &readfds)) {
            int valread = read(tcp_socket, buffer, 1024);
            
            // If read returns 0 or less, the server crashed or closed connection
            if (valread <= 0) {
                printf("\nServer disconnected.\n");
                is_running = 0;
                break;
            }
            buffer[valread] = '\0'; // Add null terminator to make it a valid string
            
            // --- UI REPRINT LOGIC (Kept exactly as requested) ---
            printf("\n\n*** INCOMING MESSAGE ***\n");
            printf("From: %s\n", buffer);
            printf("------------------------\n");
            printf("1. Send Message | 2. Exit\n");
            printf("> "); 
            fflush(stdout); 
        }

        // --- CHECK 2: Is there a new Admin Broadcast? (UDP) ---
        if (FD_ISSET(udp_socket, &readfds)) {
            int valread = recvfrom(udp_socket, buffer, 1024, 0, NULL, NULL);
            buffer[valread] = '\0';
            
            // --- UI REPRINT LOGIC (Kept exactly as requested) ---
            printf("\n\n*** ADMIN BROADCAST ***\n");
            printf("Msg: %s\n", buffer);
            printf("------------------------\n");
            printf("1. Send Message | 2. Exit\n");
            printf("> ");
            fflush(stdout);
        }
    }
    return NULL;
}

// --- MAIN FUNCTION ---
int main() {
    struct sockaddr_in server_address; // For TCP connection
    struct sockaddr_in my_udp_address; // For listening to broadcasts
    pthread_t heartbeat_thread;        // ID for the heartbeat thread
    pthread_t receiver_thread;         // ID for the receiver thread

    // -----------------------------------------------------------
    // PART 1: SETUP UDP SOCKET (For Heartbeats & Broadcasts)
    // -----------------------------------------------------------
    udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_socket < 0) {
        perror("Error creating UDP Socket");
        return -1;
    }

    // Prepare address to listen for broadcasts on any port
    memset(&my_udp_address, 0, sizeof(my_udp_address));
    my_udp_address.sin_family = AF_INET;
    my_udp_address.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    my_udp_address.sin_port = 0;                 // Let the OS pick a random free port for us
    
    // Bind the UDP socket
    if (bind(udp_socket, (struct sockaddr *)&my_udp_address, sizeof(my_udp_address)) < 0) {
        perror("Error binding UDP Socket");
        return -1;
    }

    // Prepare the Server's UDP address (Where we send heartbeats TO)
    memset(&server_udp_address, 0, sizeof(server_udp_address));
    server_udp_address.sin_family = AF_INET;
    server_udp_address.sin_port = htons(UDP_PORT);
    server_udp_address.sin_addr.s_addr = inet_addr(SERVER_IP);

    // -----------------------------------------------------------
    // PART 2: SETUP TCP SOCKET (For Chatting)
    // -----------------------------------------------------------
    tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_socket < 0) {
        printf("\nError creating TCP Socket \n");
        return -1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(TCP_PORT);

    // Convert IP string to binary format
    if (inet_pton(AF_INET, SERVER_IP, &server_address.sin_addr) <= 0) {
        printf("\nInvalid address / Address not supported \n");
        return -1;
    }

    // -----------------------------------------------------------
    // PART 3: CONNECT & LOGIN
    // -----------------------------------------------------------
    printf("Enter Campus Name (e.g., Lahore): ");
    scanf("%s", campus_name);

    // Try to connect to the server
    if (connect(tcp_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        printf("\nConnection Failed. Is the server running? \n");
        return -1;
    }

    // Send the Authentication message automatically (e.g., "AUTH:Lahore")
    char auth_msg[100];
    snprintf(auth_msg, sizeof(auth_msg), "AUTH:%s", campus_name);
    send(tcp_socket, auth_msg, strlen(auth_msg), 0);
    
    printf("Connected to Server. Authenticated as %s\n", campus_name);

    // -----------------------------------------------------------
    // PART 4: LAUNCH BACKGROUND TASKS
    // -----------------------------------------------------------
    // Start the thread that sends heartbeats
    pthread_create(&heartbeat_thread, NULL, send_heartbeats, NULL);
    // Start the thread that listens for incoming messages
    pthread_create(&receiver_thread, NULL, receive_messages, NULL);

    // -----------------------------------------------------------
    // PART 5: MAIN MENU LOOP (User Interface)
    // -----------------------------------------------------------
    int choice;
    char target_campus[50];
    char message_body[1024];
    char final_packet[1100];

    while(is_running) {
        // Display Menu
        printf("\n--- %s Client Menu ---\n", campus_name);
        printf("1. Send Message\n");
        printf("2. Exit\n");
        printf("> ");
        
        // Read user choice
        if (scanf("%d", &choice) != 1) {
            // If user types a letter instead of number, clear the input to prevent infinite loop
            while(getchar() != '\n');
            continue;
        }

        if (choice == 1) {
            printf("Enter Target Campus: ");
            scanf("%s", target_campus);
            
            printf("Enter Message: ");
            getchar(); // Remove the "Enter" key left over from previous scanf
            fgets(message_body, 1024, stdin);
            
            // Remove the newline character at the end of the message
            message_body[strcspn(message_body, "\n")] = 0; 

            // Format the packet: "SEND:Target:Message"
            snprintf(final_packet, sizeof(final_packet), "SEND:%s:%s", target_campus, message_body);
            
            // Send it via TCP
            send(tcp_socket, final_packet, strlen(final_packet), 0);
            
        } else if (choice == 2) {
            is_running = 0; // Stop the loop
        } else {
            printf("\nError: Invalid Input!\n");
        }
    }

    // Cleanup
    close(tcp_socket);
    close(udp_socket);
    return 0;
}
