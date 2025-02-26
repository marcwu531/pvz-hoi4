#ifndef WINDOW_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define WINDOW_H

void zoomViewAt(sf::RectangleShape& worldRect, float zoom);
std::array<std::string, 2> clickingState(sf::Image& image, float mouseInMapPosX, float mouseInMapPosY);
extern const std::vector<sf::Keyboard::Key> konamiCode;
bool isKonamiCodeEntered(const std::queue<sf::Keyboard::Key>& inputs);
void stdcoutMap(std::unordered_map<int, int>* map);

#ifdef RUN_DEBUG
extern void AttachConsole();
extern void handle_aborts(int signal_number);
#endif
#endif
