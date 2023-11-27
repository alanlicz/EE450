#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

struct StudentRecord {
    std::string department;
    int studentID;
    std::vector<int> scores;  // Changed to store integer scores
};

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

int main() {
    std::string file_path = "dataA.csv";
    auto student_data = load_and_store_data(file_path);

    std::string departments = extract_department_names(student_data);
    std::cout << "Departments: " << departments << std::endl;

    return 0;
}
