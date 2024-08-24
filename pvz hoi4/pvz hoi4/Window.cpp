#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <SFML/Graphics.hpp>

#include "Colour.h"
#include "State.h"
#include "Window.h"

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View& view) {
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

std::array<std::string, 2> clickingState(sf::Image& image, float mouseInMapPosX, float mouseInMapPosY) {
	int x = static_cast<int>(mouseInMapPosX), y = static_cast<int>(mouseInMapPosY);

	auto rgba = getRGBA(image, x, y);
	for (const auto& region : Regions) {
		for (const std::string& state : region.second) {
			auto state_rgba_iter = state_rgba.find(state);
			auto state_int_iter = state_int.find(state);
			if (state_rgba_iter != state_rgba.end() && state_int_iter != state_int.end()) {
				if (rgba == state_rgba_iter->second.at("RGBA")()) {
					if (x >= state_int_iter->second.at("sx")() && x <= state_int_iter->second.at("lx")()
						&& y >= state_int_iter->second.at("sy")() && y <= state_int_iter->second.at("ly")()) {
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
	for (const auto& key : konamiCode) {
		if (tempQueue.front() != key) {
			return false;
		}
		tempQueue.pop();
	}
	return true;
}

void stdcoutMap(std::unordered_map<int, int>* map) {
	for (const auto& pair : *map) {
		std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
	}
	std::cout << std::endl;
}