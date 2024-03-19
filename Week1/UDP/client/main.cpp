#include <stdio.h>
#include <winsock2.h>

#include <ws2tcpip.h>
#define SERVER_PORT 5500

#define SERVER_ADDR "127.0.0.1"
#define BUFF_SIZE 2048

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char *argv[])
{

    // Step 1: Initialize Winsock
    WSADATA wsaData;

    WORD wVersion = MAKEWORD(2, 2);
    if (WSAStartup(wVersion, &wsaData))
    {
        printf("Winsock 2.2 is not supported\n");
        return 0;
    }

    // Step 2: Create a socket.
    SOCKET client;

    client = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client == INVALID_SOCKET)
    {
        printf("Error %d: Cannot create server socket.", WSAGetLastError());
        return 0;
    }

    printf("Client started!\n");

    // (optional) Set receive timeout
    int tv = 10000; // Timeout period: 10000ms
    setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(int));

    // Step 3: Define server address
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

    // Step 4: Communicate with server
    char buff[BUFF_SIZE];

    int ret, serverAddrLen;
    while (1)
    {
        serverAddrLen = sizeof(serverAddr);
        int messageLen;

        // Send message
        printf("Send to server: ");
        gets_s(buff, BUFF_SIZE);
        messageLen = strlen(buff);

        if (messageLen == 0)
            break;

        ret = sendto(client, buff, messageLen, 0, (sockaddr *)&serverAddr, serverAddrLen);
        if (ret == SOCKET_ERROR)
        {
            printf("Error %d: Cannot send message.", WSAGetLastError());
            return 0;
        }

        // Receive response message
        ret = recvfrom(client, buff, BUFF_SIZE, 0, (sockaddr *)&serverAddr, &serverAddrLen);

        if (ret == SOCKET_ERROR)
        {
            if (WSAGetLastError() == WSAETIMEDOUT)
            {
                printf("Timeout!\n");
            }
            else
            {
                printf("Error %d: Cannot receive message.", WSAGetLastError());
                return 0;
            }
        }
        else if (strlen(buff) > 0)
        {
            buff[ret] = 0;
            printf("Received from server: %s\n", buff);
        }
    }

    // Step 5: Close socket
    closesocket(client);

    // Step 6: Shutdown Winsock
    WSACleanup();

    return 0;
}