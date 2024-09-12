#include <fstream>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <string>
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

std::variant<std::string, float> parseStringOrFloat(const nlohmann::json& value) {
	return value.is_string() ? std::variant<std::string, float>(value.get<std::string>()) :
		std::variant<std::string, float>(value.get<float>());
}

std::unordered_map<std::string, Emitter> parseEmitterData(const nlohmann::json& json) {
	std::unordered_map<std::string, Emitter> emitters;

	for (const auto& item : json["Emitters"]) {
		Emitter emitter;
		emitter.name = item["Name"].get<std::string>();

		if (item.contains("SpawnRate")) emitter.spawnRate = item["SpawnRate"].get<int>();
		if (item.contains("SpawnMinActive")) emitter.spawnMinActive = item["SpawnMinActive"].get<int>();
		if (item.contains("SpawnMaxActive")) emitter.spawnMaxActive = item["SpawnMaxActive"].get<int>();
		if (item.contains("SpawnMinLaunched")) emitter.spawnMinLaunched = item["SpawnMinLaunched"].get<int>();
		if (item.contains("SpawnMaxLaunched")) emitter.spawnMaxLaunched = item["SpawnMaxLaunched"].get<int>();
		if (item.contains("LaunchSpeed")) emitter.launchSpeed = item["LaunchSpeed"].get<std::string>();
		if (item.contains("LaunchAngle")) emitter.launchAngle = item["LaunchAngle"].get<float>();
		if (item.contains("ParticleScale")) emitter.particleScale = item["ParticleScale"].get<std::string>();
		if (item.contains("ParticleDuration")) emitter.particleDuration = item["ParticleDuration"].get<std::string>();
		if (item.contains("ParticleSpinSpeed")) emitter.particleSpinSpeed = item["ParticleSpinSpeed"].get<std::string>();
		if (item.contains("ParticleStretch")) emitter.particleStretch = item["ParticleStretch"].get<float>();
		if (item.contains("ParticleLoops")) emitter.particleLoops = item["ParticleLoops"].get<bool>();
		if (item.contains("ParticleAlpha")) emitter.particleAlpha = parseStringOrFloat(item["ParticleAlpha"]);
		if (item.contains("ParticleRed")) emitter.particleRed = parseStringOrFloat(item["ParticleRed"]);
		if (item.contains("ParticleGreen")) emitter.particleGreen = parseStringOrFloat(item["ParticleGreen"]);
		if (item.contains("ParticleBlue")) emitter.particleBlue = parseStringOrFloat(item["ParticleBlue"]);
		if (item.contains("ParticleBrightness")) emitter.particleBrightness = item["ParticleBrightness"].get<float>();
		if (item.contains("ParticleSpinAngle")) emitter.particleSpinAngle = item["ParticleSpinAngle"].get<float>();
		if (item.contains("SystemDuration")) emitter.systemDuration = item["SystemDuration"].get<int>();
		if (item.contains("SystemAlpha")) emitter.systemAlpha = item["SystemAlpha"].get<std::string>();
		if (item.contains("SystemLoops")) emitter.systemLoops = item["SystemLoops"].get<bool>();
		if (item.contains("CollisionReflect")) emitter.collisionReflect = item["CollisionReflect"].get<float>();
		if (item.contains("CollisionSpin")) emitter.collisionSpin = item["CollisionSpin"].get<float>();
		if (item.contains("EmitterRadius")) emitter.emitterRadius = item["EmitterRadius"].get<std::string>();
		if (item.contains("EmitterOffset")) emitter.emitterOffset = item["EmitterOffset"].get<std::string>();
		if (item.contains("Additive")) emitter.additive = item["Additive"].get<bool>();
		if (item.contains("Image")) emitter.image = item["Image"].get<std::string>();
		if (item.contains("ImageFrames")) emitter.imageFrames = item["ImageFrames"].get<int>();
		if (item.contains("ImageRow")) emitter.imageRow = item["ImageRow"].get<int>();
		if (item.contains("RandomLaunchSpin")) emitter.randomLaunchSpin = item["RandomLaunchSpin"].get<int>();

		if (item.contains("Field")) {
			if (item["Field"].contains("FieldType")) emitter.field.fieldType = item["Field"]["FieldType"].get<std::string>();
			if (item["Field"].contains("X")) emitter.field.x = item["Field"]["X"].get<std::string>();
			if (item["Field"].contains("Y")) emitter.field.y = item["Field"]["Y"].get<std::string>();
		}

		emitters[emitter.name] = std::move(emitter);
	}

	return emitters;
}