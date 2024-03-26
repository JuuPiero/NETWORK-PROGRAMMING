#include <iostream>
#include <string>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

// Hàm gửi yêu cầu và nhận kết quả từ server
std::string send_request(const std::string& server_ip, int server_port, const std::string& request) {
    // Khởi tạo Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return "";
    }

    // Tạo socket UDP
    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Error creating socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return "";
    }

    // Thiết lập địa chỉ và số hiệu cổng của server
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str());

    // Gửi yêu cầu tới server
    int bytes_sent = sendto(client_socket, request.c_str(), request.length(), 0,
                             (sockaddr*)&server_addr, sizeof(server_addr));
    if (bytes_sent == SOCKET_ERROR) {
        std::cerr << "sendto() failed with error: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return "";
    }

    // Nhận kết quả từ server
    char recv_buffer[1024];
    memset(recv_buffer, 0, sizeof(recv_buffer));
    sockaddr_in from_addr;
    int from_addr_len = sizeof(from_addr);
    int bytes_received = recvfrom(client_socket, recv_buffer, sizeof(recv_buffer), 0,
                                   (sockaddr*)&from_addr, &from_addr_len);
    if (bytes_received == SOCKET_ERROR) {
        std::cerr << "recvfrom() failed with error: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return "";
    }

    // Đóng socket và dọn dẹp Winsock
    closesocket(client_socket);
    WSACleanup();

    return std::string(recv_buffer, bytes_received);
}

int main(int argc, char* argv[]) {
    // Kiểm tra số lượng tham số dòng lệnh
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " ServerIPAddress ServerPortNumber\n";
        return 1;
    }

    // Lấy địa chỉ IP và số hiệu cổng của server từ tham số dòng lệnh
    std::string server_ip = argv[1];
    int server_port = std::atoi(argv[2]);

    while (true) {
        // Nhập tên miền từ người dùng
        std::string domain_name;
        std::cout << "Enter domain name (or enter empty string to exit): ";
        std::getline(std::cin, domain_name);

        // Kiểm tra nếu người dùng nhập chuỗi rỗng thì thoát khỏi vòng lặp
        if (domain_name.empty()) {
            break;
        }

        // Gửi yêu cầu tới server và nhận kết quả
        std::string request_result = send_request(server_ip, server_port, domain_name);
        std::cout << "OUTPUT" << std::endl;
        std::cout << request_result << std::endl;
    }

    return 0;
}