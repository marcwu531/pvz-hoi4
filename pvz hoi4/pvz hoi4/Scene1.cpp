#include <iostream>
#include <nlohmann/json.hpp>
#include <SFML/Graphics.hpp>
#include <shared_mutex>
#include <string>
#include <thread>
#include <windows.h>

#include "Async.h"
#include "Display.h"
#include "General.h"
#include "Json.h"
#include "Scene1.h"

int pvzScene = 0;
int pvzSun = 150;
int seedPacketSelected = 0;
int maxPlantSelectAmount = 0;
bool selectingSeedPacket = false;

int world, level;

std::array<std::string, maxPlantAmount> idlePlantToString = { "peashooter", "sunflower" };
std::string seedPacketIdToString(int id) {
	return "seedPacket_" + idlePlantToString[id];
}

std::unordered_map<std::string, sf::RectangleShape> seedPackets = {
	{seedPacketIdToString(0), sf::RectangleShape()},
	{seedPacketIdToString(1), sf::RectangleShape()}
};
std::unordered_map<std::string, sf::Sprite> idlePlants = {
	{idlePlantToString[0], sf::Sprite()},
	{idlePlantToString[1], sf::Sprite()}
};
std::vector<std::unordered_map<int, float>> seedPacketState(maxPlantAmount); //state, state1 moving time

int seedPacketSelectedId;

void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime) {
	//if (elapsedTime <= 0) return; //100ms to run and finish

	auto& state = seedPacketState[i];
	if (state[1] == 0 && state[0] != 4) {
		seedPacketSelected += 2 - static_cast<int>(state[0]);
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
	static sf::Texture* textures[] = { &peashooterIdleSprites, &sunflowerIdleSprites };
	return textures[id];
}

std::unordered_map<int, SpriteFrame>* getPlantIdleFrameById(int id) {
	static std::unordered_map<int, SpriteFrame>* frames[] = { &peashooterIdleFrames, &sunflowerIdleFrames };
	return frames[id];
}

sf::Texture* getPlantAttackTextureById(int id) {
	static sf::Texture* attackTextures[] = { &peashooterShootSprites, &sunflowerIdleSprites };
	return attackTextures[id];
}

std::unordered_map<int, SpriteFrame>* getPlantAttackFrameById(int id) {
	static std::unordered_map<int, SpriteFrame>* attackFrames[] = { &peashooterShootFrames, &sunflowerIdleFrames };
	return attackFrames[id];
}

int getPlantJsonIdById(int id) {
	static const int jsonIds[] = { 116, 134 };
	return jsonIds[id];
}

int getPlantMaxFrameById(int id) {
	static const int maxFrames = 24;
	return maxFrames;
}

bool plantExist(int id) {
	return account.plantsLevel[id] > 0;
}

void unlockPlant(int id) {
	if (account.plantsLevel[id] == 0) {
		account.plantsLevel[id] = 1;
		++maxPlantSelectAmount;
	}
}

int getPlantIdByLevel(int vWorld, int vLevel) {
	return --vWorld * 10 + vLevel;
}

void unlockPlantByLevel(int vWorld, int vLevel) {
	unlockPlant(getPlantIdByLevel(vWorld, vLevel));
}

static void initPlantsStatus() {
	for (int i = 0; i < maxPlantAmount; ++i) {
		account.plantsLevel[i] = 0;
	}
	/*unlockPlant(0);
	unlockPlant(1); //RUN_DEBUG*/
}

float scene1ZoomSize = 1.7f;
void initializeScene1() {
	initPlantsStatus();

	auto bgImage = getPvzImage("background", "bg1");
	texture_background.loadFromImage(bgImage);
	float bgCamSizeY = view_background.getSize().y;
	background.setSize(sf::Vector2f(1400.0f / 600.0f * bgCamSizeY, bgCamSizeY)); //1920.0f, 1046.0f -> bg png size 1400 x 600
	background.setTexture(&texture_background);
	view_background.move((background.getSize().x - view_background.getSize().x) / 2.0f, 10.0f);

	auto seedBankImage = getPvzImage("seed_selector", "seedBank");
	texture_seedBank.loadFromImage(seedBankImage);
	seedBank.setSize(sf::Vector2f(446.0f * scene1ZoomSize, 87.0f * scene1ZoomSize));
	seedBank.setTexture(&texture_seedBank);
	seedBank.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
		view_background.getCenter().y - view_background.getSize().y / 2.0f);

	auto seedChooserBgImage = getPvzImage("seed_selector", "seedChooser_background");
	texture_seedChooser_background.loadFromImage(seedChooserBgImage);
	seedChooser_background.setSize(sf::Vector2f(seedBank.getSize().x,
		513.0f / 465.0f * seedBank.getSize().x)); //465 x 513
	seedChooser_background.setTexture(&texture_seedChooser_background);
	seedChooser_background.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
		view_background.getCenter().y - view_background.getSize().y / 2.0f + seedBank.getSize().y);

	auto seedChooserDisabledImage = getPvzImage("seed_selector", "seedChooserDisabled");
	auto seedChooserButtonImage = getPvzImage("seed_selector", "seedChooserButton");
	texture_seedChooserDisabled.loadFromImage(seedChooserDisabledImage);
	texture_seedChooser.loadFromImage(seedChooserButtonImage);
	seedChooserButton.setSize(sf::Vector2f(278.0f, 72.0f));
	seedChooserButton.setTexture(&texture_seedChooserDisabled);
	seedChooserButton.setPosition(seedChooser_background.getPosition().x +
		(seedChooser_background.getSize().x - seedChooserButton.getSize().x) / 2.0f,
		seedChooser_background.getPosition().y + seedChooser_background.getSize().y -
		seedChooserButton.getSize().y - 15.0f);

	pvzStartText.setPosition(view_background.getCenter());
	auto startReadyImage = getPvzImage("seed_selector", "startReady");
	auto startSetImage = getPvzImage("seed_selector", "startSet");
	auto startPlantImage = getPvzImage("seed_selector", "startPlant");
	pvzStartText_ready.loadFromImage(startReadyImage);
	pvzStartText_set.loadFromImage(startSetImage);
	pvzStartText_plant.loadFromImage(startPlantImage);

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
	hoverShade.setColor(sf::Color(0, 0, 0, 175));

	auto zombieIdleJson = loadJsonFromResource(118);
	auto zombieIdleImage = getPvzImage("animations", "zombieIdle");
	zombieIdleSprites.loadFromImage(zombieIdleImage);
	zombieIdleFrames = parseSpriteSheetData(zombieIdleJson);
	zombieIdle.setTexture(zombieIdleSprites);
	zombieIdle.setTextureRect(zombieIdleFrames[0].frameRect);
	zombieIdle.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieIdle.setOrigin(zombieIdle.getTextureRect().getSize().x / 2.0f,
		zombieIdle.getTextureRect().getSize().y / 2.0f);

	auto zombieIdle1Json = loadJsonFromResource(120);
	auto zombieIdle1Image = getPvzImage("animations", "zombieIdle1");
	zombieIdle1Sprites.loadFromImage(zombieIdle1Image);
	zombieIdle1Frames = parseSpriteSheetData(zombieIdle1Json);
	zombieIdle1.setTexture(zombieIdle1Sprites);
	zombieIdle1.setTextureRect(zombieIdle1Frames[0].frameRect);
	zombieIdle1.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieIdle1.setOrigin(zombieIdle1.getTextureRect().getSize().x / 2.0f,
		zombieIdle1.getTextureRect().getSize().y / 2.0f);

	auto zombieWalkJson = loadJsonFromResource(125);
	auto zombieWalkImage = getPvzImage("animations", "zombieWalk");
	zombieWalkSprites.loadFromImage(zombieWalkImage);
	zombieWalkFrames = parseSpriteSheetData(zombieWalkJson);
	zombieWalk.setTexture(zombieWalkSprites);
	zombieWalk.setTextureRect(zombieWalkFrames[0].frameRect);
	zombieWalk.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieWalk.setOrigin(zombieWalk.getTextureRect().getSize().x / 2.0f,
		zombieWalk.getTextureRect().getSize().y / 4.0f * 3.0f);

	auto peaImage = getPvzImage("projectiles", "pea");
	peaTexture.loadFromImage(peaImage);
	pea.setTexture(peaTexture);
	pea.setScale(1.5f, 1.5f);
	pea.setOrigin(pea.getGlobalBounds().width / 2.0f, pea.getGlobalBounds().height / 2.0f + 10.0f);

	auto peaSplatsJson = loadJsonFromResource(127);
	auto peaSplatsImage = getPvzImage("animations", "peaSplats");
	peaSplatsSprites.loadFromImage(peaSplatsImage);
	peaSplatsFrames = parseSpriteSheetData(peaSplatsJson);
	peaSplats.setTexture(peaSplatsSprites);
	peaSplats.setTextureRect(peaSplatsFrames[0].frameRect);
	peaSplats.setScale(scene1ZoomSize, scene1ZoomSize);
	peaSplats.setOrigin(peaSplats.getTextureRect().getSize().x / 2.0f,
		peaSplats.getTextureRect().getSize().y / 2.0f + 10.0f);

	auto zombieEatJson = loadJsonFromResource(130);
	auto zombieEatImage = getPvzImage("animations", "zombieEat");
	zombieEatSprites.loadFromImage(zombieEatImage);
	zombieEatFrames = parseSpriteSheetData(zombieEatJson);
	zombieEat.setTexture(zombieEatSprites);
	zombieEat.setTextureRect(zombieEatFrames[0].frameRect);
	zombieEat.setScale(scene1ZoomSize, scene1ZoomSize);
	zombieEat.setOrigin(zombieEat.getTextureRect().getSize().x / 2.0f,
		zombieEat.getTextureRect().getSize().y / 4.0f * 3.0f);

	auto peashooterShootJson = loadJsonFromResource(123);
	auto peashooterShootImage = getPvzImage("animations", "peashooterShoot");
	peashooterShootFrames = parseSpriteSheetData(peashooterShootJson);
	peashooterShootSprites.loadFromImage(peashooterShootImage);

	auto sunJson = loadJsonFromResource(136);
	auto sunImage = getPvzImage("animations", "sun");
	sunFrames = parseSpriteSheetData(sunJson);
	sunSprites.loadFromImage(sunImage);
	sun.setTexture(sunSprites);
	sun.setTextureRect(sunFrames[0].frameRect);
	sun.setScale(scene1ZoomSize, scene1ZoomSize);
	sun.setOrigin(sun.getTextureRect().getSize().x / 2.0f,
		sun.getTextureRect().getSize().y / 2.0f);

	for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
		auto plantJson = loadJsonFromResource(getPlantJsonIdById(i));
		auto plantIdleImage = getPvzImage("animations", idlePlantToString[i] + "Idle");
		getPlantIdleTextureById(i)->loadFromImage(plantIdleImage);
		*getPlantIdleFrameById(i) = parseSpriteSheetData(plantJson);
		idlePlants[idlePlantToString[i]].setTexture(*getPlantIdleTextureById(i));
		idlePlants[idlePlantToString[i]].setTextureRect(getPlantIdleFrameById(i)->find(0)->second.frameRect);
		idlePlants[idlePlantToString[i]].setScale(scene1ZoomSize, scene1ZoomSize);
		idlePlants[idlePlantToString[i]].setOrigin(idlePlants[idlePlantToString[i]]
			.getTextureRect().getSize().x / 2.0f,
			(float)idlePlants[idlePlantToString[i]].getTextureRect().getSize().y);

		auto seedPacketImage = getPvzImage("seed_packet", idlePlantToString[i]);
		seedPacketsTexture[i].loadFromImage(seedPacketImage);
		seedPackets[seedPacketIdToString(i)].setTexture(&seedPacketsTexture[i]);
	}

	background.setOrigin(background.getSize() / 2.0f);

	hideTempPlants();
}

void initSeedPacketPos() {
	for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
		seedPackets[seedPacketIdToString(i)].setSize(sf::Vector2f(50.0f * scene1ZoomSize,
			70.0f * scene1ZoomSize));
		seedPackets[seedPacketIdToString(i)].setOrigin(0.0f, 0.0f);

		if (plantExist(i)) {
			seedPackets[seedPacketIdToString(i)].setPosition(seedChooser_background.getPosition() +
				sf::Vector2f(20.0f + i * 50.0f * scene1ZoomSize, 55.0f));
		}
		else {
			seedPackets[seedPacketIdToString(i)].setPosition(10000.f, 10000.f);
		}
	}
}

bool canPlant(sf::Vector2f pos) {
	return pos.x >= -210 && pos.x <= 910 && pos.y >= -310 && pos.y <= 370;
}

std::vector<plantState> plantsOnScene;
std::vector<zombieState> zombiesOnScene;
std::vector<projectileState> projectilesOnScene;
std::vector<vanishProjState> vanishProjectilesOnScene;
std::vector<sunState> sunsOnScene;

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

static int getSunByTypeAndId(int type, int id) { //type 0: plant
	static int cost[] = { 100, 50 };
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

	if (newZombie.getTexture() == nullptr) {
		std::cout << "Failed to create sprite for animId: " << animId << std::endl;
		return;
	}

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
		auto it = seedPackets.find(seedPacketIdToString(id));
		if (it != seedPackets.end()) {
			//if (seedPacketState[i][0] == 2) {
			pvzPacketOnSelected = true;
			seedPacketSelectedId = id;

			auto& plant = idlePlants[idlePlantToString[id]];
			hoverPlant.setTexture(*plant.getTexture());
			hoverPlant.setTextureRect(plant.getTextureRect());
			hoverPlant.setScale(scene1ZoomSize, scene1ZoomSize);
			hoverPlant.setOrigin(hoverPlant.getTextureRect().getSize().x / 2.0f,
				hoverPlant.getTextureRect().getSize().y / 2.0f);

			//seedPacketState[i][0] = 1;
			overlayShade.setPosition(seedPackets.find(seedPacketIdToString(id))->second.getPosition());
		}
	}
	else {
		blinkSunText = 0;
	}
}

static int getProjectileDamageById(int id) {
	return id == 0 ? 20 : 0;
}

bool damageZombie(projectileState projectile, zombieState& zombie) {
	zombie.hp -= getProjectileDamageById(projectile.id);
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

void winLevel() {
	pvzScene = 4;
	
	for (auto& sun : sunsOnScene) {
		selectSun(sun);
	}

	sf::RectangleShape& unlockSP = seedPackets[seedPacketIdToString(getPlantIdByLevel())];
	unlockSP.setOrigin(unlockSP.getSize().x / 2.0f, unlockSP.getSize().y / 2.0f);
	unlockSP.setPosition(view_background.getCenter());

	unlockPlantByLevel();
}