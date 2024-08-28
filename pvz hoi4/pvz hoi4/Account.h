#ifndef ACCOUNT_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define ACCOUNT_H

#include "General.h"

std::string encryptAccount(const playerAccount& account);
playerAccount decryptAccount(const std::string& encryptedAccount);
#endif