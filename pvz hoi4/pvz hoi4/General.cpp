#include <SFML/Graphics.hpp>
#include <string>
#include <unordered_map>

#include "General.h"

void mapShift(std::unordered_map<int, int>& targetMap) {
	std::unordered_map<int, int> tempMap;
	int newKey = 0;

	for (const auto& [key, value] : targetMap) {
		if (value != -1) {
			tempMap[newKey++] = value;
		}
	}

	targetMap = std::move(tempMap);
}

sf::RectangleShape winLevelScreen;
sf::RectangleShape awardScreen;