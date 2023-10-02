#include "error_detection.h"

// Test bean

// Calculate a one byte checksum
bitset<8> calculateChecksum(string binaryInput) {
    int sum = 0, quotient = 0, remainder = 0, checksum = 0;
    string codeword;
    if (binaryInput.length() != 64) {
        cerr << "Input is not 64 bits!" << endl;
        return 0;
    }

    for (int i = 0; i < 64; i += 8) {
        string chunk = binaryInput.substr(i, 8);
        sum += bitset<8>(chunk).to_ulong();
    }

    quotient = sum / 256;
    remainder = sum % 256;

    while (quotient + remainder > 256) {
        checksum = quotient + remainder;
        quotient = checksum / 256;
        remainder = checksum % 256;
    }
    checksum = 255 - (quotient + remainder);
    bitset<8> binChecksum(checksum);
    return binChecksum;
}

// Perform XOR operation on 'a' and 'b'
string xor1(string a, string b) {
    string result = "";

    int n = b.length();

    // Exclusive or calculation
    // The reason we start from i = 1 here is
    for (int i = 1; i < n; i++) {
        if (a[i] == b[i])
            result += "0";
        else
            result += "1";
    }
    return result;
}

// Performs Modulo-2 division
string divideMod2(string dividend, string divisor) {
    // Number of bits to be XORed at a time.
    int pick = divisor.length();

    // Slicing the dividend to appropriate
    // length for particular step
    string tmp = dividend.substr(0, pick);

    int n = dividend.length();

    while (pick < n) {
        if (tmp[0] == '1')

            // Replace the dividend by the result
            // of XOR and pull 1 bit down
            tmp = xor1(divisor, tmp) + dividend[pick];
        else

            // If leftmost bit is '0'.
            // If the leftmost bit of the dividend (or the
            // part used in each step) is 0, the step cannot
            // use the regular divisor; we need to use an
            // all-0s divisor.
            tmp = xor1(string(pick, '0'), tmp) + dividend[pick];

        // Increment pick to move further
        pick += 1;
    }

    // For the last n bits, we have to carry it out
    // normally as increased value of pick will cause
    // Index Out of Bounds.
    if (tmp[0] == '1')
        tmp = xor1(divisor, tmp);
    else
        tmp = xor1(string(pick, '0'), tmp);

    return tmp;
}

// Function used at the sender side to encode
string sendCodeword(string dividend, string divisor) {
    // Appends n-1 zeroes at end of data
    string appended_data = (dividend + string(divisor.length() - 1, '0'));

    string remainder = divideMod2(appended_data, divisor);

    // Replace the 0s with remainders
    string codeword = dividend + remainder;
    return codeword;
}

// Check on the receiver side, if all 0 then the data is correct
int receiverData(string data, string key) {
    string curRemainder = divideMod2(data.substr(0, key.size()), key);
    int currPointer = key.size();

    // Iterate over the remaining bits of the data
    for (int i = key.size(); i < data.size(); i++) {
        // If the length of the current remainder is less than key.size(),
        // append the next bit of data
        if (curRemainder.size() < key.size()) {
            curRemainder.push_back(data[i]);
        } else {
            // If the length of the current remainder is equal to key.size(),
            // compute the CRC-8 checksum
            curRemainder = divideMod2(curRemainder, key);
            curRemainder.push_back(data[i]);
        }
    }

    if (curRemainder.size() == key.size()) {
        curRemainder = divideMod2(curRemainder, key);
    }
    if (curRemainder.find('1') != string::npos) {
        return 1;
    } else {
        return 0;
    }
}

string xorLeftAlign(const string& data, const string& error) {
    // Determine the length of the shorter binary string
    const int n = min(data.length(), error.length());

    // Perform the XOR operation on the common bits of a and b
    string result;
    for (int i = 0; i < n; i++) {
        result += (data[i] == error[i]) ? '0' : '1';
    }

    if (data.length() > error.length()) {
        result += data.substr(n);
    } else if (error.length() > data.length()) {
        result += error.substr(n);
    }

    return result;
}

// Calculate the parity bit for a binary string
char parity(const string& s) {
    int count = 0;
    for (auto c : s) {
        if (c == '1') {
            count++;
        }
    }
    return (count % 2 == 0) ? '0' : '1';
}

void parityCheck(string data, string& rowParity, string& colParity) {
    // Split the input data into eight 8-bit blocks
    vector<string> blocks;
    for (int i = 0; i < 8; i++) {
        blocks.push_back(data.substr(i * 8, 8));
    }

    // Calculate the column parity for each block

    for (int i = 0; i < 8; i++) {
        string col;
        for (int j = 0; j < 8; j++) {
            col += blocks[j][i];
        }
        colParity += parity(col);
    }

    // Calculate the row parity for all blocks

    for (int i = 0; i < 8; i++) {
        rowParity += parity(blocks[i]);
    }
    rowParity += parity(colParity);
}

int main() {
    ifstream inputFile("data.txt");
    if (!inputFile.is_open()) {
        cerr << "Failed to open the file." << endl;
        return 1;
    }

    string inputData;
    string inputError;

    string line;

    while (getline(inputFile, line)) {
        istringstream iss(line);
        string data, error;

        if (getline(iss, data, ' ') && getline(iss, error)) {
            bitset<8> binChecksum = calculateChecksum(data);
            string codeword = data + binChecksum.to_string();

            // Introduce error
            bitset<72> codewordBits(codeword);
            bitset<72> errorBits(error);
            bitset<72> receiveCode = codewordBits ^ errorBits;
            string receiveString72 = receiveCode.to_string();
            string leftmost64 = receiveString72.substr(0, 64);
            bitset<64> leftmostBits64(leftmost64);

            cout << "==========================\n";
            cout << "Data:" << data << "\n";
            cout << "Error:" << error << endl;

            // CRC-8 x^8+x^5+x^4+1  1 0011 0001
            string key = "100110001";
            string appended_data = (data + string(key.length() - 1, '0'));
            cout << "CRC-8\n";
            cout << "CRC: " << divideMod2(appended_data, key)
                 << "                             ";
            string dataWithError = xorLeftAlign(sendCodeword(data, key), error);
            if (receiverData(dataWithError, key)) {
                cout << "Result: Not Pass" << endl;
            } else {
                cout << "Result: Pass" << endl;
            }

            // CRC-16 x^16+x^12+x^5+1 = 1 0001 0000 0010 0001
            string key_16 = "10001000000100001";
            string appended_data_16 = (data + string(key_16.length() - 1, '0'));
            cout << "CRC-16\n";
            cout << "CRC: " << divideMod2(appended_data_16, key_16)
                 << "                     ";
            string dataWithError_16 =
                xorLeftAlign(sendCodeword(data, key_16), error);
            if (receiverData(dataWithError_16, key_16)) {
                cout << "Result: Not Pass" << endl;
            } else {
                cout << "Result: Pass" << endl;
            }

            // 2D parity
            cout << "2D Parity\n";
            string rowParityError, colParityError;
            string rowParity, colParity;
            string parityErrorData = xorLeftAlign(data, error);
            parityCheck(parityErrorData, rowParityError, colParityError);
            parityCheck(data, rowParity, colParity);
            cout << "Col: " << colParity << "; "
                 << "Row: " << rowParity << ";";
            if (rowParity != rowParityError || colParity != colParityError) {
                cout << "            Result: Not Pass" << endl;
            } else {
                cout << "            Result: Pass" << endl;
            }

            // Checksum
            bitset<8> receiveChecksum =
                calculateChecksum(leftmostBits64.to_string());
            cout << "Checksum\n";
            cout << "Checksum: " << binChecksum << "                        ";
            if (receiveChecksum != binChecksum) {
                cout << "Result: Not Pass" << endl;
            } else {
                cout << "Result: Pass" << endl;
            }
        }
    }

    inputFile.close();

    return 0;
}