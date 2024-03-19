#include <stdio.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 2048

int main(int argc, char* argv[]) {

    // Step 1: Initialize Winsock
    WSADATA wsaData;
    WORD wVersion = MAKEWORD(2, 2);
    if (WSAStartup(wVersion, &wsaData)) {
        printf("Error: Unable to initialize Winsock.\n");
        return 1;
    }

    // Step 2: Create socket
    SOCKET listenSock;
    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSock == INVALID_SOCKET) {
        printf("Error: Unable to create server socket.\n");
        return 1;
    }

    // Step 3: Bind socket to address
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
    if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
        printf("Error: Unable to bind address to server socket.\n");
        return 1;
    }

    // Step 4: Listen for connections from clients
    if (listen(listenSock, 10)) {
        printf("Error: Unable to set server socket to listen state.\n");
        return 1;
    }

    printf("Server is running!\n");

    // Step 5: Communicate with clients
    while (true) {
        sockaddr_in clientAddr;
        char buff[BUFFER_SIZE], clientIP[INET_ADDRSTRLEN];
        int ret, clientAddrLen = sizeof(clientAddr), clientPort;
        SOCKET connSock;

        // Accept connection request
        connSock = accept(listenSock, (sockaddr*)&clientAddr, &clientAddrLen);
        if (connSock == INVALID_SOCKET) {
            printf("Error: Unable to accept incoming connection.\n");
            return 1;
        } else {
            inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
            clientPort = ntohs(clientAddr.sin_port);
            printf("Connection from %s:%d\n", clientIP, clientPort);
        }

        while (true) {
            // Receive message from client
            ret = recv(connSock, buff, BUFFER_SIZE, 0);
            if (ret == SOCKET_ERROR) {
                printf("Error: Unable to receive data.\n");
                break;
            } else if (ret == 0) {
                printf("Client disconnected.\n");
                break;
            }

            buff[ret] = '\0';
            printf("Received from client [%s:%d]: %s\n", clientIP, clientPort, buff);

            // Send message to client
            ret = send(connSock, buff, strlen(buff), 0);
            if (ret == SOCKET_ERROR) {
                printf("Error: Unable to send data.\n");
                break;
            }
        }

        // Close connection socket
        closesocket(connSock);
    }

    // Step 6: Clean up
    closesocket(listenSock);
    WSACleanup();

    return 0;
}