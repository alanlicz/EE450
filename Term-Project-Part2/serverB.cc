#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

const int CLIENT_PORT = 31675;
const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 33675;

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket" << std::endl;
        exit(1);
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        std::cerr << "Error binding socket to client port" << std::endl;
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    const char* message = "ECE Art Law";
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&server_addr,
           sizeof(server_addr));

    std::cout << "Client A sent: " << message << std::endl;

    close(sockfd);
    return 0;
}
