#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include "ws2tcpip.h"
#define WM_SOCKET WM_USER + 1
#define SERVER_PORT 5500
#define SERVER_ADDR "127.0.0.1" // Server address
#define MAX_CLIENT 1024
#define BUFF_SIZE 2048
#pragma comment (lib,"ws2_32.lib")
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

SOCKET client[MAX_CLIENT];
SOCKET listenSock;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "WindowClass";

    if (!RegisterClass(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONERROR | MB_OK);
        
        return 0;
    }

    HWND hWnd = CreateWindowEx(0, "WindowClass", "WSAAsyncSelect TCP Server", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONERROR | MB_OK);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);

    // Initiate WinSock
    WSADATA wsaData;
    WORD wVersion = MAKEWORD(2, 2);
    if (WSAStartup(wVersion, &wsaData) != 0)
    {
        MessageBox(hWnd, "Winsock 2.2 is not supported.", "Error!", MB_OK);
        return 0;
    }

    // Construct socket    
    listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // requests Windows message-based notification of network events for listenSock
    WSAAsyncSelect(listenSock, hWnd, WM_SOCKET, FD_ACCEPT | FD_CLOSE | FD_READ);

    // Bind address to socket
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_ADDR, &serverAddr.sin_addr);

    if (bind(listenSock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        MessageBox(hWnd, "Cannot associate a local address with server socket.", "Error!", MB_OK);
        return 0;
    }

    // Listen request from client
    if (listen(listenSock, MAX_CLIENT) == SOCKET_ERROR)
    {
        MessageBox(hWnd, "Cannot place server socket in state LISTEN.", "Error!", MB_OK);
        return 0;
    }

    // Main message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Clean up Winsock
    WSACleanup();

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_SOCKET:
    {
        if (WSAGETSELECTERROR(lParam))
        {
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client[i] == (SOCKET)wParam)
                {
                    closesocket(client[i]);
                    client[i] = 0;
                    break;
                }
            }
        }

        switch (WSAGETSELECTEVENT(lParam))
        {
        case FD_ACCEPT:
        {
            SOCKET connSock;
            sockaddr_in clientAddr;
            int clientAddrLen = sizeof(clientAddr);
            connSock = accept((SOCKET)wParam, (sockaddr*)&clientAddr, &clientAddrLen);
            if (connSock == INVALID_SOCKET)
                break;
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client[i] == 0)
                {
                    client[i] = connSock;
                    // requests Windows message-based notification of network events for listenSock
                    WSAAsyncSelect(client[i], hwnd, WM_SOCKET, FD_READ | FD_CLOSE);
                    break;
                }
            }
            if (client[MAX_CLIENT - 1] != 0)
                MessageBox(hwnd, "Too many clients!", "Notice", MB_OK);
        }
        break;

        case FD_READ:
        {
            char rcvBuff[BUFF_SIZE], sendBuff[BUFF_SIZE];
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client[i] == (SOCKET)wParam)
                {
                    int ret = recv(client[i], rcvBuff, BUFF_SIZE, 0);
                    if (ret > 0)
                    {
                        // echo to client
                        memcpy(sendBuff, rcvBuff, ret);
                        send(client[i], sendBuff, ret, 0);
                    }
                    break;
                }
            }
        }
        break;

        case FD_CLOSE:
        {
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (client[i] == (SOCKET)wParam)
                {
                    closesocket(client[i]);
                    client[i] = 0;
                    break;
                }
            }
        }
        break;
        }
    }
    break;

    case WM_DESTROY:
        PostQuitMessage(0);
        shutdown(listenSock, SD_BOTH);
        closesocket(listenSock);
        break;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}