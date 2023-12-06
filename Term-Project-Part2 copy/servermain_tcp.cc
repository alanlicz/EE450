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
const int BACKEND_COUNT = 3;
const int BACKEND_PORTS[BACKEND_COUNT] = {41675, 42675, 43675};

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
    if (listen(tcp_sockfd, BACKEND_COUNT) < 0) {
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
    int client_sockfd;

    cout << "Main server is up and running" << endl;

    struct sockaddr_in backend_addr1;
    memset(&backend_addr1, 0, sizeof(backend_addr1));
    backend_addr1.sin_family = AF_INET;
    backend_addr1.sin_port = htons(BACKEND_PORTS[0]);
    backend_addr1.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in backend_addr2;
    memset(&backend_addr2, 0, sizeof(backend_addr2));
    backend_addr2.sin_family = AF_INET;
    backend_addr2.sin_port = htons(BACKEND_PORTS[1]);
    backend_addr2.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in backend_addr3;
    memset(&backend_addr3, 0, sizeof(backend_addr3));
    backend_addr3.sin_family = AF_INET;
    backend_addr3.sin_port = htons(BACKEND_PORTS[2]);
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
    while (received_client < BACKEND_COUNT) {
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

    // Listen on the TCP socket
    if (listen(tcp_sockfd, 2) < 0) {
        cerr << "Listening on TCP socket failed." << endl;
        close(tcp_sockfd);
        return 1;
    }

    while (true) {
        // Accept incoming TCP connections
        client_sockfd = accept(tcp_sockfd, (struct sockaddr *)&client_addr,
                               &client_addr_len);
        if (client_sockfd < 0) {
            cerr << "Error accepting connection." << endl;
            continue;
        }

        // Fork a new process to handle the connection
        pid_t pid = fork();
        if (pid < 0) {
            cerr << "Failed to fork." << endl;
            close(client_sockfd);
        } else if (pid == 0) {
            // Child process
            close(tcp_sockfd);

            // Handle the client connection
            // You can add your code here to communicate with the client
            // For example, read and write data to client_sockfd
            char client_message[1024];  // Buffer to store the message
            ssize_t bytes_read;
            bytes_read = recv(client_sockfd, client_message,
                              sizeof(client_message) - 1, 0);

            if (bytes_read < 0) {
                cerr << "Failed to receive message." << endl;
            } else {
                // Null-terminate the received data to make it a valid C-string
                client_message[bytes_read] = '\0';
                string receivedMessage(client_message);

                // Print the received message
                cout << "Received message from client: " << client_message
                     << endl;

                // Extract department name and student ID
                size_t deptPos = receivedMessage.find("Department: ");
                size_t idPos = receivedMessage.find(", Student ID: ");
                string departmentName, studentID;

                if (deptPos != string::npos && idPos != string::npos) {
                    // ! Important
                    deptPos += strlen(
                        "Department: ");  // Adjust position to the start of the
                                          // actual department name
                    departmentName =
                        receivedMessage.substr(deptPos, idPos - deptPos);
                    idPos += strlen(
                        ", Student ID: ");  // Adjust position to the start of
                                            // the actual student ID
                    studentID = receivedMessage.substr(idPos);
                    cout << departmentName << " " << studentID << endl;
                } else {
                    cerr << "Invalid message format." << endl;
                    // Handle invalid format...
                }

                auto it = departmentMap.find(departmentName);
                int responsibleServerIndex = -1;

                if (it != departmentMap.end()) {
                    // Return the server index (0, 1, 2)
                    responsibleServerIndex = it->second;
                    cout << responsibleServerIndex << endl;
                } else {
                    cerr << "Department not found." << endl;
                    // send(client_sockfd, "Student record not found.",
                    //      strlen("Student record not found."), 0);
                }

                string student_info = departmentName + " " + studentID;

                if (responsibleServerIndex == 0) {
                    sendto(udp_sockfd, student_info.c_str(),
                           student_info.length(), 0,
                           (struct sockaddr *)&backend_addr1,
                           sizeof(backend_addr1));
                } else if (responsibleServerIndex == 1) {
                    sendto(udp_sockfd, student_info.c_str(),
                           student_info.length(), 0,
                           (struct sockaddr *)&backend_addr2,
                           sizeof(backend_addr2));
                } else if (responsibleServerIndex == 2) {
                    sendto(udp_sockfd, student_info.c_str(),
                           student_info.length(), 0,
                           (struct sockaddr *)&backend_addr3,
                           sizeof(backend_addr3));
                }

                // Receive the response from Server A
                int n = recvfrom(udp_sockfd, buffer, sizeof(buffer) - 1, 0,
                                 (struct sockaddr *)&udp_addr, &len);
                if (n < 0) {
                    cerr << "Error receiving data from Server A" << endl;
                } else {
                    buffer[n] = '\0';  // Null-terminate the string
                    string response(buffer);

                    // Print the response
                    cout << "Received from Server A: " << response << endl;
                }

                // Receive the response from Server A
                n = recvfrom(udp_sockfd, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr *)&udp_addr, &len);
                if (n < 0) {
                    cerr << "Error receiving data from Server A" << endl;
                } else {
                    buffer[n] = '\0';  // Null-terminate the string
                    string response(buffer);

                    // Parse the response to extract average score and
                    // percentage rank Assuming the response format is "Average
                    // Score for Student ID [ID]: [Average], Percentage Rank in
                    // Department: [Rank]%"
                    size_t pos =
                        response.find("Percentage Rank in Department: ");
                    string averageScore = response.substr(0, pos);
                    string percentageRank = response.substr(pos);

                    // Print the extracted information
                    cout << averageScore << endl;
                    cout << percentageRank << endl;
                }
            }

            close(client_sockfd);  // Close the client socket when done
            return 0;              // Exit child process
        } else {
            // Parent process
            close(client_sockfd);  // Close the client socket in the parent
                                   // process
        }
    }

    // ! should I close it here
    // Close the TCP server socket before exiting
    close(tcp_sockfd);
    close(udp_sockfd);
    return 0;
}
