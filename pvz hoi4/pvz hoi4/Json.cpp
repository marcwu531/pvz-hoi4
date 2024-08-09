#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <map>
#include <thread>
#include <atomic>
#include <chrono>
#include <string>
#include <windows.h>
#include <fstream>
#include <stdexcept>
#include <queue>
#include <shellapi.h>
#include <memory>
#include <nlohmann/json.hpp>

#include "Window.h"
#include "Colour.h"
#include "Resource.h"
#include "State.h"
#include "Display.h"
#include "Async.h"
#include "Scene1.h"
#include "Audio.h"
#include "Json.h"

nlohmann::json loadJson(const std::string& filePath) {
    std::ifstream file(filePath);
    nlohmann::json json;
    file >> json;
    return json;
}

std::map<int, SpriteFrame> parseSpriteSheetData(const nlohmann::json& json) {
    std::map<int, SpriteFrame> spriteFrames;
    for (auto& [key, value] : json["frames"].items()) {
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

        spriteFrames[std::stoi(key)] = spriteFrame;
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