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

sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close); //(1920, 1046)
sf::View view_world(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));
sf::View view_background(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));

int fps = 60;
int scene = 0;

std::atomic<bool> running(true);
std::atomic<bool> blinkMap_loadingCoords(false);
std::atomic<bool> blinkMap_readyToDraw(false);
std::atomic<bool> loadFlag_readyToDraw(false);
std::atomic<bool> loadLevelStart_readyToDraw(false);

std::vector<std::array<int, 2>> targetCoords;
//std::vector<std::array<int, 2>> nullVector = { {NULL, NULL} };
float mapRatio = 20.0f;
sf::RectangleShape world_blink; //(sf::Vector2f(mapRatio* (State::T::lx - State::T::sx + 1), mapRatio* (State::T::ly - State::T::sy + 1))); //15s
//std::atomic<int> blinkCd(0); //int blinkCd = 0;
sf::Texture texture_blink;

HINSTANCE nullHInstance = GetModuleHandle(NULL);

sf::Image world_image = loadImageFromResource(nullHInstance, 101);

std::map<std::string, sf::Image> flagImages = {
    {"Taiwan", loadImageFromResource(nullHInstance, 102)}
};

std::map<std::string, std::map<std::string, sf::Image>> pvzImages = {
    {"background", {
        {"bg1", loadImageFromResource(nullHInstance, 103)}
    }},
    {"seed_selector", {
        {"seedChooser_background", loadImageFromResource(nullHInstance, 104)},
        {"seedBank", loadImageFromResource(nullHInstance, 105)},
        {"seedChooserDisabled", loadImageFromResource(nullHInstance, 107)},
        {"seedChooserButton", loadImageFromResource(nullHInstance, 108)},
        {"startReady", loadImageFromResource(nullHInstance, 109)},
        {"startSet", loadImageFromResource(nullHInstance, 110)},
        {"startPlant", loadImageFromResource(nullHInstance, 111)}
    }},
    {"seed_packet", {
        {"peashooter", loadImageFromResource(nullHInstance, 106)}
    }}
};

sf::Image getFlagImage(std::string country) {
    return flagImages.at(country);
}

sf::Image getPvzImage(std::string type, std::string target) {
    return pvzImages.at(type).at(target);
}

sf::Music chooseYourSeeds;
sf::Music grasswalk;
sf::Music readysetplant;
sf::Music battleofwuhan;

sf::Font defaultFont;

sf::Texture flag_texture;
sf::RectangleShape flag_rect;
std::string current_flag;

sf::Text pvzSunText("", defaultFont, 50);
sf::Texture texture_background;
sf::RectangleShape background;
sf::Texture texture_seedChooser_background;
sf::RectangleShape seedChooser_background;
sf::Texture texture_seedBank;
sf::RectangleShape seedBank;
sf::Texture texture_seedPacket_peashooter;
sf::RectangleShape seedChooserButton;
sf::Texture texture_seedChooser;
sf::Texture texture_seedChooserDisabled;
sf::RectangleShape pvzStartText;
sf::Texture pvzStartText_ready;
sf::Texture pvzStartText_set;
sf::Texture pvzStartText_plant;
sf::RectangleShape overlayShape;

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

void checkClickingState(float mouseInMapPosX, float mouseInMapPosY) {
    std::array<std::string, 2> target = clickingState(world_image, mouseInMapPosX, mouseInMapPosY);
    std::string targetState = target[1];

    //std::cout << targetState << std::endl;
    if (!targetState.empty() && clicking_state != targetState) {
        blinkMap_loadingCoords.store(true);
        clicking_state = targetState;
        targetCoords.clear();

        //std::cout << "sx: " << state_int[clicking_state]["sx"]() << std::endl;
        //std::cout << "lx: " << state_int[clicking_state]["lx"]() << std::endl;

        for (int x = state_int[clicking_state]["sx"](); x <= state_int[clicking_state]["lx"](); ++x) {
            for (int y = state_int[clicking_state]["sy"](); y <= state_int[clicking_state]["ly"](); ++y) {
                if (getRGBA(world_image, x, y) == state_rgba[clicking_state]["RGBA"]()) {
                    targetCoords.push_back({ x, y });
                }
            }
        }

        if (flag != target[0]) {
            flag = target[0];
        }
        blinkMap_loadingCoords.store(false);
    }
}

const int maxPlantAmount = 1;
int maxSeedPacketAmount = 1;
std::array<std::string, maxPlantAmount> seedPacketIdToString = { "seedPacket_peashooter" };
std::map<std::string, sf::RectangleShape> seedPackets = {
    {seedPacketIdToString[0], sf::RectangleShape()}
};
std::vector<std::map<int, int>> seedPacketState(maxPlantAmount); //state, state1 moving time
int seedPacketSelected = 0;
int pvzSun = 150;

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
    {1, sf::Vector2f(-570.0f, -480.0f)},
    {3, sf::Vector2f(-680.0f, -290.0f)}
};

int pvzScene = 0;

float easeInOutQuad(float t, float easeRatio = 0.4f, float easeAccMax = 2.5f) {
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

void asyncPvzSceneUpdate() {
    int pvzScene1moving = 0;
    auto lastTime = std::chrono::high_resolution_clock::now();

    const int moveAmount = 50;
    const float duration = moveAmount * (1000.0f / fps) * 0.5f;
    float elapsedTimeTotal = 0.0f;
    int pvzStartScene = 0;

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= static_cast<unsigned int>(1000 / fps) && scene == 1) {
            pvzSunText.setString(std::to_string(pvzSun));
            sf::FloatRect pvzSunTextRect = pvzSunText.getLocalBounds();
            pvzSunText.setOrigin(pvzSunTextRect.left + pvzSunTextRect.width / 2.0f,
                pvzSunTextRect.top + pvzSunTextRect.height / 2.0f);
            switch (pvzScene) {
            default:
            case 0:
                if (chooseYourSeeds.getStatus() != sf::Music::Playing) chooseYourSeeds.play();

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
                break;
            case 1: {
                if (chooseYourSeeds.getStatus() == sf::Music::Playing) chooseYourSeeds.stop();

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
                    readysetplant.play();
                    break;
                }

                elapsedTimeTotal += elapsedTime.count();
                float t = elapsedTimeTotal / duration;
                //if (t > 2.0f) t = 2.0f;

                float easedT = easeInOutQuad(t) * static_cast<float>(elapsedTime.count());

                seedChooser_background.move(0.0f, 2.0f * easedT);
                seedChooserButton.move(0.0f, 2.0f * easedT);
                background.move(0.9f * easedT, 0.0f);

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
                if (grasswalk.getStatus() != sf::Music::Playing) grasswalk.play();
                break;
            }
            
            lastTime = currentTime;
        }
    }
}

void initializeScene1() {
    float zoomSize = 1.7f;

    texture_background.loadFromImage(getPvzImage("background", "bg1"));
    float bgCamSizeY = view_background.getSize().y;
    background.setSize(sf::Vector2f(1400.0f / 600.0f * bgCamSizeY, bgCamSizeY)); //1920.0f, 1046.0f -> bg png size 1400 x 600
    background.setTexture(&texture_background);
    view_background.move((background.getSize().x - view_background.getSize().x) / 2.0f, 30.0f);

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
    pvzSunText.setPosition(-633.5f, -370.0f);

    overlayShape.setSize(sf::Vector2f(50.0f * zoomSize, 70.0f * zoomSize));
    overlayShape.setFillColor(sf::Color(0, 0, 0, 200));

    background.setOrigin(background.getSize() / 2.0f);
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

void changeScene(int targetScene) {
    stopAllThreads();
    switch (scene) {
    case 0:
        if (battleofwuhan.getStatus() == sf::Music::Playing) battleofwuhan.stop();
        thread_asyncPacketMove = std::thread(asyncPvzSceneUpdate);
    default:
        scene = targetScene;
        break;
    }
}

std::vector<char> loadResourceData(HINSTANCE hInstance, int resourceId) {
    HRSRC hResource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!hResource) {
        throw std::runtime_error("Failed to find resource!");
    }

    HGLOBAL hLoadedResource = LoadResource(hInstance, hResource);
    if (!hLoadedResource) {
        throw std::runtime_error("Failed to load resource!");
    }

    void* pResourceData = LockResource(hLoadedResource);
    DWORD resourceSize = SizeofResource(hInstance, hResource);
    if (!pResourceData || resourceSize == 0) {
        throw std::runtime_error("Failed to lock resource or resource size is zero!");
    }

    return std::vector<char>(static_cast<char*>(pResourceData), static_cast<char*>(pResourceData) + resourceSize);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) { //int main() {   
    std::vector<char> fontData = loadResourceData(nullHInstance, 4);
    defaultFont.loadFromMemory(fontData.data(), fontData.size());
    
    initializeScene1();

    if (!chooseYourSeeds.openFromFile("audio/lawnbgm/lawnbgm(6).mp3")) {
        return -1;
    }
    if (!grasswalk.openFromFile("audio/lawnbgm/lawnbgm(1).mp3")) {
        return -1;
    }
    if (!readysetplant.openFromFile("audio/sounds/readysetplant.ogg")) {
        return -1;
    }
    if (!battleofwuhan.openFromFile("audio/battleofwuhan.ogg")) {
        return -1;
    }
    battleofwuhan.setVolume(25);

    sf::RectangleShape world(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
    //window.create(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close);
    //world.setOrigin(93000.0f, 19500.0f);
    sf::Texture texture_world;
    //world_image.loadFromFile("world_images/world.png");
    texture_world.loadFromImage(world_image); //sf::IntRect(4555, 920, 200, 200)
    world.setTexture(&texture_world);
    view_world.setCenter(sf::Vector2f(93000.0f, 19000.0f)); //94000.0f, 20000.0f
    window.setFramerateLimit(fps);

    float tx = 0.0f;
    float ty = 0.0f;

    thread_asyncBlinkMap = std::thread(asyncBlinkMap);
    thread_asyncLoadFlag = std::thread(asyncLoadFlag);
    thread_asyncLoadLevelStart = std::thread(asyncLoadLevelStart);

    sf::RectangleShape levelStart(sf::Vector2f(view_world.getSize().x / 2.0f, view_world.getSize().y));
    levelStart.setFillColor(sf::Color::White);

    sf::Text levelStartText("START", defaultFont, 50);
    levelStartText.setFillColor(sf::Color::Black);

    sf::RectangleShape levelStartButton(sf::Vector2f(view_world.getSize().x / 20.0f, view_world.getSize().y / 10.0f));
    levelStartButton.setFillColor(sf::Color::Green);

    bool leftClicking = false;

    std::queue<sf::Keyboard::Key> inputs;

    bool pvzDrawPacketShade = false;

    while (window.isOpen())
    {
        sf::Event e;

        while (window.pollEvent(e))
        {
            switch (scene) {
            default:
            case 0:
                switch (e.type) {
                case sf::Event::MouseWheelScrolled:
                    zoomViewAt({ e.mouseWheelScroll.x, e.mouseWheelScroll.y }, window, std::pow(1.1f, -e.mouseWheelScroll.delta), view_world);
                    view_world = window.getView();
                    //std::cout << view.getCenter().x << " " << view.getCenter().y << std::endl;
                    break;
                }
            case 1:
                switch (e.type) {
                case sf::Event::KeyPressed:
                    inputs.push(e.key.code);
                    if (inputs.size() > konamiCode.size()) {
                        inputs.pop();
                    }
                    if (isKonamiCodeEntered(inputs)) {
                        ShellExecute(0, 0, L"https://tinyurl.com/marcwu531underphaith706", 0, 0, SW_SHOW);
                        while (!inputs.empty()) {
                            inputs.pop();
                        }
                    }

                    if (e.key.code != sf::Keyboard::Escape) break;
                case sf::Event::Closed:
                    changeScene(-1);
                    window.close();
                    break;
                    /*case sf::Event::Resized:
                        std::cout << "width:" << e.size.width << " height:" << e.size.height << std::endl;
                        break;*/
                        /*case sf::Event::TextEntered:
                            printf("%c", e.text.unicode);
                            break;*/
                case sf::Event::MouseButtonReleased:
                    //std::cout << "A" << std::endl;
                    if (e.mouseButton.button == sf::Mouse::Left && leftClicking) {
                        leftClicking = false;
                    }
                    break;
                }
                break;
            }
        }

        switch (scene) {
        default:
        case 0: {
            if (battleofwuhan.getStatus() != sf::Music::Playing) battleofwuhan.play();
            //view = window.getView();
            int dtx = 0;
            int dty = 0;

            if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) &&
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dtx = 1;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dtx = -1;
            }
            if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))) {
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dty = 1;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dty = -1;
            }

            tx += dtx * std::max(std::min(tx, -80.0f), 80.0f);
            tx *= 0.9f;
            float dx = 0.02f * tx * view_world.getSize().x / 1920.0f;
            ty += dty * std::max(std::min(ty, -80.0f), 80.0f);
            ty *= 0.9f;
            float dy = 0.02f * ty * view_world.getSize().y / 1046.0f;

            //view.move(sf::Vector2f(dx, dy));
            world.move(sf::Vector2f(dx, dy));
            //world_blink.move(sf::Vector2f(dx, dy));

            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
                leftClicking = true;
                /*std::cout << "x: " << window.mapPixelToCoords(sf::Mouse::getPosition(window)).x
                    << " y: " << window.mapPixelToCoords(sf::Mouse::getPosition(window)).y << std::endl;*/
                    /*std::cout << getRGBA(texture_world,
                        window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio,
                        window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio)
                        << std::endl;*/
                        /*auto mx = sf::Mouse::getPosition(window);
                        float x = window.mapPixelToCoords(mx).x;
                        auto random = getRGBA(world_image, window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio,
                            window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio);*/
                sf::FloatRect rectBounds = levelStart.getGlobalBounds();
                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (clicking_state.empty() || !rectBounds.contains(mousePos)) {
                    float mouseInMapPosX = (mousePos.x - world.getPosition().x) / mapRatio;
                    float mouseInMapPosY = (mousePos.y - world.getPosition().y) / mapRatio;
                    //std::cout << mouseInMapPosX << std::endl;
                    //std::cout << mouseInMapPosY << std::endl;

                    checkClickingState(mouseInMapPosX, mouseInMapPosY);
                }
                else if (levelStartButton.getGlobalBounds().contains(mousePos)) {
                    changeScene(1);
                }
            }
            break;
        }
        case 1:
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
                leftClicking = true;

                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
                sf::Vector2i real = sf::Mouse::getPosition(window);

                if (pvzScene == 0) {
                    for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
                        if (seedPackets.find(seedPacketIdToString[i])->second.getGlobalBounds().contains(mousePos)) {
                            switch (seedPacketState[i][0]) {
                            case 0: //select place
                                seedPacketState[i][0] = 1;
                                break;
                            case 2: //selected
                                seedPacketState[i][0] = 3;
                            default:
                                //case 1: //moving
                                    //seedPacketState[i][1]++; //put in async loop thread
                                break;
                            }
                        }
                    }

                    if (seedPacketSelected == maxSeedPacketAmount) {
                        if (seedChooserButton.getGlobalBounds().contains(mousePos)) {
                            pvzScene = 1;
                        }
                    }
                }
                else if (pvzScene == 3) {
                    for (size_t i = 0; i < static_cast<size_t>(maxPlantAmount); ++i) {
                        if (seedPackets.find(seedPacketIdToString[i])->second.getGlobalBounds().contains(mousePos)) {
                            if (seedPacketState[i][0] == 2) {
                                pvzDrawPacketShade = true;
                                seedPacketState[i][0] = 1;
                                overlayShape.setPosition(seedPackets.find(seedPacketIdToString[i])->second.getPosition());
                            }
                        }
                    }
                }
            }
            break;
        }

        window.clear();

        switch (scene) {
        default:
        case 0:
            window.setView(view_world);
            window.draw(world); //render first: at bottom

            if (blinkMap_readyToDraw.load()) {
                world_blink.setTexture(&texture_blink);
                blinkMap_readyToDraw.store(false);
            }
            if (!clicking_state.empty()) {
                world_blink.setPosition(sf::Vector2f(world.getPosition().x + blinkCoords[0] * mapRatio,
                    world.getPosition().y + blinkCoords[1] * mapRatio));
                window.draw(world_blink);

                levelStart.setSize(sf::Vector2f(view_world.getSize().x / 2.0f, view_world.getSize().y));
                levelStart.setPosition(view_world.getCenter().x - view_world.getSize().x / 2.0f,
                    view_world.getCenter().y - view_world.getSize().y / 2.0f);
                int size = static_cast<unsigned int>(view_world.getSize().x / 38.4f);

                levelStartText.setCharacterSize(size);
                sf::FloatRect rectBounds = levelStart.getGlobalBounds();
                sf::FloatRect textBounds = levelStartText.getLocalBounds();
                levelStartText.setOrigin(textBounds.left + textBounds.width / 2.0f,
                    textBounds.top + textBounds.height / 2.0f);
                levelStartText.setPosition(
                    rectBounds.left + rectBounds.width / 2.0f,
                    rectBounds.top + rectBounds.height / 2.0f
                );

                levelStartButton.setSize(sf::Vector2f(textBounds.width, textBounds.height));
                levelStartButton.setPosition(levelStartText.getPosition().x - textBounds.width / 2.0f,
                    levelStartText.getPosition().y - textBounds.height / 2.0f);

                window.draw(levelStart);
                window.draw(levelStartButton);
                window.draw(levelStartText);
            }

            if (loadFlag_readyToDraw.load()) {
                flag_rect.setTexture(&flag_texture);
                loadFlag_readyToDraw.store(false);
            }
            if (!flag.empty()) {
                flag_rect.setSize(sf::Vector2f(15 * mapRatio * view_world.getSize().x / window.getSize().x,
                    10 * mapRatio * view_world.getSize().y / window.getSize().y)); //3:2
                flag_rect.setPosition(view_world.getCenter().x - view_world.getSize().x / 2, view_world.getCenter().y - view_world.getSize().y / 2);
                window.draw(flag_rect);
            }
            break;
        case 1:
            window.setView(view_background);
            window.draw(background);

            if (pvzScene == 0 || pvzScene == 1) {
                window.draw(seedChooser_background);
                window.draw(seedChooserButton);
            }
            else if (pvzScene == 2) {
                window.draw(pvzStartText);
            }

            window.draw(seedBank);
            window.draw(pvzSunText);
            window.draw(seedPackets.find(seedPacketIdToString[0])->second);

            if (pvzScene == 3 && pvzDrawPacketShade) {
                window.draw(overlayShape);
            }

            break;
        }

        window.display();
    }

    changeScene(-1);
    return 0;
}

//Version 1.0.17