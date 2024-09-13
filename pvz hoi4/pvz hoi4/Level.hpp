#include <array>
#ifndef LEVEL_HPP
#define LEVEL_HPP

class Level {
public:
	class World_1 {
	public:
		class Level_1 {
		public:
			static const std::array<int, 1> zombies() {
				return { 0 };
			}
			static const int waves = 4;
			static const int spawnTier = 1;
		};

		class Level_2 {
		public:
			static const std::array<int, 1> zombies() {
				return { 0 };
			}
			static const int waves = 6;
			static const int spawnTier = 2;
		};

		class Level_3 {
		public:
			static const std::array<int, 1> zombies() {
				return { 0 };
			}
			static const int waves = 8;
			static const int spawnTier = 1;
		};
	};
};

#endif