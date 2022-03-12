#include <arpa/inet.h>   // for sockaddr_in and inet_addr().
#include <sys/socket.h>  // for socket(), connect(), sendto() and recvfrom().
#include <unistd.h>      // for close().

#include <cstdio>   // for printf() and fprintf().
#include <cstdlib>  // for atoi() and exit().
#include <cstring>  // for memset().

#define ECHOMAX 255  // Longest string to echo.

int main(int argc, char *argv[]) {
    int sock;                         // Socket descriptor.
    struct sockaddr_in echoServAddr;  // Echo server address.
    struct sockaddr_in fromAddr;      // Source address of echo.
    uint16_t echoServPort;            // Echo server port.
    unsigned int fromSize;            // In-out of address size for recvfrom().
    char *servIP;                     // IP address of server.
    char *echoString;                 // String to send to echo server.
    char echoBuffer[ECHOMAX + 1];     // Buffer for receiving echoed string.
    size_t echoStringLen;             // Length of string to echo.
    ssize_t respStringLen;            // Length of received response.

    if(argc < 3 or 4 < argc) {  // Test for correct number of arguments.
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
    servIP = argv[1];                                     // First arg: server IP address (dotted quad).
    echoString = argv[2];                                 // Second arg: string to echo.
    if((echoStringLen = strlen(echoString)) > ECHOMAX) {  // Check input length.
        perror("Echo word too long");
        exit(1);
    }
    echoServPort = (argc == 4 ? atoi(argv[3]) : 7);  // Use given port, if any. 7 is the well-known port for the echo service.

    // Create a datagram/UDP socket.
    if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Construct the server address structure.
    memset(&echoServAddr, 0, sizeof(echoServAddr));    // Zero out structure.
    echoServAddr.sin_family = AF_INET;                 // Internet addr family.
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  // Server IP address.
    echoServAddr.sin_port = htons(echoServPort);       // Server port.

    // Send the string to the server.
    if(sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) != echoStringLen) {
        perror("sendto() sent a different number of bytes than expected");
        exit(1);
    }

    // Recv a response.
    fromSize = sizeof(fromAddr);
    if((respStringLen = recvfrom(sock, echoBuffer, ECHOMAX, 0, (struct sockaddr *)&fromAddr, &fromSize)) != echoStringLen) {
        perror("recvfrom() failed");
        exit(1);
    }

    if(echoServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr) {
        fprintf(stderr, "Error: received a packet from unknown source.\n");
        exit(1);
    }

    // null-terminate the received data.
    echoBuffer[respStringLen] = '\0';
    printf("Received: %s\n", echoBuffer);  // Print the echoed arg.

    close(sock);
    exit(0);
}
