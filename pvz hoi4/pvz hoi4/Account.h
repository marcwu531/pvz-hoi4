#ifndef ACCOUNT_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define ACCOUNT_H

#include "General.h"
struct playerAccount {
	std::string username;
	std::unordered_map<int, int> plantsLevel;
	bool unlockedLawnMower = false;
};
extern playerAccount account;
std::string encryptAccount(const playerAccount& 
);
std::optional<playerAccount> decryptAccount(const std::string& encryptedAccount);
bool tryDecryptAccount(const std::string& encryptedAccount);
#endif