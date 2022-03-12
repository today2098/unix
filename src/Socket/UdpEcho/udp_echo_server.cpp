#include <arpa/inet.h>   // for sockaddr_in and inet_ntoa().
#include <sys/socket.h>  // for socket() and bind().
#include <unistd.h>      // for close().

#include <cstdio>   // for printf() and fprintf().
#include <cstdlib>  // for atoi() and exit().
#include <cstring>  // for memset().

#define ECHOMAX 255  // Longest string to echo.

void DieWithError(char *errorMessage);  // External error handling function.

int main(int argc, char *argv[]) {
    int sock;                         // Socket.
    struct sockaddr_in echoServAddr;  // Local address.
    struct sockaddr_in echoClntAddr;  // Client address.
    unsigned int cliAddrLen;          // Length of incoming message.
    char echoBuffer[ECHOMAX];         // Buffer for echo string.
    uint16_t echoServPort;            // Server port.
    ssize_t recvMsgSize;              // Size of received message.

    if(argc != 2) {  // Test for correct number of parameters.
        fprintf(stderr, "Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    echoServPort = atoi(argv[1]);  // First arg:  local port.

    // Create socket for sending/receiving datagrams.
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Construct local address structure.
    memset(&echoServAddr, 0, sizeof(echoServAddr));    // Zero out structure.
    echoServAddr.sin_family = AF_INET;                 // Internet address family.
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);  // Any incoming interface.
    echoServAddr.sin_port = htons(echoServPort);       // Local port.

    // Bind to the local address.
    if(bind(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    while(1) {  // Run forever.
        // Set the size of the in-out parameter.
        cliAddrLen = sizeof(echoClntAddr);

        // Block until receive message from a client.
        if((recvMsgSize = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&echoClntAddr, &cliAddrLen)) < 0) {
            perror("recvfrom() failed");
            exit(1);
        }

        printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

        // Send received datagram back to the client.
        if(sendto(sock, echoBuffer, recvMsgSize, 0, (struct sockaddr *)&echoClntAddr, sizeof(echoClntAddr)) != recvMsgSize) {
            perror("sendto() sent a different number of bytes than expected");
            exit(1);
        }
    }

    // NOT REACHED.
}
