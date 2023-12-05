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

const int SERVER_PORT_UDP = 44675;
const int SERVER_PORT_TCP = 45675;
const int CLIENT_COUNT = 3;
const int CLIENTS_PORTS[CLIENT_COUNT] = {41675, 42675, 43675};

int main() {
    // Create TCP socket
    int tcp_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (tcp_sockfd < 0) {
        cerr << "TCP socket creation failed." << endl;
        return 1;
    }

    // Create UDP socket
    int udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        cerr << "UDP socket creation failed." << endl;
        return 1;
        ;
    }

    // Bind TCP socket
    struct sockaddr_in tcp_addr;
    memset(&tcp_addr, 0, sizeof(tcp_addr));
    tcp_addr.sin_family = AF_INET;
    tcp_addr.sin_port = htons(SERVER_PORT_TCP);
    tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(tcp_sockfd, (struct sockaddr *)&tcp_addr, sizeof(tcp_addr)) < 0) {
        cerr << "TCP socket binding failed." << endl;
        close(tcp_sockfd);
        close(udp_sockfd);  // Cleanup UDP socket before exiting
        return 1;
    }

    // Listen on the TCP socket
    if (listen(tcp_sockfd, CLIENT_COUNT) < 0) {
        cerr << "Listening on TCP socket failed." << endl;
        close(tcp_sockfd);
        return 1;
    }

    // Bind UDP socket
    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(SERVER_PORT_UDP);
    udp_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(udp_sockfd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
        cerr << "UDP socket binding failed." << endl;
        close(tcp_sockfd);
        close(udp_sockfd);
        return 1;
    }

    // Accept incoming TCP connections and handle them
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int client_sockfd, new_socket;

    // Set up fd_set and add tcp_sockfd and udp_sockfd
    // fd_set readfds;
    // int max_fd = std::max(tcp_sockfd, udp_sockfd);
    cout << "Main server is up and running" << endl;

    // struct timeval timeout;
    // timeout.tv_sec = 2;   // Set a 5-second timeout
    // timeout.tv_usec = 0;  // 0 microseconds

    // FD_ZERO(&readfds);
    // FD_SET(tcp_sockfd, &readfds);
    // FD_SET(udp_sockfd, &readfds);

    // // Reset timeout values each iteration
    // timeout.tv_sec = 2;
    // timeout.tv_usec = 0;

    // // Call select() with timeout
    // int activity = select(max_fd + 1, &readfds, NULL, NULL, &timeout);

    // cout << "debug" << endl;

    // if (activity < 0 && errno != EINTR) {
    //     cerr << "select error" << endl;
    //     break;
    // } else if (activity == 0) {
    //     // Timeout occurred, no activity detected
    //     // You can perform any periodic checks here or just continue
    //     continue;
    // }
    // cout << "debug" << endl;

    // Check if there is activity on the TCP socket

    struct sockaddr_in backend_addr1;
    memset(&backend_addr1, 0, sizeof(backend_addr1));
    backend_addr1.sin_family = AF_INET;
    backend_addr1.sin_port = htons(CLIENTS_PORTS[0]);
    backend_addr1.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in backend_addr2;
    memset(&backend_addr2, 0, sizeof(backend_addr2));
    backend_addr2.sin_family = AF_INET;
    backend_addr2.sin_port = htons(CLIENTS_PORTS[1]);
    backend_addr2.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in backend_addr3;
    memset(&backend_addr3, 0, sizeof(backend_addr3));
    backend_addr3.sin_family = AF_INET;
    backend_addr3.sin_port = htons(CLIENTS_PORTS[2]);
    backend_addr3.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    const char *signal = "SEND";
    sendto(udp_sockfd, signal, strlen(signal), 0,
           (struct sockaddr *)&backend_addr1, sizeof(backend_addr1));
    sendto(udp_sockfd, signal, strlen(signal), 0,
           (struct sockaddr *)&backend_addr2, sizeof(backend_addr2));
    sendto(udp_sockfd, signal, strlen(signal), 0,
           (struct sockaddr *)&backend_addr3, sizeof(backend_addr3));

    map<string, int> departmentMap;
    char buffer[1024];
    socklen_t len = sizeof(udp_addr);
    int received_client = 0;

    // Receive data from 3 UDP server
    while (received_client < CLIENT_COUNT) {
        int n = recvfrom(udp_sockfd, buffer, 1024, 0,
                         (struct sockaddr *)&udp_addr, &len);
        if (n < 0) {
            cerr << "Error receiving data" << endl;
            continue;
        }

        buffer[n] = '\0';  // Null-terminate the string
        string departments(buffer);

        int value = -1;

        // Assign value based on client's port
        switch (ntohs(udp_addr.sin_port)) {
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

    cout << "Main server has received the department list from "
            "server "
            "A/B/C using UDP over port "
         << SERVER_PORT_UDP << endl;

    // This map will store vectors of strings, grouped by their
    // server number.
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
        if (new_socket = accept(tcp_sockfd, (struct sockaddr *)&client_addr,
                                &client_addr_len) < 0) {
            cerr << "Accept failed" << endl;
            return 0;
        }
    }

    /*
    while (true) {
        cout << "Enter Department Name: ";
        string dept_input;
        cin >> dept_input;

        auto it = departmentMap.find(dept_input);
        if (it != departmentMap.end()) {
            int clientNumber = it->second;
            switch (clientNumber) {
                case 0:
                    sendto(udp_sockfd, dept_input.c_str(),
                           dept_input.size(), 0,
                           (struct sockaddr *)&backend_addr1,
                           sizeof(backend_addr1));
                    cout << dept_input << " shows up in server A"
                         << endl;
                    cout << "The Main Server has sent request for "
                         << dept_input
                         << " to server A using UDP over port "
                         << SERVER_PORT_UDP << endl;
                    break;
                case 1:
                    sendto(udp_sockfd, dept_input.c_str(),
                           dept_input.size(), 0,
                           (struct sockaddr *)&backend_addr2,
                           sizeof(backend_addr2));
                    cout << dept_input << " shows up in server B"
                         << endl;
                    cout << "The Main Server has sent request for "
                         << dept_input
                         << " to server B using UDP over port "
                         << SERVER_PORT_UDP << endl;
                    break;
                case 2:
                    sendto(udp_sockfd, dept_input.c_str(),
                           dept_input.size(), 0,
                           (struct sockaddr *)&backend_addr3,
                           sizeof(backend_addr3));
                    cout << dept_input << " shows up in server C"
                         << endl;
                    cout << "The Main Server has sent request for "
                         << dept_input
                         << " to server C using UDP over port "
                         << SERVER_PORT_UDP << endl;
                    break;
                default:
                    cout << dept_input
                         << " does not show up in Backend servers"
                         << endl;
            }
        } else {
            cout << dept_input << "does not show up in Backend servers"
                 << endl;
            continue;
        }

        char student_ID[1024];

        int n = recvfrom(udp_sockfd, student_ID, sizeof(student_ID) - 1,
                         0, (struct sockaddr *)&udp_addr, &len);

        string which_server;
        switch (ntohs(udp_addr.sin_port)) {
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
        cout << "The Main server has received searching result(s) "
                "of "
             << dept_input << " from Backend server " << which_server
             << endl;

        string str(student_ID);
        int student_count = count(str.begin(), str.end(), ',') + 1;

        cout << "There are " << student_count
             << " distinct students in " << dept_input << endl;

        if (n > 0) {
            student_ID[n] = '\0';  // Null-terminate the string
            cout << "Their IDs are " << student_ID << endl;
        }
        cout << "-----Start a new query-----" << endl;
    }
    */

    // ! should I close it here
    // Close the TCP server socket before exiting
    close(tcp_sockfd);
    close(udp_sockfd);
    return 0;
}
