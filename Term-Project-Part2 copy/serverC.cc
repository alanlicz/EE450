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
#include <vector>

const int BACKEND_PORT = 43675;
const char* SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 44675;

#define SERVER_NAME "Server C"
#define FILE_NAME "dataC.csv"

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

struct StudentRecord {
    std::string department;
    int studentID;
    std::vector<int> scores;  // Changed to store integer scores
};

std::map<std::string, StudentRecord> load_and_store_data(
    const std::string& file_path);
std::string extract_department_names(
    const std::map<std::string, StudentRecord>& student_records);

int main() {
    cout << SERVER_NAME << " is up and running using UDP on port "
         << BACKEND_PORT << endl;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        cerr << "Error opening socket" << endl;
        exit(1);
    }

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(BACKEND_PORT);
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

    auto student_data = load_and_store_data(FILE_NAME);

    std::string departments = extract_department_names(student_data);
    // std::cout << "Departments: " << departments << std::endl;

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

    // readAndStore(data, department_data);

    sendto(sockfd, departments.c_str(), departments.length(), 0,
           (struct sockaddr*)&server_addr, sizeof(server_addr));

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
    return 0;
}

std::map<std::string, StudentRecord> load_and_store_data(
    const std::string& file_path) {
    std::map<std::string, StudentRecord> student_records;
    std::ifstream file(file_path);

    if (!file.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return student_records;
    }

    std::string line;
    // Skipping the header line
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream ss(line);
        StudentRecord record;
        std::string score;
        std::string key;

        std::getline(ss, record.department, ',');
        std::getline(ss, score, ',');
        record.studentID = std::stoi(score);
        key = record.department + "_" + std::to_string(record.studentID);

        while (std::getline(ss, score, ',')) {
            if (score.empty()) {
                record.scores.push_back(
                    -1);  // Using -1 to represent missing scores
            } else {
                record.scores.push_back(std::stoi(score));
            }
        }

        student_records[key] = record;
    }

    file.close();
    return student_records;
}

std::string extract_department_names(
    const std::map<std::string, StudentRecord>& student_records) {
    std::set<std::string> departments;
    for (const auto& pair : student_records) {
        departments.insert(pair.second.department);
    }

    std::string department_names;
    for (const auto& dept : departments) {
        if (!department_names.empty()) {
            department_names += " ";
        }
        department_names += dept;
    }

    return department_names;
}