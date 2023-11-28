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
using std::cin;
using std::cout;
using std::endl;
using std::istringstream;
using std::map;
using std::string;
using std::vector;

const int UDP_SERVER_PORT = 44675;
const int BACKEND_COUNT = 3;
const int BACKEND_PORTS[BACKEND_COUNT] = {41675, 42675, 43675};

vector<string> extractDepartments(const string &data);

int main() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(UDP_SERVER_PORT);
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
    client_addr1.sin_port = htons(BACKEND_PORTS[0]);
    client_addr1.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in client_addr2;
    memset(&client_addr2, 0, sizeof(client_addr2));
    client_addr2.sin_family = AF_INET;
    client_addr2.sin_port = htons(BACKEND_PORTS[1]);
    client_addr2.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    struct sockaddr_in client_addr3;
    memset(&client_addr3, 0, sizeof(client_addr3));
    client_addr3.sin_family = AF_INET;
    client_addr3.sin_port = htons(BACKEND_PORTS[2]);
    client_addr3.sin_addr.s_addr =
        inet_addr("127.0.0.1");  // Using loopback address

    const char *signal = "SEND";
    sendto(sockfd, signal, strlen(signal), 0, (struct sockaddr *)&client_addr1,
           sizeof(client_addr1));
    sendto(sockfd, signal, strlen(signal), 0, (struct sockaddr *)&client_addr2,
           sizeof(client_addr2));
    sendto(sockfd, signal, strlen(signal), 0, (struct sockaddr *)&client_addr3,
           sizeof(client_addr3));

    cout << "Signal sent!" << endl;

    vector<string> serverADepts, serverBDepts, serverCDepts;
    char buffer[10240];  // Buffer for incoming data
    socklen_t len = sizeof(client_addr);
    int received_client = 0;

    while (received_client < BACKEND_COUNT) {
        cout << "Debug" << endl;
        // memset(buffer, 0, sizeof(buffer));  // Clear the buffer
        int n = recvfrom(sockfd, buffer, 10240, 0,
                         (struct sockaddr *)&client_addr, &len);
        cout << n << endl;
        if (n < 0) {
            cerr << "Error receiving data" << endl;
            continue;
        }
        cout << "Data received!" << endl;

        buffer[n] = '\0';  // Null-terminate the string
        string departmentsStr(buffer,
                              n);  // Create a string with the received data
        vector<string> departments = extractDepartments(departmentsStr);

        int serverNumber = -1;

        // Determine which server sent the data based on the client's port
        switch (ntohs(client_addr.sin_port)) {
            case 41675:
                serverADepts.insert(serverADepts.end(), departments.begin(),
                                    departments.end());
                break;
            case 42675:
                serverBDepts.insert(serverBDepts.end(), departments.begin(),
                                    departments.end());
                break;
            case 43675:
                serverCDepts.insert(serverCDepts.end(), departments.begin(),
                                    departments.end());
                break;
            default:
                cerr << "Unknown client port" << endl;
                continue;
        }

        received_client++;
    }

    cout << "Main server has received the department lists from Backend "
            "servers A/B/C using UDP over port "
         << UDP_SERVER_PORT << endl;

    // After exiting the loop, print the departments for each server
    cout << "Server A: ";
    for (const auto &dept : serverADepts) {
        cout << dept << ", ";
    }
    cout << endl;

    cout << "Server B: ";
    for (const auto &dept : serverBDepts) {
        cout << dept << ", ";
    }
    cout << endl;

    cout << "Server C: ";
    for (const auto &dept : serverCDepts) {
        cout << dept << ", ";
    }
    cout << endl;

    close(sockfd);
    return 0;
}

// Function to split the CSV string and extract the department names
vector<string> extractDepartments(const string &data) {
    vector<string> departments;
    std::stringstream ss(data);
    string line;
    std::getline(ss, line);  // Skip the header line

    while (getline(ss, line)) {
        std::stringstream lineStream(line);
        string cell;
        std::getline(lineStream, cell, ',');
        if (!cell.empty()) {
            departments.push_back(cell);
        }
    }
    return departments;
}
