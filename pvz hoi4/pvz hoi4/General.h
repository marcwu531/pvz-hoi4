#ifndef GENERAL_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define GENERAL_H

struct playerAccount {
	std::string username;
	std::unordered_map<int, int> plantsLevel;
};
extern playerAccount account;

void mapShift(std::unordered_map<int, int>& myMap);
#endif GENERAL_H
