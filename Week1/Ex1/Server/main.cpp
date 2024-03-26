#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// Hàm phân giải tên miền hoặc địa chỉ IP
std::string resolve_domain(const std::string& domain_name) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;  // Sử dụng IPv4
    hints.ai_socktype = SOCK_DGRAM;  // Sử dụng giao thức UDP

    int result = getaddrinfo(domain_name.c_str(), nullptr, &hints, &res);
    if (result != 0) {
        return "Failed to resolve domain name";
    }

    char ip_buffer[INET_ADDRSTRLEN];
    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    const char* ip = inet_ntop(AF_INET, &(addr->sin_addr), ip_buffer, INET_ADDRSTRLEN);
    if (ip == nullptr) {
        freeaddrinfo(res);
        return "Failed to resolve domain name";
    }

    freeaddrinfo(res);
    return ip;
}

int main(int argc, char* argv[]) {
    // Kiểm tra số lượng tham số dòng lệnh
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " PortNumber\n";
        return 1;
    }

    // Lấy số hiệu cổng từ tham số dòng lệnh
    int port = std::atoi(argv[1]);

    // Khởi tạo Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    // Tạo socket UDP
    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Bind socket vào địa chỉ và số hiệu cổng đã chỉ định
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port " << port << "..." << std::endl;

    while (true) {
        sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        #define BUFF_SIZE 2048
        // char recv_buffer[1024];
        char recv_buffer[2048];

        memset(recv_buffer, 0, sizeof(recv_buffer));

        // Nhận dữ liệu từ client
        int bytes_received = recvfrom(server_socket, recv_buffer, sizeof(recv_buffer), 0,
                                      (sockaddr*)&client_addr, &client_addr_len);
        if (bytes_received == SOCKET_ERROR) {
            std::cerr << "recvfrom() failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }

        // Phân giải tên miền hoặc địa chỉ IP
        std::string domain_name(recv_buffer, bytes_received);
        std::string resolved_ip = resolve_domain(domain_name);

        // Gửi kết quả phân giải về client
        int bytes_sent = sendto(server_socket, resolved_ip.c_str(), resolved_ip.length(), 0,
                                 (sockaddr*)&client_addr, sizeof(client_addr));
        if (bytes_sent == SOCKET_ERROR) {
            std::cerr << "sendto() failed with error: " << WSAGetLastError() << std::endl;
            continue;
        }
    }

    // Đóng socket và dọn dẹp Winsock
    closesocket(server_socket);
    WSACleanup();

    return 0;
}