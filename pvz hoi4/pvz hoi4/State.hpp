#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>

class State {
public:
    class Taipei {
    public:
        static std::string const RGBA() {
            return "89 171 196 255";
        };
        static int const r = 89;
        static int const sx = 4699;
        static int const lx = 4709;
        static int const sy = 986;
        static int const ly = 997;
    };
    class Hualian {
    public:
        static std::string const RGBA() {
            return "112 128 144 255";
        }
        static int const r = 112;
        static int const sx = 5700;
        static int const lx = 5710;
        static int const sy = 1000;
        static int const ly = 1010;
    };
};