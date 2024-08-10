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

std::map<int, SpriteFrame> getAnimFrameFromId(int id) {
    switch (id) {
    default:
    case 0:
        return zombieIdleFrames;
    case 1:
        return zombieIdle1Frames;
    }
};

int getMaxAnimFramesById(int id) {
    switch (id) {
    default:
    case 0:
        return 27;
    case 1:
        return 13;
    }
}

float getAnimRatioById(int id) {
    //switch (id) {
   //default:
        return 1.125f;
    //}
}

void updateZombieAnim() {
    for (auto& zombie : zombiesOnScene) {
        auto& sprite = zombie.sprite;
        ++zombie.frameId;

        if (zombie.frameId > (float)getMaxAnimFramesById(zombie.animId) * animSpeed 
            * getAnimRatioById(zombie.animId)) zombie.frameId = 0;
       
        sprite.setTextureRect(getAnimFrameFromId(zombie.animId)[
            static_cast<int>(std::floor(zombie.frameId / animSpeed 
                / getAnimRatioById(zombie.animId)))].frameRect);
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

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime >= frameTime && scene == 1) {
            switch (pvzScene) {
            default:
            case 0:
                if (audios["lawnbgm"]["6"]->getStatus() != sf::Music::Playing) audios["lawnbgm"]["6"]->play();

                if (seedPacketSelected == maxSeedPacketAmount) {
                    seedChooserButton.setTexture(&texture_seedChooser);
                }
                else {
                    seedChooserButton.setTexture(&texture_seedChooserDisabled);
                }

                for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
                    auto it = stateToTargetPosition.find(seedPacketState[i][0]);
                    if (it != stateToTargetPosition.end()) {
                        updatePacketPosition(i, it->second, static_cast<int>(elapsedTime.count()));
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
                        zombie.sprite.move(0.9f * easedT, 0.0f);
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
                    for (int i = 0; i < 3; i++) {
                        createRandomZombie();
                    }
                }
                if (pvzPacketOnSelected) {
                    peashooterIdle.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
                    sf::Vector2f hoverCoords = peashooterIdle.getPosition();
                    hoverPlant.setPosition(roundf((hoverCoords.x + 70.0f) / 140.0f) * 140.0f - 70.0f,
                        roundf((hoverCoords.y - 30.0f) / 170.0f) * 170.0f + 30.0f);
                    hoverShade.setPosition(hoverPlant.getPosition());
                }
                if (!zombiesOnScene.empty()) {
                    updateZombieAnim();
                    for (auto& zombie : zombiesOnScene) {
                        zombie.sprite.move(-0.5f, 0.0f);
                    }
                }
                if (!plantsOnScene.empty()) {
                    for (auto& plant : plantsOnScene) {
                        auto& sprite = plant.anim.sprite;
                        ++plant.anim.frameId;
                        if (plant.anim.frameId > 24.0f * animSpeed) plant.anim.frameId = 0;

                        int frame = static_cast<int>(std::floor(plant.anim.frameId / animSpeed));

                        if (!zombiesOnScene.empty()) {
                            if (frame >= 20) {
                                plant.attack = false;
                                for (auto& zombie : zombiesOnScene) {
                                    if (plant.anim.row == zombie.row) {
                                        plant.attack = true;
                                        break;
                                    }
                                }
                            }
                        }

                        if (plant.attack) {
                            sprite.setTexture(peashooterShootSprites);
                            sprite.setTextureRect(peashooterShootFrames[frame].frameRect);
                        }
                        else {
                            sprite.setTexture(peashooterIdleSprites);
                            sprite.setTextureRect(peashooterIdleFrames[frame].frameRect);
                        }

                        if (plant.attack) {
                            if (std::fabs(plant.anim.frameId - 12.0f * animSpeed) < 1.0f) createProjectile(0, plant.anim.sprite.getPosition());
                        }
                    }
                }
                projectilesOnScene.erase(std::remove_if(projectilesOnScene.begin(), projectilesOnScene.end(),
                    [&](const projectileState& projectile) {
                        const_cast<sf::Sprite&>(projectile.sprite).move(10.0f, 0.0f);

                        for (auto& zombie : zombiesOnScene) {
                            sf::FloatRect projectileBounds = projectile.sprite.getGlobalBounds();
                            sf::FloatRect zombieBounds = zombie.sprite.getGlobalBounds();

                            bool isTouching = projectileBounds.left < zombieBounds.left + zombieBounds.width &&
                                projectileBounds.left + projectileBounds.width > zombieBounds.left;

                            if (isTouching && projectile.row == zombie.row) {
                                return true;
                            }
                        }
                        return false;
                    }), projectilesOnScene.end());
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