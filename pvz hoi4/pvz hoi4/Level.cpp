#include <unordered_map>

#include "Level.h"

#include "Level.hpp"

std::unordered_map<int, std::unordered_map<int, std::vector<int>>> world_level_zombies = {
    {
        1, {
            {1, []() {
                auto arr = Level::World_1::Level_1::zombies();
                return std::vector<int>(arr.begin(), arr.end());
            }()},
            {2, []() {
                auto arr = Level::World_1::Level_2::zombies();
                return std::vector<int>(arr.begin(), arr.end());
            }()},
            {3, []() {
                auto arr = Level::World_1::Level_3::zombies();
                return std::vector<int>(arr.begin(), arr.end());
            }()}
        }
    }
};

std::unordered_map<int, std::unordered_map<int, int>> world_level_waves = {
    {
        1, {
            {1, Level::World_1::Level_1::waves},
            {2, Level::World_1::Level_2::waves},
            {3, Level::World_1::Level_3::waves}
        }
    }
};

std::unordered_map<int, std::unordered_map<int, int>> world_level_spawnTier = {
    {
        1, {
            {1, Level::World_1::Level_1::spawnTier},
            {2, Level::World_1::Level_2::spawnTier},
            {3, Level::World_1::Level_2::spawnTier}
        }
    }
};