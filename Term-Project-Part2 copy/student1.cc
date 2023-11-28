#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>

using std::cin;
using std::cout;
using std::endl;
using std::string;

const char *SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 45675;

int main() {
    // Create a socket for the client
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cout << "Error creating socket" << endl;
        return 1;
    }

    // Define the server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0) {
        cout << "Connection failed" << endl;
        return 1;
    }

    cout << "Connected to server" << endl;

    // Input from user
    string departmentName, studentID;
    cout << "Department name: ";
    getline(cin, departmentName);
    cout << "Student ID: ";
    getline(cin, studentID);

    // Send data to server
    string message =
        "Department: " + departmentName + ", Student ID: " + studentID;
    send(sockfd, message.c_str(), message.size(), 0);

    cout << "Data sent to server" << endl;

    // Close the socket
    close(sockfd);

    return 0;
}
