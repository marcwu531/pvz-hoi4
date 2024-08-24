#ifndef COLOUR_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define COLOUR_H

template <typename T>
constexpr T clamp(T value, T low, T high);
sf::Image pixelsToBlink(const std::vector<sf::Vector2i>& coords, sf::Image image);
std::string getRGBA(const sf::Image& texture, int imageX, int imageY);
std::array<int, 3> RGBtoHSL(std::array<int, 3> rgb);
std::array<int, 3> HSLtoRGB(std::array<int, 3> hsl);

#endif