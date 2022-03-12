#include <arpa/inet.h>   // for sockaddr_in and inet_addr().
#include <sys/socket.h>  // for socket(), connect(), send() and recv().
#include <unistd.h>      // for close().

#include <cstdio>   // for printf() and fprintf().
#include <cstdlib>  // for atoi() and exit().
#include <cstring>  // for memset().

#define RCVBUFSIZE 32  // Size of receive buffer.

int main(int argc, char *argv[]) {
    int sock;                         // Socket descriptor.
    struct sockaddr_in echoServAddr;  // Echo server address.
    uint16_t echoServPort;            // Echo server port.
    char *servIP;                     // Server IP address (dotted quad).
    char *echoString;                 // String to send to echo server.
    char echoBuffer[RCVBUFSIZE];      // Buffer for echo string.
    size_t echoStringLen;             // Length of string to echo.
    int bytesRcvd, totalBytesRcvd;    // Bytes read in single recv() and total bytes read.

    if((argc < 3) or (4 < argc)) {  // Test for correct number of arguments.
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
    servIP = argv[1];                                // First arg: server IP address (dotted quad).
    echoString = argv[2];                            // Second arg: string to echo.
    echoServPort = (argc == 4 ? atoi(argv[3]) : 7);  // Use given port, if any. 7 is the well-known port for the echo service.

    // Create a reliable, stream socket using TCP.
    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    // Construct the server address structure.
    memset(&echoServAddr, 0, sizeof(echoServAddr));    // Zero out structure.
    echoServAddr.sin_family = AF_INET;                 // Internet address family.
    echoServAddr.sin_addr.s_addr = inet_addr(servIP);  // Server IP address.
    echoServAddr.sin_port = htons(echoServPort);       // Server port.

    // Establish the connection to the echo server.
    if(connect(sock, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr)) < 0) {
        perror("connect() failed");
        exit(1);
    }

    echoStringLen = strlen(echoString);  // Determine input length.

    // Send the string to the server.
    if(send(sock, echoString, echoStringLen, 0) != echoStringLen) {
        perror("send() sent a different number of bytes than expected");
        exit(1);
    }

    // Receive the same string back from the server.
    totalBytesRcvd = 0;
    printf("Received: ");  // Setup to print the echoed string.
    while(totalBytesRcvd < echoStringLen) {
        // Receive up to the buffer size (minus 1 to leave space for a null terminator) bytes from the sender.
        if((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0) {
            perror("recv() failed or connection closed prematurely");
            exit(1);
        }
        totalBytesRcvd += bytesRcvd;   // Keep tally of total bytes.
        echoBuffer[bytesRcvd] = '\0';  // Terminate the string!
        printf("%s", echoBuffer);      // Print the echo buffer.
    }
    printf("\n");  // Print a final linefeed.

    close(sock);
    exit(0);
}
