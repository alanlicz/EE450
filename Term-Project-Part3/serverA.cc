#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#define SERVER_NAME "Server A"
#define FILE_PATH "dataA.csv"

const int SERVER_PORT = 41675;
const char* SERVER_IP = "127.0.0.1";
const int MAIN_SERVER_PORT = 44675;

using std::cerr;
using std::cout;
using std::endl;

struct Student {
    std::string department;
    std::string student_id;
    std::vector<int> scores;
};

void readFileToBuffer(const std::string& filePath, std::string& buffer);

int main() {
    cout << SERVER_NAME << " is up and running using UDP on port "
         << SERVER_PORT << endl;

    // !char data
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERVER_PORT);
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

    // std::string student_id_to_find = "76903";
    // int class_index_to_find = 0;

    // Define a suitable buffer size
    const int MESSAGE_SIZE = 10240;
    std::string message;

    std::vector<Student> students;
    readFileToBuffer(FILE_PATH, message);

    sendto(sockfd, message.c_str(), message.length(), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

    cout << SERVER_NAME << " has sent a department list to Main Server" << endl;

    // Search for the student by student ID
    // for (const Student& student : students) {
    //     if (student.student_id == student_id_to_find) {
    //         // Check if the class index is valid
    //         if (class_index_to_find >= 0 &&
    //             class_index_to_find < student.scores.size()) {
    //             int score = student.scores[class_index_to_find];
    //             std::cout << "Student ID: " << student_id_to_find <<
    //             std::endl; std::cout << "Score for Class Index " <<
    //             class_index_to_find
    //                       << ": " << score << std::endl;
    //         } else {
    //             std::cout << "Invalid class index: " << class_index_to_find
    //                       << std::endl;
    //         }
    //         break;  // Stop searching once the student is found
    //     }
    // }

    return 0;
}

void readFileToBuffer(const std::string& filePath, std::string& buffer) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filePath << std::endl;
        return;
    }

    buffer.assign((std::istreambuf_iterator<char>(file)),
                  std::istreambuf_iterator<char>());
    file.close();
}