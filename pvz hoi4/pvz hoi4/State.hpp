#ifndef STATE_HPP
#define STATE_HPP

#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <cstring>

class State {
public:
    class Taiwan {
    public:
        class Taipei {
        public:
            static std::string const RGBA() {
                return "30 144 255 255"; //Taipei City (台北市): #1E90FF (Dodger Blue)
            };
            static int const sx = 4702;
            static int const lx = 4703;
            static int const sy = 988;
            static int const ly = 991;
        };

        class NewTaipei {
        public:
            static std::string const RGBA() {
                return "70 130 180 255"; //New Taipei City (新北市): #4682B4 (Steel Blue)
            }
            static int const sx = 4700;
            static int const lx = 4708;
            static int const sy = 986;
            static int const ly = 995;
        };

        class Taoyuan {
        public:
            static std::string const RGBA() {
                return "50 205 50 255"; //Taoyuan City (桃園市): #32CD32 (Lime Green)
            }
            static int const sx = 4695;
            static int const lx = 4701;
            static int const sy = 989;
            static int const ly = 997;
        };

        class Taichung {
        public:
            static std::string const RGBA() {
                return "255 215 0 255"; //Taichung City (台中市): #FFD700 (Gold)
            }
            static int const sx = 4687;
            static int const lx = 4700;
            static int const sy = 1000;
            static int const ly = 1007;
        };

        class Tainan {
        public:
            static std::string const RGBA() {
                return "255 140 0 255"; //Tainan City (台南市): #FF8C00 (Dark Orange)
            }
            static int const sx = 4680;
            static int const lx = 4688;
            static int const sy = 1018;
            static int const ly = 1025;
        };

        class Kaohsiung {
        public:
            static std::string const RGBA() {
                return "220 20 60 255"; //Kaohsiung City (高雄市): #DC143C (Crimson)
            }
            static int const sx = 4682;
            static int const lx = 4695;
            static int const sy = 1017;
            static int const ly = 1032;
        };

        class Keelung {
        public:
            static std::string const RGBA() {
                return "138 43 226 255"; //Keelung City (基隆市): #8A2BE2 (Blue Violet)
            }
            static int const sx = 4705;
            static int const lx = 4705;
            static int const sy = 988;
            static int const ly = 989;
        };

        class Hsinchu_City {
        public:
            static std::string const RGBA() {
                return "0 206 209 255"; //Hsinchu City (新竹市): #00CED1 (Dark Turquoise)
            }
            static int const sx = 4693;
            static int const lx = 4694;
            static int const sy = 994;
            static int const ly = 995;
        };

        class Hsinchu_County {
        public:
            static std::string const RGBA() {
                return "32 178 170 255"; //Hsinchu County (新竹縣): #20B2AA (Light Sea Green)
            }
            static int const sx = 4694;
            static int const lx = 4700;
            static int const sy = 992;
            static int const ly = 999;
        };

        class Miaoli {
        public:
            static std::string const RGBA() {
                return "127 255 0 255"; //Miaoli County (苗栗縣): #7FFF00 (Chartreuse)
            }
            static int const sx = 4689;
            static int const lx = 4698;
            static int const sy = 996;
            static int const ly = 1002;
        };

        class Changhua {
        public:
            static std::string const RGBA() {
                return "255 69 0 255"; //Changhua County (彰化縣): #FF4500 (Orange Red)
            }
            static int const sx = 4683;
            static int const lx = 4689;
            static int const sy = 1005;
            static int const ly = 1010;
        };

        class Nantou {
        public:
            static std::string const RGBA() {
                return "139 69 19 255"; //Nantou County (南投縣): #8B4513 (Saddle Brown)
            }
            static int const sx = 4689;
            static int const lx = 4699;
            static int const sy = 1004;
            static int const ly = 1016;
        };

        class Yunlin {
        public:
            static std::string const RGBA() {
                return "106 90 205 255"; //Yunlin County (雲林縣): #6A5ACD (Slate Blue)
            }
            static int const sx = 4682;
            static int const lx = 4690;
            static int const sy = 1011;
            static int const ly = 1015;
        };

        class Chiayi_City {
        public:
            static std::string const RGBA() {
                return "123 104 238 255"; //Chiayi City (嘉義市): #7B68EE (Medium Slate Blue)
            }
            static int const sx = 4686;
            static int const lx = 4686;
            static int const sy = 1016;
            static int const ly = 1016;
        };

        class Chiayi_County {
        public:
            static std::string const RGBA() {
                return "178 34 34 255"; //Chiayi County (嘉義縣): #B22222 (Fire Brick)
            }
            static int const sx = 4682;
            static int const lx = 4692;
            static int const sy = 1014;
            static int const ly = 1020;
        };

        class Pingtung {
        public:
            static std::string const RGBA() {
                return "255 99 71 255"; //Pingtung County (屏東縣): #FF6347 (Tomato)
            }
            static int const sx = 4686;
            static int const lx = 4692;
            static int const sy = 1027;
            static int const ly = 1042;
        };

        class Yilan {
        public:
            static std::string const RGBA() {
                return "46 139 87 255"; //Yilan County (宜蘭縣): #2E8B57 (Sea Green)
            }
            static int const sx = 4700;
            static int const lx = 4707;
            static int const sy = 994;
            static int const ly = 1001;
        };

        class Hualien {
        public:
            static std::string const RGBA() {
                return "72 209 204 255"; //Hualien County (花蓮縣): #48D1CC (Medium Turquoise)
            }
            static int const sx = 4695;
            static int const lx = 4706;
            static int const sy = 1001;
            static int const ly = 1021;
        };

        class Taitung {
        public:
            static std::string const RGBA() {
                return "218 165 32 255"; //Taitung County (台東縣): #DAA520 (Golden Rod)
            }
            static int const sx = 4691;
            static int const lx = 4701;
            static int const sy = 1017;
            static int const ly = 1036;
        };
    };
};

#endif