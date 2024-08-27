#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "Account.h"
#include "General.h"

static std::string shiftString(const std::string& str, int shift) {
    int normalizedShift = shift % 95;
    std::string result;
    result.reserve(str.size());
    for (char ch : str) {
        unsigned char newChar = static_cast<unsigned char>(ch) - 32;
        newChar = (newChar + normalizedShift + 95) % 95 + 32;
        result += static_cast<char>(newChar);
    }
    return result;
}

std::string encryptAccount(const playerAccount& account, int shift) {
    std::string encryptedUsername = shiftString(account.username, shift);

    std::string encryptedPlantsLevel;
    encryptedPlantsLevel.reserve(account.plantsLevel.size() * 3);
    for (const auto& [plantID, level] : account.plantsLevel) {
        encryptedPlantsLevel += shiftString(std::string(1, static_cast<char>(plantID)), shift);
        encryptedPlantsLevel += shiftString(std::string(1, static_cast<char>(level)), shift);
        encryptedPlantsLevel += '^';
    }

    if (!encryptedPlantsLevel.empty()) {
        encryptedPlantsLevel.pop_back();
    }

    return encryptedUsername + '$' + encryptedPlantsLevel;
}

playerAccount decryptAccount(const std::string& encryptedAccount, int shift) {
    playerAccount account;
    std::istringstream encryptedStream(encryptedAccount);
    std::string segment;

    std::getline(encryptedStream, segment, '$');
    account.username = shiftString(segment, -shift);

    while (std::getline(encryptedStream, segment, '^')) {
        if (segment.length() >= 2) {
            char decryptedPlantID = shiftString(std::string(1, segment[0]), -shift)[0];
            char decryptedLevel = shiftString(std::string(1, segment[1]), -shift)[0];
            int plantID = static_cast<int>(decryptedPlantID);
            int level = static_cast<int>(decryptedLevel);
            account.plantsLevel[plantID] = level;
        }
    }

    return account;
}