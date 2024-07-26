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
#include "State.hpp"
#include "Colour.h"

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

std::string clickingState(sf::Image image, float mouseInMapPosX, float mouseInMapPosY) {
    if (getRGBA(image, (int)std::floor(mouseInMapPosX), (int)std::floor(mouseInMapPosY)) == State::T::RGBA()) {
        if (mouseInMapPosX > State::T::sx && mouseInMapPosX < State::T::lx
            && mouseInMapPosY > State::T::sy && mouseInMapPosY < State::T::ly) {
            return "T";
        }
    }
    return "";
}

sf::Image cropImage(const sf::Image image, const sf::IntRect& cropArea) {
    sf::Image cropped_image;
    cropped_image.create(cropArea.width, cropArea.height, sf::Color::Transparent);

    for (int x = 0; x < cropArea.width; x++) {
        for (int y = 0; y < cropArea.height; y++) {
            cropped_image.setPixel(x, y, image.getPixel(cropArea.left + x, cropArea.top + y));
        }
    }

    return cropped_image;
}