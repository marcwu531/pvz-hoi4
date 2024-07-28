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

sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));

std::atomic<bool> running(true);
std::atomic<bool> blinkMap_loadingCoords(false);
std::vector<std::array<int, 2>> targetCoords;
//std::vector<std::array<int, 2>> nullVector = { {NULL, NULL} };
float mapRatio = 20.0f;
sf::RectangleShape world_blink; //(sf::Vector2f(mapRatio* (State::T::lx - State::T::sx + 1), mapRatio* (State::T::ly - State::T::sy + 1))); //15s

//std::atomic<int> blinkCd(0);  //int blinkCd = 0;
std::atomic<bool> blinkMap_readyToDraw(false);

sf::Texture texture_blink;

HINSTANCE hInstance = GetModuleHandle(NULL);
sf::Image world_image = loadImageFromResource(hInstance, WORLD_IMAGE);

int fps = 60;

void asyncBlinkMap() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= static_cast<unsigned int>(1000 / fps)) { //(int)(1000.0f/60.0f)
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
                world_blink.setPosition(sf::Vector2f(sx * mapRatio, sy * mapRatio)); //view.getCenter().x, view.getCenter().y

                blinkMap_readyToDraw.store(true);
            }
            lastTime = currentTime;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

std::atomic<bool> loadFlag_readyToDraw(false);
sf::Image flag_image;
sf::Texture flag_texture;
sf::RectangleShape flag_rect;
std::string current_flag;

void asyncLoadFlag() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= static_cast<unsigned int>(1000 / fps)) {
            if (!flag.empty() && !loadFlag_readyToDraw.load()) {
                if (current_flag != flag) {
                    flag_image.loadFromFile("images/flags/" + flag + ".png");

                    if (current_flag.empty()) {
                        flag_texture.create(383, 256);
                        flag_rect.setTexture(&flag_texture);
                        flag_texture.loadFromImage(flag_image);
                    }
                    else {
                        flag_texture.update(flag_image);
                    }

                    current_flag = flag;
                    flag_texture.loadFromImage(flag_image);
                }

                flag_rect.setSize(sf::Vector2f(15 * mapRatio * view.getSize().x / window.getSize().x, 
                    10 * mapRatio * view.getSize().y / window.getSize().y)); //3:2
                flag_rect.setPosition(view.getCenter().x - view.getSize().x / 2, view.getCenter().y - view.getSize().y / 2);
                loadFlag_readyToDraw.store(true);
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

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) { //int main() {
    //window.create(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close);
    view.setCenter(sf::Vector2f(93000.0f, 19000.0f)); //94000.0f, 20000.0f

    sf::RectangleShape world(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
    //world.setOrigin(93000.0f, 19500.0f);
    sf::Texture texture_world;
    //world_image.loadFromFile("world_images/world.png");
    texture_world.loadFromImage(world_image); //, sf::IntRect(4555, 920, 200, 200)
    world.setTexture(&texture_world);

    float tx = 0.0f;
    float ty = 0.0f;
    bool leftClicking = false;
    window.setFramerateLimit(fps);

    std::thread thread_asyncBlinkMap(asyncBlinkMap);
    std::thread thread_asyncLoadFlag(asyncLoadFlag);

    while (window.isOpen())
    {
        sf::Event e;
        while (window.pollEvent(e))
        {
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
                case sf::Event::MouseWheelScrolled:
                    zoomViewAt({ e.mouseWheelScroll.x, e.mouseWheelScroll.y }, window, std::pow(1.1f, -e.mouseWheelScroll.delta), view);
                    view = window.getView();
                    //std::cout << view.getCenter().x << " " << view.getCenter().y << std::endl;
                    break;
            }
        }

        //view = window.getView();
        int dtx = 0;
        int dty = 0;

        if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) dtx = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) dtx = 1;
        }
        if (!(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) &&
            sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) dty = -1;
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) dty = 1;
        }

        tx += dtx * std::max(std::min(tx, -80.0f), 80.0f);
        tx *= 0.9f;
        float dx = 0.02f * tx * view.getSize().x / 1920.0f;
        ty += dty * std::max(std::min(ty, -80.0f), 80.0f);
        ty *= 0.9f;
        float dy = 0.02f * ty * view.getSize().y / 1046.0f;

        view.move(sf::Vector2f(dx, dy));

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
            float mouseInMapPosX = window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio;
            float mouseInMapPosY = window.mapPixelToCoords(sf::Mouse::getPosition(window)).y / mapRatio;
            //std::cout << mouseInMapPosX << std::endl;
            //std::cout << mouseInMapPosY << std::endl;

            checkClickingState(mouseInMapPosX, mouseInMapPosY);
        }

        window.clear();
        window.setView(view);
        window.draw(world);

        if (blinkMap_readyToDraw.load()) {
            world_blink.setTexture(&texture_blink);
            blinkMap_readyToDraw.store(false);
        }
        if (!clicking_state.empty()) window.draw(world_blink);

        if (loadFlag_readyToDraw.load()) {
            flag_rect.setTexture(&flag_texture);
            loadFlag_readyToDraw.store(false);
        }
        if (!flag.empty()) window.draw(flag_rect);

        window.display();
    }
    running.store(false);
    thread_asyncBlinkMap.join();
    thread_asyncLoadFlag.join();

	return 0;
}

//Version 1.0.10.a