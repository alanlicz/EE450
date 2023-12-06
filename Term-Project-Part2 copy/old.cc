#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <map>
#include <sstream>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::istringstream;
using std::map;
using std::string;
using std::vector;

const int SERVER_PORT = 44675;
const int CLIENT_COUNT = 3;
const int CLIENTS_PORTS[CLIENT_COUNT] = {41675, 42675, 43675};

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

    struct sockaddr_in client_addr1;
    memset(&client_addr1, 0, sizeof(client_addr1));
    client_addr1.sin_family = AF_INET;
    client_addr1.sin_port = htons(CLIENTS_PORTS[0]);
    client_addr1.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in client_addr2;
    memset(&client_addr2, 0, sizeof(client_addr2));
    client_addr2.sin_family = AF_INET;
    client_addr2.sin_port = htons(CLIENTS_PORTS[1]);
    client_addr2.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in client_addr3;
    memset(&client_addr3, 0, sizeof(client_addr3));
    client_addr3.sin_family = AF_INET;
    client_addr3.sin_port = htons(CLIENTS_PORTS[2]);
    client_addr3.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    const char *signal = "SEND";
    sendto(sockfd, signal, strlen(signal), 0, (struct sockaddr *)&client_addr1,
           sizeof(client_addr1));
    sendto(sockfd, signal, strlen(signal), 0, (struct sockaddr *)&client_addr2,
           sizeof(client_addr2));
    sendto(sockfd, signal, strlen(signal), 0, (struct sockaddr *)&client_addr3,
           sizeof(client_addr3));

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
        switch (ntohs(client_addr.sin_port)) {
            case 41675:
                value = 0;
                break;
            case 42675:
                value = 1;
                break;
            case 43675:
                value = 2;
                break;
            default:
                cerr << "Unknown client port" << endl;
                continue;
        }

        istringstream iss(departments);
        string dept;
        // string firstWord, secondWord, dept;
        // iss >> firstWord >> secondWord;
        while (iss >> dept) {
            departmentMap[dept] = value;
        }
        received_client++;
    }

    cout << "Main server has received the department list from server "
            "A/B/C using UDP over port "
         << SERVER_PORT << endl;

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
            cout << "Server A: ";
        } else if (group.first == 1) {
            cout << "Server B: ";
        } else if (group.first == 2) {
            cout << "Server C: ";
        }

        // Print all departments for this server.
        for (const auto &dept : group.second) {
            cout << dept << " ";
        }
        cout << endl;
    }

    while (true) {
        cout << "Enter Department Name: ";
        string dept_input;
        cin >> dept_input;

        auto it = departmentMap.find(dept_input);
        if (it != departmentMap.end()) {
            int clientNumber = it->second;
            switch (clientNumber) {
                case 0:
                    sendto(sockfd, dept_input.c_str(), dept_input.size(), 0,
                           (struct sockaddr *)&client_addr1,
                           sizeof(client_addr1));
                    cout << dept_input << " shows up in server A" << endl;
                    cout << "The Main Server has sent request for "
                         << dept_input << " to server A using UDP over port "
                         << SERVER_PORT << endl;
                    break;
                case 1:
                    sendto(sockfd, dept_input.c_str(), dept_input.size(), 0,
                           (struct sockaddr *)&client_addr2,
                           sizeof(client_addr2));
                    cout << dept_input << " shows up in server B" << endl;
                    cout << "The Main Server has sent request for "
                         << dept_input << " to server B using UDP over port "
                         << SERVER_PORT << endl;
                    break;
                case 2:
                    sendto(sockfd, dept_input.c_str(), dept_input.size(), 0,
                           (struct sockaddr *)&client_addr3,
                           sizeof(client_addr3));
                    cout << dept_input << " shows up in server C" << endl;
                    cout << "The Main Server has sent request for "
                         << dept_input << " to server C using UDP over port "
                         << SERVER_PORT << endl;
                    break;
                default:
                    cout << dept_input << " does not show up in Backend servers"
                         << endl;
            }
        } else {
            cout << dept_input << "does not show up in Backend servers" << endl;
            continue;
        }

        char student_ID[1024];

        int n = recvfrom(sockfd, student_ID, sizeof(student_ID) - 1, 0,
                         (struct sockaddr *)&client_addr, &len);

        string which_server;
        switch (ntohs(client_addr.sin_port)) {
            case 30675:
                which_server = "A";
                break;
            case 31675:
                which_server = "B";
                break;
            case 32675:
                which_server = "C";
                break;
            default:
                cerr << "Unknown client port" << endl;
                continue;
        }
        cout << "The Main server has received searching result(s) of "
             << dept_input << " from Backend server " << which_server << endl;

        string str(student_ID);
        int student_count = count(str.begin(), str.end(), ',') + 1;

        cout << "There are " << student_count << " distinct students in "
             << dept_input << endl;

        if (n > 0) {
            student_ID[n] = '\0';  // Null-terminate the string
            cout << "Their IDs are " << student_ID << endl;
        }
        cout << "-----Start a new query-----" << endl;
    }

    close(sockfd);
    return 0;
}
