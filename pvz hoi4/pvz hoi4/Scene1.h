#ifndef SCENE1_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SCENE1_H

#include <SFML/Graphics.hpp>

extern int pvzScene;
extern int pvzSun;
extern int seedPacketSelected;
const int maxPlantAmount = 1;
extern int maxSeedPacketAmount;
extern std::array<std::string, maxPlantAmount> seedPacketIdToString;
extern std::map<std::string, sf::RectangleShape> seedPackets;
extern std::vector<std::map<int, int>> seedPacketState;
void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime);
extern const std::map<int, sf::Vector2f> stateToTargetPosition;
extern float easeInOutQuad(float t, float easeRatio = 0.4f, float easeAccMax = 2.5f);
void initializeScene1();
struct spriteAnim {
    sf::Sprite sprite;
    int animId;
    int frameId;
};
extern std::vector<spriteAnim> plantsOnScene;
extern std::vector<spriteAnim> zombiesOnScene;
void createPlant(sf::Vector2f pos);
void createZombie(sf::Vector2f pos);
extern bool canPlant(sf::Vector2f pos);
void selectSeedPacket(sf::Vector2f mousePos);
void selectSeedPacket(int id);
#endif
