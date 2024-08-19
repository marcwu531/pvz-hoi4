#ifndef SCENE1_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SCENE1_H

extern int pvzScene;
extern int pvzSun;
extern int seedPacketSelected;
const int maxPlantAmount = 2;
std::string seedPacketIdToString(int id);
extern std::map<std::string, sf::RectangleShape> seedPackets;
extern std::vector<std::map<int, float>> seedPacketState;
void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime);
extern const std::map<int, sf::Vector2f> stateToTargetPosition;
extern float easeInOutQuad(float t, float easeRatio = 0.4f, float easeAccMax = 2.5f);
extern float scene1ZoomSize;
void initializeScene1();
struct spriteAnim {
    sf::Sprite sprite;
    int animId;
    int frameId;
    int row;
};
struct plantState {
    spriteAnim anim;
    bool attack;
    int hp;
    int damagedCd;
};
struct projectileState {
    sf::Sprite sprite;
    int id;
    int row;
};
struct zombieState {
    spriteAnim anim;
    int hp;
    int damagedCd;
    sf::Vector2f movementSpeed;
    plantState* targetPlant;
};
struct vanishProjState {
    projectileState proj;
    int frame;
};
extern std::vector<plantState> plantsOnScene;
extern std::vector<zombieState> zombiesOnScene;
extern std::vector<projectileState> projectilesOnScene;
extern std::vector<vanishProjState> vanishProjectilesOnScene;
void createPlant(sf::Vector2f pos, int id);
void createZombie(sf::Vector2f pos);
extern bool canPlant(sf::Vector2f pos);
void selectSeedPacket(sf::Vector2f mousePos);
void selectSeedPacket(int id);
void createZombie(sf::Vector2f pos, int style);
void createRandomZombie();
void createProjectile(int type, sf::Vector2f pos);
bool damageZombie(projectileState projectile, zombieState& zombie);
bool damagePlant(plantState& plant);
void createProjectileVanishAnim(projectileState proj);
extern int seedPacketSelectedId;
extern std::map<std::string, sf::Sprite> idlePlants;
extern std::array<std::string, maxPlantAmount> idlePlantToString;
sf::Texture* getPlantIdleTextureById(int id);
std::map<int, SpriteFrame>* getPlantIdleFrameById(int id);
int getPlantMaxFrameById(int id);
sf::Texture* getPlantAttackTextureById(int id);
std::map<int, SpriteFrame>* getPlantAttackFrameById(int id);
#endif
