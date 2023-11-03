#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

#define SERVER_PORT 30675
#define CLIENT_PORT 33675
#define SERVER_IP "127.0.0.1"  // Use the appropriate IP address for the server

int main() {
    int sock = 0, val_read;
    struct sockaddr_in serv_addr, client_addr;

    // Create a socket
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        cerr << "Socket creation error" << endl;
        return -1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    // Filling server information
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(CLIENT_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Bind client socket to the server port
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERVER_PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;

    // Convert IPv4 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        cerr << "Invalid address/ Address not supported" << endl;
        return -1;
    }

    char buffer[1024] = {0};
    string userInput;

    // Allow the user to type a message
    cout << "Enter message: ";
    getline(cin, userInput);

    // Send the message to the server
    sendto(sock, userInput.c_str(), userInput.size(), 0,
           (const struct sockaddr*)&serv_addr, sizeof(serv_addr));
    cout << "Message sent to server: " << userInput << endl;

    // Optionally receive a reply from the server
    struct sockaddr_in from;
    socklen_t fromLength = sizeof(struct sockaddr_in);
    val_read =
        recvfrom(sock, buffer, 1024, 0, (struct sockaddr*)&from, &fromLength);
    if (val_read > 0) {
        buffer[val_read] = '\0';
        cout << "Server's reply: " << buffer << endl;
    } else {
        cout << "No reply received from server." << endl;
    }

    // Close the socket
    close(sock);
    return 0;
}
