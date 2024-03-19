#include "stdio.h"
#include "winsock2.h"

#include "ws2tcpip.h"

#define SERVER_PORT 5500

#define SERVER_ADDR "127.0.0.1"

#define BUFF_SIZE 2048

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {

  // Step 1: Initialize Winsock
  WSADATA wsaData;
  WORD wVersion = MAKEWORD(3, 2);
  if (WSAStartup(wVersion, &wsaData)) {
    printf("Winsock 2.2 not supported\n");
    return 0;
  }

  // Step 2: Create socket
  SOCKET server;
  server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (server == INVALID_SOCKET) {
    printf("Error: Cannot create server socket.\n");
    return 0;
  }

  // Step 3: Bind address to socket
  sockaddr_in serverAddr;
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(SERVER_PORT);
  inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);
  if (bind(server, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
    printf("Error: Cannot bind address to socket.\n");
    return 0;
  }

  printf("Server started!\n");

  // Step 4: Communicate with client
  sockaddr_in clientAddr;
  char buff[BUFF_SIZE], clientIP[INET_ADDRSTRLEN];
  int ret, clientAddrLen = sizeof(clientAddr), clientPort;

  while (true) {
    // Receive message
    ret = recvfrom(server, buff, BUFF_SIZE, 0, (sockaddr*)&clientAddr,
                   &clientAddrLen);
    if (ret == SOCKET_ERROR) {
      printf("Error: Cannot receive data.\n");
    } else if (strlen(buff) > 0) {
      buff[ret] = 0;
      inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, sizeof(clientIP));
      clientPort = ntohs(clientAddr.sin_port);
      printf("Received from client [%s:%d]: %s\n", clientIP, clientPort, buff);

      // Send message back to client
      ret = sendto(server, buff, strlen(buff), 0, (sockaddr*)&clientAddr,
                   sizeof(clientAddr));
      if (ret == SOCKET_ERROR) {
        printf("Error: Cannot send data.\n");
      }
    }
  }

  // Step 5: Close socket
  closesocket(server);

  // Step 6: Clean up Winsock
  WSACleanup();

  return 0;
}