#ifndef SCENE1_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define SCENE1_H

extern int pvzScene;
extern int pvzSun;
extern int seedPacketSelected;
const int maxPlantAmount = 3;
std::string seedPacketIdToString(int id);
extern std::unordered_map<std::string, sf::RectangleShape> seedPackets;
extern std::vector<std::unordered_map<int, float>> seedPacketState;
void updatePacketPosition(size_t i, const sf::Vector2f& targetPosition, int elapsedTime);
extern const std::unordered_map<int, sf::Vector2f> stateToTargetPosition;
extern float easeInOutQuad(float t, float easeRatio = 0.4f, float easeAccMax = 2.5f);
extern float scene1ZoomSize;
void initScene1Place();
void initializeScene1();
struct spriteAnim {
	sf::Sprite sprite;
	int animId;
	int frameId;
	std::optional<int> row;
};
struct plantState {
	spriteAnim anim;
	bool attack;
	int hp;
	int damagedCd;
	int cd;
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
struct sunState {
	spriteAnim anim;
	int type;
	int style;
	sf::Vector2f targetPos;
	int existTime;
};
struct lawnMowerState {
	spriteAnim anim;
	int state;
};
struct particleState {
	spriteAnim anim;
	Emitter emitter;
	sf::Sprite ogSprite;
};
extern std::vector<plantState> plantsOnScene;
extern std::vector<zombieState> zombiesOnScene;
extern std::vector<projectileState> projectilesOnScene;
extern std::vector<vanishProjState> vanishProjectilesOnScene;
extern std::vector<sunState> sunsOnScene;
extern std::vector<lawnMowerState> lawnMowersOnScene;
extern std::vector<particleState> particlesOnScene;
void createPlant(std::optional<sf::Vector2f> pos, int id);
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
extern std::unordered_map<std::string, sf::Sprite> idlePlants;
extern std::array<std::string, maxPlantAmount> idlePlantToString;
sf::Texture* getPlantIdleTextureById(int id);
std::unordered_map<int, SpriteFrame>* getPlantIdleFrameById(int id);
int getPlantMaxFrameById(int id, bool idle = true);
sf::Texture* getPlantAttackTextureById(int id);
std::unordered_map<int, SpriteFrame>* getPlantAttackFrameById(int id);
extern std::unordered_map<int, int> seedPacketsSelectedOrder;
bool plantExist(int id);
int getOwnedPlantsAmount();
extern int maxPlantSelectAmount;
extern bool selectingSeedPacket;
void createSun(sf::Vector2f pos, int sunType, int style);
void createSkySun();
bool selectSun(sf::Vector2f mousePos);
int getSunAmountByType(int id);
extern int blinkSunText;
void addSun(int amount);
extern int maxSun;
extern bool loggingIn;
extern bool shopping;
extern int world;
extern int level;
void unlockPlant(int id);
void unlockPlantByLevel(int vWorld = world, int vLevel = level);
void winLevel();
int getUnlockPlantIdByLevel(int vWorld = world, int vLevel = level);
extern bool isMoneyBag;
int getStartSunByLevel(int vWorld = world, int vLevel = level);
void createLawnMower(float x, float y);
void loseLevel();
extern bool openingMenu;
void openMenu();
void clearPvzVar();
#endif
