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
#include "Window.h"
#include "Colour.h"
#include "Resource.h"
#include "State.h"
#include <fstream>
#include <stdexcept>
#include <queue>
#include <shellapi.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include "Display.h"
#include "Async.h"
#include "Scene1.h"
#include "Json.h"

int pvzScene = 0;
int pvzSun = 150;
int seedPacketSelected = 0;
int maxSeedPacketAmount = 1;

std::array<std::string, maxPlantAmount> seedPacketIdToString = { "seedPacket_peashooter" };
std::map<std::string, sf::RectangleShape> seedPackets = {
    {seedPacketIdToString[0], sf::RectangleShape()}
};
std::vector<std::map<int, int>> seedPacketState(maxPlantAmount); //state, state1 moving time

void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime) {
    if (elapsedTime <= 0) return;

    if (seedPacketState[i][1] == 0) {
        seedPacketSelected += 2 - seedPacketState[i][0];
    }

    seedPacketState[i][1] += elapsedTime;

    float distanceMoved = seedPacketState[i][1] / 5.0f;
    auto packetIterator = seedPackets.find(seedPacketIdToString[i]);
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
            seedPacketState[i][0] = (seedPacketState[i][0] == 1) ? 2 : 0;
            seedPacketState[i][1] = 0;
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
    peashooterIdle.setPosition(10000, 10000);
    hoverPlant.setPosition(10000, 10000);
    hoverShade.setPosition(10000, 10000);
}

void initializeScene1() {
    float zoomSize = 1.7f;

    texture_background.loadFromImage(getPvzImage("background", "bg1"));
    float bgCamSizeY = view_background.getSize().y;
    background.setSize(sf::Vector2f(1400.0f / 600.0f * bgCamSizeY, bgCamSizeY)); //1920.0f, 1046.0f -> bg png size 1400 x 600
    background.setTexture(&texture_background);
    view_background.move((background.getSize().x - view_background.getSize().x) / 2.0f, 10.0f);

    texture_seedBank.loadFromImage(getPvzImage("seed_selector", "seedBank"));
    seedBank.setSize(sf::Vector2f(446.0f * zoomSize, 87.0f * zoomSize));
    seedBank.setTexture(&texture_seedBank);
    seedBank.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
        view_background.getCenter().y - view_background.getSize().y / 2.0f);

    texture_seedChooser_background.loadFromImage(getPvzImage("seed_selector", "seedChooser_background"));
    seedChooser_background.setSize(sf::Vector2f(seedBank.getSize().x, 513.0f / 465.0f * seedBank.getSize().x)); //465 x 513
    seedChooser_background.setTexture(&texture_seedChooser_background);
    seedChooser_background.setPosition(view_background.getCenter().x - view_background.getSize().x / 2.0f,
        view_background.getCenter().y - view_background.getSize().y / 2.0f + seedBank.getSize().y);

    texture_seedPacket_peashooter.loadFromImage(getPvzImage("seed_packet", "peashooter"));
    seedPackets.find(seedPacketIdToString[0])->second.setSize(sf::Vector2f(50.0f * zoomSize, 70.0f * zoomSize));
    seedPackets.find(seedPacketIdToString[0])->second.setTexture(&texture_seedPacket_peashooter);
    seedPackets.find(seedPacketIdToString[0])->second.setPosition(seedChooser_background.getPosition() + sf::Vector2f(20.0f, 55.0f));

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

    pvzSunText.setFillColor(sf::Color::Black);
    pvzSunText.setPosition(-633.5f, -390.0f);

    overlayShade.setSize(sf::Vector2f(50.0f * zoomSize, 70.0f * zoomSize));
    overlayShade.setFillColor(sf::Color(0, 0, 0, 180));

    nlohmann::json peashooterIdleJson = loadJsonFromResource(116);
    peashooterIdleSprites.loadFromImage(getPvzImage("animations", "peashooterIdle"));
    peashooterIdleFrames = parseSpriteSheetData(peashooterIdleJson);
    peashooterIdle.setTexture(peashooterIdleSprites);
    peashooterIdle.setTextureRect(peashooterIdleFrames[0].frameRect);
    peashooterIdle.setScale(zoomSize, zoomSize);
    peashooterIdle.setOrigin(peashooterIdle.getTextureRect().getSize().x / 2.0f,
        peashooterIdle.getTextureRect().getSize().y / 2.0f);

    hoverPlant.setTexture(peashooterIdleSprites);
    hoverPlant.setTextureRect(peashooterIdleFrames[0].frameRect);
    hoverPlant.setScale(zoomSize, zoomSize);
    hoverPlant.setOrigin(hoverPlant.getTextureRect().getSize().x / 2.0f,
        hoverPlant.getTextureRect().getSize().y / 2.0f);

    hoverShade.setTexture(peashooterIdleSprites);
    hoverShade.setTextureRect(peashooterIdleFrames[0].frameRect);
    hoverShade.setScale(zoomSize, zoomSize);
    hoverShade.setOrigin(hoverShade.getTextureRect().getSize().x / 2.0f,
        hoverShade.getTextureRect().getSize().y / 2.0f);
    hoverShade.setColor(sf::Color(0, 0, 0, 175));

    background.setOrigin(background.getSize() / 2.0f);

    hideTempPlants();
}

bool canPlant(sf::Vector2f pos) {
    return pos.x >= -210 && pos.x <= 910 && pos.y >= -310 && pos.y <= 370;
}

std::vector<spriteAnim> plantsOnScene;

void createPlant(sf::Vector2f pos) {
    if (canPlant(hoverPlant.getPosition())) {
        sf::Sprite newPlant;
        newPlant = hoverPlant;
        plantsOnScene.push_back({ newPlant, 0 });
        hideTempPlants();
        pvzSun -= 100;
        pvzPacketOnSelected = false;
    }
}