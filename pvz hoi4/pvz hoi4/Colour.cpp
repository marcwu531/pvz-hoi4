#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Image.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>
#include <vector>
#include <array>
#include <future>
#include <thread>
#include "Colour.h"
#include "State.hpp"
#include "Window.h"

int blinkSpeed = 5;
int alpha = 254;
int ra = 1;

std::array<int, 3> RGBtoHSL(std::array<int, 3> rgb) {
    int r = rgb[0];
    int g = rgb[1];
    int b = rgb[2];

    float fr = r / 255.0f;
    float fg = g / 255.0f;
    float fb = b / 255.0f;

    float fMin = std::min({ fr, fg, fb });
    float fMax = std::max({ fr, fg, fb });
    float delta = fMax - fMin;

    float h = 0.0f;
    float s = 0.0f;
    float l = (fMax + fMin) / 2.0f;

    if (delta != 0) {
        s = (l < 0.5f) ? delta / (fMax + fMin) : delta / (2.0f - fMax - fMin);

        if (fr == fMax) {
            h = (fg - fb) / delta;
        }
        else if (fg == fMax) {
            h = 2.0f + (fb - fr) / delta;
        }
        else {
            h = 4.0f + (fr - fg) / delta;
        }

        h /= 6.0f;
        if (h < 0) h += 1;
    }

    h *= 360.0f;
    s *= 100.0f;
    l *= 100.0f;

    return { (int)std::round(h), (int)std::round(s), (int)std::round(l) };
}

std::array<int, 3> HSLtoRGB(std::array<int, 3> hsl) {
    int h = hsl[0];
    int s = hsl[1];
    int l = hsl[2];

    float fh = h / 360.0f;
    float fs = s / 100.0f;
    float fl = l / 100.0f;

    auto hueToRGB = [](float p, float q, float t) -> float {
        if (t < 0) t += 1;
        if (t > 1) t -= 1;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    float r, g, b;

    if (fs == 0) {
        r = g = b = fl; // achromatic
    }
    else {
        float q = fl < 0.5f ? fl * (1 + fs) : fl + fs - fl * fs;
        float p = 2 * fl - q;
        r = hueToRGB(p, q, fh + 1.0f / 3.0f);
        g = hueToRGB(p, q, fh);
        b = hueToRGB(p, q, fh - 1.0f / 3.0f);
    }

    r *= 255.0f;
    g *= 255.0f;
    b *= 255.0f;

    return { (int)std::round(r), (int)std::round(g), (int)std::round(b) };
}

sf::Image pixelsToBlink(std::vector<std::array<int, 2>> coords, sf::Image image) {
    if (ra > 0 && alpha <= 100) {
        ra = -1;
    }
    else if (ra < 0 && alpha >= 255) {
        ra = 1;
    }
    alpha -= blinkSpeed * ra;
    alpha = std::max(std::min(alpha, 254), 100);
    for (const auto& coord : coords) {
        //int alpha = getPixelColour(image, coord[0], coord[1], 'a');
        //std::cout << alpha << std::endl;
        sf::Color ogColor = image.getPixel(coord[0], coord[1]);
        image.setPixel(coord[0], coord[1], sf::Color(ogColor.r, ogColor.g, ogColor.b, alpha));
    }
    return image;
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
    std::array<int, 3> ArrayFloat = HSLtoRGB(RGBtoHSL({ getPixelColour(texture, imageX, imageY, 'r'),
        getPixelColour(texture, imageX, imageY, 'g'),
        getPixelColour(texture, imageX, imageY, 'b') }));
    std::cout << ArrayFloat[0] << ", " << ArrayFloat[1] << ", " << ArrayFloat[2] << std::endl;

    std::string string = std::to_string(getPixelColour(texture, imageX, imageY, 'r')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'g')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'b')) + ' '
        + std::to_string(getPixelColour(texture, imageX, imageY, 'a'));
    return string;
}

/*std::array<int, 3> HSLtoRGB(int h, int s, int l) {
    int r, g, b;
    return { r, g, b };
}*/