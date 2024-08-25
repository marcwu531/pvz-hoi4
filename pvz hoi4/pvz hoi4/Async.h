#ifndef ASYNC_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define ASYNC_H

extern std::atomic<bool> running;
extern std::atomic<bool> blinkMap_loadingCoords;
extern std::atomic<bool> blinkMap_readyToDraw;
extern std::atomic<bool> loadFlag_readyToDraw;
extern std::atomic<bool> loadLevelStart_readyToDraw;
extern int fps;
extern int scene;
extern std::vector<sf::Vector2i> targetCoords;
extern sf::Vector2i blinkCoords;
void asyncBlinkMap();
void asyncLoadFlag();
void asyncLoadLevelStart();
void asyncPvzSceneUpdate();
extern std::thread thread_asyncBlinkMap;
extern std::thread thread_asyncLoadFlag;
extern std::thread thread_asyncLoadLevelStart;
extern std::thread thread_asyncPvzSceneUpdate;
void stopAllThreads();
extern bool pvzPacketOnSelected;
extern std::mutex plantsMutex, zombiesMutex, projsMutex, vanishProjsMutex, sunsMutex;
#endif
