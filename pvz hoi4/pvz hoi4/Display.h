#ifndef DISPLAY_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define DISPLAY_H

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

extern sf::Text pvzSunText;
extern sf::Texture texture_background;
extern sf::RectangleShape background;
extern sf::Texture texture_seedChooser_background;
extern sf::RectangleShape seedChooser_background;
extern sf::Texture texture_seedBank;
extern sf::RectangleShape seedBank;
extern sf::Texture texture_seedPacket_peashooter;
extern sf::RectangleShape seedChooserButton;
extern sf::Texture texture_seedChooser;
extern sf::Texture texture_seedChooserDisabled;
extern sf::RectangleShape pvzStartText;
extern sf::Texture pvzStartText_ready;
extern sf::Texture pvzStartText_set;
extern sf::Texture pvzStartText_plant;
extern sf::RectangleShape overlayShade;
extern sf::Texture peashooterIdleSprites;
extern sf::Sprite peashooterIdle;
extern std::map<int, SpriteFrame> peashooterIdleFrames;
extern sf::Sprite hoverPlant;
extern sf::Sprite hoverShade;
extern sf::Texture zombieIdleSprites;
extern sf::Sprite zombieIdle;
extern std::map<int, SpriteFrame> zombieIdleFrames;

sf::Image loadImageFromResource(HINSTANCE hInstance, UINT resourceID);
sf::Image cropImage(const sf::Image image, const sf::IntRect cropArea);

extern HINSTANCE nullHInstance;
extern sf::Image world_image;
extern std::map<std::string, sf::Image> flagImages;
extern std::map<std::string, std::map<std::string, sf::Image>> pvzImages;
extern sf::Texture texture_blink;
extern float mapRatio;
extern sf::RectangleShape world_blink;
extern sf::RenderWindow window;
extern sf::View view_world;
extern sf::View view_background;
sf::Image getFlagImage(const std::string& country);
sf::Image getPvzImage(const std::string& type, std::string target);
std::vector<char> loadResourceData(HINSTANCE hInstance, int resourceId);
void checkClickingState(float mouseInMapPosX, float mouseInMapPosY);
void changeScene(int targetScene);
#endif
