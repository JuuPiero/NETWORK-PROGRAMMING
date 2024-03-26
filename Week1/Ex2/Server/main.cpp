#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PortNumber" << std::endl;
        return 1;
    }

    int port = atoi(argv[1]);

    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return 1;
    }

    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(listenSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port " << port << std::endl;

    while (true) {
        SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            closesocket(listenSocket);
            WSACleanup();
            return 1;
        }

        char recvbuf[DEFAULT_BUFLEN];
        int recvbuflen = DEFAULT_BUFLEN;

        int iResult = recv(clientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            int sum = 0;
            bool isValid = true;
            for (int i = 0; i < iResult; ++i) {
                if (isdigit(recvbuf[i])) {
                    sum += recvbuf[i] - '0';
                } else {
                    isValid = false;
                    break;
                }
            }
            if (isValid) {
                std::cout << "Received string: " << recvbuf << std::endl;
                std::cout << "Sum of digits: " << sum << std::endl;

                std::string sumStr = std::to_string(sum);
                send(clientSocket, sumStr.c_str(), sumStr.length(), 0);
            } else {
                const char* errorMsg = "Failed: String contains non-number character.";
                send(clientSocket, errorMsg, strlen(errorMsg), 0);
            }
        } else if (iResult == 0) {
            std::cout << "Connection closed" << std::endl;
        } else {
            std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        closesocket(clientSocket);
    }

    closesocket(listenSocket);
    WSACleanup();

    return 0;
}