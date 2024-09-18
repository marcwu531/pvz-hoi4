#ifndef DISPLAY_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define DISPLAY_H

#include <variant>

extern sf::Font defaultFont;

extern sf::Texture flag_texture;
extern sf::RectangleShape flag_rect;
extern std::string current_flag;

struct SpriteFrame {
	sf::IntRect frameRect;
	bool rotated = false;
	bool trimmed = false;
	sf::Vector2i spriteSourceSize;
	sf::Vector2i sourceSize;
};

struct EmitterField {
	std::string fieldType;
	std::string x;
	std::string y;
};

struct Emitter {
    std::string name = "";
    int spawnRate = -1;
    int spawnMinActive = -1;
    int spawnMaxActive = -1;
    int spawnMinLaunched = -1;
    int spawnMaxLaunched = -1;
    std::string launchSpeed = "";
    float launchAngle = -1.0f;
    std::string particleScale = "";
    std::string particleDuration = "";
    std::string particleSpinSpeed = "";
    float particleStretch = -1.0f;
    bool particleLoops = false;
    std::variant<std::string, float> particleAlpha = -1.0f;
    std::variant<std::string, float> particleRed = -1.0f;
    std::variant<std::string, float> particleGreen = -1.0f;
    std::variant<std::string, float> particleBlue = -1.0f;
    float particleBrightness = -1.0f;
    float particleSpinAngle = -1.0f;
    int systemDuration = -1;
    std::string systemAlpha = "";
    bool systemLoops = false;
    float collisionReflect = -1.0f;
    float collisionSpin = -1.0f;
    std::string emitterRadius = "";
    std::string emitterOffset = "";
    bool additive = false;
    std::string image = "";
    int imageFrames = -1;
    int imageRow = -1;
    int randomLaunchSpin = -1;
    EmitterField field;
};

extern sf::Text pvzSunText;
extern sf::Texture texture_background;
extern sf::RectangleShape background;
extern sf::Texture texture_seedChooser_background;
extern sf::RectangleShape seedChooser_background;
extern sf::Texture texture_seedBank;
extern sf::RectangleShape seedBank;
extern sf::RectangleShape seedChooserButton;
extern sf::Texture texture_seedChooser;
extern sf::Texture texture_seedChooserDisabled;
extern sf::RectangleShape pvzStartText;
extern sf::Texture pvzStartText_ready;
extern sf::Texture pvzStartText_set;
extern sf::Texture pvzStartText_plant;
extern sf::RectangleShape overlayShade;
extern sf::Texture peashooterIdleSprites;
extern sf::Texture peashooterShootSprites;
extern std::unordered_map<int, SpriteFrame> peashooterIdleFrames;
extern std::unordered_map<int, SpriteFrame> peashooterShootFrames;
extern sf::Sprite hoverPlant;
extern sf::Sprite hoverShade;
extern sf::Texture zombieIdleSprites;
extern sf::Sprite zombieIdle;
extern std::unordered_map<int, SpriteFrame> zombieIdleFrames;
extern sf::Texture zombieIdle1Sprites;
extern sf::Sprite zombieIdle1;
extern std::unordered_map<int, SpriteFrame> zombieIdle1Frames;
extern sf::Texture peaTexture;
extern sf::Sprite pea;
extern sf::Texture zombieWalkSprites;
extern sf::Sprite zombieWalk;
extern std::unordered_map<int, SpriteFrame> zombieWalkFrames;
extern sf::Sprite peaSplats;
extern std::unordered_map<int, SpriteFrame> peaSplatsFrames;
extern sf::Texture peaSplatsSprites;
extern sf::Sprite zombieEat;
extern std::unordered_map<int, SpriteFrame> zombieEatFrames;
extern sf::Texture zombieEatSprites;
extern sf::Texture sunflowerIdleSprites;
extern sf::Sprite sunflowerIdle;
extern std::unordered_map<int, SpriteFrame> sunflowerIdleFrames;
extern std::unordered_map<int, SpriteFrame> sunFrames;
extern sf::Texture sunSprites;
extern sf::Sprite sun;
extern sf::Texture moneyBagTexture;
extern sf::RectangleShape moneyBag;
extern sf::Sprite lawnMower;
extern sf::Texture lawnMowerTexture;
extern std::unordered_map<int, SpriteFrame> lawnMowerFrames;
extern sf::RectangleShape carKeys;
extern sf::Texture carKeysTexture;
extern sf::Texture carKeysHighlightTexture;
extern sf::RectangleShape storeCar;
extern sf::Texture storeCarTexture;
extern sf::RectangleShape shopLawnMower;
extern sf::RectangleShape zombiesWon;
extern sf::Texture zombiesWonTexture;
extern std::unordered_map<int, SpriteFrame> zombiesWonFrames;
extern sf::RectangleShape zombiesWonDark;
extern sf::RectangleShape optionsMenuback;
extern sf::Texture optionsMenubackTexture;
extern sf::Text menuBackText;
extern sf::Text menuRestartText;
extern sf::Text menuMenuText;
extern sf::Texture cherrybombExplodeSprites;
extern std::unordered_map<int, SpriteFrame> cherrybombExplodeFrames;
extern sf::Texture cherrybombIdleSprites;
extern std::unordered_map<int, SpriteFrame> cherrybombIdleFrames;
extern sf::Texture explosionCloudTexture;
extern sf::Texture explosionPowieTexture;
extern sf::Sprite explosionCloud;
extern sf::Sprite explosionPowie;
extern sf::Sprite zombieFlagWalk;
extern std::unordered_map<int, SpriteFrame> zombieFlagWalkFrames;
extern sf::Texture zombieFlagWalkSprites;

sf::Image loadImageFromResource(HINSTANCE hInstance, UINT resourceID);
sf::Image cropImage(const sf::Image& image, const sf::IntRect& cropArea);

extern HINSTANCE nullHInstance;
extern sf::Image world_image;
extern std::unordered_map<std::string, sf::Image> flagImages;
extern std::unordered_map<std::string, std::unordered_map<std::string, sf::Image>> pvzImages;
extern sf::Texture texture_blink;
extern float mapRatio;
extern sf::RectangleShape world_blink;
extern sf::RenderWindow window;
extern sf::View view_world;
extern sf::View view_background;
sf::Image getFlagImage(const std::string& country);
sf::Image getPvzImage(const std::string& type, std::string target);
std::vector<char> loadResourceData(HINSTANCE hInstance, int resourceId);
std::string checkClickingState(float mouseInMapPosX, float mouseInMapPosY);
void changeScene(int targetScene);
extern int zombiesWonFrameId;
void spawnParticle(int id, sf::Vector2f pos);
void initParticle();
const int maxParticleAmount = 1;
float getParticalInitialFloat(const std::variant<std::string, float>& var);
std::optional<std::array<float, 2>> getParticleFloatAsArray(const std::variant<std::string, float>& var);
sf::Uint8 clampColor(float value);
#endif
