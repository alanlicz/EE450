#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>
#include <string.h>  // for memset
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <iostream>

#define SERVER_PORT 8080

int main() {
    // Display client boot up message
    std::cout << "Client is up and running." << std::endl;

    std::cout << "Enter Department Name: ";
    std::string deptName;
    std::cin >> deptName;

    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error." << std::endl;
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address or address not supported." << std::endl;
        return 0;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cerr << "Connection failed." << std::endl;
        return 0;
    }

    send(sock, deptName.c_str(), deptName.length(), 0);
    char buffer[1024] = {0};
    read(sock, buffer, 1024);
    std::cout << "Received Backend server ID for " << deptName << ": " << buffer
              << std::endl;

    close(sock);
    return 0;
}
