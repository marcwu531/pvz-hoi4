#ifndef ASYNC_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define ASYNC_H

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
#include <fstream>
#include "Scene1.h"

extern std::atomic<bool> running;
extern std::atomic<bool> blinkMap_loadingCoords;
extern std::atomic<bool> blinkMap_readyToDraw;
extern std::atomic<bool> loadFlag_readyToDraw;
extern std::atomic<bool> loadLevelStart_readyToDraw;
extern int fps;
extern int scene;
extern std::vector<std::array<int, 2>> targetCoords;
extern int blinkCoords[2];
void asyncBlinkMap();
void asyncLoadFlag();
void asyncLoadLevelStart();
void asyncPvzSceneUpdate();
extern std::thread thread_asyncBlinkMap;
extern std::thread thread_asyncLoadFlag;
extern std::thread thread_asyncLoadLevelStart;
extern std::thread thread_asyncPacketMove;
void stopAllThreads();
#endif
