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
#include <fstream>
#include <stdexcept>
#include <queue>
#include <shellapi.h>
#include <memory>
#include <nlohmann/json.hpp>

#ifdef RUN_DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <vld.h>
#endif

#include "Window.h"
#include "Colour.h"
#include "Resource.h"
#include "State.h"
#include "Display.h"
#include "Async.h"
#include "Scene1.h"
#include "Audio.h"
#include "Json.h"

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) { //int main() {   
    #ifdef RUN_DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    #endif

    srand(static_cast<unsigned>(time(0)));
    
    initializeAudios(hInstance);
    
    std::vector<char> fontData = loadResourceData(nullHInstance, 4);
    defaultFont.loadFromMemory(fontData.data(), fontData.size());
    
    initializeScene1();

    audios["soundtrack"]["battleofwuhan"]->setVolume(25);

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

    //changeScene(1); //DEBUG

    sf::Shader brightness_shader;
    std::vector<char> brightness_shader_data = loadResourceData(nullHInstance, 129);
    std::string brightness_shader_str(brightness_shader_data.begin(), brightness_shader_data.end());
    brightness_shader.loadFromMemory(brightness_shader_str, sf::Shader::Fragment);
    brightness_shader.setUniform("texture", sf::Shader::CurrentTexture);
    brightness_shader.setUniform("brightness", 1.75f);

    while (window.isOpen())
    {
        sf::Event e;
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

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
                [[fallthrough]];
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

                    /*if (e.key.code == 27) {
                        int a = e.key.code;
                        int b = e.key.code;
                        //e.key.code >= 27 && <= ?
                    }*/

                    if (scene == 1 && pvzScene == 3 && !pvzPacketOnSelected && e.key.code == 27) selectSeedPacket(e.key.code - 27);

                    if (e.key.code != sf::Keyboard::Escape) break;
                    [[fallthrough]];
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
            if (audios["soundtrack"]["battleofwuhan"]->getStatus() != sf::Music::Playing) audios["soundtrack"]["battleofwuhan"]->play();
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
            changeScene(1); //RUN_DEBUG
            break;
        }
        case 1:
            if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !leftClicking) {
                leftClicking = true;

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
                                break;
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
                    if (!pvzPacketOnSelected) {
                        selectSeedPacket(mousePos);
                    }
                    else { //plant Plant
                        createPlant(hoverPlant.getPosition());
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

            if (!zombiesOnScene.empty()) {
                for (auto& zombie : zombiesOnScene) {
                    std::sort(zombiesOnScene.begin(), zombiesOnScene.end(), [](const auto& a, const auto& b) {
                        return a.anim.row < b.anim.row;
                    });

                    if (zombie.damagedCd > 0) {
                        window.draw(zombie.anim.sprite, &brightness_shader);
                    }
                    else {
                        window.draw(zombie.anim.sprite);
                    }
                }
            }

            if (pvzScene == 0 || pvzScene == 1) {
                window.draw(seedChooser_background);
                window.draw(seedChooserButton);
            }
            
            if (pvzScene == 2) {
                window.draw(pvzStartText);
            }

            pvzSunText.setString(std::to_string(pvzSun));
            sf::FloatRect pvzSunTextRect = pvzSunText.getLocalBounds();
            pvzSunText.setOrigin(pvzSunTextRect.left + pvzSunTextRect.width / 2.0f,
                pvzSunTextRect.top + pvzSunTextRect.height / 2.0f);

            window.draw(seedBank);
            window.draw(pvzSunText);
            window.draw(seedPackets.find(seedPacketIdToString[0])->second);

            if (pvzScene == 3) {
                if (pvzPacketOnSelected) {
                    window.draw(overlayShade);
                    if (canPlant(hoverPlant.getPosition())) {
                        window.draw(hoverPlant);
                        window.draw(hoverShade);
                    }
                    window.draw(peashooterIdle);
                }
                if (!plantsOnScene.empty()) {
                    for (auto& plant : plantsOnScene) {
                        window.draw(plant.anim.sprite);
                    }
                }
                if (!projectilesOnScene.empty()) {
                    for (auto& projectile : projectilesOnScene) {
                        window.draw(projectile.sprite);
                    }
                }
                if (!vanishProjectilesOnScene.empty()) {
                    vanishProjectilesOnScene.erase(
                        std::remove_if(vanishProjectilesOnScene.begin(), vanishProjectilesOnScene.end(),
                            [&](vanishProjState& vanish_projectile) {
                                vanish_projectile.proj.sprite.setTextureRect(
                                    peaSplatsFrames[std::min(vanish_projectile.frame, 3)].frameRect);
                                window.draw(vanish_projectile.proj.sprite);
                                return ++vanish_projectile.frame >= 13;
                            }),
                        vanishProjectilesOnScene.end()
                    );
                }
            }

            break;
        }

        window.display();
    }

    changeScene(-1);

    #ifdef RUN_DEBUG
    _CrtDumpMemoryLeaks();
    #endif

    return 0;
}

//Version 1.0.28