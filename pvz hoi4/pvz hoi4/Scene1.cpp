#include <iostream>
#include <nlohmann/json.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <shared_mutex>
#include <string>
#include <thread>
#include <windows.h>

#include "Account.h"
#include "Async.h"
#include "Audio.h"
#include "Display.h"
#include "General.h"
#include "Json.h"
#include "Scene1.h"

int pvzScene = 0;
int pvzSun = 0;
int seedPacketSelected = 0;
int maxPlantSelectAmount = 6;
bool selectingSeedPacket = false;

int world, level;

std::array<std::string, maxPlantAmount> idlePlantToString = { "peashooter", "sunflower", "cherrybomb" };
std::string seedPacketIdToString(int id) {
	return "seedPacket_" + idlePlantToString[id];
}

std::unordered_map<std::string, sf::RectangleShape> seedPackets = {
	{seedPacketIdToString(0), sf::RectangleShape()},
	{seedPacketIdToString(1), sf::RectangleShape()},
	{seedPacketIdToString(2), sf::RectangleShape()}
};
std::unordered_map<std::string, sf::Sprite> idlePlants = {
	{idlePlantToString[0], sf::Sprite()},
	{idlePlantToString[1], sf::Sprite()},
	{idlePlantToString[2], sf::Sprite()}
};
std::vector<std::unordered_map<int, float>> seedPacketState(maxPlantAmount); //state, state1 moving time

int seedPacketSelectedId;

void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime) {
	//if (elapsedTime <= 0) return; //100ms to run and finish

	auto& state = seedPacketState[i];
	if (state[1] == 0 && state[0] != 4) {
		seedPacketSelected += 2 - static_cast<int>(state[0]);
		//std::cout << seedPacketSelected << std::endl;
	}

	state[1] += elapsedTime * 2;

	float distanceMoved = state[1] / 5.0f;
	auto packetIterator = seedPackets.find(seedPacketIdToString(i));
	if (packetIterator != seedPackets.end()) {
		sf::Vector2f currentPosition = packetIterator->second.getPosition();

		sf::Vector2f direction = targetPosition - currentPosition;

		float length = std::hypot(direction.x, direction.y);
		if (length > 0) {
			direction /= length;
		}

		sf::Vector2f newPosition = currentPosition + direction * distanceMoved;

		if ((direction.x > 0 && newPosition.x > targetPosition.x) ||
			(direction.x < 0 && newPosition.x < targetPosition.x) ||
			(direction.y > 0 && newPosition.y > targetPosition.y) ||
			(direction.y < 0 && newPosition.y < targetPosition.y)) {
			newPosition = targetPosition;
			state[0] = (state[0] == 1.0f || state[0] == 4.0f) ? 2.0f : 0.0f;
			state[1] = 0.0f;
			selectingSeedPacket = false;
		}

		packetIterator->second.setPosition(newPosition);
	}
}

const std::unordered_map<int, sf::Vector2f> stateToTargetPosition = {
	{1, sf::Vector2f(-570.0f, -500.0f)},
	{3, sf::Vector2f(-680.0f, -310.0f)}
};

float easeInOutQuad(float t, float easeRatio, float easeAccMax) {
	t /= 0.5f;
	t += 0.5f;

	if (t < easeAccMax) {
		return 0.5f * t * t * easeRatio;
	}

	t -= (easeAccMax + 0.5f);
	if (t > 2.0f) t = 2.0f;

	t *= 0.5f;
	if (t <= 0.25f) {
		t = 2.0f * (1.0f - 8.0f * t * t);
	}
	else if (t <= 0.5f) {
		t = powf(2.0f * t - 1.5f, 2.0f);
	}
	else {
		t = powf(t - 1.0f, 2.0f);
	}

	return std::min(1.5f, t * 2.0f * easeRatio);
}

static void hideTempPlants() {
	const sf::Vector2f offscreenPosition(10000.f, 10000.f);

	hoverPlant.setPosition(offscreenPosition);
	hoverShade.setPosition(offscreenPosition);

	for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
		idlePlants[idlePlantToString[i]].setPosition(offscreenPosition); //idlePlants[0]
	}
}

std::unordered_map<int, sf::Texture> seedPacketsTexture;

sf::Texture* getPlantIdleTextureById(int id) {
	static sf::Texture* textures[] = { &peashooterIdleSprites, &sunflowerIdleSprites, &cherrybombExplodeSprites };
	return textures[id];
}

std::unordered_map<int, SpriteFrame>* getPlantIdleFrameById(int id) {
	static std::unordered_map<int, SpriteFrame>* frames[] = { &peashooterIdleFrames, &sunflowerIdleFrames,
		&cherrybombIdleFrames };
	return frames[id];
}

sf::Texture* getPlantAttackTextureById(int id) {
	static sf::Texture* attackTextures[] = { &peashooterShootSprites, &sunflowerIdleSprites, 
		&cherrybombIdleSprites };
	return attackTextures[id];
}

std::unordered_map<int, SpriteFrame>* getPlantAttackFrameById(int id) {
	static std::unordered_map<int, SpriteFrame>* attackFrames[] = { &peashooterShootFrames, &sunflowerIdleFrames,
		&cherrybombExplodeFrames };
	return attackFrames[id];
}

std::string getAttackAnimName(int id) {
	static const std::string names[] = { "Shoot", "Idle", "Explode" };
	return names[id];
}

static int getPlantJsonIdById(int id, bool idle = true) {
	static const int idleJsonIds[] = { 116, 134, 169 };
	static const int attackJsonIds[] = { 123, 134, 164 };
	return (idle ? idleJsonIds : attackJsonIds)[id];
}

int getPlantMaxFrameById(int id, bool idle) {
	static const int maxIdleFrames[] = { 24, 24, 11 };
	static const int maxAttackFrames[] = { 24, 24, 13 };
	return (idle ? maxIdleFrames : maxAttackFrames)[id];
}

bool plantExist(int id) {
	return account.plantsLevel[id] > 0;
}

void unlockPlant(int id) {
	if (account.plantsLevel[id] == 0) {
		account.plantsLevel[id] = 1;
	}
}

int getUnlockPlantIdByLevel(int vWorld, int vLevel) {
	return --vWorld * 10 + vLevel;
}

void unlockPlantByLevel(int vWorld, int vLevel) {
	unlockPlant(getUnlockPlantIdByLevel(vWorld, vLevel));
}

static void initPlantsStatus() {
	for (int i = 0; i < maxPlantAmount; ++i) {
		account.plantsLevel[i] = 0;
	}
	//unlockPlant(2); //RUN_DEBUG
}

void initSeedPacketPos() {
	for (int i = 0; i < maxPlantAmount; ++i) {
		seedPackets[seedPacketIdToString(i)].setSize(sf::Vector2f(50.0f * scene1ZoomSize,
			70.0f * scene1ZoomSize));
		seedPackets[seedPacketIdToString(i)].setOrigin(0.0f, 0.0f);
		seedPackets[seedPacketIdToString(i)].setScale(1.0f, 1.0f);

		if (plantExist(i)) {
			seedPackets[seedPacketIdToString(i)].setFillColor(sf::Color(255, 255, 255, 255));
			seedPackets[seedPacketIdToString(i)].setPosition(seedChooser_background.getPosition() +
				sf::Vector2f(20.0f + i * 50.0f * scene1ZoomSize, 55.0f));
		}
		else {
			seedPackets[seedPacketIdToString(i)].setPosition(10000.f, 10000.f);
		}
	}
}

void initScene1Place() {
	pvzScene = 0;
	float bgCamSizeY = view_background.getSize().y;
	background.setPosition(0.0f, 0.0f);
	background.setSize(sf::Vector2f(1400.0f / 600.0f * bgCamSizeY, bgCamSizeY)); //1920.0f, 1046.0f -> bg png size 1400 x 600
	background.setTexture(&texture_background);

	view_background.setCenter((background.getSize().x - view_background.getSize().x) / 2.0f, 10.0f);

	seedBank.setSize(sf::Vector2f(446.0f * scene1ZoomSize, 87.0f * scene1ZoomSize));
	seedBank.setTexture(&texture_seedBank);
	seedBank.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
		view_background.getCenter().y - view_background.getSize().y / 2.0f);

	pvzStartText.setPosition(view_background.getCenter());

	seedChooser_background.setSize(sf::Vector2f(seedBank.getSize().x,
		513.0f / 465.0f * seedBank.getSize().x)); //465 x 513
	seedChooser_background.setTexture(&texture_seedChooser_background);
	seedChooser_background.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
		view_background.getCenter().y - view_background.getSize().y / 2.0f + seedBank.getSize().y);

	seedChooserButton.setSize(sf::Vector2f(278.0f, 72.0f));
	seedChooserButton.setTexture(&texture_seedChooserDisabled);
	seedChooserButton.setPosition(seedChooser_background.getPosition().x +
		(seedChooser_background.getSize().x - seedChooserButton.getSize().x) / 2.0f,
		seedChooser_background.getPosition().y + seedChooser_background.getSize().y -
		seedChooserButton.getSize().y - 15.0f);

	pvzSunText = sf::Text("", defaultFont, 50);
	pvzSunText.setFillColor(sf::Color::Black);
	pvzSunText.setPosition(-633.5f, -390.0f);

	overlayShade.setSize(sf::Vector2f(50.0f * scene1ZoomSize, 70.0f * scene1ZoomSize));
	overlayShade.setFillColor(sf::Color(0, 0, 0, 180));

	hoverShade.setTexture(peashooterIdleSprites);
	hoverShade.setTextureRect(peashooterIdleFrames[0].frameRect);
	hoverShade.setScale(scene1ZoomSize, scene1ZoomSize);
	hoverShade.setOrigin(hoverShade.getTextureRect().getSize().x / 2.0f,
		hoverShade.getTextureRect().getSize().y / 2.0f);
	hoverShade.setColor(sf::Color(0, 0, 0, 150));

	zombieIdle.setTexture(zombieIdleSprites);
	zombieIdle.setTextureRect(zombieIdleFrames[0].frameRect);
	zombieIdle.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieIdle.setOrigin(zombieIdle.getTextureRect().getSize().x / 2.0f,
		zombieIdle.getTextureRect().getSize().y / 2.0f);

	zombieIdle1.setTexture(zombieIdle1Sprites);
	zombieIdle1.setTextureRect(zombieIdle1Frames[0].frameRect);
	zombieIdle1.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieIdle1.setOrigin(zombieIdle1.getTextureRect().getSize().x / 2.0f,
		zombieIdle1.getTextureRect().getSize().y / 2.0f);

	zombieWalk.setTexture(zombieWalkSprites);
	zombieWalk.setTextureRect(zombieWalkFrames[0].frameRect);
	zombieWalk.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieWalk.setOrigin(zombieWalk.getTextureRect().getSize().x / 2.0f,
		zombieWalk.getTextureRect().getSize().y / 4.0f * 3.0f);

	pea.setTexture(peaTexture);
	pea.setScale(1.5f, 1.5f);
	pea.setOrigin(pea.getGlobalBounds().width / 2.0f, pea.getGlobalBounds().height / 2.0f + 10.0f);

	peaSplats.setTexture(peaSplatsSprites);
	peaSplats.setTextureRect(peaSplatsFrames[0].frameRect);
	peaSplats.setScale(scene1ZoomSize, scene1ZoomSize);
	peaSplats.setOrigin(peaSplats.getTextureRect().getSize().x / 2.0f,
		peaSplats.getTextureRect().getSize().y / 2.0f + 10.0f);

	zombieEat.setTexture(zombieEatSprites);
	zombieEat.setTextureRect(zombieEatFrames[0].frameRect);
	zombieEat.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieEat.setOrigin(zombieEat.getTextureRect().getSize().x / 2.0f,
		zombieEat.getTextureRect().getSize().y / 4.0f * 3.0f);

	sun.setTexture(sunSprites);
	sun.setTextureRect(sunFrames[0].frameRect);
	sun.setScale(scene1ZoomSize, scene1ZoomSize);
	sun.setOrigin(sun.getTextureRect().getSize().x / 2.0f,
		sun.getTextureRect().getSize().y / 2.0f);

	moneyBag.setTexture(&moneyBagTexture);
	moneyBag.setSize(sf::Vector2f(scene1ZoomSize * 91.0f, scene1ZoomSize * 78.0f));
	moneyBag.setScale(1.0f, 1.0f);

	lawnMower.setTexture(lawnMowerTexture);
	lawnMower.setTextureRect(lawnMowerFrames[0].frameRect);
	lawnMower.setScale(scene1ZoomSize, scene1ZoomSize);
	lawnMower.setOrigin(lawnMower.getTextureRect().getSize().x / 2.0f,
		lawnMower.getTextureRect().getSize().y / 2.0f);

	carKeys.setTexture(&carKeysTexture);
	carKeys.setPosition(10000.0f, 10000.0f);

	storeCar.setTexture(&storeCarTexture);
	storeCar.setPosition(10000.0f, 10000.0f);

	shopLawnMower.setTexture(&lawnMowerTexture);
	shopLawnMower.setTextureRect(lawnMowerFrames[0].frameRect);

	zombiesWon.setTexture(&zombiesWonTexture);
	zombiesWon.setTextureRect(zombiesWonFrames[0].frameRect);
	zombiesWon.setSize(sf::Vector2f(view_background.getSize().y / 3.0f * 4.0f, 
		view_background.getSize().y)); //800*600

	optionsMenuback.setTexture(&optionsMenubackTexture);
	optionsMenuback.setSize(sf::Vector2f(view_background.getSize().y * 3.0f / 4.0f / 498.0f * 423.0f,
		view_background.getSize().y * 3.0f / 4.0f));
	optionsMenuback.setOrigin(optionsMenuback.getSize().x / 2.0f, optionsMenuback.getSize().y / 2.0f);

	explosionCloud.setTexture(explosionCloudTexture);
	explosionCloud.setScale(scene1ZoomSize, scene1ZoomSize);
	explosionCloud.setOrigin(explosionCloud.getGlobalBounds().width / 2.0f,
		explosionCloud.getGlobalBounds().height / 2.0f);

	explosionPowie.setTexture(explosionPowieTexture);
	explosionPowie.setOrigin(explosionPowie.getGlobalBounds().width / 2.0f,
		explosionPowie.getGlobalBounds().height / 2.0f);

	for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
		idlePlants[idlePlantToString[i]].setTexture(*getPlantIdleTextureById(i));
		idlePlants[idlePlantToString[i]].setTextureRect(getPlantIdleFrameById(i)->find(0)->second.frameRect);
		idlePlants[idlePlantToString[i]].setScale(scene1ZoomSize, scene1ZoomSize);
		idlePlants[idlePlantToString[i]].setOrigin(idlePlants[idlePlantToString[i]]
			.getTextureRect().getSize().x / 2.0f,
			(float)idlePlants[idlePlantToString[i]].getTextureRect().getSize().y);

		seedPackets[seedPacketIdToString(i)].setTexture(&seedPacketsTexture[i]);
	}

	background.setOrigin(background.getSize() / 2.0f);

	hideTempPlants();
	initSeedPacketPos();
}

float scene1ZoomSize = 1.7f;
void initializeScene1() {
	initPlantsStatus();
	//initAccount();

	auto bgImage = getPvzImage("background", "bg1");
	texture_background.loadFromImage(bgImage);

	auto seedBankImage = getPvzImage("seed_selector", "seedBank");
	texture_seedBank.loadFromImage(seedBankImage);

	auto seedChooserBgImage = getPvzImage("seed_selector", "seedChooser_background");
	texture_seedChooser_background.loadFromImage(seedChooserBgImage);

	auto seedChooserDisabledImage = getPvzImage("seed_selector", "seedChooserDisabled");
	auto seedChooserButtonImage = getPvzImage("seed_selector", "seedChooserButton");
	texture_seedChooserDisabled.loadFromImage(seedChooserDisabledImage);
	texture_seedChooser.loadFromImage(seedChooserButtonImage);

	auto startReadyImage = getPvzImage("seed_selector", "startReady");
	auto startSetImage = getPvzImage("seed_selector", "startSet");
	auto startPlantImage = getPvzImage("seed_selector", "startPlant");
	pvzStartText_ready.loadFromImage(startReadyImage);
	pvzStartText_set.loadFromImage(startSetImage);
	pvzStartText_plant.loadFromImage(startPlantImage);

	auto zombieIdleJson = loadJsonFromResource(118);
	auto zombieIdleImage = getPvzImage("animations", "zombieIdle");
	zombieIdleSprites.loadFromImage(zombieIdleImage);
	zombieIdleFrames = parseSpriteSheetData(zombieIdleJson);

	auto zombieIdle1Json = loadJsonFromResource(120);
	auto zombieIdle1Image = getPvzImage("animations", "zombieIdle1");
	zombieIdle1Sprites.loadFromImage(zombieIdle1Image);
	zombieIdle1Frames = parseSpriteSheetData(zombieIdle1Json);

	auto zombieWalkJson = loadJsonFromResource(125);
	auto zombieWalkImage = getPvzImage("animations", "zombieWalk");
	zombieWalkSprites.loadFromImage(zombieWalkImage);
	zombieWalkFrames = parseSpriteSheetData(zombieWalkJson);

	auto peaImage = getPvzImage("projectiles", "pea");
	peaTexture.loadFromImage(peaImage);

	auto peaSplatsJson = loadJsonFromResource(127);
	auto peaSplatsImage = getPvzImage("animations", "peaSplats");
	peaSplatsSprites.loadFromImage(peaSplatsImage);
	peaSplatsFrames = parseSpriteSheetData(peaSplatsJson);

	auto zombieEatJson = loadJsonFromResource(130);
	auto zombieEatImage = getPvzImage("animations", "zombieEat");
	zombieEatSprites.loadFromImage(zombieEatImage);
	zombieEatFrames = parseSpriteSheetData(zombieEatJson);

	auto sunJson = loadJsonFromResource(136);
	auto sunImage = getPvzImage("animations", "sun");
	sunFrames = parseSpriteSheetData(sunJson);
	sunSprites.loadFromImage(sunImage);

	moneyBagTexture.loadFromImage(getPvzImage("money", "moneybag"));

	auto lawnMowerJson = loadJsonFromResource(143);
	lawnMowerFrames = parseSpriteSheetData(lawnMowerJson);
	lawnMowerTexture.loadFromImage(getPvzImage("animations", "lawnMower"));

	carKeysTexture.loadFromImage(getPvzImage("money", "carKeys"));
	carKeysHighlightTexture.loadFromImage(getPvzImage("money", "carKeysHighlight"));

	storeCarTexture.loadFromImage(getPvzImage("money", "storeCar"));

	zombiesWonTexture.loadFromImage(getPvzImage("animations", "zombiesWon"));
	zombiesWonFrames = parseSpriteSheetData(loadJsonFromResource(161));

	optionsMenubackTexture.loadFromImage(getPvzImage("window", "optionsMenuback"));

	explosionCloudTexture.loadFromImage(getPvzImage("particles", "explosionCloud"));
	explosionPowieTexture.loadFromImage(getPvzImage("particles", "explosionPowie"));

	for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
		getPlantIdleTextureById(i)->loadFromImage(getPvzImage("animations", idlePlantToString[i] + "Idle"));
		*getPlantIdleFrameById(i) = parseSpriteSheetData(loadJsonFromResource(getPlantJsonIdById(i)));

		getPlantAttackTextureById(i)->loadFromImage(getPvzImage("animations", 
			idlePlantToString[i] + getAttackAnimName(i)));
		*getPlantAttackFrameById(i) = parseSpriteSheetData(loadJsonFromResource(getPlantJsonIdById(i, false)));

		auto seedPacketImage = getPvzImage("seed_packet", idlePlantToString[i]);
		seedPacketsTexture[i].loadFromImage(seedPacketImage);
	}

	initScene1Place();
}

static int getRowByY(float posY) { //0:-310 1:-140 2:30 3:200 4:370
	switch (static_cast<int>(posY)) {
	case -310:
	default:
		return 0;
	case -140:
		return 1;
	case 30:
		return 2;
	case 200:
		return 3;
	case 370:
		return 4;
	}
}

bool canPlant(sf::Vector2f pos) {
	if (!plantsOnScene.empty()) {
		std::shared_lock<std::shared_mutex> plantReadLock(plantsMutex);

		for (const auto& plant : plantsOnScene) {
			if (pos.x == plant.anim.sprite.getPosition().x && getRowByY(pos.y) == plant.anim.row) {
				return false;
			}
		}
	}

	return pos.x >= -210 && pos.x <= 910 && pos.y >= -310 && pos.y <= 370;
}

std::vector<plantState> plantsOnScene;
std::vector<zombieState> zombiesOnScene;
std::vector<projectileState> projectilesOnScene;
std::vector<vanishProjState> vanishProjectilesOnScene;
std::vector<sunState> sunsOnScene;
std::vector<lawnMowerState> lawnMowersOnScene;
std::vector<particleState> particlesOnScene;

static int getSunByTypeAndId(int type, int id) { //type 0: plant
	static int cost[] = { 100, 50, 150 };
	static int* costT[] = { cost };

	return costT[type][id];
}

int blinkSunText = -1;

void createPlant(std::optional<sf::Vector2f> pos, int id) {
	if (canPlant(hoverPlant.getPosition()) || id == -1) {
		if (id != -1 && pos.has_value()) {
			if (pvzSun >= getSunByTypeAndId(0, id)) {
				int cd = 0;
				if (id == 1) cd = 12500 + (std::rand() % 1001);

				plantsOnScene.push_back({ {hoverPlant, id, 0, getRowByY(pos.value().y)}, false, 300, 0, cd });
				addSun(-getSunByTypeAndId(0, id));

				playRngAudio("plant");
			}
			else {
				blinkSunText = 0;
				return;
			}
		}
		hideTempPlants();
		pvzPacketOnSelected = false;
	}
}

sf::Sprite getZombieSpriteById(int id) {
	static sf::Sprite sprites[] = { zombieIdle, zombieIdle1, zombieWalk };
	return sprites[id];
}

void createZombie(sf::Vector2f pos) {
	createZombie(pos, 0);
}


void createRandomZombie() {
	createZombie(sf::Vector2f((float)(1300 + rand() % 200), (float)(-310 + 170 * (rand() % 5))), 1);
}

void createZombie(sf::Vector2f pos, int style) {
	//type: world_level_zombies[world][level]
	sf::Sprite newZombie;
	int animId = style == 0 ? rand() % 2 : 2;
	int row = style == 1 ? getRowByY(pos.y) : 0;

	newZombie = getZombieSpriteById(animId);

	newZombie.setPosition(pos);
	zombiesOnScene.push_back({ {newZombie, animId, rand() % 28, row}, 200, 0,
		sf::Vector2f(-0.5f - (rand() % 26) / 100.0f, 0.0f), nullptr });
}

void createProjectile(int type, sf::Vector2f pos) {
	if (type == 0) {
		sf::Sprite newProjectile = pea;
		newProjectile.setPosition(pos);
		projectilesOnScene.push_back({ newProjectile, type, getRowByY(pos.y) });
	}
}

void selectSeedPacket(sf::Vector2f mousePos) {
	auto end = seedPackets.end();
	for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
		if (plantExist(i)) {
			auto it = seedPackets.find(seedPacketIdToString(i));
			if (it != end && it->second.getGlobalBounds().contains(mousePos)) {
				selectSeedPacket(i);
			}
		}
	}
}

static void selectSun(sunState& sun) {
	if (sun.style != 1) {
		audios["sounds"]["points"]->play();
		sun.existTime = -1;
		sun.style = 1;
		sun.targetPos = sf::Vector2f(-630.0f, -455.0f);
	}
}

bool selectSun(sf::Vector2f mousePos) {
	std::shared_lock<std::shared_mutex> sunReadLock(sunsMutex);
	bool collected = false;

	for (auto& sun : sunsOnScene) {
		if (sun.anim.sprite.getGlobalBounds().contains(mousePos)) {
			selectSun(sun);
			collected = true;
		}
	}
	return collected;
}

void selectSeedPacket(int id) { //--id;
	if (pvzSun >= getSunByTypeAndId(0, id)) {
		audios["sounds"]["seedlift"]->play();
		auto it = seedPackets.find(seedPacketIdToString(id));
		if (it != seedPackets.end()) {
			//if (seedPacketState[i][0] == 2) {
			pvzPacketOnSelected = true;
			seedPacketSelectedId = id;

			auto& plant = idlePlants[idlePlantToString[id]];
			hoverPlant.setTexture(*getPlantIdleTextureById(id));
			hoverPlant.setTextureRect(plant.getTextureRect());
			hoverPlant.setScale(scene1ZoomSize, scene1ZoomSize);
			hoverPlant.setOrigin(hoverPlant.getTextureRect().getSize().x / 2.0f,
				hoverPlant.getTextureRect().getSize().y / 2.0f);

			hoverShade.setTexture(*getPlantIdleTextureById(id));
			hoverShade.setTextureRect(plant.getTextureRect());
			hoverShade.setScale(scene1ZoomSize, scene1ZoomSize);
			hoverShade.setOrigin(hoverShade.getTextureRect().getSize().x / 2.0f,
				hoverShade.getTextureRect().getSize().y / 2.0f);

			//seedPacketState[i][0] = 1;
			overlayShade.setPosition(seedPackets.find(seedPacketIdToString(id))->second.getPosition());
		}
	}
	else {
		if (audios["sounds"]["buzzer"]->getStatus() != sf::Music::Playing) 
			audios["sounds"]["buzzer"]->play();
		blinkSunText = 0;
	}
}

static int getProjectileDamageById(int id) {
	return id == 0 ? 20 : 0;
}

bool damageZombie(projectileState projectile, zombieState& zombie) {
	return damageZombie(getProjectileDamageById(projectile.id), zombie);
}

bool damageZombie(int amount, zombieState& zombie) {
	zombie.hp -= amount;
	return zombie.hp <= 0;
}

void createProjectileVanishAnim(projectileState proj) {
	sf::Sprite vanishAnim = peaSplats;
	vanishAnim.setPosition(proj.sprite.getPosition());
	vanishProjectilesOnScene.push_back({ {vanishAnim, 0} });
}

bool damagePlant(plantState& plant) {
	plant.hp -= 36;
	return plant.hp <= 0;
}

std::unordered_map<int, int> seedPacketsSelectedOrder;

int getOwnedPlantsAmount() {
	int ret = 0;

	for (int i = 0; i < maxPlantAmount; ++i) {
		if (plantExist(i)) ++ret;
	}

	return ret;
}

void createSun(sf::Vector2f pos, int sunType, int style) {
	//sunType 0 = normal sun, style 0 = fall from sky, 1 = collecting
	sf::Sprite tempSun = sun;
	tempSun.setPosition(pos);
	sf::Vector2f targetPos = pos;

	if (style == 0) {
		targetPos.y = static_cast<float>(-400 + rand() % 801);
	}
	else if (style == 2) {
		targetPos.x += static_cast<float>((std::rand() % 6 + 10) * (1 - 2 * (std::rand() % 2)));
		targetPos.y += 80.0f;
		tempSun.setScale(0.25f, 0.25f);
	}

	sunsOnScene.push_back({ {tempSun, 0, 0, std::nullopt}, sunType, style, targetPos, 0 });
}

void createSkySun() {
	createSun(static_cast<sf::Vector2f>(sf::Vector2i(rand() % 1101 - 200, rand() % 51 - 600)), 0, 0);
}

int getSunAmountByType(int id) {
	static int amount[] = { 25 };
	return amount[id];
}

int maxSun = 9900;
void addSun(int amount) {
	if (maxSun != -1) {
		if (pvzSun + amount < maxSun) {
			pvzSun += amount;
		}
		else {
			pvzSun = maxSun;
		}
	}
	else {
		pvzSun += amount;
	}
}

bool loggingIn = true;
bool shopping = false;
bool isMoneyBag = false;
bool openingMenu = false;

void winLevel() {
	if (audios["lawnbgm"]["1"]->getStatus() == sf::Music::Playing) audios["lawnbgm"]["1"]->stop();
	pvzScene = 4;

	for (auto& sun : sunsOnScene) {
		selectSun(sun);
	}

	isMoneyBag = plantExist(getUnlockPlantIdByLevel()) || getUnlockPlantIdByLevel() >= maxPlantAmount;

	if (!isMoneyBag) {
		sf::RectangleShape& unlockSP = seedPackets[seedPacketIdToString(getUnlockPlantIdByLevel())];
		unlockSP.setOrigin(unlockSP.getSize().x / 2.0f, unlockSP.getSize().y / 2.0f);
		unlockSP.setPosition(view_background.getCenter() +
			sf::Vector2f(static_cast<float>(rand() % 501) - 250.0f, static_cast<float>(rand() % 501) - 250.0f));
		unlockPlantByLevel();
	}
	else {
		moneyBag.setPosition(view_background.getCenter() +
			sf::Vector2f(static_cast<float>(rand() % 501) - 250.0f, static_cast<float>(rand() % 501) - 250.0f));
	}

	winLevelScreen.setSize(view_background.getSize());
	winLevelScreen.setOrigin(winLevelScreen.getSize().x / 2.0f, winLevelScreen.getSize().y / 2.0f);
	winLevelScreen.setPosition(view_background.getCenter());

	awardScreen.setSize(view_background.getSize());
	awardScreen.setOrigin(awardScreen.getSize().x / 2.0f, awardScreen.getSize().y / 2.0f);
	awardScreen.setPosition(view_background.getCenter());

	if(!isMoneyBag) idlePlants[idlePlantToString[getUnlockPlantIdByLevel()]].setPosition(view_background.getCenter() -
		sf::Vector2f(0.0f, 0.15f * view_background.getSize().y));

	seedPacketState.clear();
	seedPacketState.resize(maxPlantAmount);

	pvzPacketOnSelected = false;
}

int getStartSunByLevel(int vWorld, int vLevel) {
	std::unordered_map<int, std::unordered_map<int, int>> sunAmount = {
		{1, {
			{1, 1000},
			{2, 150},
			{3, 50}
		}}
	};

	return sunAmount[vWorld][vLevel];
}

void createLawnMower(float x, float y) {
	sf::Sprite tempSprite;
	tempSprite = lawnMower;
	tempSprite.setPosition(x, y);

	lawnMowersOnScene.push_back({ {tempSprite, 0, 0, getRowByY(y)}, 0 });
}

void loseLevel() {
	pvzScene = 8;
}

void openMenu() {
	openingMenu = true;
	optionsMenuback.setPosition(view_background.getCenter());
	menuBackText.setOrigin(menuBackText.getGlobalBounds().width / 2.0f,
		menuBackText.getGlobalBounds().height / 2.0f);
	menuBackText.setPosition(view_background.getCenter());
	menuRestartText.setOrigin(menuRestartText.getGlobalBounds().width / 2.0f,
		menuRestartText.getGlobalBounds().height / 2.0f);
	menuRestartText.setPosition(view_background.getCenter() + sf::Vector2f(0.0f,
		view_background.getSize().y / 32.0f));
	menuMenuText.setOrigin(menuMenuText.getGlobalBounds().width / 2.0f,
		menuMenuText.getGlobalBounds().height / 2.0f);
	menuMenuText.setPosition(view_background.getCenter() - sf::Vector2f(0.0f,
		view_background.getSize().y / 32.0f));
}

void clearPvzVar() {
	std::unique_lock<std::shared_mutex> plantWriteLock(plantsMutex);
	std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
	std::unique_lock<std::shared_mutex> projWriteLock(projsMutex);
	std::unique_lock<std::shared_mutex> vanishProjWriteLock(vanishProjsMutex);
	std::unique_lock<std::shared_mutex> sunWriteLock(sunsMutex);
	std::unique_lock<std::shared_mutex> lawnMowerWriteLock(lawnMowersMutex);
	std::unique_lock<std::shared_mutex> particleWriteLock(particlesMutex);
	std::unique_lock<std::shared_mutex> seedPacketWriteLock(seedPacketsMutex);

	seedPacketSelected = 0;
	zombiesWonFrameId = 0;
	seedPacketsSelectedOrder.clear();
	seedPacketState.clear();
	seedPacketState.resize(maxPlantAmount);

	plantsOnScene.clear();
	zombiesOnScene.clear();
	projectilesOnScene.clear();
	vanishProjectilesOnScene.clear();
	sunsOnScene.clear();
	seedPacketsSelectedOrder.clear();
	lawnMowersOnScene.clear();
	particlesOnScene.clear();
}