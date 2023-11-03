#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::map;
using std::string;
using std::to_string;
using std::vector;

const int PORT = 30675;
const int SERVER_PORT = 33675;
const char* SERVER_IP = "127.0.0.1";
#define FILENAME "dataA.txt"
#define CLIENT_NAME "Client A"

void readAndPrint();  // Read dataA.txt and print to stdout

int main() {
    // readAndPrint();
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    int message;
    socklen_t server_len = sizeof(server_addr);
    int n = recvfrom(sockfd, &message, sizeof(message), 0,
                     (struct sockaddr*)&server_addr, &server_len);

    const char* department = "ECE Art Law";
    sendto(sockfd, department, strlen(department), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    std::cout << "Client A sent: " << message << std::endl;

    if (n > 0) {
        // Convert the integer from network byte order to host byte order
        message = ntohl(message);
        cout << CLIENT_NAME << " received int: " << message << endl;
    }

    close(sockfd);
    return 0;
}

void readAndPrint() {
    ifstream file(FILENAME);
    if (!file.is_open()) {
        cerr << "Failed to open file: " << FILENAME << endl;
    }

    string department;
    while (getline(file, department)) {
        if (!isdigit(department[0])) {
            cout << department << endl;
        }
    }

    file.close();
}
