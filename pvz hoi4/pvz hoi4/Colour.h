#ifndef COLOUR_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define COLOUR_H

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <array>
#include <future>
#include <thread>

sf::Image pixelsToBlink(std::vector<std::array<int, 2>> coords, sf::Image image);
int getPixelColour(sf::Image& image, int imageX, int imageY, char colourType);
std::string getRGBA(sf::Image& texture, int imageX, int imageY);
std::array<int, 3> RGBtoHSL(std::array<int, 3> rgb);
std::array<int, 3> HSLtoRGB(std::array<int, 3> hsl);

#endif