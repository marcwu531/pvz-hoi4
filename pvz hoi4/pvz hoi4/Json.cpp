#include <fstream>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <windows.h>

#include "Display.h"
#include "Json.h"

nlohmann::json loadJson(const std::string& filePath) {
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open file");
	}

	std::ifstream::pos_type fileSize = file.tellg();
	file.seekg(0, std::ios::beg);

	std::string fileContent(static_cast<const unsigned int>(fileSize), '\0');
	file.read(&fileContent[0], fileSize);

	return nlohmann::json::parse(fileContent);
}

std::unordered_map<int, SpriteFrame> parseSpriteSheetData(const nlohmann::json& json) {
	std::unordered_map<int, SpriteFrame> spriteFrames;

	for (const auto& item : json["frames"].items()) {
		int key = std::stoi(item.key());
		const auto& value = item.value();
		const auto& frame = value["frame"];

		SpriteFrame spriteFrame;
		spriteFrame.frameRect = sf::IntRect(
			frame["x"].get<int>(),
			frame["y"].get<int>(),
			frame["w"].get<int>(),
			frame["h"].get<int>()
		);
		spriteFrame.rotated = value["rotated"].get<bool>();
		spriteFrame.trimmed = value["trimmed"].get<bool>();
		spriteFrame.spriteSourceSize = sf::Vector2i(
			value["spriteSourceSize"]["x"].get<int>(),
			value["spriteSourceSize"]["y"].get<int>()
		);
		spriteFrame.sourceSize = sf::Vector2i(
			value["sourceSize"]["w"].get<int>(),
			value["sourceSize"]["h"].get<int>()
		);

		spriteFrames[key] = std::move(spriteFrame);
	}

	return spriteFrames;
}

//std::vector<std::array<int, 2>> nullVector = { {NULL, NULL} };
//std::atomic<int> blinkCd(0); //int blinkCd = 0;

nlohmann::json loadJsonFromResource(int resourceId) {
	HRSRC resource = FindResource(NULL, MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if (!resource) {
		throw std::runtime_error("Failed to find resource");
	}

	HGLOBAL resourceData = LoadResource(NULL, resource);
	if (!resourceData) {
		throw std::runtime_error("Failed to load resource");
	}

	DWORD resourceSize = SizeofResource(NULL, resource);
	const char* resourcePtr = static_cast<const char*>(LockResource(resourceData));
	if (!resourcePtr) {
		throw std::runtime_error("Failed to lock resource");
	}

	return nlohmann::json::parse(std::string(resourcePtr, resourceSize));
}