#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::istringstream;
using std::map;
using std::memset;
using std::string;
using std::to_string;
using std::vector;

const int SERVER_PORT = 30675;
const int PORT_CLIENT_A = 30675;
const int PORT_CLIENT_B = 31675;
const int PORT_CLIENT_C = 32675;
const char* SERVER_IP = "127.0.0.1";  // Loopback for local communication

void readDataFiles(const vector<string>& filenames,
                   map<string, int>& department_backend_mapping);

void printDepartmentsByServer(
    const map<string, int>& department_backend_mapping);

void sendMessage(int message, int port);

int main() {
    cout << "Main server is up and running" << endl;

    vector<string> filenames = {"dataA.txt", "dataB.txt", "dataC.txt"};
    map<string, int> department_backend_mapping;
    string dept_name;
    int depart_id;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        return 1;
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "Error on binding" << endl;
        return 1;
    }

    map<string, int> departmentMap;
    char buffer[1024];
    socklen_t len = sizeof(client_addr);

    // Main loop to listen for UDP packets
    while (true) {
        int n = recvfrom(sockfd, buffer, 1024, 0,
                         (struct sockaddr*)&client_addr, &len);
        if (n < 0) {
            cerr << "Error receiving data" << endl;
            continue;
        }

        buffer[n] = '\0';  // Null-terminate the string
        string departments(buffer);
        int value = -1;

        // Assign value based on client's port
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
        string dept;
        while (iss >> dept) {
            departmentMap[dept] = value;
        }

        // Print the map contents
        cout << "Updated Department Map:" << endl;
        for (const auto& pair : departmentMap) {
            cout << pair.first << " => " << pair.second << endl;
        }

        //****************************
        cout << "Enter Department Name: ";
        cin >> dept_name;

        depart_id = department_backend_mapping[dept_name];

        cout << depart_id << endl;

        if (depart_id == 0) {
            sendMessage(depart_id, PORT_CLIENT_A);
        }
    }
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
        cout << server_names[i] << endl;
        for (const auto& pair : department_backend_mapping) {
            if (pair.second == i) {
                cout << pair.first << endl;
            }
        }
    }
}

void sendMessage(int message, int port) {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    // ! Is it inet_addr(SERVER_IP) or INADDR_ANY?
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Convert the integer from host byte order to network byte order
    int net_message = htonl(message);

    sendto(sockfd, &net_message, sizeof(net_message), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    close(sockfd);
}