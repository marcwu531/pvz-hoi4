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
			//static const int totalMeta = 4000;
		};

		class Level_2 {
		public:
			static const std::array<int, 2> zombies() {
				return { 0, 1 };
			}
			static const int waves = 6;
		};
	};
};

#endif