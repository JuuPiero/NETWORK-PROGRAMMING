#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " ServerIPAddress ServerPortNumber" << std::endl;
        return 1;
    }

    const char* serverIP = argv[1];
    int serverPort = atoi(argv[2]);

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET connectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connectSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIP, &serverAddr.sin_addr);

    result = connect(connectSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        std::cerr << "Error connecting to server: " << WSAGetLastError() << std::endl;
        closesocket(connectSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to server" << std::endl;

    char sendbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    do {
        std::cout << "Enter a string (or empty string to exit): ";
        std::cin.getline(sendbuf, DEFAULT_BUFLEN);

        result = send(connectSocket, sendbuf, strlen(sendbuf), 0);
        if (result == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(connectSocket);
            WSACleanup();
            return 1;
        }

        if (sendbuf[0] != '\0') {
            char recvbuf[DEFAULT_BUFLEN];
            result = recv(connectSocket, recvbuf, recvbuflen, 0);
            if (result > 0) {
                std::cout << "Received: " << recvbuf << std::endl;
            } else if (result == 0) {
                std::cout << "Connection closed" << std::endl;
                break;
            } else {
                std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
                closesocket(connectSocket);
                WSACleanup();
                return 1;
            }
        }
    } while (sendbuf[0] != '\0');

    closesocket(connectSocket);
    WSACleanup();

    return 0;
}