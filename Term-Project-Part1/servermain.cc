#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <cctype>
#include <cstring>  // for memset
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::isdigit;
using std::map;
using std::set;
using std::string;
using std::stringstream;
using std::to_string;

#define SERVER_PORT 23675

int main() {
    // Display server boot up message
    cout << "Main server is up and running." << endl;

    // Reading the text file and storing department info (assuming
    // "department:ID" format for simplicity)
    map<int, set<string>> serverDepartments;  // for storing departments
                                              // for each server
    ifstream infile("list.txt");
    string line;          // for storing each line (department name)
    int currentServerID;  // for storing the current server ID
    while (getline(infile, line)) {
        // Check if line is a single number (server ID)
        if (isdigit(line[0])) {
            currentServerID = stoi(line);
        } else {
            stringstream ss(line);  // for splitting departments
            string department;
            while (getline(ss, department,
                           ';')) {  // ';' delimiter for departments
                serverDepartments[currentServerID].insert(department);
            }
        }
    }
    cout << "Main server has read the department list from list.txt." << endl;
    cout << "Total number of Backend Servers: " << serverDepartments.size()
         << endl;
    for (auto it = serverDepartments.begin(); it != serverDepartments.end();
         ++it) {
        cout << "Backend Server " << it->first << " contains "
             << it->second.size() << " distinct departments." << endl;
    }

    // Setting up the server socket
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        cerr << "Socket creation failed." << endl;
        return 0;
    }

    address.sin_family = AF_INET;  // IPv4
    address.sin_addr.s_addr =
        INADDR_ANY;  // Server socket accept any connections
    address.sin_port =
        htons(SERVER_PORT);  // Convert port num to network byte order

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        cerr << "Bind failed." << endl;
        return 0;
    }
    if (listen(server_fd, 3) < 0) {
        cerr << "Listen failed." << endl;
        return 0;
    }

    // cout << "Main server is up and running on port " << SERVER_PORT << endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t *)&addrlen)) < 0) {
            cerr << "Accept failed." << endl;
            return 0;
        }

        char buffer[1024] = {0};
        read(new_socket, buffer, 1024);
        string deptName(buffer);
        bool found = false;
        for (const auto &[key, value] : serverDepartments) {
            if (value.find(deptName) != value.end()) {
                send(new_socket, to_string(key).c_str(),
                     to_string(key).length(), 0);
                close(new_socket);
                found = true;
                break;
            }
        }

        if (!found) {
            send(new_socket, "Not found", 9, 0);
            close(new_socket);
        }
    }

    return 0;
}
