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

int pvzScene = 0;
int pvzSun = 150;
int seedPacketSelected = 0;

std::array<std::string, maxPlantAmount> idlePlantToString = { "peashooter", "sunflower" };
std::string seedPacketIdToString(int id) {
    return "seedPacket_" + idlePlantToString[id];
}

std::map<std::string, sf::RectangleShape> seedPackets = {
    {seedPacketIdToString(0), sf::RectangleShape()},
    {seedPacketIdToString(1), sf::RectangleShape()}
};
std::map<std::string, sf::Sprite> idlePlants = {
    {idlePlantToString[0], sf::Sprite()},
    {idlePlantToString[1], sf::Sprite()}
};
std::vector<std::map<int, float>> seedPacketState(maxPlantAmount); //state, state1 moving time

int seedPacketSelectedId;

void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime) {
    if (elapsedTime <= 0) return;

    if (seedPacketState[i][1] == 0 && seedPacketState[i][0] != 4) {
        seedPacketSelected += 2 - (int)seedPacketState[i][0];
    }

    seedPacketState[i][1] += elapsedTime;

    float distanceMoved = seedPacketState[i][1] / 5.0f;
    auto packetIterator = seedPackets.find(seedPacketIdToString(i));
    if (packetIterator != seedPackets.end()) {
        sf::Vector2f currentPosition = packetIterator->second.getPosition();

        sf::Vector2f direction = targetPosition - currentPosition;

        float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
        if (length != 0) {
            direction /= length;
        }

        sf::Vector2f newPosition = currentPosition + direction * distanceMoved;

        if ((direction.x > 0 && newPosition.x > targetPosition.x) ||
            (direction.x < 0 && newPosition.x < targetPosition.x) ||
            (direction.y > 0 && newPosition.y > targetPosition.y) ||
            (direction.y < 0 && newPosition.y < targetPosition.y)) {
            newPosition = targetPosition;
            seedPacketState[i][0] = seedPacketState[i][0] == 1.0f || seedPacketState[i][0] == 4.0f ? 2.0f : 0.0f;
            seedPacketState[i][1] = 0.0f;
        }

        packetIterator->second.setPosition(newPosition);
    }
}

const std::map<int, sf::Vector2f> stateToTargetPosition = {
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

void hideTempPlants() {
    hoverPlant.setPosition(10000, 10000);
    hoverShade.setPosition(10000, 10000);

    for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
        idlePlants[idlePlantToString[i]].setPosition(10000, 10000); //idlePlants[0]
    }
}

std::string getPlantNameById(int id) {
    switch (id) {
    default:
    case 0:
        return "peashooter";
    case 1:
        return "sunflower";
    }
}

std::map<int, sf::Texture> seedPacketsTexture;

sf::Texture* getPlantIdleTextureById(int id) {
    switch (id) {
    default:
    case 0:
        return &peashooterIdleSprites;
    case 1:
        return &sunflowerIdleSprites;
    }
}

std::map<int, SpriteFrame>* getPlantIdleFrameById(int id) {
    switch (id) {
    default:
    case 0:
        return &peashooterIdleFrames;
    case 1:
        return &sunflowerIdleFrames;
    }
}

sf::Texture* getPlantAttackTextureById(int id) {
    switch (id) {
    default:
    case 0:
        return &peashooterShootSprites;
    case 1:
        return &sunflowerIdleSprites;
    }
}

std::map<int, SpriteFrame>* getPlantAttackFrameById(int id) {
    switch (id) {
    default:
    case 0:
        return &peashooterShootFrames;
    case 1:
        return &sunflowerIdleFrames;
    }
}

int getPlantJsonIdById(int id) {
    switch (id) {
    default:
    case 0:
        return 116;
    case 1:
        return 134;
    }
}

int getPlantMaxFrameById(int id) {
    switch (id) {
    default:
    case 0:
    case 1:
        return 24;
    }
}

float scene1ZoomSize = 1.7f;
void initializeScene1() {
    initPlantsStatus();

    texture_background.loadFromImage(getPvzImage("background", "bg1"));
    float bgCamSizeY = view_background.getSize().y;
    background.setSize(sf::Vector2f(1400.0f / 600.0f * bgCamSizeY, bgCamSizeY)); //1920.0f, 1046.0f -> bg png size 1400 x 600
    background.setTexture(&texture_background);
    view_background.move((background.getSize().x - view_background.getSize().x) / 2.0f, 10.0f);

    texture_seedBank.loadFromImage(getPvzImage("seed_selector", "seedBank"));
    seedBank.setSize(sf::Vector2f(446.0f * scene1ZoomSize, 87.0f * scene1ZoomSize));
    seedBank.setTexture(&texture_seedBank);
    seedBank.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
        view_background.getCenter().y - view_background.getSize().y / 2.0f);

    texture_seedChooser_background.loadFromImage(getPvzImage("seed_selector", "seedChooser_background"));
    seedChooser_background.setSize(sf::Vector2f(seedBank.getSize().x, 513.0f / 465.0f * seedBank.getSize().x)); //465 x 513
    seedChooser_background.setTexture(&texture_seedChooser_background);
    seedChooser_background.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
        view_background.getCenter().y - view_background.getSize().y / 2.0f + seedBank.getSize().y);
    
    texture_seedChooserDisabled.loadFromImage(getPvzImage("seed_selector", "seedChooserDisabled"));
    texture_seedChooser.loadFromImage(getPvzImage("seed_selector", "seedChooserButton"));
    seedChooserButton.setSize(sf::Vector2f(278.0f, 72.0f));
    seedChooserButton.setTexture(&texture_seedChooserDisabled);
    seedChooserButton.setPosition(seedChooser_background.getPosition().x + (seedChooser_background.getSize().x - seedChooserButton.getSize().x) / 2.0f,
        seedChooser_background.getPosition().y + seedChooser_background.getSize().y - seedChooserButton.getSize().y - 15.0f);

    pvzStartText.setPosition(view_background.getCenter());
    pvzStartText_ready.loadFromImage(getPvzImage("seed_selector", "startReady"));
    pvzStartText_set.loadFromImage(getPvzImage("seed_selector", "startSet"));
    pvzStartText_plant.loadFromImage(getPvzImage("seed_selector", "startPlant"));

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

    nlohmann::json zombieIdleJson = loadJsonFromResource(118);
    zombieIdleSprites.loadFromImage(getPvzImage("animations", "zombieIdle"));
    zombieIdleFrames = parseSpriteSheetData(zombieIdleJson);
    zombieIdle.setTexture(zombieIdleSprites);
    zombieIdle.setTextureRect(zombieIdleFrames[0].frameRect);
    zombieIdle.setScale(scene1ZoomSize, scene1ZoomSize);
    zombieIdle.setOrigin(zombieIdle.getTextureRect().getSize().x / 2.0f,
        zombieIdle.getTextureRect().getSize().y / 2.0f);

    nlohmann::json zombieIdle1Json = loadJsonFromResource(120);
    zombieIdle1Sprites.loadFromImage(getPvzImage("animations", "zombieIdle1"));
    zombieIdle1Frames = parseSpriteSheetData(zombieIdle1Json);
    zombieIdle1.setTexture(zombieIdle1Sprites);
    zombieIdle1.setTextureRect(zombieIdle1Frames[0].frameRect);
    zombieIdle1.setScale(scene1ZoomSize, scene1ZoomSize);
    zombieIdle1.setOrigin(zombieIdle1.getTextureRect().getSize().x / 2.0f,
        zombieIdle1.getTextureRect().getSize().y / 2.0f);

    nlohmann::json zombieWalkJson = loadJsonFromResource(125);
    zombieWalkSprites.loadFromImage(getPvzImage("animations", "zombieWalk"));
    zombieWalkFrames = parseSpriteSheetData(zombieWalkJson);
    zombieWalk.setTexture(zombieWalkSprites);
    zombieWalk.setTextureRect(zombieWalkFrames[0].frameRect);
    zombieWalk.setScale(scene1ZoomSize, scene1ZoomSize);
    zombieWalk.setOrigin(zombieWalk.getTextureRect().getSize().x / 2.0f,
        zombieWalk.getTextureRect().getSize().y / 4.0f * 3.0f);

    peaTexture.loadFromImage(getPvzImage("projectiles", "pea"));
    pea.setTexture(peaTexture);
    pea.setScale(1.5f, 1.5f);
    pea.setOrigin(pea.getGlobalBounds().width / 2.0f, pea.getGlobalBounds().height / 2.0f + 10.0f);

    nlohmann::json peaSplatsJson = loadJsonFromResource(127);
    peaSplatsSprites.loadFromImage(getPvzImage("animations", "peaSplats"));
    peaSplatsFrames = parseSpriteSheetData(peaSplatsJson);
    peaSplats.setTexture(peaSplatsSprites);
    peaSplats.setTextureRect(peaSplatsFrames[0].frameRect);
    peaSplats.setScale(scene1ZoomSize, scene1ZoomSize);
    peaSplats.setOrigin(peaSplats.getTextureRect().getSize().x / 2.0f,
        peaSplats.getTextureRect().getSize().y / 2.0f + 10.0f);

    nlohmann::json zombieEatJson = loadJsonFromResource(130);
    zombieEatSprites.loadFromImage(getPvzImage("animations", "zombieEat"));
    zombieEatFrames = parseSpriteSheetData(zombieEatJson);
    zombieEat.setTexture(zombieEatSprites);
    zombieEat.setTextureRect(zombieEatFrames[0].frameRect);
    zombieEat.setScale(scene1ZoomSize, scene1ZoomSize);
    zombieEat.setOrigin(zombieEat.getTextureRect().getSize().x / 2.0f,
        zombieEat.getTextureRect().getSize().y / 4.0f * 3.0f);

    nlohmann::json peashooterShootJson = loadJsonFromResource(123);
    peashooterShootFrames = parseSpriteSheetData(peashooterShootJson);
    peashooterShootSprites.loadFromImage(getPvzImage("animations", "peashooterShoot"));

    for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
        nlohmann::json tempPlantJson = loadJsonFromResource(getPlantJsonIdById(i));

        getPlantIdleTextureById(i)->loadFromImage(getPvzImage("animations", idlePlantToString[i] + "Idle"));
        *getPlantIdleFrameById(i) = parseSpriteSheetData(tempPlantJson);
        idlePlants[idlePlantToString[i]].setTexture(*getPlantIdleTextureById(i));
        idlePlants[idlePlantToString[i]].setTextureRect(getPlantIdleFrameById(i)->find(0)->second.frameRect);
        idlePlants[idlePlantToString[i]].setScale(scene1ZoomSize, scene1ZoomSize);
        idlePlants[idlePlantToString[i]].setOrigin(idlePlants[idlePlantToString[i]].getTextureRect().getSize().x / 2.0f,
            (float)idlePlants[idlePlantToString[i]].getTextureRect().getSize().y);

        seedPacketsTexture[i].loadFromImage(getPvzImage("seed_packet", getPlantNameById(i)));
        seedPackets.find(seedPacketIdToString(i))->second.setSize(sf::Vector2f(50.0f * scene1ZoomSize, 70.0f * scene1ZoomSize));
        seedPackets.find(seedPacketIdToString(i))->second.setTexture(&seedPacketsTexture[i]);
        seedPackets.find(seedPacketIdToString(i))->second.setPosition(seedChooser_background.getPosition() + sf::Vector2f(20.0f + i * 50.0f * scene1ZoomSize, 55.0f));
    }

    background.setOrigin(background.getSize() / 2.0f);

    hideTempPlants();
}

bool canPlant(sf::Vector2f pos) {
    return pos.x >= -210 && pos.x <= 910 && pos.y >= -310 && pos.y <= 370;
}

std::vector<plantState> plantsOnScene;
std::vector<zombieState> zombiesOnScene;
std::vector<projectileState> projectilesOnScene;
std::vector<vanishProjState> vanishProjectilesOnScene;

int getRowByY(float posY) { //0:-310 1:-140 2:30 3:200 4:370
    switch((int)posY) {
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

void getSunByTypeAndId(int type, int id) { //type 0: plant
    
}

void createPlant(sf::Vector2f pos, int id) {
    if (canPlant(hoverPlant.getPosition())) {
        sf::Sprite newPlant;
        newPlant = hoverPlant;
        plantsOnScene.push_back({ {newPlant, id, 0, getRowByY(pos.y)}, false, 300, 0 });
        hideTempPlants();
        pvzSun -= 100;
        pvzPacketOnSelected = false;
    }
}

sf::Sprite getZombieSpriteById(int id) {
    switch (id) {
    default:
    case 0:
        return zombieIdle;
    case 1:
        return zombieIdle1;
    case 2:
        return zombieWalk;
    }
}

void createZombie(sf::Vector2f pos) {
    createZombie(pos, 0);
}


void createRandomZombie() {
    createZombie(sf::Vector2f((float)(1300 + rand() % 200), (float)(-310 + 170 * (rand() % 5))), 1);
}

void createZombie(sf::Vector2f pos, int style) {
    sf::Sprite newZombie;
    int animId = 0;
    int row = 0;

    if (style == 0) {
        animId = rand() % 2;
    }
    else if (style == 1) {
        animId = 2;
        row = getRowByY(pos.y);
    }

    newZombie = getZombieSpriteById(animId);
    newZombie.setPosition(pos);
    zombiesOnScene.push_back({ {newZombie, animId, rand() % 28, row}, 200, 0, 
        sf::Vector2f(-0.5f - (rand() % 26) / 100.0f, 0.0f), nullptr });
}

void createProjectile(int type, sf::Vector2f pos) {
    if (type == 0) {
        sf::Sprite newProjectile;
        int row = getRowByY(pos.y);

        newProjectile = pea;
        newProjectile.setPosition(pos);
        projectilesOnScene.push_back({ newProjectile, type, row });
    }
}

void selectSeedPacket(sf::Vector2f mousePos) {
    for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
        if (seedPackets.find(seedPacketIdToString(i))->second.getGlobalBounds().contains(mousePos)) {
            selectSeedPacket(i);
        }
    }
}

void selectSeedPacket(int id) { //--id;
    if (seedPackets.find(seedPacketIdToString(id)) != seedPackets.end()) {
            //if (seedPacketState[i][0] == 2) {
            pvzPacketOnSelected = true;
            seedPacketSelectedId = id;

            hoverPlant.setTexture(*idlePlants[idlePlantToString[id]].getTexture());
            hoverPlant.setTextureRect(idlePlants[idlePlantToString[id]].getTextureRect());
            hoverPlant.setScale(scene1ZoomSize, scene1ZoomSize);
            hoverPlant.setOrigin(hoverPlant.getTextureRect().getSize().x / 2.0f,
                hoverPlant.getTextureRect().getSize().y / 2.0f);

            //seedPacketState[i][0] = 1;
            overlayShade.setPosition(seedPackets.find(seedPacketIdToString(id))->second.getPosition());
    }
}

int getProjectileDamageById(int id) {
    switch (id) {
    case 0:
        return 20;
    default:
        return 0;
    }
}

bool damageZombie(projectileState projectile, zombieState& zombie) {
    zombie.hp -= getProjectileDamageById(projectile.id);
    return (zombie.hp <= 0);
}

void createProjectileVanishAnim(projectileState proj) {
    projectileState vanishAnim;
    vanishAnim.sprite = peaSplats;
    vanishAnim.sprite.setPosition(proj.sprite.getPosition());
    vanishProjectilesOnScene.push_back({ vanishAnim, 0 });
}

bool damagePlant(plantState& plant) {
    plant.hp -= 36;
    return (plant.hp <= 0);
}

std::map<int, int> seedPacketsSelectedOrder;
std::map<int, int> plantsLevel;

void initPlantsStatus() {
    for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
        plantsLevel[i] = 0;
    }

    plantsLevel[0] = 1;
}

int getOwnedPlantsAmount() {
    int ret = 0;

    for (auto& plants : plantsLevel) {
        if (plants.second > 0) ++ret;
    }

    return ret;
}