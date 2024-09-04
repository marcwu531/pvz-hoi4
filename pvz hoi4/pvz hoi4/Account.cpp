#include <optional>
#include <random>
#include <SFML/Graphics.hpp>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "Account.h"
#include "General.h"

static char shiftChar(char ch, int shift) {
	int ascii = static_cast<int>(ch);
	int newPos = ascii + shift;

	if (newPos > 126) {
		newPos -= 94;
	}
	else if (newPos < 33) {
		newPos += 94;
	}

	return static_cast<char>(newPos);
}

static bool isValidChar(char ch) {
	return (ch >= 33 && ch <= 126 && ch != '$' && ch != '^' && ch != '\\' && ch != '|');
}

static std::string generateOffsetString(size_t length) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::vector<char> allowedChars;

	for (int i = 33; i <= 126; ++i) {
		if (isValidChar(static_cast<char>(i))) {
			allowedChars.push_back(static_cast<char>(i));
		}
	}

	std::uniform_int_distribution<> dis(0, allowedChars.size() - 1);
	std::string offsetString(length, '\0');

	for (size_t i = 0; i < length; ++i) {
		offsetString[i] = allowedChars[dis(gen)];
	}

	return offsetString;
}

static std::string shiftOffsetString(const std::string& str) {
	std::string result(str.size(), '\0');

	for (size_t i = 0; i < str.size(); ++i) {
		result[i] = shiftChar(str[i], static_cast<int>(i) + 1);
	}

	return result;
}

static std::string unshiftOffsetString(const std::string& str) {
	std::string result(str.size(), '\0');

	for (size_t i = 0; i < str.size(); ++i) {
		result[i] = shiftChar(str[i], -static_cast<int>(i) - 1);
	}

	return result;
}

static std::string shiftStringWithOffsets(const std::string& str,
	const std::vector<int>& offsets, bool encrypt = true) {
	std::string result(str.size(), '\0');

	for (size_t i = 0; i < str.size(); ++i) {
		int shift = encrypt ? offsets[i] : -offsets[i];
		result[i] = shiftChar(str[i], shift);
	}

	return result;
}

std::string encryptAccount(const playerAccount& account) {
	std::string offsetString = generateOffsetString(account.username.size());
	std::string encryptedOffsetString = shiftOffsetString(offsetString);

	std::vector<int> offsets;
	offsets.reserve(offsetString.size());
	for (char ch : offsetString) {
		offsets.push_back(static_cast<int>(ch) - 33);
	}

	std::string encryptedUsername = shiftStringWithOffsets(account.username, offsets);

	std::string encryptedPlantsLevel;
	encryptedPlantsLevel.reserve(account.plantsLevel.size() * 4);
	for (const auto& [plantID, level] : account.plantsLevel) {
		encryptedPlantsLevel += shiftStringWithOffsets(std::string(1,
			static_cast<char>(plantID + 33)), offsets);
		encryptedPlantsLevel += shiftStringWithOffsets(std::string(1,
			static_cast<char>(level + 33)), offsets);
		encryptedPlantsLevel += '^';
	}
	if (!encryptedPlantsLevel.empty()) {
		encryptedPlantsLevel.pop_back();
	}

	return encryptedOffsetString + '$' + encryptedUsername + '$' + encryptedPlantsLevel;
}

std::optional<playerAccount> decryptAccount(const std::string& encryptedAccount) {
	try {
		playerAccount pAccount;
		std::istringstream encryptedStream(encryptedAccount);
		std::string segment;

		if (!std::getline(encryptedStream, segment, '$') || segment.empty()) {
			return std::nullopt;
		}

		std::string encryptedOffsetString = segment;

		std::string offsetString = unshiftOffsetString(encryptedOffsetString);
		if (offsetString.empty()) return std::nullopt;

		std::vector<int> offsets;
		offsets.reserve(offsetString.size());
		for (char ch : offsetString) {
			offsets.push_back(static_cast<int>(ch) - 33);
		}

		if (!std::getline(encryptedStream, segment, '$') || segment.empty()) {
			return std::nullopt;
		}
		
		if (segment.length() != offsets.size()) return std::nullopt;
		pAccount.username = shiftStringWithOffsets(segment, offsets, false);

		std::string plantsLevelString;
		if (!std::getline(encryptedStream, plantsLevelString) || plantsLevelString.empty()) {
			return std::nullopt;
		}

		std::istringstream plantsStream(plantsLevelString);
		std::string plantSegment;
		while (std::getline(plantsStream, plantSegment, '^')) {
			if (plantSegment.length() >= 2) {
				char decryptedPlantID = shiftStringWithOffsets(std::string(1, plantSegment[0]), offsets, false)[0];
				char decryptedLevel = shiftStringWithOffsets(std::string(1, plantSegment[1]), offsets, false)[0];
				int plantID = static_cast<int>(decryptedPlantID) - 33;
				int level = static_cast<int>(decryptedLevel) - 33;
				pAccount.plantsLevel[plantID] = level;
			}
		}

		return pAccount;
	}
	catch (...) {
		return std::nullopt;
	}
}

bool tryDecryptAccount(const std::string& encryptedAccount) {
	if (encryptedAccount.empty() || std::count(encryptedAccount.begin(), encryptedAccount.end(), '$') < 2)
		return false;
	std::optional<playerAccount> tempAcc = decryptAccount(encryptedAccount);
	if (!tempAcc) return false;
	account = tempAcc.value();
	return true;
}