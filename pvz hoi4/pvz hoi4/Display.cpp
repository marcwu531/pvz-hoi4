#include <array>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <nlohmann/json.hpp>
#include <queue>
#include <random>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

#include "Account.h"
#include "Async.h"
#include "Audio.h"
#include "Colour.h"
#include "Display.h"
#include "Json.h"
#include "Scene1.h"
#include "State.h"
#include "Window.h"

#define NROC

sf::Font defaultFont;
sf::Texture flag_texture;
sf::RectangleShape flag_rect;
std::string current_flag;

sf::Text pvzSunText;
sf::Texture texture_background;
sf::RectangleShape background;
sf::Texture texture_seedChooser_background;
sf::RectangleShape seedChooser_background;
sf::Texture texture_seedBank;
sf::RectangleShape seedBank;
sf::RectangleShape seedChooserButton;
sf::Texture texture_seedChooser;
sf::Texture texture_seedChooserDisabled;
sf::RectangleShape pvzStartText;
sf::Texture pvzStartText_ready;
sf::Texture pvzStartText_set;
sf::Texture pvzStartText_plant;
sf::RectangleShape overlayShade;
sf::Texture peashooterIdleSprites;
sf::Texture peashooterShootSprites;
std::unordered_map<int, SpriteFrame> peashooterIdleFrames;
std::unordered_map<int, SpriteFrame> peashooterShootFrames;
sf::Sprite hoverPlant;
sf::Sprite hoverShade;
sf::Texture zombieIdleSprites;
sf::Sprite zombieIdle;
std::unordered_map<int, SpriteFrame> zombieIdleFrames;
sf::Texture zombieIdle1Sprites;
sf::Sprite zombieIdle1;
std::unordered_map<int, SpriteFrame> zombieIdle1Frames;
sf::Texture peaTexture;
sf::Sprite pea;
sf::Sprite zombieWalk;
std::unordered_map<int, SpriteFrame> zombieWalkFrames;
sf::Texture zombieWalkSprites;
sf::Sprite peaSplats;
std::unordered_map<int, SpriteFrame> peaSplatsFrames;
sf::Texture peaSplatsSprites;
sf::Sprite zombieEat;
std::unordered_map<int, SpriteFrame> zombieEatFrames;
sf::Texture zombieEatSprites;
sf::Texture sunflowerIdleSprites;
sf::Sprite sunflowerIdle;
std::unordered_map<int, SpriteFrame> sunflowerIdleFrames;
std::unordered_map<int, SpriteFrame> sunFrames;
sf::Texture sunSprites;
sf::Sprite sun;
sf::Texture moneyBagTexture;
sf::RectangleShape moneyBag;
sf::Sprite lawnMower;
sf::Texture lawnMowerTexture;
std::unordered_map<int, SpriteFrame> lawnMowerFrames;
sf::RectangleShape carKeys;
sf::Texture carKeysTexture;
sf::Texture carKeysHighlightTexture;
sf::RectangleShape storeCar;
sf::Texture storeCarTexture;
sf::RectangleShape shopLawnMower;
sf::RectangleShape zombiesWon;
sf::Texture zombiesWonTexture;
std::unordered_map<int, SpriteFrame> zombiesWonFrames;
sf::RectangleShape zombiesWonDark;
sf::RectangleShape optionsMenuback;
sf::Texture optionsMenubackTexture;
sf::Text menuBackText;
sf::Text menuRestartText;
sf::Texture texture_blink;
sf::Text menuMenuText;
sf::Texture cherrybombExplodeSprites;
std::unordered_map<int, SpriteFrame> cherrybombExplodeFrames;
sf::Texture cherrybombIdleSprites;
std::unordered_map<int, SpriteFrame> cherrybombIdleFrames;
sf::Texture explosionCloudTexture;
sf::Texture explosionPowieTexture;
sf::Sprite explosionCloud;
sf::Sprite explosionPowie;

sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close);
//(1920, 1046)
sf::View view_world(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));
sf::View view_background(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));

float mapRatio = 20.0f;
sf::RectangleShape world_blink;
//(sf::Vector2f(mapRatio* (State::T::lx - State::T::sx + 1), mapRatio* (State::T::ly - State::T::sy + 1))); 
//15s

int zombiesWonFrameId;

sf::Image cropImage(const sf::Image& image, const sf::IntRect& cropArea) {
	sf::Image cropped_image;
	cropped_image.create(cropArea.width, cropArea.height);
	cropped_image.copy(image, 0, 0, cropArea);
	return cropped_image;
}

sf::Image loadImageFromResource(HINSTANCE hInstance, UINT resourceID) {
	sf::Image image;

	HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_RCDATA);
	if (!hRes) throw std::runtime_error("Failed to find resource");

	HGLOBAL hResData = LoadResource(hInstance, hRes);
	if (!hResData) throw std::runtime_error("Failed to load resource");

	void* pResData = LockResource(hResData);
	if (!pResData) throw std::runtime_error("Failed to lock resource");

	DWORD resSize = SizeofResource(hInstance, hRes);
	if (resSize == 0) throw std::runtime_error("Failed to get resource size");

	if (!image.loadFromMemory(pResData, resSize)) throw std::runtime_error("Failed to load image from memory");

	return image;
}

HINSTANCE nullHInstance = GetModuleHandle(NULL);

sf::Image world_image = loadImageFromResource(nullHInstance, 101);

#ifdef NROC
std::unordered_map<std::string, sf::Image> flagImages = {
	{"Taiwan", loadImageFromResource(nullHInstance, 133)}
};
#else
std::unordered_map<std::string, sf::Image> flagImages = {
	{"Taiwan", loadImageFromResource(nullHInstance, 102)}
};
#endif

//#define CENSORED

#ifdef CENSORED
std::unordered_map<std::string, std::unordered_map<std::string, sf::Image>> pvzImages = {
	{"background", {
		{"bg1", loadImageFromResource(nullHInstance, 133)}
	}},
	{"seed_selector", {
		{"seedChooser_background", loadImageFromResource(nullHInstance, 133)},
		{"seedBank", loadImageFromResource(nullHInstance, 133)},
		{"seedChooserDisabled", loadImageFromResource(nullHInstance, 133)},
		{"seedChooserButton", loadImageFromResource(nullHInstance, 133)},
		{"startReady", loadImageFromResource(nullHInstance, 133)},
		{"startSet", loadImageFromResource(nullHInstance, 133)},
		{"startPlant", loadImageFromResource(nullHInstance, 133)}
	}},
	{"seed_packet", {
		{"peashooter", loadImageFromResource(nullHInstance, 133)},
		{"sunflower", loadImageFromResource(nullHInstance, 133)},
		{"cherrybomb", loadImageFromResource(nullHInstance, 133)}
	}},
	{"animations", {
		{"peashooterShoot", loadImageFromResource(nullHInstance, 133)},
		{"zombieIdle", loadImageFromResource(nullHInstance, 133)},
		{"zombieIdle1", loadImageFromResource(nullHInstance, 133)},
		{"zombieWalk", loadImageFromResource(nullHInstance, 133)},
		{"peaSplats", loadImageFromResource(nullHInstance, 133)},
		{"zombieEat", loadImageFromResource(nullHInstance, 133)},
		{"peashooterIdle", loadImageFromResource(nullHInstance, 133)},
		{"sunflowerIdle", loadImageFromResource(nullHInstance, 133)},
		{"sun", loadImageFromResource(nullHInstance, 133)},
		{"lawnMower", loadImageFromResource(nullHInstance, 133)},
		{"cherrybombExplode", loadImageFromResource(nullHInstance, 133)},
		{"cherrybombIdle", loadImageFromResource(nullHInstance, 133)}
	}},
	{"projectiles", {
		{"pea", loadImageFromResource(nullHInstance, 133)}
	}},
	{"money", {
		{"moneybag", loadImageFromResource(nullHInstance, 133)},
		{"carKeys", loadImageFromResource(nullHInstance, 133)},
		{"carKeysHighlight", loadImageFromResource(nullHInstance, 133)},
		{"storeCar", loadImageFromResource(nullHInstance, 133)}
	}},
	{"window", {
		{"optionsMenuback", loadImageFromResource(nullHInstance, 133)}
	}},
	{"particles", {
		{"explosionCloud", loadImageFromResource(nullHInstance, 133)},
		{"explosionPowie", loadImageFromResource(nullHInstance, 133)}
	}}
};
#else
std::unordered_map<std::string, std::unordered_map<std::string, sf::Image>> pvzImages = {
	{"background", {
		{"bg1", loadImageFromResource(nullHInstance, 103)}
	}},
	{"seed_selector", {
		{"seedChooser_background", loadImageFromResource(nullHInstance, 104)},
		{"seedBank", loadImageFromResource(nullHInstance, 105)},
		{"seedChooserDisabled", loadImageFromResource(nullHInstance, 107)},
		{"seedChooserButton", loadImageFromResource(nullHInstance, 108)},
		{"startReady", loadImageFromResource(nullHInstance, 109)},
		{"startSet", loadImageFromResource(nullHInstance, 110)},
		{"startPlant", loadImageFromResource(nullHInstance, 111)}
	}},
	{"seed_packet", {
		{"peashooter", loadImageFromResource(nullHInstance, 106)},
		{"sunflower", loadImageFromResource(nullHInstance, 132)},
		{"cherrybomb", loadImageFromResource(nullHInstance, 171)}
	}},
	{"animations", {
		{"peashooterShoot", loadImageFromResource(nullHInstance, 124)},
		{"zombieIdle", loadImageFromResource(nullHInstance, 119)},
		{"zombieIdle1", loadImageFromResource(nullHInstance, 121)},
		{"zombieWalk", loadImageFromResource(nullHInstance, 126)},
		{"peaSplats", loadImageFromResource(nullHInstance, 128)},
		{"zombieEat", loadImageFromResource(nullHInstance, 131)},
		{"peashooterIdle", loadImageFromResource(nullHInstance, 117)},
		{"sunflowerIdle", loadImageFromResource(nullHInstance, 135)},
		{"sun", loadImageFromResource(nullHInstance, 137)},
		{"lawnMower", loadImageFromResource(nullHInstance, 144)},
		{"zombiesWon", loadImageFromResource(nullHInstance, 162)},
		{"cherrybombExplode", loadImageFromResource(nullHInstance, 165)},
		{"cherrybombIdle", loadImageFromResource(nullHInstance, 170)}
	}},
	{"projectiles", {
		{"pea", loadImageFromResource(nullHInstance, 122)}
	}},
	{"money", {
		{"moneybag", loadImageFromResource(nullHInstance, 142)},
		{"carKeys", loadImageFromResource(nullHInstance, 145)},
		{"carKeysHighlight", loadImageFromResource(nullHInstance, 158)},
		{"storeCar", loadImageFromResource(nullHInstance, 159)}
	}},
	{"window", {
		{"optionsMenuback", loadImageFromResource(nullHInstance, 163)}
	}},
	{"particles", {
		{"explosionCloud", loadImageFromResource(nullHInstance, 167)},
		{"explosionPowie", loadImageFromResource(nullHInstance, 168)}
	}}
};
#endif

sf::Image getFlagImage(const std::string& country) {
	return flagImages.at(country);
}

sf::Image getPvzImage(const std::string& type, std::string target) {
	return pvzImages.at(type).at(target);
}

std::vector<char> loadResourceData(HINSTANCE hInstance, int resourceId) {
	HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
	if (!hResource) throw std::runtime_error("Failed to find resource!");

	HGLOBAL hLoadedResource = LoadResource(hInstance, hResource);
	if (!hLoadedResource) throw std::runtime_error("Failed to load resource!");

	void* pResourceData = LockResource(hLoadedResource);
	DWORD resourceSize = SizeofResource(hInstance, hResource);
	if (!pResourceData || resourceSize == 0)
		throw std::runtime_error("Failed to lock resource or resource size is zero!");

	return std::vector<char>(static_cast<char*>(pResourceData),
		static_cast<char*>(pResourceData) + resourceSize);
}

std::string checkClickingState(float mouseInMapPosX, float mouseInMapPosY) {
	std::array<std::string, 2> target = clickingState(world_image, mouseInMapPosX, mouseInMapPosY);
	std::string targetState = target[1];

	//std::cout << targetState << std::endl;
	if (!targetState.empty() && clicking_state != targetState) {
		blinkMap_loadingCoords.store(true);
		clicking_state = targetState;
		targetCoords.clear();

		//std::cout << "sx: " << state_int[clicking_state]["sx"]() << std::endl;
		//std::cout << "lx: " << state_int[clicking_state]["lx"]() << std::endl;

		for (int x = state_int[clicking_state]["sx"](); x <= state_int[clicking_state]["lx"](); ++x) {
			for (int y = state_int[clicking_state]["sy"](); y <= state_int[clicking_state]["ly"](); ++y) {
				if (getRGBA(world_image, x, y) == state_rgba[clicking_state]["RGBA"]()) {
					targetCoords.push_back({ x, y });
				}
			}
		}

		flag = target[0];
		blinkMap_loadingCoords.store(false);

		world = countryId[target[0]];
		level = state_int[clicking_state]["id"]();

		return std::to_string(world) + "-" + std::to_string(level);
	}

	return "";
}

void changeScene(int targetScene) {
	stopAllThreads();

	if (targetScene == 0) {
		thread_asyncBlinkMap = std::thread(asyncBlinkMap);
		thread_asyncLoadFlag = std::thread(asyncLoadFlag);
		thread_asyncLoadLevelStart = std::thread(asyncLoadLevelStart);
	}
	else if (targetScene == 1) {
		if (audios["soundtrack"]["battleofwuhan"]->getStatus() == sf::Music::Playing)
			audios["soundtrack"]["battleofwuhan"]->stop();
		thread_asyncPvzSceneUpdate = std::thread(asyncPvzSceneUpdate);
	}
	/*else if (targetScene == -1) {
		if (!account.username.empty()) {
			std::ofstream outFile("account.txt", std::ios::app);

			if (!outFile.is_open()) {
				char buffer[256];
				strerror_s(buffer, sizeof(buffer), errno);
				std::cerr << "Failed to open file. Error: " << buffer << std::endl;
			}
			else {
				std::cerr << "File opened successfully." << std::endl;
				outFile << "Hello, this is a message inside the file.\n";

				if (outFile.fail()) {
					char buffer[256];
					strerror_s(buffer, sizeof(buffer), errno);
					std::cerr << "Error occurred while writing to file. Error: " << buffer << std::endl;
				}
				else {
					std::cerr << "Write successful." << std::endl;
				}

				outFile.flush();
				outFile.close();

				if (outFile.fail()) {
					char buffer[256];
					strerror_s(buffer, sizeof(buffer), errno);
					std::cout << "Error occurred while closing the file. Error: " << buffer << std::endl;
				}
				else {
					std::cerr << "File closed successfully." << std::endl;
				}
			}
		}
	}*/

	scene = targetScene;

	if (targetScene == -1) cleanupAudios();
}

static int getJsonByParticleId(int id) {
	static const int ids[] = { 166 };
	return ids[id];
}

std::vector<std::unordered_map<std::string, Emitter>> particlesData(maxParticleAmount);

void initParticle() {
	for (int i = 0; i < maxParticleAmount; ++i) {
		nlohmann::json targetJson = loadJsonFromResource(getJsonByParticleId(i));
		particlesData[i] = parseEmitterData(targetJson);
	}
}

static std::optional<sf::Sprite> getParticleElem(std::string str) {
	if (str == "IMAGE_EXPLOSIONCLOUD") {
		return explosionCloud;
	}
	return std::nullopt;
}

inline static std::mt19937& getRandomEngine() {
	static std::mt19937 engine(std::random_device{}());
	return engine;
}

inline static float generateRandomNumber(const std::string& range, bool radius = true) {
	float a, b;

	std::stringstream ss(range);
	char ignore;
	ss >> ignore >> a >> b >> ignore;

	std::uniform_real_distribution<> dis(a, b);
	float randomValue = static_cast<float>(dis(getRandomEngine()));

	if (radius) {
		return randomValue * (std::uniform_int_distribution<>(0, 1)(getRandomEngine()) == 0 ? -1.0f : 1.0f);
	}

	return randomValue;
}

float getParticalInitialFloat(const std::variant<std::string, float>& var) {
	if (std::holds_alternative<float>(var)) {
		return std::get<float>(var);
	}

	const std::string& str = std::get<std::string>(var);

	size_t startPos = str.find('[');
	size_t endPos = str.find(']', startPos);

	if (startPos == std::string::npos || endPos == std::string::npos) {
		size_t pos = str.find_first_of(", ");
		return std::stof(str.substr(0, pos));
	}

	return generateRandomNumber(str.substr(startPos, endPos - startPos + 1), false);
}

std::optional<std::array<float, 2>> getParticleFloatAsArray(const std::variant<std::string, float>& var) {
	if (std::holds_alternative<float>(var)) return std::nullopt;

	const std::string& str = std::get<std::string>(var);
	if (str.empty()) return std::nullopt;

	size_t secondPos = str.find(',');
	if (secondPos == std::string::npos) return std::nullopt;

	std::string afterComma = str.substr(secondPos + 1);
	afterComma.erase(0, afterComma.find_first_not_of(' '));

	std::array<float, 2> result = { 0.0f, 0.0f };

	size_t spacePos = afterComma.find(' ');
	if (spacePos != std::string::npos) {
		std::string secondValueStr = afterComma.substr(0, spacePos);
		result[0] = std::stof(secondValueStr);

		std::string afterSpace = afterComma.substr(spacePos + 1);
		afterSpace.erase(0, afterSpace.find_first_not_of(' '));

		if (!afterSpace.empty()) result[1] = std::stof(afterSpace);
	}
	else {
		result[0] = std::stof(afterComma);
	}

	return result;
}

sf::Uint8 clampColor(float value) {
	return static_cast<sf::Uint8>(std::clamp(value * 255.0f, 0.0f, 255.0f));
}

void spawnParticle(int id, sf::Vector2f pos) {
	for (const auto& [name, emitter] : particlesData[id]) {
		std::optional<sf::Sprite> tempRect = getParticleElem(emitter.image);
		if (!tempRect) continue;

		int repeat = emitter.spawnRate;
		if (emitter.spawnMinLaunched != -1) repeat = std::max(repeat, emitter.spawnMinLaunched);
		if (emitter.spawnMaxLaunched != -1) repeat = std::min(repeat, emitter.spawnMaxLaunched);

		for (int i = 0; i < repeat; ++i) {
			sf::Sprite particleRect = tempRect.value();
			particleRect.setPosition(pos + sf::Vector2f(
				generateRandomNumber(emitter.emitterRadius),
				generateRandomNumber(emitter.emitterRadius)));

			float red = getParticalInitialFloat(emitter.particleRed);
			float green = getParticalInitialFloat(emitter.particleGreen);
			float blue = getParticalInitialFloat(emitter.particleBlue);
			float alpha = getParticalInitialFloat(emitter.particleAlpha);
			particleRect.setColor(sf::Color(
				clampColor(red),
				clampColor(green),
				clampColor(blue),
				clampColor(alpha)
			));

			if (!emitter.particleScale.empty()) {
				float scale = getParticalInitialFloat(emitter.particleScale);
				particleRect.scale(scale, scale);
			}

			particlesOnScene.push_back({ { particleRect, id, 0, std::nullopt }, emitter, particleRect });
		}
	}
}