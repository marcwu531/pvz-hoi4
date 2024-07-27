#ifndef STATE_HPP
#define STATE_HPP

#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>

class State {
public:
    class Taipei {
    public:
        static std::string const RGBA() {
            return "30 144 255 255";
        };
        static int const sx = 4702;
        static int const lx = 4703;
        static int const sy = 988;
        static int const ly = 991;
    };

    class NewTaipei {
    public:
        static std::string const RGBA() {
            return "70 130 180 255";
        }
        static int const sx = 4700;
        static int const lx = 4708;
        static int const sy = 986;
        static int const ly = 995;
    };

    class Taoyuan {
    public:
        static std::string const RGBA() {
            return "50 205 50 255";
        }
        static int const sx = 4695;
        static int const lx = 4701;
        static int const sy = 989;
        static int const ly = 997;
    };

    class Taichung {
    public:
        static std::string const RGBA() {
            return "255 215 0 255";
        }
        static int const sx = 4687;
        static int const lx = 4700;
        static int const sy = 1000;
        static int const ly = 1007;
    };

    class Tainan {
    public:
        static std::string const RGBA() {
            return "255 140 0 255";
        }
        static int const sx = 4680;
        static int const lx = 4688;
        static int const sy = 1018;
        static int const ly = 1025;
    };

    class Kaohsiung {
    public:
        static std::string const RGBA() {
            return "220 20 60 255";
        }
        static int const sx = 4682;
        static int const lx = 4695;
        static int const sy = 1017;
        static int const ly = 1032;
    };

    class Keelung {
    public:
        static std::string const RGBA() {
            return "138 43 226 255";
        }
        static int const sx = 4705;
        static int const lx = 4705;
        static int const sy = 988;
        static int const ly = 989;
    };

    class Hsinchu_City {
    public:
        static std::string const RGBA() {
            return "0 206 209 255";
        }
        static int const sx = 4693;
        static int const lx = 4694;
        static int const sy = 994;
        static int const ly = 995;
    };

    class Hsinchu_County {
    public:
        static std::string const RGBA() {
            return "32 178 170 255";
        }
        static int const sx = 4694;
        static int const lx = 4700;
        static int const sy = 992;
        static int const ly = 999;
    };

    class Miaoli {
    public:
        static std::string const RGBA() {
            return "127 255 0 255";
        }
        static int const sx = 4689;
        static int const lx = 4698;
        static int const sy = 996;
        static int const ly = 1002;
    };
};

#endif // HEADER_FILE_NAME_HPP

/*
Taipei City (̨����): #1E90FF (Dodger Blue)
New Taipei City (�±���): #4682B4 (Steel Blue)
Taoyuan City (�҈@��): #32CD32 (Lime Green)
Taichung City (̨����): #FFD700 (Gold)
Tainan City (̨����): #FF8C00 (Dark Orange)
Kaohsiung City (������): #DC143C (Crimson)
Keelung City (��¡��): #8A2BE2 (Blue Violet)
Hsinchu City (������): #00CED1 (Dark Turquoise)
Hsinchu County (����h): #20B2AA (Light Sea Green)
Miaoli County (�����h): #7FFF00 (Chartreuse)
Changhua County (�û��h): #FF4500 (Orange Red)
Nantou County (��Ͷ�h): #8B4513 (Saddle Brown)
Yunlin County (��ֿh): #6A5ACD (Slate Blue)
Chiayi City (���x��): #7B68EE (Medium Slate Blue)
Chiayi County (���x�h): #B22222 (Fire Brick)
Pingtung County (���|�h): #FF6347 (Tomato)
Yilan County (���m�h): #2E8B57 (Sea Green)
Hualien County (��ɏ�h): #48D1CC (Medium Turquoise)
Taitung County (̨�|�h): #DAA520 (Golden Rod)
*/