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

std::vector<std::array<int, 2>> targetCoords;

std::atomic<bool> running(true);
std::atomic<bool> blinkMap_loadingCoords(false);
std::atomic<bool> blinkMap_readyToDraw(false);
std::atomic<bool> loadFlag_readyToDraw(false);
std::atomic<bool> loadLevelStart_readyToDraw(false);

int fps = 60;
int scene = 0;

int blinkCoords[2] = { 0, 0 };

void asyncBlinkMap() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= static_cast<unsigned int>(1000 / fps) && scene == 0) { //(int)(1000.0f/60.0f)
            if (!targetCoords.empty() && !blinkMap_readyToDraw.load() && !clicking_state.empty() && !blinkMap_loadingCoords.load()) {
                /*int currentBlinkCd = blinkCd.load();
                //if (currentBlinkCd < 5) {
                    //blinkCd.store(currentBlinkCd + 1);
                //}
                //else {
                    //blinkCd.store(0);*/

                sf::Image world_image_blink = pixelsToBlink(targetCoords, world_image);

                int sx = state_int[clicking_state]["sx"]();
                int sy = state_int[clicking_state]["sy"]();
                int lx = state_int[clicking_state]["lx"]();
                int ly = state_int[clicking_state]["ly"]();

                //std::cout << "sx: " << sx << ", sy: " << sy << ", lx: " << lx << ", ly: " << ly << std::endl;

                sf::IntRect cropArea(sx, sy, lx - sx + 1, ly - sy + 1);
                world_image_blink = cropImage(world_image_blink, cropArea);

                if (texture_blink.getSize().x != static_cast<unsigned int>(cropArea.width) ||
                    texture_blink.getSize().y != static_cast<unsigned int>(cropArea.height)) {
                    texture_blink.create(cropArea.width, cropArea.height);
                }

                texture_blink.update(world_image_blink);
                world_blink.setTextureRect(sf::IntRect(0, 0, cropArea.width, cropArea.height));
                world_blink.setSize(sf::Vector2f(mapRatio * cropArea.width, mapRatio * cropArea.height));
                blinkCoords[0] = sx;
                blinkCoords[1] = sy;
                //world_blink.setPosition(sf::Vector2f(sx * mapRatio, sy * mapRatio)); //view.getCenter().x, view.getCenter().y

                blinkMap_readyToDraw.store(true);
            }
            lastTime = currentTime;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void asyncLoadFlag() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= static_cast<unsigned int>(1000 / fps) && scene == 0) {
            if (!flag.empty() && !loadFlag_readyToDraw.load()) {
                if (current_flag != flag) {
                    if (current_flag.empty()) {
                        flag_texture.create(383, 256);
                        flag_rect.setTexture(&flag_texture);
                        flag_texture.loadFromImage(getFlagImage(flag));
                    }
                    else {
                        flag_texture.update(getFlagImage(flag));
                    }

                    current_flag = flag;
                    flag_texture.loadFromImage(getFlagImage(flag));
                }
                loadFlag_readyToDraw.store(true);
            }
            lastTime = currentTime;
        }
    }
}

void asyncLoadLevelStart() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= static_cast<unsigned int>(1000 / fps) && scene == 0) {
            if (!clicking_state.empty() && !loadLevelStart_readyToDraw.load()) {
                loadLevelStart_readyToDraw.store(true);
            }
            lastTime = currentTime;
        }
    }
}

bool pvzPacketOnSelected = false;
float animSpeed = 3.5f;

std::map<int, SpriteFrame> getZombieAnimFrameFromId(int id) {
    switch (id) {
    default:
    case 0:
        return zombieIdleFrames;
    case 1:
        return zombieIdle1Frames;
    case 2:
        return zombieWalkFrames;
    case 3:
        return zombieEatFrames;
    }
};

int getZombieMaxAnimFramesById(int id) {
    switch (id) {
    default:
    case 0:
        return 27;
    case 1:
        return 13;
    case 2:
        return 45;
    case 3:
        return 38;
    }
}

sf::Texture& getZombieTextureById(int id) {
    switch (id) {
    default:
    case 0:
        return zombieIdleSprites;
    case 1:
        return zombieIdle1Sprites;
    case 2:
        return zombieWalkSprites;
    case 3:
        return zombieEatSprites;
    }
}

float getAnimRatioById(int id) {
    switch (id) {
    default:
        return 1.125f;
    case 3:
        return 1.0f;
    }
}

void updateZombieAnim() {
    for (auto& zombie : zombiesOnScene) {
        auto& sprite = zombie.anim.sprite;
        ++zombie.anim.frameId;

        if (zombie.anim.frameId > (float)getZombieMaxAnimFramesById(zombie.anim.animId) * animSpeed
            * getAnimRatioById(zombie.anim.animId)) zombie.anim.frameId = 0;
       
        if (&getZombieTextureById(zombie.anim.animId) != zombie.anim.sprite.getTexture()) {
            zombie.anim.sprite.setTexture(getZombieTextureById(zombie.anim.animId));
        }

        sprite.setTextureRect(getZombieAnimFrameFromId(zombie.anim.animId)[
            static_cast<int>(std::floor(zombie.anim.frameId / animSpeed
                / getAnimRatioById(zombie.anim.animId)))].frameRect);
    }
}

void asyncPvzSceneUpdate() {
    int pvzScene1moving = 0;
    auto lastTime = std::chrono::high_resolution_clock::now();

    const int moveAmount = 50;
    const float duration = moveAmount * (1000.0f / fps) * 0.5f;
    float elapsedTimeTotal = 0.0f;
    int pvzStartScene = 0;

    const std::chrono::milliseconds frameTime(1000 / fps);

    int repeatSpawnZombiesTimes = 5 + rand() % 6;
    std::vector<float> yCoords;

    for (int i = 0; i < repeatSpawnZombiesTimes; i++) {
        float yCoord = static_cast<float>(-350 + (45 + rand() % 11) * (rand() % 15));
        yCoords.push_back(yCoord);
    }

    std::sort(yCoords.begin(), yCoords.end());

    for (const auto& y : yCoords) {
        float x = static_cast<float>(775 + (30 + rand() % 11) * (rand() % 11));
        createZombie(sf::Vector2f(x, y));
    }

    int zombieSpawnTimer = 0;

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime >= frameTime && scene == 1) {
            switch (pvzScene) {
            default:
            case 0:
                if (audios["lawnbgm"]["6"]->getStatus() != sf::Music::Playing) audios["lawnbgm"]["6"]->play();

                if (seedPacketSelected == maxPlantAmount) {
                    seedChooserButton.setTexture(&texture_seedChooser);
                }
                else {
                    seedChooserButton.setTexture(&texture_seedChooserDisabled);
                }

                for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
                    auto it = stateToTargetPosition.find((int)seedPacketState[i][0]);
                    if (it != stateToTargetPosition.end()) {
                        if (seedPacketState[i][0] == 3 && seedPacketState[i][1] == 0) {
                            //sf::Mouse::setPosition(sf::Vector2i(175, 300));
                            for (size_t j = 0; j < static_cast<size_t>(maxPlantAmount); ++j) {
                                if (i == j) continue;
                                if (seedPacketState[j][0] == 2) {
                                    if (seedPackets.find(seedPacketIdToString(j))->second.getPosition().x <
                                        seedPackets.find(seedPacketIdToString(i))->second.getPosition().x) continue;
                                    seedPacketState[j][0] = 4;
                                    seedPacketState[j][2] = seedPackets.find(seedPacketIdToString(j))->second.getPosition().x
                                        - seedPackets.find(seedPacketIdToString(i))->second.getSize().x; //-570.0f
                                    seedPacketState[j][3] = seedPackets.find(seedPacketIdToString(j))->second.getPosition().y;
                                }
                                else if (seedPacketState[j][0] == 1) {
                                    seedPacketState[j][1] = 0;
                                    seedPacketState[j][0] = 4;
                                    seedPacketState[j][2] = stateToTargetPosition.find(1)->second.x + 50.0f * scene1ZoomSize * seedPacketSelected;
                                    seedPacketState[j][3] = seedPackets.find(seedPacketIdToString(i))->second.getPosition().y;
                                }
                            }
                        }
                        updatePacketPosition(i, it->second + sf::Vector2f(50.0f * scene1ZoomSize * (seedPacketState[i][0] == 3 ? 
                            i : (seedPacketSelected - 1)), 0.0f),
                            static_cast<int>(elapsedTime.count()));
                    }
                    else if (seedPacketState[i][0] == 4) {
                        updatePacketPosition(i, sf::Vector2f(seedPacketState[i][2], seedPacketState[i][3]), static_cast<int>(elapsedTime.count()));
                    }
                }

                if (!zombiesOnScene.empty()) {
                    updateZombieAnim();
                }
                break;
            case 1: {
                if (audios["lawnbgm"]["6"]->getStatus() == sf::Music::Playing) audios["lawnbgm"]["6"]->stop();

                if (pvzScene1moving < moveAmount) {
                    pvzScene1moving++;
                }
                else {
                    background.setPosition(495.0f, 0.0f);
                    pvzStartText.setTexture(&pvzStartText_ready);
                    pvzStartText.setSize(sf::Vector2f(pvzStartText_ready.getSize()));
                    pvzStartText.setOrigin(pvzStartText.getSize() / 2.0f);
                    elapsedTimeTotal = 0.0f;
                    pvzScene = 2;
                    zombiesOnScene.clear();
                    audios["sounds"]["readysetplant"]->play();
                    break;
                }

                elapsedTimeTotal += elapsedTime.count();
                float t = elapsedTimeTotal / duration;
                //if (t > 2.0f) t = 2.0f;

                float easedT = easeInOutQuad(t) * static_cast<float>(elapsedTime.count());

                seedChooser_background.move(0.0f, 2.0f * easedT);
                seedChooserButton.move(0.0f, 2.0f * easedT);
                background.move(0.9f * easedT, 0.0f);

                if (!zombiesOnScene.empty()) {
                    updateZombieAnim();
                    for (auto& zombie : zombiesOnScene) {
                        zombie.anim.sprite.move(0.9f * easedT, 0.0f);
                    }
                }

                break;
            }
            case 2: {
                elapsedTimeTotal += elapsedTime.count();

                if (elapsedTimeTotal >= 531.0f) {
                    if (pvzStartScene >= 2) {
                        pvzScene = 3;
                        break;
                    }
                    pvzStartScene++;
                    elapsedTimeTotal = 0.0f;
                    pvzStartText.setScale(1.0f, 1.0f);
                    pvzStartText.setTexture(pvzStartScene == 1 ? &pvzStartText_set : &pvzStartText_plant);
                    pvzStartText.setSize(sf::Vector2f(
                        pvzStartScene == 1 ? pvzStartText_set.getSize() : pvzStartText_plant.getSize()));
                }

                if (pvzStartScene < 2) {
                    pvzStartText.scale(1.0f + elapsedTime.count() / 1350.0f, 1.0f + elapsedTime.count() / 1350.0f);
                }
                else {
                    pvzStartText.setScale(1.5f, 1.5f);
                }

                break;
            }
            case 3:
                if (audios["lawnbgm"]["1"]->getStatus() != sf::Music::Playing) {
                    audios["lawnbgm"]["1"]->play();
                }
                if (zombieSpawnTimer <= 0) {
                    zombieSpawnTimer = 1000 + rand() % 500;
                    for (int i = 0; i < 3 + rand() % 5; i++) {
                        createRandomZombie();
                    }
                }
                else {
                    --zombieSpawnTimer;
                }
                if (pvzPacketOnSelected) { //getPlantSpriteById(seedPacketSelectedId)
                    idlePlants[idlePlantToString[seedPacketSelectedId]].setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)) +
                        sf::Vector2f(0.0f, 36.0f));
                    sf::Vector2f hoverCoords = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                    hoverPlant.setPosition(roundf((hoverCoords.x + 70.0f) / 140.0f) * 140.0f - 70.0f,
                        roundf((hoverCoords.y - 30.0f) / 170.0f) * 170.0f + 30.0f);
                    hoverShade.setPosition(hoverPlant.getPosition());
                }
                if (!zombiesOnScene.empty()) {
                    updateZombieAnim();
                    for (auto& zombie : zombiesOnScene) {
                        bool isColliding = false;

                        if (!plantsOnScene.empty()) {
                            for (auto& plant : plantsOnScene) {
                                if (plant.anim.row == zombie.anim.row) {
                                    sf::FloatRect plantBounds = plant.anim.sprite.getGlobalBounds();
                                    sf::FloatRect zombieBounds = zombie.anim.sprite.getGlobalBounds();
                                    if (plantBounds.left < zombieBounds.left + zombieBounds.width &&
                                        plantBounds.left + plantBounds.width > zombieBounds.left) {
                                        isColliding = true;
                                        zombie.targetPlant = &plant;
                                        break;
                                    }
                                }
                            }
                        }

                        //zombie.eating = isColliding;
                        if (isColliding) {
                            zombie.anim.animId = 3;
                            if (zombie.targetPlant != nullptr && 
                                (zombie.anim.frameId == std::roundf(12 * animSpeed 
                                    * getAnimRatioById(zombie.anim.animId) - 0.5f) ||
                                    zombie.anim.frameId == std::roundf(33 * animSpeed 
                                        * getAnimRatioById(zombie.anim.animId) - 0.5f))) {
                                if (damagePlant(*zombie.targetPlant)) {
                                    plantsOnScene.erase(std::remove_if(plantsOnScene.begin(), plantsOnScene.end(),
                                        [&](const plantState& plant) {
                                            return &plant == zombie.targetPlant;
                                        }),
                                        plantsOnScene.end());
                                }
                                else {
                                    zombie.targetPlant->damagedCd = 10;
                                }
                            }
                        }
                        else {
                            zombie.anim.animId = 2;
                            zombie.anim.sprite.move(zombie.movementSpeed);
                            zombie.targetPlant = nullptr;
                        }

                        if (zombie.damagedCd > 0) {
                            --zombie.damagedCd;
                        }
                    }
                }
                if (!plantsOnScene.empty()) {
                    for (auto& plant : plantsOnScene) {
                        if (plant.damagedCd > 0) {
                            --plant.damagedCd;
                        }

                        auto& sprite = plant.anim.sprite;
                        ++plant.anim.frameId;
                        if (plant.anim.frameId > getPlantMaxFrameById(plant.anim.animId) * animSpeed) plant.anim.frameId = 0;

                        int frame = static_cast<int>(std::floor(plant.anim.frameId / animSpeed));

                        switch (plant.anim.animId) {
                        case 0:
                            if ((!plant.attack && frame >= 20) || (plant.attack && frame <= 7)) {
                                plant.attack = false;
                                for (auto& zombie : zombiesOnScene) {
                                    if (plant.anim.row == zombie.anim.row
                                        && zombie.anim.sprite.getPosition().x <= 1300) {
                                        plant.attack = true;
                                        break;
                                    }
                                }
                            }
                            break;
                        }

                        if (plant.attack) {
                            sprite.setTexture(*getPlantAttackTextureById(plant.anim.animId));
                            sprite.setTextureRect(getPlantAttackFrameById(plant.anim.animId)->find(frame)->second.frameRect);
                        }
                        else {
                            sprite.setTexture(*getPlantIdleTextureById(plant.anim.animId));
                            sprite.setTextureRect(getPlantIdleFrameById(plant.anim.animId)->find(frame)->second.frameRect);
                        }

                        sf::FloatRect bounds = sprite.getGlobalBounds();
                        sprite.setOrigin(sprite.getTextureRect().getSize().x / 2.0f,
                            sprite.getTextureRect().getSize().y - 36.0f); //fix for 2nd plant

                        if (plant.attack) {
                            if (std::fabs(plant.anim.frameId - 12.0f * animSpeed) < 1.0f)
                                createProjectile(0, plant.anim.sprite.getPosition() 
                                    + sf::Vector2f(72.0f, 0.0f));
                        }
                    }
                }
                auto it = projectilesOnScene.begin();
                while (it != projectilesOnScene.end()) {
                    it->sprite.move(10.0f, 0.0f);

                    bool shouldEraseProjectile = false;

                    if (it->sprite.getPosition().x > 1500) shouldEraseProjectile = true;

                    if (!shouldEraseProjectile) {
                        for (auto zombieIt = zombiesOnScene.begin(); zombieIt != zombiesOnScene.end(); ++zombieIt) {
                            sf::FloatRect projectileBounds = it->sprite.getGlobalBounds();
                            sf::FloatRect zombieBounds = zombieIt->anim.sprite.getGlobalBounds();

                            bool isTouching = projectileBounds.left < zombieBounds.left + zombieBounds.width &&
                                projectileBounds.left + projectileBounds.width > zombieBounds.left;

                            if (isTouching && it->row == zombieIt->anim.row) {
                                if (damageZombie(*it, *zombieIt)) {
                                    zombiesOnScene.erase(zombieIt);
                                }
                                else {
                                    zombieIt->damagedCd = 10;
                                }
                                createProjectileVanishAnim(*it);
                                shouldEraseProjectile = true;
                                break;
                            }
                        }
                    }

                    if (shouldEraseProjectile) {
                        it = projectilesOnScene.erase(it);
                    }
                    else {
                        ++it;
                    }
                }
                break;
            }

            lastTime = currentTime;
        }
        auto timeTaken = std::chrono::high_resolution_clock::now() - currentTime;
        if (timeTaken < frameTime) {
            std::this_thread::sleep_for(frameTime - timeTaken);
        }
    }
}

std::thread thread_asyncBlinkMap;
std::thread thread_asyncLoadFlag;
std::thread thread_asyncLoadLevelStart;
std::thread thread_asyncPacketMove;

void stopAllThreads() {
    running.store(false);
    if (thread_asyncBlinkMap.joinable()) thread_asyncBlinkMap.join();
    if (thread_asyncLoadFlag.joinable()) thread_asyncLoadFlag.join();
    if (thread_asyncLoadLevelStart.joinable()) thread_asyncLoadLevelStart.join();
    if (thread_asyncPacketMove.joinable()) thread_asyncPacketMove.join();
    running.store(true);
}