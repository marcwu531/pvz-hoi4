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
#include "Window.h"
#include "Colour.h"
#include "Resource.h"
#include "State.h"
#include <fstream>
#include <stdexcept>
#include <queue>
#include <shellapi.h>
#include <memory>
#include <nlohmann/json.hpp>
#include <fstream>
#include "Display.h"
#include "Async.h"
#include "Scene1.h"
#include "Audio.h"
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>

std::unique_ptr<sf::Music> loadMusicFromResource(HINSTANCE hInstance, int resourceId) {
    HRSRC resource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA);
    if (!resource) {
        throw std::runtime_error("Failed to find resource");
    }

    HGLOBAL resourceData = LoadResource(hInstance, resource);
    if (!resourceData) {
        throw std::runtime_error("Failed to load resource");
    }

    DWORD resourceSize = SizeofResource(hInstance, resource);
    const void* data = LockResource(resourceData);

    auto music = std::make_unique<sf::Music>();
    if (!music->openFromMemory(data, resourceSize)) {
        throw std::runtime_error("Failed to load music from memory");
    }
    return music;
}

std::map<std::string, std::map<std::string, std::unique_ptr<sf::Music>>> audios;

void initializeAudios(HINSTANCE hInstance) {
    audios["lawnbgm"]["6"] = loadMusicFromResource(nullHInstance, 112);
    audios["lawnbgm"]["1"] = loadMusicFromResource(nullHInstance, 113);
    audios["sounds"]["readysetplant"] = loadMusicFromResource(nullHInstance, 114);
    audios["soundtrack"]["battleofwuhan"] = loadMusicFromResource(nullHInstance, 115);
}