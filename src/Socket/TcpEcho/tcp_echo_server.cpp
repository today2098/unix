#include <arpa/inet.h>   // for sockaddr_in and inet_ntoa().
#include <sys/socket.h>  // for socket(), bind() and connect().
#include <unistd.h>      // for close().

#include <cstdio>   // for printf() and fprintf().
#include <cstdlib>  // for atoi() and exit().
#include <cstring>  // for memset().

#define MAXPENDING 5   // Maximum outstanding connection requests.
#define RCVBUFSIZE 32  // Size of receive buffer.

void HandleTCPClient(int clntSocket);  // TCP client handling function.

int main(int argc, char *argv[]) {
    int servSock;                     // Socket descriptor for server.
    int clntSock;                     // Socket descriptor for client.
    struct sockaddr_in echoServAddr;  // Local address.
    struct sockaddr_in echoClntAddr;  // Client address.
    uint16_t echoServPort;            // Server port.
    socklen_t clntLen;                // Length of client address data structure.

    if(argc != 2) {  // Test for correct number of arguments.
        fprintf(stderr, "Usage:  %s <Server Port>\n", argv[0]);
        exit(1);
    }
    echoServPort = atoi(argv[1]);  // First arg: local port.

    // Create socket for incoming connections.
    if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Construct local address structure.
    memset(&echoServAddr, 0, sizeof(echoServAddr));    // Zero out structure.
    echoServAddr.sin_family = AF_INET;                 // Internet address family.
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // Any incoming interface.
    echoServAddr.sin_port = htons(echoServPort);       // Local port.

    // Bind to the local address.
    if(bind(servSock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    // Mark the socket so it will listen for incoming connections.
    if(listen(servSock, MAXPENDING) < 0) {
        perror("listen() failed");
        exit(1);
    }

    while(1) {  // Run forever.
        // Set the size of the in-out parameter.
        clntLen = sizeof(echoClntAddr);

        // Wait for a client to connect.
        if((clntSock = accept(servSock, (struct sockaddr *)&echoClntAddr, &clntLen)) < 0) {
            perror("accept() failed");
            exit(1);
        }

        // clntSock is connected to a client!.
        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));
        HandleTCPClient(clntSock);
    }

    // NOT REACHED.
}

void HandleTCPClient(int clntSocket) {
    char echoBuffer[RCVBUFSIZE];  // Buffer for echo string.
    int recvMsgSize;              // Size of received message.

    // Receive message from client.
    if((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
        perror("recv() failed");
        exit(1);
    }

    // Send received string and receive again until end of transmission.
    while(recvMsgSize > 0) {  // zero indicates end of transmission.
        // Echo message back to client.
        if(send(clntSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize) {
            perror("send() failed");
            exit(1);
        }

        // See if there is more data to receive.
        if((recvMsgSize = recv(clntSocket, echoBuffer, RCVBUFSIZE, 0)) < 0) {
            perror("recv() failed");
            exit(1);
        }
    }

    close(clntSocket);  // Close client socket.
}
