#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <SFML/Graphics.hpp>

#include "Colour.h"
#include "State.h"
#include "Window.h"

#ifdef RUN_DEBUG
#include <windows.h>
#include <signal.h>

void AttachConsole() {
	AllocConsole();
	FILE* consoleOutput;
	freopen_s(&consoleOutput, "CONOUT$", "w", stdout);
}

static std::string WideStringToString(const std::wstring& wideStr) {
	int size_needed = WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.length(),
		NULL, 0, NULL, NULL);
	std::string narrowStr(size_needed, 0);
	WideCharToMultiByte(CP_UTF8, 0, wideStr.c_str(), (int)wideStr.length(), &narrowStr[0],
		size_needed, NULL, NULL);
	return narrowStr;
}

static void DisplayLastError() {
	DWORD error = GetLastError();
	if (error != 0) {
		LPWSTR msgBuffer = nullptr;
		DWORD formatResult = FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			error,
			0,
			(LPWSTR)&msgBuffer,
			0,
			NULL
		);
		if (formatResult > 0 && msgBuffer) {
			std::string errorMessage = WideStringToString(msgBuffer);
			std::cout << "Error: " << errorMessage << std::endl;
			LocalFree(msgBuffer);
		}
		else {
			std::cout << "Failed to format error message. Error code: " << error << std::endl;
		}
	}
	else {
		std::cout << "No error information available. Error code: " << error << std::endl;
	}
}

void handle_aborts(int signal_number) {
	std::cout << "Caught signal " << signal_number << " (SIGABRT)." << std::endl;
	DisplayLastError();
	std::abort();
}
#endif

float currentZoom = 1.0f;

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View& view) {
	//std::cout << zoom << std::endl;
	//sf::View view = window.getView();
	if (currentZoom * zoom <= 7.06f && currentZoom * zoom >= 0.531f) {
		const sf::Vector2f beforeCoord{ window.mapPixelToCoords(pixel) };

		view.zoom(zoom);
		currentZoom *= zoom;

		window.setView(view);
		const sf::Vector2f afterCoord{ window.mapPixelToCoords(pixel) };
		const sf::Vector2f offsetCoords{ beforeCoord - afterCoord };
		view.move(offsetCoords);
		window.setView(view);
	}
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