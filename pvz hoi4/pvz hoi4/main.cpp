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

sf::RenderWindow window(sf::VideoMode(1920, 1046), "Pvz Hoi4", sf::Style::Close | sf::Style::Resize);
sf::View view(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));

std::atomic<bool> running(true);
std::atomic<bool> loadingCoords(false);
std::vector<std::array<int, 2>> targetCoords;
//std::vector<std::array<int, 2>> nullVector = { {NULL, NULL} };
float mapRatio = 20.0f;
sf::RectangleShape world_blink; //(sf::Vector2f(mapRatio* (State::T::lx - State::T::sx + 1), mapRatio* (State::T::ly - State::T::sy + 1))); //15s

//std::atomic<int> blinkCd(0);  //int blinkCd = 0;
std::atomic<bool> readyToDraw(false);

sf::Texture texture_blink;

HINSTANCE hInstance = GetModuleHandle(NULL);
sf::Image image = loadImageFromResource(hInstance, WORLD_IMAGE);

void asyncUpdate() {
    auto lastTime = std::chrono::high_resolution_clock::now();

    while (running.load()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastTime);

        if (elapsedTime.count() >= 16) { //(int)(1000.0f/60.0f)
            if (!targetCoords.empty() && !readyToDraw.load() && !clicking_state.empty() && !loadingCoords.load()) {
                /*int currentBlinkCd = blinkCd.load();
                //if (currentBlinkCd < 5) {
                    //blinkCd.store(currentBlinkCd + 1);
                //}
                //else {
                    //blinkCd.store(0);*/

                sf::Image image_blink = pixelsToBlink(targetCoords, image);

                int sx = state_int[clicking_state]["sx"]();
                int sy = state_int[clicking_state]["sy"]();
                int lx = state_int[clicking_state]["lx"]();
                int ly = state_int[clicking_state]["ly"]();

                //std::cout << "sx: " << sx << ", sy: " << sy << ", lx: " << lx << ", ly: " << ly << std::endl;

                sf::IntRect cropArea(sx, sy, lx - sx + 1, ly - sy + 1);
                image_blink = cropImage(image_blink, cropArea);

                if (texture_blink.getSize().x != static_cast<unsigned int>(cropArea.width) ||
                    texture_blink.getSize().y != static_cast<unsigned int>(cropArea.height)) {
                    texture_blink.create(cropArea.width, cropArea.height);
                }

                texture_blink.update(image_blink);
                world_blink.setTextureRect(sf::IntRect(0, 0, cropArea.width, cropArea.height));
                world_blink.setSize(sf::Vector2f(mapRatio * cropArea.width, mapRatio * cropArea.height));
                world_blink.setPosition(sf::Vector2f(sx * mapRatio, sy * mapRatio)); //view.getCenter().x, view.getCenter().y

                readyToDraw.store(true);
            }
            lastTime = currentTime;
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void checkClickingState(float mouseInMapPosX, float mouseInMapPosY) {
    std::string targetState = clickingState(image, mouseInMapPosX, mouseInMapPosY);

    //std::cout << targetState << std::endl;
    if (!targetState.empty() && clicking_state != targetState) {
        loadingCoords.store(true);
        clicking_state = targetState;
        targetCoords.clear();

        //std::cout << "sx: " << state_int[clicking_state]["sx"]() << std::endl;
        //std::cout << "lx: " << state_int[clicking_state]["lx"]() << std::endl;

        for (int x = state_int[clicking_state]["sx"](); x <= state_int[clicking_state]["lx"](); ++x) {
            for (int y = state_int[clicking_state]["sy"](); y <= state_int[clicking_state]["ly"](); ++y) {
                if (getRGBA(image, x, y) == state_rgba[clicking_state]["RGBA"]()) {
                    targetCoords.push_back({ x, y });
                }
            }
        }
        loadingCoords.store(false);
    }
}

int main() {
    view.setCenter(sf::Vector2f(93000.0f, 19000.0f)); //94000.0f, 20000.0f

    sf::RectangleShape world(sf::Vector2f(mapRatio * 5632.0f, mapRatio * 2048.0f)); //5632*2048
    //world.setOrigin(93000.0f, 19500.0f);
    sf::Texture texture_world;
    //image.loadFromFile("images/world.png");
    texture_world.loadFromImage(image); //, sf::IntRect(4555, 920, 200, 200)
    world.setTexture(&texture_world);

    float tx = 0.0f;
    float ty = 0.0f;
    bool leftClicking = false;
    window.setFramerateLimit(60);

    std::thread updateThread(asyncUpdate);

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
            auto random = getRGBA(image, window.mapPixelToCoords(sf::Mouse::getPosition(window)).x / mapRatio, 
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
        if (readyToDraw.load()) {
            world_blink.setTexture(&texture_blink);
            readyToDraw.store(false);
        }
        window.draw(world_blink);
        window.display();
    }
    running.store(false);
    updateThread.join();

	return 0;
}

//Version 1.0.9