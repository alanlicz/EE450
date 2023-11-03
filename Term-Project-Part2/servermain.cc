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
using std::getline;
using std::ifstream;
using std::map;
using std::memset;
using std::string;
using std::vector;

#define SERVER_PORT 33675

void readDataFiles(const vector<string>& filenames,
                   map<string, int>& department_backend_mapping);

void printDepartmentsByServer(
    const map<string, int>& department_backend_mapping);

int main() {
    cout << "Main server is up and running" << endl;

    vector<string> filenames = {"dataA.txt", "dataB.txt", "dataC.txt"};
    map<string, int> department_backend_mapping;
    readDataFiles(filenames, department_backend_mapping);

    cout << "Main server has received the department list from Backend server "
            "A/B/C using UDP over port "
         << SERVER_PORT << endl;

    printDepartmentsByServer(department_backend_mapping);

    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "socket creation failed";
        return 1;
    }

    // Filling server information
    server_addr.sin_family = AF_INET;  // IPv4
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // Bind the socket with the server address
    if (bind(server_fd, (const struct sockaddr*)&server_addr,
             sizeof(server_addr)) < 0) {
        cerr << "Bind failed: " << strerror(errno) << endl;
        close(server_fd);
        return 1;
    }

    // Main loop to listen for UDP packets
    while (true) {
        char buffer[1024] = {0};
        socklen_t len = sizeof(client_addr);  // Length of client address

        // Receive data from the client
        int n = recvfrom(server_fd, (char*)buffer, 1024, MSG_WAITALL,
                         (struct sockaddr*)&client_addr, &len);
        buffer[n] = '\0';  // Null-terminate the string

        // ! Print the received message
        cout << "Main server has received the request on department " << buffer
             << " from the client using UDP over port " << SERVER_PORT << endl;

        // Process the received data just like before (logic remains the same)
        // ...

        // Send the response back to the client using sendto
        // Instead of `send`, use `sendto` which requires the client's address
        const char* response =
            "Your response here";  // Replace with actual response
        sendto(server_fd, (const char*)response, strlen(response), MSG_CONFIRM,
               (const struct sockaddr*)&client_addr, len);

        // Rest of the code for handling the message stays mostly unchanged
        // ...

        // No need for a fork or closing the socket each time since UDP is
        // connectionless
    }
    close(server_fd);
    return 0;
}

void readDataFiles(const vector<string>& filenames,
                   map<string, int>& department_backend_mapping) {
    for (int i = 0; i < filenames.size(); ++i) {
        ifstream file(filenames[i]);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filenames[i] << endl;
            continue;
        }

        string department;
        while (getline(file, department)) {
            if (!isdigit(department[0])) {
                department_backend_mapping[department] =
                    i;  // Assign the index of the file to the department
            }
        }

        file.close();
    }
}

void printDepartmentsByServer(
    const map<string, int>& department_backend_mapping) {
    vector<string> server_names = {"Server A", "Server B", "Server C"};

    for (int i = 0; i < server_names.size(); ++i) {
        cout << server_names[i] << std::endl;
        for (const auto& pair : department_backend_mapping) {
            if (pair.second == i) {
                cout << pair.first << endl;
            }
        }
    }
}