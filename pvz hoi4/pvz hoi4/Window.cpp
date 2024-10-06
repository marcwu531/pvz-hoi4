#include <iostream>
#include <nlohmann/json.hpp>
#include <queue>
#include <SFML/Graphics.hpp>
#include <windows.h>

#include "Colour.h"
#include "Display.h"
#include "State.h"
#include "Window.h"

#ifdef RUN_DEBUG
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

void zoomViewAt(sf::RectangleShape& worldRect, float zoom) {
	// Get the current scale of the worldRect
	sf::Vector2f currentScale = worldRect.getScale();

	// Get the world position of the mouse before zooming
	sf::Vector2f beforeZoomCoord = window.mapPixelToCoords(sf::Mouse::getPosition(window));

	// Use the original scaling logic you provided
	float scaleF = currentScale.x + --zoom;
	sf::Vector2f newScale = sf::Vector2f(scaleF, scaleF);

	// Clamp the scale values to be within limits (0.531 to 7.06)
	newScale.x = std::clamp(newScale.x, 0.531f, 7.06f);
	newScale.y = std::clamp(newScale.y, 0.531f, 7.06f);

	// Only apply scaling if the new scale is different
	if (newScale != currentScale) {
		sf::Vector2f rectPosition = worldRect.getPosition();

		// Calculate the size of the rectangle based on the current scale
		sf::Vector2f currentSize = worldRect.getGlobalBounds().getSize();

		// Calculate the center of the rectangle
		sf::Vector2f rectCenter = rectPosition + currentSize / 2.0f;

		// Calculate the offset of the mouse position relative to the rectangle's center
		sf::Vector2f mouseOffset = beforeZoomCoord - rectCenter;

		// Apply the new scale to the worldRect
		worldRect.setScale(newScale);

		// Get the size after scaling
		sf::Vector2f newSize = worldRect.getGlobalBounds().getSize();

		// Calculate the new center of the rectangle after scaling
		sf::Vector2f newRectCenter = rectPosition + newSize / 2.0f;

		// Move the rectangle to maintain the mouse offset from the center
		worldRect.setPosition(rectCenter - mouseOffset + newRectCenter);
	}
}

std::array<std::string, 2> clickingState(sf::Image& image, float mouseInMapPosX, float mouseInMapPosY) {
	int x = static_cast<int>(mouseInMapPosX), y = static_cast<int>(mouseInMapPosY);

	auto rgba = getRGBA(og_world_image, x, y);
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