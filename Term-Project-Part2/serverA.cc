#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>

const int CLIENT_PORT = 30675;
const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 33675;

#define SERVER_NAME "Server A"
#define FILE_NAME "dataA.txt"

using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::isalpha;
using std::istringstream;
using std::map;
using std::set;
using std::stoi;
using std::strcpy;
using std::string;
using std::to_string;

int readAndStore(char*& data, map<string, set<int>>& department_data);

int main() {
    cout << SERVER_NAME << " is up and running using UDP on port "
         << CLIENT_PORT << endl;
    char* data;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(CLIENT_PORT);
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0) {
        cerr << "Error binding socket to client port" << endl;
        exit(1);
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    char signal[1024];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    while (true) {
        // Wait for signal from main server
        int recv_len = recvfrom(sockfd, signal, sizeof(signal) - 1, 0,
                                (struct sockaddr*)&from_addr, &from_len);
        if (recv_len < 0) {
            cerr << "Error receiving signal from main server" << endl;
            continue;  // Continue listening for the signal
        }

        // Null-terminate the received message and check if it is the 'SEND'
        // signal
        signal[recv_len] = '\0';
        if (strcmp(signal, "SEND") == 0) {
            // cout << "Received signal to send data." << endl;
            break;  // Exit the loop and proceed to send data
        } else {
            cerr << "Received unknown signal from main server: " << signal
                 << endl;
        }
    }

    map<string, set<int>> department_data;

    readAndStore(data, department_data);

    sendto(sockfd, data, strlen(data), 0, (struct sockaddr*)&server_addr,
           sizeof(server_addr));

    cout << SERVER_NAME << " has sent a department list to Main Server" << endl;

    while (true) {
        char message[1024];

        int n = recvfrom(sockfd, message, sizeof(message) - 1, 0,
                         (struct sockaddr*)&from_addr, &from_len);

        if (n > 0) {
            message[n] = '\0';  // Null-terminate the string
            cout << SERVER_NAME << " has received a request for " << message
                 << endl;
        }

        string data_to_send;
        auto it = department_data.find(message);
        if (it != department_data.end()) {
            cout << SERVER_NAME << " found " << it->second.size()
                 << " distinct students for " << message << ": ";
            for (auto numIt = it->second.begin(); numIt != it->second.end();
                 ++numIt) {
                if (numIt != it->second.begin()) {
                    data_to_send += ", ";
                    cout << ", ";
                }
                data_to_send += to_string(*numIt);  // Convert numIt to string
                cout << *numIt;
            }
            data_to_send += "\n";
            cout << endl;
            sendto(sockfd, data_to_send.c_str(), data_to_send.size(), 0,
                   (struct sockaddr*)&server_addr, sizeof(server_addr));
        } else {
            cout << SERVER_NAME << " did not find the department " << message
                 << endl;
        }

        cout << SERVER_NAME << " has sent the results to Main Server" << endl;
    }

    close(sockfd);
    delete[] data;
    return 0;
}

int readAndStore(char*& data, map<string, set<int>>& department_data) {
    ifstream file(FILE_NAME);
    if (!file.is_open()) {
        cerr << "Could not open the file." << endl;
        return 1;
    }

    string line;
    string currentDepartment;

    while (getline(file, line)) {
        // If the line contains department name (not digits)
        if (isalpha(line[0])) {
            currentDepartment = line;
        } else {
            istringstream numbersStream(line);
            string number;
            while (getline(numbersStream, number, ';')) {
                department_data[currentDepartment].insert(stoi(number));
            }
        }
    }

    file.close();  // Close the file after reading

    size_t total_length = 0;
    for (const auto& depart : department_data) {
        total_length += depart.first.length() + 1;
    }

    // Allocate memory for the data
    data = new char[total_length];
    char* currentPos = data;

    // Copy keys into the buffer, separated by spaces
    for (const auto& depart : department_data) {
        strcpy(currentPos, depart.first.c_str());
        currentPos += depart.first.length();
        *currentPos = ' ';  // Add a space between keys
        ++currentPos;
    }

    // Replace the last space with a null terminator
    if (data != currentPos) {
        *(currentPos - 1) = '\0';
    } else {
        *data = '\0';
    }

    return 0;
}