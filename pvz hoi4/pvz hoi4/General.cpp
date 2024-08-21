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

#include "Window.h"
#include "Colour.h"
#include "Resource.h"
#include "State.h"
#include "Display.h"
#include "Async.h"
#include "Scene1.h"
#include "Audio.h"
#include "Json.h"
#include "General.h"

void mapShift(std::map<int, int>& myMap) {
    std::map<int, int> tempMap;
    int newKey = 0;

    for (auto it = myMap.begin(); it != myMap.end(); ++it) {
        if (it->second == -1) {
            continue;
        }

        tempMap[newKey] = it->second;
        ++newKey;
    }

    myMap = std::move(tempMap);
}