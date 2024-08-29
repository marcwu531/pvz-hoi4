#include <array>
#include <SFML/Graphics.hpp>

#include "Colour.h"

//int alpha = 254;
//int ra = 1;

template <typename T>
constexpr T clamp(T value, T low, T high) {
	return std::max(low, std::min(value, high));
}

std::array<int, 3> RGBtoHSL(const std::array<int, 3> rgb) {
	float r = rgb[0] / 255.0f, g = rgb[1] / 255.0f, b = rgb[2] / 255.0f,
		fMin = std::min({ r, g, b }), fMax = std::max({ r, g, b }), delta = fMax - fMin,
		h = 0.0f, s = 0.0f, l = (fMax + fMin) / 2.0f;

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

inline std::array<int, 3> HSLtoRGB(const std::array<int, 3> hsl) {
	auto hueToRGB = [](float p, float q, float t) -> float {
		if (t < 0) t += 1;
		if (t > 1) t -= 1;
		if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
		if (t < 1.0f / 2.0f) return q;
		if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
		return p;
		};

	float r, g, b,
		h = hsl[0] / 360.0f, s = hsl[1] / 100.0f, l = hsl[2] / 100.0f;

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

int blinkSpeed = 1;
int lRatio = 0;
int lRatioF = -1;

inline static void updatePixelColor(sf::Color& color, int blinkSpeed, int& lRatio, int& lRatioF) {
	std::array<int, 3> hsl = RGBtoHSL({ color.r, color.g, color.b });
	hsl[2] -= blinkSpeed * lRatio;

	hsl[2] = clamp(hsl[2], 0, 100);
	std::array<int, 3> rgb = HSLtoRGB(hsl);

	color = sf::Color(rgb[0], rgb[1], rgb[2]);

	if ((hsl[2] < 30 && lRatioF > 0) || (hsl[2] > 70 && lRatioF < 0)) {
		lRatioF = -lRatioF;
		lRatio += 2 * lRatioF;
	}
}

sf::Image pixelsToBlink(const std::vector<sf::Vector2i>& coords, sf::Image& image, const sf::IntRect& cropArea) {
	lRatio += lRatioF;

	for (const auto& coord : coords) {
		int relX = coord.x - cropArea.left;
		int relY = coord.y - cropArea.top;

		if (relX >= 0 && relX < cropArea.width && relY >= 0 && relY < cropArea.height) {
			sf::Color color = image.getPixel(relX, relY);
			updatePixelColor(color, blinkSpeed, lRatio, lRatioF);
			image.setPixel(relX, relY, color);
		}
	}
	return image;
}

inline int getPixelColorComponent(const sf::Color& color, char component) {
	switch (component) {
	case 'r': return color.r;
	case 'g': return color.g;
	case 'b': return color.b;
	case 'a': return color.a;
	default: return 0;
	}
}

std::string getRGBA(const sf::Image& texture, int imageX, int imageY) {
	/*std::array<int, 3> ArrayFloat = HSLtoRGB(RGBtoHSL({ getPixelColour(texture, imageX, imageY, 'r'),
		getPixelColour(texture, imageX, imageY, 'g'),
		getPixelColour(texture, imageX, imageY, 'b') }));
	std::cout << ArrayFloat[0] << ", " << ArrayFloat[1] << ", " << ArrayFloat[2] << std::endl;*/
	if (imageX < 0 || static_cast<unsigned int>(imageX) >= texture.getSize().x || 
		imageY < 0 || static_cast<unsigned int>(imageY) >= texture.getSize().y) return "-1 -1 -1 -1";

	sf::Color color = texture.getPixel(imageX, imageY);
	return std::to_string(color.r) + ' ' +
		std::to_string(color.g) + ' ' +
		std::to_string(color.b) + ' ' +
		std::to_string(color.a);
}