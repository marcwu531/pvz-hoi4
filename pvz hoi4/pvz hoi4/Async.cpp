#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <shared_mutex>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <thread>
#include <windows.h>

#include "Async.h"
#include "Audio.h"
#include "Colour.h"
#include "Display.h"
#include "Scene1.h"
#include "State.h"

std::vector<sf::Vector2i> targetCoords;

std::atomic<bool> running(true);
std::atomic<bool> blinkMap_loadingCoords(false);
std::atomic<bool> blinkMap_readyToDraw(false);
std::atomic<bool> loadFlag_readyToDraw(false);
std::atomic<bool> loadLevelStart_readyToDraw(false);

int maxSpeed = 1, fps = 60 * maxSpeed, scene = 0;

sf::Vector2i blinkCoords(0, 0);

std::shared_mutex plantsMutex, zombiesMutex, projsMutex, vanishProjsMutex, sunsMutex;

inline static void updateImage(sf::Image& image, const sf::IntRect& cropArea, sf::Texture& texture) {
	if (texture.getSize().x != static_cast<unsigned int>(cropArea.width) ||
		texture.getSize().y != static_cast<unsigned int>(cropArea.height)) {
		texture.create(cropArea.width, cropArea.height);
	}
	texture.update(image);
}

void asyncBlinkMap() {
	auto nextTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1000 / fps);

	while (running.load()) {
		if (scene == 0) { //(int)(1000.0f/60.0f)
			if (!targetCoords.empty() && !blinkMap_readyToDraw.load() &&
				!clicking_state.empty() && !blinkMap_loadingCoords.load()) {
				/*int currentBlinkCd = blinkCd.load();
				//if (currentBlinkCd < 5) {
					//blinkCd.store(currentBlinkCd + 1);
				//}
				//else {
					//blinkCd.store(0);*/

				sf::Image world_image_blink = pixelsToBlink(targetCoords, world_image);

				int sx = state_int[clicking_state]["sx"](), sy = state_int[clicking_state]["sy"](),
					lx = state_int[clicking_state]["lx"](), ly = state_int[clicking_state]["ly"]();

				//std::cout << "sx: " << sx << ", sy: " << sy << ", lx: " << lx << ", ly: " << ly << std::endl;

				sf::IntRect cropArea(sx, sy, lx - sx + 1, ly - sy + 1);
				world_image_blink = cropImage(world_image_blink, cropArea);

				if (texture_blink.getSize().x != static_cast<unsigned int>(cropArea.width) ||
					texture_blink.getSize().y != static_cast<unsigned int>(cropArea.height)) {
					texture_blink.create(cropArea.width, cropArea.height);
				}

				updateImage(world_image_blink, cropArea, texture_blink);
				world_blink.setTextureRect(sf::IntRect(0, 0, cropArea.width, cropArea.height));
				world_blink.setSize(sf::Vector2f(mapRatio * cropArea.width, mapRatio * cropArea.height));
				blinkCoords = sf::Vector2i(sx, sy);
				//world_blink.setPosition(sf::Vector2f(sx * mapRatio, sy * mapRatio)); //view.getCenter().x, view.getCenter().y

				blinkMap_readyToDraw.store(true);
			}
		}
		std::this_thread::sleep_until(nextTime);
		nextTime += std::chrono::milliseconds(1000 / fps);
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void asyncLoadFlag() {
	auto nextTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1000 / fps);

	while (running.load()) {
		if (scene == 0) {
			if (!flag.empty() && !loadFlag_readyToDraw.load()) {
				if (current_flag != flag) {
					flag_texture.create(383, 256);
					flag_rect.setTexture(&flag_texture);
					flag_texture.update(getFlagImage(flag));

					current_flag = flag;
				}
				loadFlag_readyToDraw.store(true);
			}
		}
		std::this_thread::sleep_until(nextTime);
		nextTime += std::chrono::milliseconds(1000 / fps);
	}
}

void asyncLoadLevelStart() {
	auto nextTime = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(1000 / fps);

	while (running.load()) {
		if (scene == 0) {
			if (!clicking_state.empty() && !loadLevelStart_readyToDraw.load()) {
				loadLevelStart_readyToDraw.store(true);
			}
		}
		std::this_thread::sleep_until(nextTime);
		nextTime += std::chrono::milliseconds(1000 / fps);
	}
}

bool pvzPacketOnSelected = false;
float animSpeed = 3.5f;

inline static const std::unordered_map<int, SpriteFrame> getZombieAnimFrameFromId(int id) {
	static std::unordered_map<int, SpriteFrame> frames[] = 
	{ zombieIdleFrames, zombieIdle1Frames, zombieWalkFrames, zombieEatFrames };
	return frames[id];
}

inline static int getZombieMaxAnimFramesById(int id) {
	static int maxF[] = { 27, 13, 45, 38 };
	return maxF[id];
}

inline static sf::Texture& getZombieTextureById(int id) {
	static sf::Texture texture[] = 
	{ zombieIdleSprites, zombieIdle1Sprites, zombieWalkSprites, zombieEatSprites };
	return texture[id];
}

inline static float getAnimRatioById(int id) {
	return id == 3 ? 1.0f : 1.125f;
}

static void updateZombieAnim() {
	for (auto& zombie : zombiesOnScene) {
		auto& sprite = zombie.anim.sprite;
		++zombie.anim.frameId;

		if (zombie.anim.frameId > getZombieMaxAnimFramesById(zombie.anim.animId) * animSpeed
			* getAnimRatioById(zombie.anim.animId)) zombie.anim.frameId = 0;

		if (&getZombieTextureById(zombie.anim.animId) != zombie.anim.sprite.getTexture()) {
			zombie.anim.sprite.setTexture(getZombieTextureById(zombie.anim.animId));
		}

		sprite.setTextureRect(getZombieAnimFrameFromId(zombie.anim.animId).find(
			static_cast<int>(std::trunc(zombie.anim.frameId / animSpeed
				/ getAnimRatioById(zombie.anim.animId))))->second.frameRect);
	}
}

void asyncPvzSceneUpdate() {
	int pvzScene1moving = 0;
	const int moveAmount = 50;
	const float duration = moveAmount * (1000.0f / fps) * 0.5f;
	float elapsedTimeTotal = 0.0f;
	int pvzStartScene = 0;

	std::vector<float> yCoords(5 + rand() % 6);

	for (auto& y : yCoords) {
		y = static_cast<float>(-350 + (45 + rand() % 11) * (rand() % 15));
	}

	std::sort(yCoords.begin(), yCoords.end());

	{
		std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
		for (const auto& y : yCoords) {
			float x = static_cast<float>(775 + (30 + rand() % 11) * (rand() % 11));
			createZombie(sf::Vector2f(x, y));
		}
	}

	int zombieSpawnTimer = 0;
	int sunSpawnTimer = 0;
	float sunAnimSpeed = 0.75f;

	const auto frameTime = std::chrono::milliseconds(1000 / fps);
	auto nextTime = std::chrono::high_resolution_clock::now() + frameTime;

	while (running.load()) {
		if (scene == 1) {
			switch (pvzScene) {
			default:
			case 0:
			{
				if (audios["lawnbgm"]["6"]->getStatus() != sf::Music::Playing) audios["lawnbgm"]["6"]->play();

				seedChooserButton.setTexture(seedPacketSelected >= std::min(maxPlantSelectAmount,
					getOwnedPlantsAmount()) ? &texture_seedChooser : &texture_seedChooserDisabled);

				float sceneZoom = 50.0f * scene1ZoomSize;

				for (int i = 0; i < maxPlantAmount; ++i) {
					auto it = stateToTargetPosition.find((int)seedPacketState[i][0]);
					if (it != stateToTargetPosition.end()) {
						if (seedPacketState[i][0] == 3 && seedPacketState[i][1] == 0) {
							//sf::Mouse::setPosition(sf::Vector2i(175, 300));
							for (int j = 0; j < maxPlantAmount; ++j) {
								if (i == j) continue;

								sf::RectangleShape& spI = seedPackets[seedPacketIdToString(i)];
								sf::RectangleShape& spJ = seedPackets[seedPacketIdToString(j)];

								if (seedPacketState[j][0] == 2) {
									if (spJ.getPosition().x < spI.getPosition().x) continue;
									seedPacketState[j][0] = 4;
									seedPacketState[j][2] = spJ.getPosition().x - spI.getSize().x; //-570.0f
									seedPacketState[j][3] = spJ.getPosition().y;
								}
								else if (seedPacketState[j][0] == 1) {
									seedPacketState[j][1] = 0;
									seedPacketState[j][0] = 4;
									seedPacketState[j][2] = stateToTargetPosition.find(1)->second.x +
										sceneZoom * seedPacketSelected;
									seedPacketState[j][3] = spI.getPosition().y;
								}
							}
						}
						updatePacketPosition(i, it->second +
							sf::Vector2f(sceneZoom * (seedPacketState[i][0] == 3 ?
								i : (seedPacketSelected - 1)), 0.0f),
							static_cast<int>(frameTime.count()));
					}
					else if (seedPacketState[i][0] == 4) {
						updatePacketPosition(i, sf::Vector2f(seedPacketState[i][2], seedPacketState[i][3]),
							static_cast<int>(frameTime.count()));
					}
				}

				if (!zombiesOnScene.empty()) {
					std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
					updateZombieAnim();
				}
				break;
			}
			case 1: {
				if (audios["lawnbgm"]["6"]->getStatus() == sf::Music::Playing) audios["lawnbgm"]["6"]->stop();

				if (++pvzScene1moving >= moveAmount) {
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

				elapsedTimeTotal += frameTime.count();
				float t = elapsedTimeTotal / duration; //1500ms
				//if (t > 2.0f) t = 2.0f;

				float easedT = easeInOutQuad(t) * static_cast<float>(frameTime.count());

				seedChooser_background.move(0.0f, 2.0f * easedT);
				seedChooserButton.move(0.0f, 2.0f * easedT);
				background.move(0.9f * easedT, 0.0f);

				if (!zombiesOnScene.empty()) {
					std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
					updateZombieAnim();
					for (auto& zombie : zombiesOnScene) {
						zombie.anim.sprite.move(0.9f * easedT, 0.0f);
					}
				}

				break;
			}
			case 2: {
				elapsedTimeTotal += frameTime.count();

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
					pvzStartText.scale(1.0f + frameTime.count() / 1350.0f, 
						1.0f + frameTime.count() / 1350.0f);
				}
				else {
					pvzStartText.setScale(1.5f, 1.5f);
				}

				break;
			}
			case 3:
				if (audios["lawnbgm"]["1"]->getStatus() != sf::Music::Playing) 
					audios["lawnbgm"]["1"]->play();

				if (--zombieSpawnTimer <= 0) {
					zombieSpawnTimer = 1000 + rand() % 500;

					std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
					for (int i = 0; i < 3 + rand() % 5; i++) {
						createRandomZombie();
					}
				}

				if (--sunSpawnTimer <= 0) {
					sunSpawnTimer = 100;//2000 + rand() % 300;

					std::unique_lock<std::shared_mutex> sunWriteLock(sunsMutex);
					createSkySun();
				}

				//if (pvzSun > 9900) pvzSun = 9900;

				if (pvzPacketOnSelected) { //getPlantSpriteById(seedPacketSelectedId)
					sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
					idlePlants[idlePlantToString[seedPacketSelectedId]]
						.setPosition(mousePos + sf::Vector2f(0.0f, 36.0f));
					hoverPlant.setPosition(roundf((mousePos.x + 70.0f) / 140.0f) * 140.0f - 70.0f, 
						roundf((mousePos.y - 30.0f) / 170.0f) * 170.0f + 30.0f);
					hoverShade.setPosition(hoverPlant.getPosition());
				}
				if (!zombiesOnScene.empty()) {
					std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
					updateZombieAnim();

					std::vector<std::size_t> tempRemoveZombies;

					for (std::size_t i = 0; i < zombiesOnScene.size(); ++i) {
						auto& zombie = zombiesOnScene[i];

						if (zombie.anim.sprite.getPosition().x < -700) {
							tempRemoveZombies.push_back(i);
							continue;
						}
						else {
							bool isColliding = false;

							for (auto& plant : plantsOnScene) {
								if (plant.anim.row == zombie.anim.row) {
									sf::FloatRect plantBounds = plant.anim.sprite.getGlobalBounds();
									sf::FloatRect zombieBounds = zombie.anim.sprite.getGlobalBounds();
									if (plantBounds.intersects(zombieBounds)) {
										isColliding = true;
										zombie.targetPlant = &plant;
										break;
									}
								}
							}

							//zombie.eating = isColliding;
							if (isColliding) {
								zombie.anim.animId = 3;
								if (zombie.targetPlant != nullptr &&
									(zombie.anim.frameId == std::trunc(12 * animSpeed
										* getAnimRatioById(zombie.anim.animId)) ||
										zombie.anim.frameId == std::trunc(33 * animSpeed
											* getAnimRatioById(zombie.anim.animId)))) {
									if (damagePlant(*zombie.targetPlant)) {
										std::unique_lock<std::shared_mutex> plantWriteLock(plantsMutex);
										plantsOnScene.erase(std::remove_if(plantsOnScene.begin(),
											plantsOnScene.end(),
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

					zombieReadLock.unlock();
					std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);

					for (auto it = tempRemoveZombies.rbegin(); it != tempRemoveZombies.rend(); ++it) {
						zombiesOnScene.erase(zombiesOnScene.begin() + *it);
					}
				}
				if (!plantsOnScene.empty()) {
					std::shared_lock<std::shared_mutex> plantReadLock(plantsMutex);

					for (auto& plant : plantsOnScene) {
						if (plant.damagedCd > 0) {
							--plant.damagedCd;
						}

						auto& sprite = plant.anim.sprite;
						++plant.anim.frameId;
						if (plant.anim.frameId > getPlantMaxFrameById(plant.anim.animId) * animSpeed) 
							plant.anim.frameId = 0;

						int frame = static_cast<int>(std::trunc(plant.anim.frameId / animSpeed));

						switch (plant.anim.animId) {
						case 0:
							if ((!plant.attack && frame >= 20) || (plant.attack && frame <= 7)) {
								plant.attack = false;

								std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
								for (auto& zombie : zombiesOnScene) {
									sf::FloatRect zb = zombie.anim.sprite.getGlobalBounds();
									sf::FloatRect pb = plant.anim.sprite.getGlobalBounds();
									if (plant.anim.row == zombie.anim.row
										&& zombie.anim.sprite.getPosition().x <= 1300 &&
										zb.left + zb.width > pb.left - pb.width) {
										plant.attack = true;
										break;
									}
								}
							}
							break;
						case 1:
							plant.cd -= static_cast<int>(frameTime.count());
							if (plant.cd <= 0) {
								if (std::fabs(plant.anim.frameId - 17.0f * animSpeed) < 1.0f) {
									plant.cd = 24000 + (std::rand() % 1001);
									std::unique_lock<std::shared_mutex> sunWriteLock(sunsMutex);
									createSun(plant.anim.sprite.getPosition() 
										- sf::Vector2f(0.0f, 50.0f), 0, 2);
								}
							}
							break;
						}

						if (plant.attack) {
							sprite.setTexture(*getPlantAttackTextureById(plant.anim.animId));
							sprite.setTextureRect(getPlantAttackFrameById(plant.anim.animId)->
								find(frame)->second.frameRect);
							if (std::fabs(plant.anim.frameId - 12.0f * animSpeed) < 1.0f)
								createProjectile(0, plant.anim.sprite.getPosition()
									+ sf::Vector2f(72.0f, 0.0f));
						}
						else {
							sprite.setTexture(*getPlantIdleTextureById(plant.anim.animId));
							sprite.setTextureRect(getPlantIdleFrameById(plant.anim.animId)->find(frame)->
								second.frameRect);
						}

						sf::FloatRect bounds = sprite.getGlobalBounds();
						sprite.setOrigin(sprite.getTextureRect().getSize().x / 2.0f,
							sprite.getTextureRect().getSize().y - 36.0f); //fix for 2nd plant
					}
				}

				{
					std::shared_lock<std::shared_mutex> projReadLock(projsMutex);
					auto it = projectilesOnScene.begin();
					while (it != projectilesOnScene.end()) {
						it->sprite.move(15.0f, 0.0f);

						bool shouldEraseProjectile = it->sprite.getPosition().x > 1500;

						if (!shouldEraseProjectile) {
							std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
							auto zIt = zombiesOnScene.begin();
							while (zIt != zombiesOnScene.end()) {
								sf::FloatRect projectileBounds = it->sprite.getGlobalBounds();
								sf::FloatRect zombieBounds = zIt->anim.sprite.getGlobalBounds();

								if (projectileBounds.intersects(zombieBounds) && it->row == zIt->anim.row) {
									if (damageZombie(*it, *zIt)) {
										zombieReadLock.unlock();
										{
											std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
											zIt = zombiesOnScene.erase(zIt);
										}
										zombieReadLock.lock();
									}
									else {
										zIt->damagedCd = 10;
										++zIt;
									}

									{
										std::unique_lock<std::shared_mutex> vanishProjWriteLock(vanishProjsMutex);
										createProjectileVanishAnim(*it);
									}

									shouldEraseProjectile = true;
									break;
								}
								else {
									++zIt;
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
				}

				if (blinkSunText != -1) {
					if (++blinkSunText < 5 || (blinkSunText > 10 && blinkSunText < 15)) {
						pvzSunText.setFillColor(sf::Color(255, 0, 0));
					}
					else {
						pvzSunText.setFillColor(sf::Color(0, 0, 0));
						if (blinkSunText > 20) blinkSunText = -1;
					}
				}

				std::shared_lock<std::shared_mutex> sunReadLock(sunsMutex);
				for (auto it = sunsOnScene.begin(); it != sunsOnScene.end(); ) {
					if (++it->anim.frameId / animSpeed * sunAnimSpeed > 12)
						it->anim.frameId = 0;

					it->anim.sprite.setTextureRect(sunFrames.find(static_cast<int>(
						std::trunc(it->anim.frameId / animSpeed * sunAnimSpeed)))->second.frameRect);

					if (it->anim.sprite.getPosition() != it->targetPos) {
						if (it->style == 0) {
							it->anim.sprite.move(0.0f, 1.0f);

							if (it->anim.sprite.getPosition().y > it->targetPos.y)
								it->anim.sprite.setPosition(it->targetPos);
						}
						else if (it->style == 1 || it->style == 2) {
							float maxSpeed = it->style == 1 ? 50.0f : 5.0f;

							sf::Vector2f currentPos = it->anim.sprite.getPosition();
							sf::Vector2f direction = it->targetPos - currentPos;
							float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);

							if (length != 0.0f) {
								direction.x /= length;
								direction.y /= length;
							}

							float speed = maxSpeed * (length / 500.0f);
							if (speed < 2.5f) speed = 2.5f;

							if (it->style == 2 && it->anim.sprite.getScale() != 
								sf::Vector2f(scene1ZoomSize, scene1ZoomSize)) {
								it->anim.sprite.setScale(it->anim.sprite.getScale()
									+ sf::Vector2f(0.1f, 0.1f));

								if (it->anim.sprite.getScale().x > scene1ZoomSize ||
									it->anim.sprite.getScale().y > scene1ZoomSize) {
									it->anim.sprite.setScale(scene1ZoomSize, scene1ZoomSize);
								}
							} else if (it->anim.sprite.getScale() !=
								sf::Vector2f(scene1ZoomSize, scene1ZoomSize)) {
								it->anim.sprite.setScale(scene1ZoomSize, scene1ZoomSize);
							}

							if (it->style == 1 || (it->anim.sprite.getScale().x > 0.75f))
								it->anim.sprite.move(direction.x * speed, direction.y * speed);

							if (std::abs(currentPos.x - it->targetPos.x) < speed &&
								std::abs(currentPos.y - it->targetPos.y) < speed) {
								if (it->style == 1) {
									addSun(getSunAmountByType(it->type));
									sunReadLock.unlock();
									{
										std::unique_lock<std::shared_mutex> sunWriteLock(sunsMutex);
										it = sunsOnScene.erase(it);
									}
									sunReadLock.lock();
									continue;
								}
								else {
									it->anim.sprite.setPosition(it->targetPos);
								}
							}
						}
					}
					++it;
				}
				break;
			}
		}

		std::this_thread::sleep_until(nextTime);
		nextTime = std::chrono::high_resolution_clock::now() + frameTime;
	}
}

std::thread thread_asyncBlinkMap;
std::thread thread_asyncLoadFlag;
std::thread thread_asyncLoadLevelStart;
std::thread thread_asyncPvzSceneUpdate;

void stopAllThreads() {
	running.store(false);
	if (thread_asyncBlinkMap.joinable()) thread_asyncBlinkMap.join();
	if (thread_asyncLoadFlag.joinable()) thread_asyncLoadFlag.join();
	if (thread_asyncLoadLevelStart.joinable()) thread_asyncLoadLevelStart.join();
	if (thread_asyncPvzSceneUpdate.joinable()) thread_asyncPvzSceneUpdate.join();
	running.store(true);
}