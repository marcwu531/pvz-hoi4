#include "Display.h"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include "Colour.h"
#include "State.h"
#include "Window.h"
#include "Async.h"
#include "Audio.h"

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
sf::RectangleShape overlayShade;
sf::Texture peashooterIdleSprites;
sf::Sprite peashooterIdle;
std::map<int, SpriteFrame> peashooterIdleFrames;
sf::Sprite hoverPlant;
sf::Sprite hoverShade;
sf::Texture zombieIdleSprites;
sf::Sprite zombieIdle;
std::map<int, SpriteFrame> zombieIdleFrames;

sf::Texture texture_blink;

sf::RenderWindow window(sf::VideoMode::getDesktopMode(), "Pvz Hoi4", sf::Style::Resize | sf::Style::Close); //(1920, 1046)
sf::View view_world(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));
sf::View view_background(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(1920.0f, 1046.0f));

float mapRatio = 20.0f;
sf::RectangleShape world_blink; //(sf::Vector2f(mapRatio* (State::T::lx - State::T::sx + 1), mapRatio* (State::T::ly - State::T::sy + 1))); //15s

sf::Image cropImage(const sf::Image image, const sf::IntRect cropArea) {
    sf::Image cropped_image;
    cropped_image.create(cropArea.width, cropArea.height);

    for (int x = 0; x < cropArea.width; ++x) {
        for (int y = 0; y < cropArea.height; ++y) {
            cropped_image.setPixel(x, y, image.getPixel(cropArea.left + x, cropArea.top + y));
        }
    }

    return cropped_image;
}

sf::Image loadImageFromResource(HINSTANCE hInstance, UINT resourceID) {
    // Find the resource
    sf::Image image;

    HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(resourceID), RT_RCDATA);
    if (!hRes) {
        throw std::runtime_error("Failed to find resource");
    }

    // Load the resource data
    HGLOBAL hResData = LoadResource(hInstance, hRes);
    if (!hResData) {
        throw std::runtime_error("Failed to load resource");
    }

    // Lock the resource to get a pointer to the data
    void* pResData = LockResource(hResData);
    if (!pResData) {
        throw std::runtime_error("Failed to lock resource");
    }

    // Get the size of the resource data
    DWORD resSize = SizeofResource(hInstance, hRes);
    if (resSize == 0) {
        throw std::runtime_error("Failed to get resource size");
    }

    // Load image from memory
    if (!image.loadFromMemory(pResData, resSize)) {
        throw std::runtime_error("Failed to load image from memory");
    }

    return image;
}

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
    }},
    {"animations", {
        {"peashooterIdle", loadImageFromResource(nullHInstance, 117)},
        {"zombieIdle", loadImageFromResource(nullHInstance, 119)}
    }}
};

sf::Image getFlagImage(std::string country) {
    return flagImages.at(country);
}

sf::Image getPvzImage(std::string type, std::string target) {
    return pvzImages.at(type).at(target);
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

void changeScene(int targetScene) {
    stopAllThreads();
    switch (scene) {
    case 0:
        if (audios["soundtrack"]["battleofwuhan"]->getStatus() == sf::Music::Playing) audios["soundtrack"]["battleofwuhan"]->stop();
        thread_asyncPacketMove = std::thread(asyncPvzSceneUpdate);
        [[fallthrough]];
    default:
        scene = targetScene;
        break;
    }
}