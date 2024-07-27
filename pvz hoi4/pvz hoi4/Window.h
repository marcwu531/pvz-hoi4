#ifndef WINDOW_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define WINDOW_H

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <array>
#include <future>
#include <thread>
#include "windows.h"

sf::Image loadImageFromResource(HINSTANCE hInstance, UINT resourceID);
void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View view);
std::string clickingState(sf::Image image, float mouseInMapPosX, float mouseInMapPosY);
sf::Image cropImage(const sf::Image image, const sf::IntRect& cropArea);

#endif
