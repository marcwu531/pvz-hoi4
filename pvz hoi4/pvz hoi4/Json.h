#ifndef JSON_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define JSON_H

nlohmann::json loadJson(const std::string& filePath);
std::unordered_map<int, SpriteFrame> parseSpriteSheetData(const nlohmann::json& json);
nlohmann::json loadJsonFromResource(int resourceId);
std::unordered_map<std::string, Emitter> parseEmitterData(const nlohmann::json& json);
#endif
