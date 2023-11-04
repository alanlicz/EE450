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
#define CLIENT_NAME "Client A"

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

int readAndStore(char*& data);

int main() {
    cout << SERVER_NAME << " is up and running using UDP on port " << SERVER_PORT
         << endl;
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

    // Additions start here
    char signal[1024];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    // cout << SERVER_NAME << " waiting for signal from main server..." << endl;

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
            // Optionally, you can decide to break or continue based on specific
            // conditions
        }
    }

    readAndStore(data);

    sendto(sockfd, data, strlen(data), 0, (struct sockaddr*)&server_addr,
           sizeof(server_addr));

    cout << CLIENT_NAME << " has sent a department list to Main server" << endl;

    close(sockfd);
    delete[] data;
    return 0;
}

int readAndStore(char*& data) {
    ifstream file(FILE_NAME);  // Replace with your file path if needed
    if (!file.is_open()) {
        cerr << "Could not open the file." << endl;
        return 1;
    }

    map<string, set<int>> department_data;
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

    // Calculate the total length of the string
    // const string serverPrefix = SERVER_NAME + string(" ");
    // size_t total_length = serverPrefix.length() + 1;
    size_t total_length = 0;
    for (const auto& depart : department_data) {
        total_length += depart.first.length() + 1;
    }

    // Allocate memory for the data
    data = new char[total_length];
    // strcpy(data, serverPrefix.c_str());
    // char* currentPos = data + serverPrefix.length();
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

    // Output the result
    // cout << "Buffer containing keys: '" << data << "'" << endl;

    // Printing the map to see the results
    // for (const auto& dept : department_data) {
    //     cout << dept.first << ":" << endl;
    //     for (int num : dept.second) {
    //         cout << num << " ";
    //     }
    //     cout << endl << endl;
    // }

    return 0;
}