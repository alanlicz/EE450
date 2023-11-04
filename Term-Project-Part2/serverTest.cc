#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::istringstream;
using std::map;
using std::string;
using std::vector;

const int SERVER_PORT = 33675;
const int CLIENT_COUNT = 3;
const int SERVER_PORTS[CLIENT_COUNT] = {30675, 31675, 32675};

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

    // Send the signal to all client servers to start sending their data
    for (int i = 0; i < CLIENT_COUNT; ++i) {
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_port = htons(SERVER_PORTS[i]);
        client_addr.sin_addr.s_addr =
            inet_addr("127.0.0.1");  // Using loopback address

        const char *signal = "SEND";
        sendto(sockfd, signal, strlen(signal), 0,
               (struct sockaddr *)&client_addr, sizeof(client_addr));
        // Optionally check the return value of sendto() for error handling
    }

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

        // Clear the contents of the departmentMap before inserting new items
        // departmentMap.clear();  // This line clears the map for new client
        // data

        istringstream iss(departments);
        string firstWord, secondWord, dept;

        iss >> firstWord >> secondWord;
        while (iss >> dept) {
            departmentMap[dept] = value;
        }
        received_client++;
    }

    // This map will store vectors of strings, grouped by their server number.
    map<int, vector<string>> groupedDepartments;

    // Group department names by their server number.
    for (const auto &pair : departmentMap) {
        groupedDepartments[pair.second].push_back(pair.first);
    }

    // Now print the groups in order.
    for (const auto &group : groupedDepartments) {
        // Print the server name once.
        if (group.first == 0) {
            cout << "Server A" << endl;
        } else if (group.first == 1) {
            cout << "Server B" << endl;
        } else if (group.first == 2) {
            cout << "Server C" << endl;
        }

        // Print all departments for this server.
        for (const auto &dept : group.second) {
            cout << dept << endl;
        }
    }

    // Print the map contents
    // for (const auto &pair : departmentMap) {
    //     if (pair.second == 0) {
    //         cout << "Server A" << endl;
    //         cout << pair.first << pair.second << endl;
    //     } else if (pair.second == 1) {
    //         cout << "Server B" << endl;
    //         cout << pair.first << pair.second << endl;
    //     } else if (pair.second == 2) {
    //         cout << "Server C" << endl;
    //         cout << pair.first << pair.second << endl;
    //     }
    // }

    close(sockfd);
    return 0;
}
