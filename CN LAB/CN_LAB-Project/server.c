#include <stdio.h>
#include <string.h>   
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>   
#include <arpa/inet.h>    
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 

#define TRUE   1
#define FALSE  0
#define TCP_PORT 8080
#define UDP_PORT 8081
#define MAX_CLIENTS 30

typedef struct {
    int socket_id;
    char name[50];
    int active;
    struct sockaddr_in udp_addr; 
    int has_udp;
} Client;

Client clients[MAX_CLIENTS];

// --- HELPER FUNCTIONS ---

void trim_newline(char *str) {
    char *pos;
    if ((pos = strchr(str, '\n')) != NULL) *pos = '\0';
}

int get_socket_by_name(char *name) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && strcmp(clients[i].name, name) == 0) {
            return clients[i].socket_id;
        }
    }
    return -1;
}

// --- ADMIN FUNCTIONS ---

void print_status_board() {
    printf("\n--- CAMPUS STATUS BOARD ---\n");
    printf("%-15s | %-8s | %-20s\n", "Campus", "SockID", "UDP Status");
    printf("------------------------------------------------\n");
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active) {
            printf("%-15s | %-8d | %-20s\n", 
                   clients[i].name, 
                   clients[i].socket_id, 
                   clients[i].has_udp ? "Online" : "Waiting...");
        }
    }
    printf("------------------------------------------------\n");
    fflush(stdout);
}

void send_broadcast(int udp_sock, char *msg) {
    printf("[ADMIN] Sending Broadcast: %s\n", msg);
    fflush(stdout);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].active && clients[i].has_udp) {
            sendto(udp_sock, msg, strlen(msg), 0, 
                  (struct sockaddr*)&clients[i].udp_addr, sizeof(clients[i].udp_addr));
        }
    }
}

int main(int argc , char *argv[]) {
    int opt = TRUE;
    int master_socket_tcp, udp_socket, addrlen, new_socket, activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address, udp_addr;
    char buffer[1025]; 
    fd_set readfds;

    // Initialize client list
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].socket_id = 0;
        clients[i].active = 0;
        clients[i].has_udp = 0;
        strcpy(clients[i].name, "Unknown");
    }

    // --- 1. SETUP TCP SOCKET ---
    if( (master_socket_tcp = socket(AF_INET , SOCK_STREAM , 0)) == 0) exit(EXIT_FAILURE);
    setsockopt(master_socket_tcp, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( TCP_PORT );
    bind(master_socket_tcp, (struct sockaddr *)&address, sizeof(address));
    listen(master_socket_tcp, 3);

    // --- 2. SETUP UDP SOCKET ---
    if( (udp_socket = socket(AF_INET , SOCK_DGRAM , 0)) == 0) exit(EXIT_FAILURE);
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    udp_addr.sin_port = htons( UDP_PORT );
    bind(udp_socket, (struct sockaddr *)&udp_addr, sizeof(udp_addr));

    printf("SERVER STARTED: TCP %d | UDP %d\n", TCP_PORT, UDP_PORT);
    printf("Commands: /status, /broadcast <msg>\n");
    fflush(stdout);

    addrlen = sizeof(address);

    // --- 3. MAIN LOOP ---
    while(TRUE) {
        FD_ZERO(&readfds);
        FD_SET(master_socket_tcp, &readfds);
        FD_SET(udp_socket, &readfds);
        FD_SET(STDIN_FILENO, &readfds); // Watch Keyboard (File Descriptor 0)

        max_sd = (master_socket_tcp > udp_socket) ? master_socket_tcp : udp_socket;
        
        // Add child sockets
        for ( i = 0 ; i < MAX_CLIENTS ; i++) {
            sd = clients[i].socket_id;
            if(sd > 0) FD_SET(sd , &readfds);
            if(sd > max_sd) max_sd = sd;
        }

        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR)) printf("select error");

        // --- EVENT A: ADMIN INPUT (Using read() instead of fgets()) ---
        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            char admin_cmd[1024];
            // CRITICAL FIX: Direct system call to read input
            int nbytes = read(STDIN_FILENO, admin_cmd, 1024);
            
            if (nbytes > 0) {
                admin_cmd[nbytes] = '\0'; // Null-terminate
                trim_newline(admin_cmd);  // Clean up

                if (strcmp(admin_cmd, "/status") == 0) {
                    print_status_board();
                } 
                else if (strncmp(admin_cmd, "/broadcast ", 11) == 0) {
                    send_broadcast(udp_socket, admin_cmd + 11);
                }
                else if (strlen(admin_cmd) > 0) { // Ignore empty enters
                    printf("[ADMIN ERROR] Unknown command: %s\n", admin_cmd);
                    fflush(stdout);
                }
            }
        }

        // --- EVENT B: NEW TCP CONNECTION ---
        if (FD_ISSET(master_socket_tcp, &readfds)) {
            if ((new_socket = accept(master_socket_tcp, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            
            for (i = 0; i < MAX_CLIENTS; i++) {
                if( !clients[i].active ) {
                    clients[i].socket_id = new_socket;
                    clients[i].active = 1;
                    clients[i].has_udp = 0;
                    strcpy(clients[i].name, "Unknown");
                    printf("[TCP:CONN] New connection on Socket %d\n", new_socket);
                    fflush(stdout);
                    break;
                }
            }
        }

        // --- EVENT C: UDP HEARTBEAT ---
        if (FD_ISSET(udp_socket, &readfds)) {
            char udp_buffer[1024];
            struct sockaddr_in sender_addr;
            socklen_t sender_len = sizeof(sender_addr);
            int len = recvfrom(udp_socket, (char *)udp_buffer, 1024, MSG_WAITALL, (struct sockaddr *) &sender_addr, &sender_len);
            udp_buffer[len] = '\0';
            
            if (strncmp(udp_buffer, "HEARTBEAT:", 10) == 0) {
                char *campus_name = udp_buffer + 10;
                trim_newline(campus_name);

                printf("[UDP:HEARTBEAT] From %s\n", campus_name);
                fflush(stdout); // Keep the logs flowing
                
                for(int j=0; j<MAX_CLIENTS; j++) {
                    if (clients[j].active && strcmp(clients[j].name, campus_name) == 0) {
                        clients[j].udp_addr = sender_addr;
                        clients[j].has_udp = 1;
                        break;
                    }
                }
            }
        }

        // --- EVENT D: CLIENT MESSAGE (TCP) ---
        for (i = 0; i < MAX_CLIENTS; i++) {
            sd = clients[i].socket_id;
            if (FD_ISSET(sd , &readfds)) {
                if ((valread = read( sd , buffer, 1024)) == 0) {
                    getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
                    printf("[TCP:DISCONNECT] %s disconnected.\n", clients[i].name);
                    fflush(stdout);
                    close( sd );
                    clients[i].socket_id = 0;
                    clients[i].active = 0;
                } else {
                    buffer[valread] = '\0';
                    trim_newline(buffer); 
                    
                    if (strncmp(buffer, "AUTH:", 5) == 0) {
                        char *name = buffer + 5;
                        strncpy(clients[i].name, name, 49);
                        clients[i].name[49] = '\0';
                        printf("[TCP:AUTH] Socket %d is now '%s'\n", sd, clients[i].name);
                        fflush(stdout);
                    }
                    else if (strncmp(buffer, "SEND:", 5) == 0) {
                        char *target = strtok(buffer + 5, ":");
                        char *msg = strtok(NULL, "");
                        
                        if (target && msg) {
                            int target_socket = get_socket_by_name(target);
                            if (target_socket != -1) {
                                char forward_msg[1100];
                                snprintf(forward_msg, sizeof(forward_msg), "From %s: %s", clients[i].name, msg);
                                send(target_socket, forward_msg, strlen(forward_msg), 0);
                                printf("[TCP:ROUTE] %s -> %s: %s\n", clients[i].name, target, msg);
                                fflush(stdout);
                            } else {
                                printf("[TCP:ERROR] Target '%s' not found.\n", target);
                                fflush(stdout);
                                char err[] = "System: Target not found.";
                                send(sd, err, strlen(err), 0);
                            }
                        }
                    }
                }
            }
        }
    }
    return 0;
}
