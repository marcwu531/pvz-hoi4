#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <vector>
#include <array>
#include <cmath>
#include "Window.h"
#include "Colour.h"
#include "windows.h"
#include <stdexcept>
#include <exception>
#include "State.h"
#include <queue>
#include <shellapi.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View view) {
    //std::cout << zoom << std::endl;
    const sf::Vector2f beforeCoord{ window.mapPixelToCoords(pixel) };
    //sf::View view = window.getView();
    view.zoom(zoom);
    window.setView(view);
    const sf::Vector2f afterCoord{ window.mapPixelToCoords(pixel) };
    const sf::Vector2f offsetCoords{ beforeCoord - afterCoord };
    view.move(offsetCoords);
    window.setView(view);
}

std::array<std::string, 2> clickingState(sf::Image image, float mouseInMapPosX, float mouseInMapPosY) {
    int x = static_cast<int>(mouseInMapPosX);
    int y = static_cast<int>(mouseInMapPosY);

    for (const auto& region : Regions) {
        for (const std::string& state : region.second) {
            if (state_rgba.find(state) != state_rgba.end() && state_int.find(state) != state_int.end()) {
                if (getRGBA(image, x, y) == state_rgba[state]["RGBA"]()) {
                    if (x >= state_int[state]["sx"]() && x <= state_int[state]["lx"]()
                        && y >= state_int[state]["sy"]() && y <= state_int[state]["ly"]()) {
                        return { region.first, state };
                    }
                }
            }
        }
    }

    return { "", "" };
}

const std::vector<sf::Keyboard::Key> konamiCode = {
    sf::Keyboard::Up, sf::Keyboard::Up,
    sf::Keyboard::Down, sf::Keyboard::Down,
    sf::Keyboard::Left, sf::Keyboard::Right,
    sf::Keyboard::Left, sf::Keyboard::Right,
    sf::Keyboard::B, sf::Keyboard::A
};

bool isKonamiCodeEntered(const std::queue<sf::Keyboard::Key>& inputs) {
    if (inputs.size() != konamiCode.size()) {
        return false;
    }

    std::queue<sf::Keyboard::Key> tempQueue = inputs;
    for (auto key : konamiCode) {
        if (tempQueue.front() != key) {
            return false;
        }
        tempQueue.pop();
    }
    return true;
}