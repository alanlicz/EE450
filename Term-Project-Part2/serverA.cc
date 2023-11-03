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
using std::string;

#define FILE_NAME "dataA.txt"

int readAndStore();

int main() {

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

    readAndStore();

    sendto(sockfd, data, strlen(data), 0, (struct sockaddr*)&server_addr,
           sizeof(server_addr));

    cout << "Client A sent: " << data << endl;

    close(sockfd);
    return 0;
}

int readAndStore() {
    ifstream file("dataA.txt");  // Replace with your file path if needed
    if (!file.is_open()) {
        cerr << "Could not open the file." << endl;
        return 1;
    }

    map<string, set<int>> department_data;
    string line;
    string currentDepartment;
    int totalLength = 0;

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
    size_t total_length = 0;
    for (const auto& depart : department_data) {
        totalLength += depart.first.length() + 1;
    }

    


    // Printing the map to see the results
    for (const auto& dept : department_data) {
        cout << dept.first << ":" << endl;
        for (int num : dept.second) {
            cout << num << " ";
        }
        cout << endl << endl;
    }

    return 0;
}