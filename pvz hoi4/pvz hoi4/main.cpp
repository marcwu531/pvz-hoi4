#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <array>
#include <future>
#include <thread>
#include "Window.h"
#include "Colour.h"
#include <atomic>
#include <windows.h>
#include "Resource.h"
#include "State.h"

sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close); //(1920, 1046)

sf::View view_world(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));
sf::View view_background(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));

std::atomic<bool> running(true);
std::atomic<bool> blinkMap_loadingCoords(false);
std::vector<std::array<int, 2>> targetCoords;
//std::vector<std::array<int, 2>> nullVector = { {NULL, NULL} };
float mapRatio = 20.0f;
sf::RectangleShape world_blink; //(sf::Vector2f(mapRatio* (State::T::lx - State::T::sx + 1), mapRatio* (State::T::ly - State::T::sy + 1))); //15s

//std::atomic<int> blinkCd(0); //int blinkCd = 0;
std::atomic<bool> blinkMap_readyToDraw(false);

sf::Texture texture_blink;

HINSTANCE hInstance = GetModuleHandle(NULL);

HRSRC hResourceInfo = FindResource(hInstance, MAKEINTRESOURCE(OPENAL32), RT_RCDATA);

sf::Image world_image = loadImageFromResource(hInstance, 101);
sf::Image flag_Taiwan_image = loadImageFromResource(hInstance, 102);
sf::Image pvz_background_bg1_image = loadImageFromResource(hInstance, 103);
sf::Image pvz_seedSelector_seedChooserBackground_image = loadImageFromResource(hInstance, 104);
sf::Image pvz_seedSelector_seedBank_image = loadImageFromResource(hInstance, 105);
sf::Image pvz_seedPacket_peashooter_image = loadImageFromResource(hInstance, 106);

std::map<std::string, sf::Image> flagImages = {
    {"Taiwan", flag_Taiwan_image}
};

std::map<std::string, std::map<std::string, sf::Image>> pvzImages = {
    {"background", {
        {"bg1", pvz_background_bg1_image}
    }},
    {"seed_selector", {
        {"seedChooser_background", pvz_seedSelector_seedChooserBackground_image},
        {"seedBank", pvz_seedSelector_seedBank_image}
    }},
    {"seed_packet", {
        {"peashooter", pvz_seedPacket_peashooter_image}
    }}
};

sf::Image getFlagImage(std::string country) {
    return flagImages[country];
}

sf::Image getPvzImage(std::string type, std::string target) {
    return pvzImages[type][target];
}

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

std::atomic<bool> loadFlag_readyToDraw(false);
sf::Texture flag_texture;
sf::RectangleShape flag_rect;
std::string current_flag;

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

std::atomic<bool> loadLevelStart_readyToDraw(false);
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

sf::Texture texture_background;
sf::RectangleShape background;
sf::Texture texture_seedChooser_background;
sf::RectangleShape seedChooser_background;
sf::Texture texture_seedBank;
sf::RectangleShape seedBank;
sf::Texture texture_seedPacket_peashooter;
sf::RectangleShape seedPacket_peashooter;

void initializeScene1() {
    float zoomSize = 1.7f;

    texture_background.loadFromImage(getPvzImage("background", "bg1"));
    float bgCamSizeY = view_background.getSize().y;
    background.setSize(sf::Vector2f(1400.0f / 600.0f * bgCamSizeY, bgCamSizeY)); //1920.0f, 1046.0f -> bg png size 1400 x 600
    background.setTexture(&texture_background);

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
    seedPacket_peashooter.setSize(sf::Vector2f(50.0f * zoomSize, 70.0f * zoomSize));
    seedPacket_peashooter.setTexture(&texture_seedPacket_peashooter);
    seedPacket_peashooter.setPosition(seedChooser_background.getPosition() + sf::Vector2f(20.0f, 55.0f));

    background.setOrigin(background.getSize() / 2.0f);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) { //int main() {   
    sf::RectangleShape world(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
    //window.create(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close);
    view_world.setCenter(sf::Vector2f(93000.0f, 19000.0f)); //94000.0f, 20000.0f

    //world.setOrigin(93000.0f, 19500.0f);
    sf::Texture texture_world;
    //world_image.loadFromFile("world_images/world.png");
    texture_world.loadFromImage(world_image); //sf::IntRect(4555, 920, 200, 200)
    world.setTexture(&texture_world);

    initializeScene1();

    float tx = 0.0f;
    float ty = 0.0f;
    bool leftClicking = false;
    window.setFramerateLimit(fps);

    std::thread thread_asyncBlinkMap(asyncBlinkMap);
    std::thread thread_asyncLoadFlag(asyncLoadFlag);
    std::thread thread_asyncLoadLevelStart(asyncLoadLevelStart);

    sf::Font defaultFont;
    defaultFont.loadFromFile("images/fonts/Brianne_s_hand.ttf");

    sf::RectangleShape levelStart(sf::Vector2f(view_world.getSize().x / 2.0f, view_world.getSize().y));
  
    std::vector<sf::RectangleShape> seedPackets = { seedPacket_peashooter };
    std::vector<std::map<int, int>> seedPacketState; //state, state1 moving time

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
                    if (e.key.code == sf::Keyboard::Escape) window.close();
                    break;
                case sf::Event::Closed:
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
                    scene = 1;
                }
            }
            break;
        }
        case 1:
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
                leftClicking = true;

                sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                for (size_t i = 0; i < seedPackets.size(); ++i) {
                    if (seedPackets[i].getGlobalBounds().contains(mousePos)) {
                        if (seedPacketState[i].empty()) seedPacketState[i] = {0, 0};
                        switch (seedPacketState[i][0]) {
                        default:
                        case 0: //select place
                            seedPacketState[i][0] = 1;
                            break;
                        case 1: //moving
                            seedPacketState[i][1]++; //put in async loop thread
                        case 2: //selected
                            break;
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
                //window.draw(flag_rect);
            }
            break;
        case 1:
            window.setView(view_background);
            window.draw(background);
            window.draw(seedChooser_background);
            window.draw(seedBank);
            window.draw(seedPacket_peashooter);
            break;
        }
        
        window.display();
    }
    running.store(false);
    thread_asyncBlinkMap.join();
    thread_asyncLoadFlag.join();
    thread_asyncLoadLevelStart.join();

	return 0;
}

//Version 1.0.14