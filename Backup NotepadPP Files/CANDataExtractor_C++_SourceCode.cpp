#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

constexpr uint32_t DONGLE_ID = 0x7E0;
constexpr uint32_t ECU_ID = 0x7E8;
constexpr uint8_t SERVICE_ID = 0x36;

struct CanMessage {
    uint32_t timestamp;
    uint32_t canId;
    uint8_t canData[8];
};

vector<CanMessage> readCanData(const string& filename);
vector<uint8_t> extractTransferData(const vector<CanMessage>& canData);
void saveBinaryFile(const string& filename, const vector<uint8_t>& data);

int main() {
    const string inputFilename = "mg1cs002-stockmapsflash.candata";
    const string outputFilename = "mg1cs002-stockmapsflash.transferdata.bin";

    const auto canData = readCanData(inputFilename);
    cout << "Number of messages read: " << canData.size() << endl;

    const auto transferData = extractTransferData(canData);
    cout << "Size of extracted transfer data: " << transferData.size() << endl;

    saveBinaryFile(outputFilename, transferData);

    cout << "Data has been saved to: " << outputFilename << endl;

    return 0;
}

vector<CanMessage> readCanData(const string& filename) {
    vector<CanMessage> canMessages;

    ifstream file(filename, ios::in | ios::binary);
    if (!file) {
        cerr << "Error opening file: " << filename << endl;
        return canMessages;
    }

    CanMessage msg;
    while (file.read(reinterpret_cast<char*>(&msg.timestamp), sizeof(msg.timestamp)) &&
           file.read(reinterpret_cast<char*>(&msg.canId), sizeof(msg.canId)) &&
           file.read(reinterpret_cast<char*>(&msg.canData), sizeof(msg.canData))) {
        if (msg.canId == ECU_ID || msg.canId == DONGLE_ID) {
            canMessages.push_back(msg);
        }
    }
    return canMessages;
}

vector<uint8_t> extractTransferData(const vector<CanMessage>& canData) {
    vector<uint8_t> transferData;

    int matching_messages = 0;
    for (const auto& msg : canData) {
        if (msg.canId == DONGLE_ID && msg.canData[0] == SERVICE_ID) {
            matching_messages++;
            transferData.insert(transferData.end(), msg.canData + 1, msg.canData + 8);
        }
    }
    cout << "Number of messages with matching DONGLE_ID and SERVICE_ID: " << matching_messages << endl;

    return transferData;
}

void saveBinaryFile(const string& filename, const vector<uint8_t>& data) {
    ofstream file(filename, ios::out | ios::binary);
    if (!file) {
        cerr << "Error creating output file: " << filename << endl;
        return;
    }

    file.write(reinterpret_cast<const char*>(data.data()), data.size());
}
