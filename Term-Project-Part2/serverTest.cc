#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <sstream>

using std::cerr;
using std::cout;
using std::endl;
using std::istringstream;
using std::map;
using std::string;

const int SERVER_PORT = 33675;
const int CLIENT_COUNT = 3;

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        cerr << "Error on binding" << endl;
        exit(1);
    }

    cout << "Main server is up and running" << endl;

    map<string, int> departmentMap;
    char buffer[1024];
    socklen_t len = sizeof(client_addr);
    int received_client = 0;

    while (received_client < CLIENT_COUNT) {
        int n = recvfrom(sockfd, buffer, 1024, 0,
                         (struct sockaddr *)&client_addr, &len);
        if (n < 0) {
            cerr << "Error receiving data" << endl;
            continue;
        }

        buffer[n] = '\0';  // Null-terminate the string
        string departments(buffer);
        int value = -1;

        // Assign value based on client's port
        // cout << "Client port: " << ntohs(client_addr.sin_port) << endl;
        switch (ntohs(client_addr.sin_port)) {
            case 30675:
                value = 0;
                break;
            case 31675:
                value = 1;
                break;
            case 32675:
                value = 2;
                break;
            default:
                cerr << "Unknown client port" << endl;
                continue;
        }

        istringstream iss(departments);
        string firstWord, secondWord, dept;

        iss >> firstWord >> secondWord;
        cout << firstWord << " " << secondWord << endl;
        while (iss >> dept) {
            departmentMap[dept] = value;
        }

        // Print the map contents
        for (const auto &pair : departmentMap) {
            cout << pair.first << endl;
        }
    }

    close(sockfd);
    return 0;
}
