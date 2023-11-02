#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using std::cerr;
using std::cout;
using std::endl;
using std::getline;
using std::ifstream;
using std::map;
using std::string;
using std::vector;

#define SERVER_PORT 30675

void readDataFiles(const vector<string>& filenames,
                   map<string, int>& department_backend_mapping);

void printDepartmentsByServer(
    const map<string, int>& department_backend_mapping);

int main() {
    cout << "Main server is up and running" << endl;

    vector<string> filenames = {"dataA.txt", "dataB.txt", "dataC.txt"};
    map<string, int> department_backend_mapping;
    readDataFiles(filenames, department_backend_mapping);

    cout << "Main server has received the department list from Backend server "
            "A/B/C using UDP over port "
         << SERVER_PORT << endl;

    printDepartmentsByServer(department_backend_mapping);
}

void readDataFiles(const vector<string>& filenames,
                   map<string, int>& department_backend_mapping) {
    for (int i = 0; i < filenames.size(); ++i) {
        ifstream file(filenames[i]);
        if (!file.is_open()) {
            cerr << "Failed to open file: " << filenames[i] << endl;
            continue;
        }

        string department;
        while (getline(file, department)) {
            if (!isdigit(department[0])) {
                department_backend_mapping[department] =
                    i;  // Assign the index of the file to the department
            }
        }

        file.close();
    }
}

void printDepartmentsByServer(
    const map<string, int>& department_backend_mapping) {
    vector<string> server_names = {"Server A", "Server B", "Server C"};

    for (int i = 0; i < server_names.size(); ++i) {
        cout << server_names[i] << std::endl;
        for (const auto& pair : department_backend_mapping) {
            if (pair.second == i) {
                cout << pair.first << endl;
            }
        }
    }
}