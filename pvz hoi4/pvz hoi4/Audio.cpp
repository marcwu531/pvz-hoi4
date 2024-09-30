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
	loadAndStoreMusic("sounds", "awooga", 146);
	loadAndStoreMusic("sounds", "buzzer", 147);
	loadAndStoreMusic("sounds", "finalwave", 148);
	loadAndStoreMusic("sounds", "lawnmower", 149);
	loadAndStoreMusic("sounds", "plant", 150);
	loadAndStoreMusic("sounds", "plant2", 151);
	loadAndStoreMusic("sounds", "points", 152);
	loadAndStoreMusic("sounds", "siren", 153);
	loadAndStoreMusic("sounds", "splat", 154);
	loadAndStoreMusic("sounds", "splat2", 155);
	loadAndStoreMusic("sounds", "splat3", 156);
	loadAndStoreMusic("sounds", "winmusic", 157);
	loadAndStoreMusic("sounds", "seedlift", 160);

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

void playRngAudio(std::string type) {
	if (type == "plant") {
		if (rand() % 2 == 0) {
			audios["sounds"]["plant"]->play();
		}
		else {
			audios["sounds"]["plant2"]->play();
		}
	}
	else if (type == "splat") {
		if (rand() % 3 == 0) {
			audios["sounds"]["splat"]->play();
		}
		else if (rand() % 2 == 0) {
			audios["sounds"]["splat2"]->play();
		}
		else {
			audios["sounds"]["splat3"]->play();
		}
	}
}