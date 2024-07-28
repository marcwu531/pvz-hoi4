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