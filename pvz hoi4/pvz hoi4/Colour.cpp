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

#ifdef _DEBUG
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

//int alpha = 254;
//int ra = 1;

template <typename T>
constexpr T clamp(T value, T low, T high) {
    return (value < low) ? low : (value > high) ? high : value;
}

std::array<int, 3> RGBtoHSL(const std::array<int, 3> rgb) {
    float r = rgb[0] / 255.0f;
    float g = rgb[1] / 255.0f;
    float b = rgb[2] / 255.0f;

    float fMin = std::min({ r, g, b });
    float fMax = std::max({ r, g, b });
    float delta = fMax - fMin;

    float h = 0.0f;
    float s = 0.0f;
    float l = (fMax + fMin) / 2.0f;

    if (delta != 0) {
        s = (l < 0.5f) ? delta / (fMax + fMin) : delta / (2.0f - fMax - fMin);

        if (r == fMax) {
            h = (g - b) / delta;
        }
        else if (g == fMax) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }

        h /= 6.0f;
        if (h < 0) h += 1;
    }

    return { static_cast<int>(std::round(h * 360.0f)),
             static_cast<int>(std::round(s * 100.0f)),
             static_cast<int>(std::round(l * 100.0f)) };
}

std::array<int, 3> HSLtoRGB(const std::array<int, 3> hsl) {
    auto hueToRGB = [](float p, float q, float t) -> float {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
        };

    float r, g, b;

    float h = hsl[0] / 360.0f;
    float s = hsl[1] / 100.0f;
    float l = hsl[2] / 100.0f;

    if (s == 0) {
        r = g = b = l; // achromatic
    }
    else {
        float q = l < 0.5f ? l * (1 + s) : l + s - l * s;
        float p = 2 * l - q;
        r = hueToRGB(p, q, h + 1.0f / 3.0f);
        g = hueToRGB(p, q, h);
        b = hueToRGB(p, q, h - 1.0f / 3.0f);
    }

    return { static_cast<int>(std::round(r * 255.0f)),
             static_cast<int>(std::round(g * 255.0f)),
             static_cast<int>(std::round(b * 255.0f)) };
}

int blinkSpeed = 2;
int lRatio = 0;
int lRatioF = -1;
sf::Image pixelsToBlink(const std::vector<std::array<int, 2>>& coords, sf::Image image) {
    lRatio += lRatioF;
    bool flipColour = false;

    for (const auto& coord : coords) {
        //int alpha = getPixelColour(image, coord[0], coord[1], 'a');
        //std::cout << alpha << std::endl;
        sf::Color ogColor = image.getPixel(coord[0], coord[1]);
        std::array<int, 3> newColor = RGBtoHSL({ ogColor.r, ogColor.g, ogColor.b });

        newColor[2] -= blinkSpeed * lRatio;

        flipColour = (newColor[2] < 30 && lRatioF > 0) || (newColor[2] > 70 && lRatioF < 0);

        newColor[2] = clamp(newColor[2], 0, 100);

        newColor = HSLtoRGB(newColor);
        image.setPixel(coord[0], coord[1], sf::Color(newColor[0], newColor[1], newColor[2]));
    }

    if (flipColour) {
        lRatioF = -lRatioF;
        lRatio += 2 * lRatioF;
    }
    return image;
    /*if (ra > 0 && alpha <= 100) {
    ra = -1;
}
else if (ra < 0 && alpha >= 255) {
    ra = 1;
}
alpha -= blinkSpeed * ra;
alpha = std::max(std::min(alpha, 254), 100);*/
}

int getPixelColour(sf::Image& image, int imageX, int imageY, char colourType) {
    sf::Color color = image.getPixel(imageX, imageY);

    switch (colourType) {
    case 'r': return color.r;
    case 'g': return color.g;
    case 'b': return color.b;
    case 'a': return color.a;
    default: return 0;
    }
}

std::string getRGBA(sf::Image& texture, int imageX, int imageY) {
    /*std::array<int, 3> ArrayFloat = HSLtoRGB(RGBtoHSL({ getPixelColour(texture, imageX, imageY, 'r'),
        getPixelColour(texture, imageX, imageY, 'g'),
        getPixelColour(texture, imageX, imageY, 'b') }));
    std::cout << ArrayFloat[0] << ", " << ArrayFloat[1] << ", " << ArrayFloat[2] << std::endl;*/

    std::string string = std::to_string(getPixelColour(texture, imageX, imageY, 'r')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'g')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'b')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'a'));
    return string;
}