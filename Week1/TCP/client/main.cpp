#include <stdio.h>
#include <string.h>

#include <stdlib.h>
#include <Winsock2.h>
#include <WS2tcpip.h>

#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1"

#define BUFF_SIZE 2048

#pragma comment(lib, "Ws2_32.lib")

int main(int argc, char* argv[]) {

  // Step 1: Initialize WinSock
  WSADATA wsaData;
  WORD wVersion = MAKEWORD(2, 2);

  if (WSAStartup(wVersion, &wsaData)) {
    printf("Winsock 2.2 is not supported\n");
    return 0;
  }

  // Step 2: Create socket
  SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (client == INVALID_SOCKET) {
    printf("Error %d: Unable to create server socket.\n", WSAGetLastError());
    return 0;
  }

  // (Optional) Set receive timeout
  int tv = 10000; // Timeout: 10000ms
  setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(int));

  // Step 3: Define server address
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

  // Step 4: Send connection request to the server
  if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
    printf("Error %d: Unable to connect to server.\n", WSAGetLastError());
    return 0;
  }

  printf("Connected to the server\n");

  // Step 5: Communicate with the server
  char buff[BUFF_SIZE];
  int ret, messageLen;

  // Send message
  while (1) {
    printf("Send to server: ");
    gets_s(buff, BUFF_SIZE);
    messageLen = strlen(buff);

    if (messageLen == 0) break;

    ret = send(client, buff, messageLen, 0);

    if (ret == SOCKET_ERROR) {
      printf("Error %d: Unable to send data.\n", WSAGetLastError());
      break;
    }

    // Receive response message
    ret = recv(client, buff, BUFF_SIZE, 0);

    if (ret == SOCKET_ERROR) {
      if (WSAGetLastError() == WSAETIMEDOUT) {
        printf("Timeout!\n");
      } else {
        printf("Error %d: Unable to receive data.\n", WSAGetLastError());
      }
      break;
    } else if (strlen(buff) > 0) {
      buff[ret] = 0;
      printf("Received from server: %s\n", buff);
    }
  }

  // Step 6: Close socket
  closesocket(client);

  // Step 7: Cleanup Winsock
  WSACleanup();

  return 0;
}