#include <iostream>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <windows.h>

#include "Audio.h"

#define NSOUND

static sf::Music* loadMusicFromResource(HINSTANCE hInstance, int resourceId, size_t& resourceSize) {
	auto music = new sf::Music();

	if (HRSRC resource = FindResource(hInstance, MAKEINTRESOURCE(resourceId), RT_RCDATA))
		if (HGLOBAL resourceData = LoadResource(hInstance, resource))
			if (resourceSize = SizeofResource(hInstance, resource))
				if (const void* data = LockResource(resourceData))
					if (music->openFromMemory(data, resourceSize))
						return music;

	delete music;
	return nullptr;
}

std::unordered_map<std::string, std::unordered_map<std::string, sf::Music*>> audios;

void initializeAudios(HINSTANCE hInstance) {
	size_t resourceSize = 0;

	auto loadAndStoreMusic = [&](const std::string& category, const std::string& name, int resourceId) {
		auto music = loadMusicFromResource(hInstance, resourceId, resourceSize);
		if (music) audios[category][name] = std::move(music);
	};

	loadAndStoreMusic("lawnbgm", "6", 112);
	loadAndStoreMusic("lawnbgm", "1", 113);
	loadAndStoreMusic("sounds", "readysetplant", 114);
	loadAndStoreMusic("soundtrack", "battleofwuhan", 115);

#ifdef NSOUND
	for (auto& categoryPair : audios) {
		for (auto& musicPair : categoryPair.second) {
			musicPair.second->setVolume(0);
		}
	}
#else
	audios["soundtrack"]["battleofwuhan"]->setVolume(25);
#endif
}

void cleanupAudios() {
	for (auto& categoryPair : audios) {
		for (auto& musicPair : categoryPair.second) {
			delete musicPair.second;
		}
	}
	audios.clear();
}