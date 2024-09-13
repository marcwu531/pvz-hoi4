#include <array>
#include <chrono>
#include <functional>
#include <iostream>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <shared_mutex>
#include <string>
#include <thread>
#include <windows.h>

#include "Account.h"
#include "Async.h"
#include "Audio.h"
#include "Colour.h"
#include "Display.h"
#include "General.h"
#include "Level.h"
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

std::shared_mutex plantsMutex, zombiesMutex, projsMutex, vanishProjsMutex, sunsMutex, mapMutex,
lawnMowersMutex, particlesMutex; //accountMutex

inline static void updateImage(sf::Image& image, const sf::IntRect& cropArea, sf::Texture& texture) {
	if (texture.getSize().x != static_cast<unsigned int>(cropArea.width) ||
		texture.getSize().y != static_cast<unsigned int>(cropArea.height)) {
		texture.create(cropArea.width, cropArea.height);
	}
	texture.update(image);
}

sf::Vector2f worldPos;
int mapSx, mapSy;

void asyncBlinkMap() {
	auto nextTime = std::chrono::high_resolution_clock::now() +
		std::chrono::milliseconds(1000 / fps);

	while (running.load()) {
		if (scene == 0) {
			if (!targetCoords.empty() && !blinkMap_readyToDraw.load() &&
				!clicking_state.empty() && !blinkMap_loadingCoords.load()) {

				int sx = state_int[clicking_state]["sx"](), sy = state_int[clicking_state]["sy"](),
					lx = state_int[clicking_state]["lx"](), ly = state_int[clicking_state]["ly"]();

				sf::IntRect cropArea(sx, sy, lx - sx + 1, ly - sy + 1);

				sf::Image world_image_blink = cropImage(world_image, cropArea);

				world_image_blink = pixelsToBlink(targetCoords, world_image_blink, cropArea);

				std::unique_lock<std::shared_mutex> mapWriteLock(mapMutex);
				mapSx = sx;
				mapSy = sy;

				updateImage(world_image_blink, cropArea, texture_blink);
				world_blink.setTextureRect(sf::IntRect(0, 0, cropArea.width, cropArea.height));
				world_blink.setSize(sf::Vector2f(mapRatio * cropArea.width, mapRatio * cropArea.height));
				//world_blink.setPosition(sf::Vector2f(sx * mapRatio, sy * mapRatio) + worldPos);

				blinkMap_readyToDraw.store(true);
			}
		}
		std::this_thread::sleep_until(nextTime);
		nextTime += std::chrono::milliseconds(1000 / fps);
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

int idleFrame = 0;

void asyncPvzSceneUpdate() {
	pvzScene = 0;

	clearPvzVar();
	randomRNG();
	initScene1Place();
	pvzSun = getStartSunByLevel();

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

	int zombieSpawnTimer = 1500 + rand() % 500;
	int sunSpawnTimer = 1000 + rand() % 300;
	float sunAnimSpeed = 0.75f;
	int currentWave = 0;

	const auto frameTime = std::chrono::milliseconds(1000 / fps);
	auto nextTime = std::chrono::high_resolution_clock::now() + frameTime;

	while (running.load()) {
		if (scene == 1 && !openingMenu) {
			switch (pvzScene) {
			default:
			case 0:
			{
				if (audios["lawnbgm"]["6"]->getStatus() != sf::Music::Playing) audios["lawnbgm"]["6"]->play();

				seedChooserButton.setTexture(seedPacketSelected >= std::min(maxPlantSelectAmount,
					getOwnedPlantsAmount()) ? &texture_seedChooser : &texture_seedChooserDisabled);

				float sceneZoom = 50.0f * scene1ZoomSize;

				for (int i = 0; i < maxPlantAmount; ++i) {
					if (!plantExist(i)) continue;
					auto it = stateToTargetPosition.find(static_cast<int>(seedPacketState[i][0]));
					if (it != stateToTargetPosition.end()) {
						if (seedPacketState[i][0] == 3 && seedPacketState[i][1] == 0) {
							//sf::Mouse::setPosition(sf::Vector2i(175, 300));
							for (int j = 0; j < maxPlantAmount; ++j) {
								if (!plantExist(j)) continue;
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

					{
						std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
						zombiesOnScene.clear();
					}

					audios["sounds"]["readysetplant"]->play();

					if (account.unlockedLawnMower) {
						int lmAmount = 0;

						std::unique_lock<std::shared_mutex> lawnMowerWriteLock(lawnMowersMutex);
						while (++lmAmount <= 5) {
							createLawnMower(-360.0f, -480.0f + 170.0f * lmAmount);
						}
					}
					break;
				}

				elapsedTimeTotal += frameTime.count();
				float t = elapsedTimeTotal / duration; //1500ms
				//if (t > 2.0f) t = 2.0f;

				float easedT = easeInOutQuad(t) * static_cast<float>(frameTime.count());

				seedChooser_background.move(0.0f, 2.0f * easedT);
				seedChooserButton.move(0.0f, 2.0f * easedT);
				background.move(0.9f * easedT, 0.0f);

				{
					std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
					if (!zombiesOnScene.empty()) {
						updateZombieAnim();
						for (auto& zombie : zombiesOnScene) {
							zombie.anim.sprite.move(0.9f * easedT, 0.0f);
						}
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
			{
				if (audios["lawnbgm"]["1"]->getStatus() != sf::Music::Playing)
					audios["lawnbgm"]["1"]->play();

				{
					{
						std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
						if (zombiesOnScene.empty() && zombieSpawnTimer > 100) {
							zombieSpawnTimer = 100;
						}
					}

					if (currentWave < world_level_waves[world][level] + 1) {
						if (--zombieSpawnTimer <= 0) {
							zombieSpawnTimer = 2000 + rand() % 500;

							if (++currentWave == world_level_waves[world][level]) {
								audios["sounds"]["finalwave"]->play();
							}
							else {
								int spawnTier = static_cast<int>( std::log10(currentWave + 1) * 10.0f * 
									std::pow(1 + world_level_spawnTier[world][level] / 25.0f, 3)) - 2;

								if (currentWave == world_level_waves[world][level] + 1) {
									audios["sounds"]["siren"]->play();
									if (world == 1 && level == 1)
										spawnTier *= 3;
								}
								else if (currentWave == 1) {
									audios["sounds"]["awooga"]->play();
								}

								{
									std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);

									while (spawnTier > 0) {
										--spawnTier;
										createRandomZombie();
									}
								}
							}
						}
					}
					else {
						std::shared_lock<std::shared_mutex> zombieReadLock(zombiesMutex);
						if (zombiesOnScene.empty()) winLevel();
					}
				}

				if (--sunSpawnTimer <= 0) {
					sunSpawnTimer = 2000 + rand() % 300;

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

				if (blinkSunText != -1) {
					if (++blinkSunText < 5 || (blinkSunText > 10 && blinkSunText < 15)) {
						pvzSunText.setFillColor(sf::Color(255, 0, 0));
					}
					else {
						pvzSunText.setFillColor(sf::Color(0, 0, 0));
						if (blinkSunText > 20) blinkSunText = -1;
					}
				}
			}
			[[fallthrough]];
			case 4:
			case 5:
			case 8:
			{
				if (pvzScene == 8) {
					if (zombiesWonFrameId < 11 * animSpeed) {
						zombiesWon.setTextureRect(zombiesWonFrames.find(static_cast<int>(
							std::trunc(++zombiesWonFrameId / animSpeed)))->second.frameRect);
					}
					else {
						pvzScene = 9;
						openMenu();
						//zombiesWonDark.setFillColor(sf::Color(0, 0, 0, 255));
					}
				}

				{
					std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
					if (!zombiesOnScene.empty()) {
						updateZombieAnim();

						for (auto it = zombiesOnScene.begin(); it != zombiesOnScene.end(); ) {
							auto& zombie = *it;

							if (zombie.anim.sprite.getPosition().x < -531 && pvzScene == 3) {
								//zombie.hp = 0;
								loseLevel();
							}
							else {
								bool isColliding = false;

								{
									std::shared_lock<std::shared_mutex> plantReadLock(plantsMutex);
									for (auto itPlant = plantsOnScene.rbegin(); itPlant != plantsOnScene.rend(); ++itPlant) {
										auto& plant = *itPlant;
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
								}

								if (isColliding) {
									zombie.anim.animId = 3;
									if (zombie.targetPlant != nullptr &&
										(zombie.anim.frameId == std::trunc(12 * animSpeed * getAnimRatioById(zombie.anim.animId)) ||
											zombie.anim.frameId == std::trunc(33 * animSpeed * getAnimRatioById(zombie.anim.animId)))) {
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
									zombie.anim.sprite.move(zombie.movementSpeed); //(-50.0f, 0.0f);

									if (zombie.anim.sprite.getPosition().x < -360.0f) {
										std::shared_lock<std::shared_mutex> lawnMowerReadLock(lawnMowersMutex);

										for (auto& lm : lawnMowersOnScene) {
											if (lm.state == 0 && lm.anim.row == zombie.anim.row) {
												lm.state = 1;
											}
										}
									}

									zombie.targetPlant = nullptr;
								}
							}

							if (zombie.damagedCd > 0) {
								--zombie.damagedCd;
							}

							if (zombie.hp <= 0) {
								it = zombiesOnScene.erase(it);
							}
							else {
								++it;
							}
						}
					}
				}

				{
					std::shared_lock<std::shared_mutex> plantReadLock(plantsMutex);
					if (!plantsOnScene.empty()) {
						for (auto it = plantsOnScene.begin(); it != plantsOnScene.end();) {
							auto& plant = *it;

							if (plant.damagedCd > 0) {
								--plant.damagedCd;
							}

							auto& sprite = plant.anim.sprite;
							++plant.anim.frameId;
							if (plant.anim.frameId > getPlantMaxFrameById(plant.anim.animId, !plant.attack) * animSpeed)
								plant.anim.frameId = 0;

							int frame = static_cast<int>(std::trunc(plant.anim.frameId / animSpeed));

							if (pvzScene == 3) {
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
								case 2:
									plant.attack = true;
									break;
								}
							}

							if (plant.attack && pvzScene == 3) {
								sprite.setTexture(*getPlantAttackTextureById(plant.anim.animId));
								sprite.setTextureRect(getPlantAttackFrameById(plant.anim.animId)->
									find(frame)->second.frameRect);

								switch (plant.anim.animId) {
								case 0:
									if (std::fabs(plant.anim.frameId - 12.0f * animSpeed) < 1.0f) {
										std::unique_lock<std::shared_mutex> projWriteLock(projsMutex);
										createProjectile(0, plant.anim.sprite.getPosition()
											+ sf::Vector2f(72.0f, 0.0f));
									}
									break;
								case 2:
									if (plant.anim.frameId == 0) {
										{
											std::unique_lock<std::shared_mutex> particleWriteLock(particlesMutex);
											spawnParticle(0, plant.anim.sprite.getPosition() + 
												plant.anim.sprite.getOrigin());
										}

										plantReadLock.unlock();
										{
											std::unique_lock<std::shared_mutex> plantWriteLock(plantsMutex);
											it = plantsOnScene.erase(it);
										}
										plantReadLock.lock();
										continue;
									}
								}
							}
							else {
								sprite.setTexture(*getPlantIdleTextureById(plant.anim.animId));
								sprite.setTextureRect(getPlantIdleFrameById(plant.anim.animId)->find(frame)->
									second.frameRect);
							}

							sf::FloatRect bounds = sprite.getGlobalBounds();
							sprite.setOrigin(sprite.getTextureRect().getSize().x / 2.0f,
								sprite.getTextureRect().getSize().y - 36.0f); //fix for 2nd plant

							++it;
						}
					}
				}

				{
					std::shared_lock<std::shared_mutex> projReadLock(projsMutex);
					auto it = projectilesOnScene.begin();
					while (it != projectilesOnScene.end()) {
						it->sprite.move(15.0f, 0.0f);

						bool shouldEraseProjectile = it->sprite.getPosition().x > 1500;

						if (!shouldEraseProjectile && pvzScene == 3) {
							std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);
							auto zIt = zombiesOnScene.begin();
							while (zIt != zombiesOnScene.end()) {
								sf::FloatRect projectileBounds = it->sprite.getGlobalBounds();
								sf::FloatRect zombieBounds = zIt->anim.sprite.getGlobalBounds();

								if (projectileBounds.intersects(zombieBounds) && it->row == zIt->anim.row) {
									playRngAudio("splat");
									if (damageZombie(*it, *zIt)) {
										zIt = zombiesOnScene.erase(zIt);
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
							projReadLock.unlock();
							{
								std::unique_lock<std::shared_mutex> projWriteLock(projsMutex);
								it = projectilesOnScene.erase(it);
							}
							projReadLock.lock();
						}
						else {
							++it;
						}
					}
				}

				{
					std::shared_lock<std::shared_mutex> sunReadLock(sunsMutex);
					for (auto it = sunsOnScene.begin(); it != sunsOnScene.end();) {
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
								}
								else if (it->anim.sprite.getScale() !=
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
				}

				{
					std::shared_lock<std::shared_mutex> lawnMowerReadLock(lawnMowersMutex);

					for (auto itLm = lawnMowersOnScene.begin(); itLm != lawnMowersOnScene.end();) {
						if (itLm->state == 1) {
							itLm->anim.sprite.move(10.0f, 0.0f);
							if (itLm->anim.sprite.getPosition().x > 1500) {
								lawnMowerReadLock.unlock();
								{
									std::unique_lock<std::shared_mutex> lawnMowerWriteLock(lawnMowersMutex);
									itLm = lawnMowersOnScene.erase(itLm);
								}
								lawnMowerReadLock.lock();
							}
							else {
								{
									std::unique_lock<std::shared_mutex> zombieWriteLock(zombiesMutex);

									for (auto itZ = zombiesOnScene.begin(); itZ != zombiesOnScene.end();) {
										if (itZ->anim.row == itLm->anim.row) {
											sf::FloatRect itZBounds = itZ->anim.sprite.getGlobalBounds();
											sf::FloatRect itLmBounds = itLm->anim.sprite.getGlobalBounds();

											if ((itZBounds.left < itLmBounds.left + itLmBounds.width) &&
												(itZBounds.left + itZBounds.width > itLmBounds.left)) {
												itZ = zombiesOnScene.erase(itZ);
												continue;
											}
										}
										++itZ;
									}
								}

								++itLm->anim.frameId;
								if (itLm->anim.frameId > 11.0f * animSpeed) itLm->anim.frameId = 0;
								itLm->anim.sprite.setTextureRect(lawnMowerFrames.find(
									static_cast<int>(std::trunc(itLm->anim.frameId / animSpeed)))->second.frameRect);

								++itLm;
							}
						}
						else {
							++itLm;
						}
					}
				}

				{
					std::shared_lock<std::shared_mutex> particleReadLock(particlesMutex);

					for (auto it = particlesOnScene.begin(); it != particlesOnScene.end();) {
						float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f, scale = 0;

						std::optional<std::array<float, 2>> particleRedArray =
							getParticleFloatAsArray(it->emitter.particleRed);
						std::optional<std::array<float, 2>> particleGreenArray =
							getParticleFloatAsArray(it->emitter.particleGreen);
						std::optional<std::array<float, 2>> particleBlueArray =
							getParticleFloatAsArray(it->emitter.particleBlue);
						std::optional<std::array<float, 2>> particleAlphaArray =
							getParticleFloatAsArray(it->emitter.particleAlpha);
						std::optional<std::array<float, 2>> particleScaleArray =
							getParticleFloatAsArray(it->emitter.particleScale);

						if (particleRedArray.has_value() || particleGreenArray.has_value() ||
							particleBlueArray.has_value() || particleAlphaArray.has_value()) {
							if (particleRedArray.has_value()) {
								r = it->ogSprite.getColor().r;
								r += it->anim.frameId * (particleRedArray.value()[1] - r) /
									particleRedArray.value()[0];
							}
							else {
								r = it->ogSprite.getColor().r;
							}
							if (particleGreenArray.has_value()) {
								g = it->ogSprite.getColor().g;
								g += it->anim.frameId * (particleGreenArray.value()[1] - g) /
									particleGreenArray.value()[0];
							}
							else {
								g = it->ogSprite.getColor().g;
							}
							if (particleBlueArray.has_value()) {
								b = it->ogSprite.getColor().b;
								b += it->anim.frameId * (particleBlueArray.value()[1] - b) /
									particleBlueArray.value()[0];
							}
							else {
								b = it->ogSprite.getColor().b;
							}
							if (particleAlphaArray.has_value()) {
								a = it->ogSprite.getColor().a;
								a += it->anim.frameId * (particleAlphaArray.value()[1] - a) /
									particleAlphaArray.value()[0];
							}
							else {
								a = it->ogSprite.getColor().a;
							}
							it->anim.sprite.setColor(sf::Color(clampColor(r), clampColor(g),
								clampColor(b), clampColor(a)));
						}

						if (particleScaleArray.has_value()) {
							scale = it->ogSprite.getScale().x;
							scale += std::min(static_cast<float>(it->anim.frameId), particleScaleArray.value()[0])
								* (particleScaleArray.value()[1] - scale) / particleScaleArray.value()[0];
							scale *= scene1ZoomSize;
							it->anim.sprite.setScale(scale, scale);
						}

						if (++it->anim.frameId > it->emitter.systemDuration) {
							particleReadLock.unlock();
							{
								std::unique_lock<std::shared_mutex> particleWriteLock(particlesMutex);
								it = particlesOnScene.erase(it);
							}
							particleReadLock.lock();
						}
						else {
							++it;
						}
					}
				}

				if (pvzScene == 5) {
					sf::RectangleShape& targetAward = isMoneyBag ?
						moneyBag :
						seedPackets[seedPacketIdToString(getUnlockPlantIdByLevel())];

					float remainingAlphaSteps = static_cast<float>(255 - winLevelScreen.getFillColor().a);
					sf::Vector2f destPlace(view_background.getCenter() -
						sf::Vector2f(0.0f, 0.2f * view_background.getSize().y));

					if (remainingAlphaSteps > 0) {
						sf::Vector2f direction = destPlace - targetAward.getPosition()
							;
						targetAward.move(direction / remainingAlphaSteps);
					}
					else {
						targetAward.setPosition(destPlace);
					}

					targetAward.scale(1.005f, 1.005f);

					winLevelScreen.setFillColor(sf::Color(255, 255, 255,
						std::min(winLevelScreen.getFillColor().a + 2, 255)));

					if (!isMoneyBag) seedPackets[seedPacketIdToString(getUnlockPlantIdByLevel())].setFillColor(
						sf::Color(255, 255, 255, 255 - winLevelScreen.getFillColor().a));

					if (winLevelScreen.getFillColor().a == 255) {
						targetAward.setPosition(destPlace);
						seedChooserButton.setTexture(&texture_seedChooser);
						seedChooserButton.setPosition(view_background.getCenter()
							+ sf::Vector2f(-seedChooserButton.getSize().x / 2.0f,
								view_background.getSize().y * 0.35f));
						pvzScene = 6;
					}
				}
				break;
			}
			case 6:
				winLevelScreen.setFillColor(sf::Color(255, 255, 255,
					std::max(winLevelScreen.getFillColor().a - 2, 0)));

				if (winLevelScreen.getFillColor().a == 0) {
					pvzScene = 7;
					clearPvzVar();
				}
				[[fallthrough]];
			case 7:
				if (!isMoneyBag) {
					if (++idleFrame > getPlantMaxFrameById(getUnlockPlantIdByLevel()) * animSpeed) idleFrame = 0;
					idlePlants[idlePlantToString[getUnlockPlantIdByLevel()]].setTextureRect(
						getPlantAttackFrameById(getUnlockPlantIdByLevel())->
						find(static_cast<int>(std::trunc(idleFrame / animSpeed)))->second.frameRect);
				}
				break;
			}
		}

		std::this_thread::sleep_until(nextTime);
		nextTime = std::chrono::high_resolution_clock::now() + frameTime;
	}
}

void randomRNG() {
	srand(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));
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