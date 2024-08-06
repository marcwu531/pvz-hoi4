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
#include <queue>
#include <shellapi.h>

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View view);
std::array<std::string, 2> clickingState(sf::Image image, float mouseInMapPosX, float mouseInMapPosY);
extern const std::vector<sf::Keyboard::Key> konamiCode;
bool isKonamiCodeEntered(const std::queue<sf::Keyboard::Key>& inputs);

#endif
