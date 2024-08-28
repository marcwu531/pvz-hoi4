#include <ctime>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Account.h"
#include "General.h"

// Function to check if a character is valid for use
bool isValidChar(char ch) {
    return (ch >= 33 && ch <= 126 && ch != '$' && ch != '^' && ch != '\\' && ch != '|');
}

// Generate a random string of valid characters
std::string generateOffsetString(size_t length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::vector<char> allowedChars;

    for (int i = 33; i <= 126; ++i) {
        if (isValidChar(static_cast<char>(i))) {
            allowedChars.push_back(static_cast<char>(i));
        }
    }

    std::uniform_int_distribution<> dis(0, allowedChars.size() - 1);
    std::string offsetString;
    offsetString.reserve(length);

    for (size_t i = 0; i < length; ++i) {
        char selectedChar;
        do {
            selectedChar = allowedChars[dis(gen)];
        } while (static_cast<int>(selectedChar) + static_cast<int>(i) + 1 > 126);

        offsetString += selectedChar;
    }

    return offsetString;
}

// Shift the offset string based on character positions
std::string shiftOffsetString(const std::string& str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        int shift = static_cast<int>(i) + 1;
        char newChar = str[i];
        int newPos = static_cast<int>(newChar) + shift;

        // Ensure the shifted character is within the valid range
        if (newPos > 126) {
            newPos = (newPos - 33) % (127 - 33) + 33;
        }

        result += static_cast<char>(newPos);
    }

    return result;
}

// Unshift the offset string based on character positions
std::string unshiftOffsetString(const std::string& str) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        int shift = static_cast<int>(i) + 1;
        char newChar = str[i];
        int newPos = static_cast<int>(newChar) - shift;

        // Ensure the unshifted character is within the valid range
        if (newPos < 33) {
            newPos = (newPos - 33 + (127 - 33)) % (127 - 33) + 33;
        }

        result += static_cast<char>(newPos);
    }

    return result;
}

// Shift the input string based on offsets
static std::string shiftStringWithOffsets(const std::string& str, const std::vector<int>& offsets, bool encrypt = true) {
    std::string result;
    result.reserve(str.size());

    for (size_t i = 0; i < str.size(); ++i) {
        int normalizedShift = offsets[i] % 95;
        if (!encrypt) {
            normalizedShift = -normalizedShift;
        }
        unsigned char newChar = static_cast<unsigned char>(str[i]) - 32;
        newChar = (newChar + normalizedShift + 95) % 95 + 32;
        result += static_cast<char>(newChar);
    }
    return result;
}

// Encrypt the account information
std::string encryptAccount(const playerAccount& account) {
    std::string offsetString = generateOffsetString(account.username.size());
    std::string encryptedOffsetString = shiftOffsetString(offsetString);

    std::vector<int> offsets;
    offsets.reserve(encryptedOffsetString.size());
    for (char ch : encryptedOffsetString) {
        offsets.push_back(static_cast<int>(ch) - 33);
    }

    std::string encryptedUsername = shiftStringWithOffsets(account.username, offsets);

    std::string encryptedPlantsLevel;
    encryptedPlantsLevel.reserve(account.plantsLevel.size() * 3);
    for (const auto& [plantID, level] : account.plantsLevel) {
        encryptedPlantsLevel += shiftStringWithOffsets(std::string(1, static_cast<char>(plantID)), offsets);
        encryptedPlantsLevel += shiftStringWithOffsets(std::string(1, static_cast<char>(level)), offsets);
        encryptedPlantsLevel += '^';
    }
    if (!encryptedPlantsLevel.empty()) {
        encryptedPlantsLevel.pop_back(); // Remove the trailing '^'
    }

    // Debug output
    /*std::cout << "Offset String: " << offsetString << std::endl;
    std::cout << "Encrypted Offset String: " << encryptedOffsetString << std::endl;
    std::cout << "Encrypted Username: " << encryptedUsername << std::endl;
    std::cout << "Encrypted Plants Level: " << encryptedPlantsLevel << std::endl;*/

    return encryptedOffsetString + '$' + encryptedUsername + '$' + encryptedPlantsLevel;
}

// Decrypt the account information
playerAccount decryptAccount(const std::string& encryptedAccount) {
    playerAccount account;
    std::istringstream encryptedStream(encryptedAccount);
    std::string segment;

    std::getline(encryptedStream, segment, '$');
    std::string encryptedOffsetString = segment;

    std::string offsetString = unshiftOffsetString(encryptedOffsetString);

    std::vector<int> offsets;
    offsets.reserve(offsetString.size());
    for (char ch : offsetString) {
        offsets.push_back(static_cast<int>(ch) - 33);
    }

    std::getline(encryptedStream, segment, '$');

    // Debug prints
    std::cout << "Decrypted Segment (Username): " << segment << std::endl;

    std::cout << "Offsets: ";
    for (int offset : offsets) {
        std::cout << offset << " ";
    }
    std::cout << std::endl;

    std::string decryptedUsername = shiftStringWithOffsets(segment, offsets, false);
    std::cout << "Decrypted Username after shiftStringWithOffsets: " << decryptedUsername << std::endl;

    account.username = decryptedUsername;

    std::string plantsLevelString;
    std::getline(encryptedStream, plantsLevelString);

    std::istringstream plantsStream(plantsLevelString);
    std::string plantSegment;
    while (std::getline(plantsStream, plantSegment, '^')) {
        if (plantSegment.length() >= 2) {
            char decryptedPlantID = shiftStringWithOffsets(std::string(1, plantSegment[0]), offsets, false)[0];
            char decryptedLevel = shiftStringWithOffsets(std::string(1, plantSegment[1]), offsets, false)[0];
            int plantID = static_cast<int>(decryptedPlantID);
            int level = static_cast<int>(decryptedLevel);
            account.plantsLevel[plantID] = level;
        }
    }

    // Debug output
    std::cout << "Offset String: " << offsetString << std::endl;
    std::cout << "Decrypted Username: " << account.username << std::endl;
    std::cout << "Plants Level Data:" << std::endl;
    for (const auto& [id, level] : account.plantsLevel) {
        std::cout << "Plant ID: " << id << ", Level: " << level << std::endl;
    }

    return account;
}