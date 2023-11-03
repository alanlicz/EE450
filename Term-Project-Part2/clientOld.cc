#include <arpa/inet.h>  // for inet_pton
#include <netinet/in.h>
#include <string.h>  // for memset
#include <sys/shm.h>
#include <sys/socket.h>
#include <unistd.h>  // for close

#include <iostream>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::to_string;

#define SERVER_PORT 23675
#define SERVER_IP "127.0.0.1"  // localhost

#define SHM_KEY 1234
#define SHM_SIZE sizeof(int)

int generateUniqueClientID() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }
    int *clientCount = (int *)shmat(shmid, NULL, 0);
    if (clientCount == (int *)-1) {
        perror("shmat");
        exit(1);
    }
    int clientID = ++(*clientCount);
    shmdt(clientCount);
    return clientID;
}

int main() {
    // Display client boot up message
    cout << "Client is up and running." << endl;

    int clientID = generateUniqueClientID();  // generate a unique client ID

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

        if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
            cerr << "Invalid address or address not supported." << endl;
            return 0;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) <
            0) {
            cerr << "Connection failed." << endl;
            return 0;
        }

        // Send the department name and client ID to the server
        string message = deptName + ";" + to_string(clientID);
        send(sock, message.c_str(), message.length(), 0);
        cout << "Client has sent Department " << deptName
             << " to Main Server using TCP." << endl;
        char buffer[1024] = {0};
        read(sock, buffer, 1024);

        if (strcmp(buffer, "Not found") == 0) {
            cout << deptName << " not found." << endl;
        } else {
            cout << "Client has received results from Main Server: " << deptName
                 << " is associated with backend server " << buffer << "." << endl;
        }
        cout << "-----Start a new query-----" << endl;

        close(sock);
        // cout << "Client ID: " << clientID << endl;  // print the client ID
    }

    return 0;
}
