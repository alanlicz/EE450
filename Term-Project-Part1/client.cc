#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>
#include <string.h>  // for memset
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <iostream>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

#define SERVER_PORT 23675

int main() {
    // Display client boot up message
    cout << "Client is up and running." << std::endl;

    while (true) {
        cout << "Enter Department Name: ";
        string deptName;
        cin >> deptName;

        int sock = 0;
        struct sockaddr_in serv_addr;

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            cerr << "Socket creation error." << endl;
            return 0;
        }

        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(SERVER_PORT);

        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
            cerr << "Invalid address or address not supported." << endl;
            return 0;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0) {
            cerr << "Connection failed." << endl;
            return 0;
        }

        send(sock, deptName.c_str(), deptName.length(), 0);
        cout << "Client has sent Department " << deptName
             << " to Main Server using TCP" << endl;
        char buffer[1024] = {0};
        read(sock, buffer, 1024);

        if (strcmp(buffer, "Not found") == 0) {
            cout << deptName << " not found." << endl;
        } else {
            cout << "Client has received results from Main server: " << deptName
                 << " is associated with backend server " << buffer << endl;
        }

        close(sock);
    }

    return 0;
}
