#ifndef WINDOW_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define WINDOW_H

void zoomViewAt(sf::Vector2i pixel, sf::RenderWindow& window, float zoom, sf::View view);
std::array<std::string, 2> clickingState(sf::Image image, float mouseInMapPosX, float mouseInMapPosY);
extern const std::vector<sf::Keyboard::Key> konamiCode;
bool isKonamiCodeEntered(const std::queue<sf::Keyboard::Key>& inputs);
void stdcoutMap(std::map<int, int>* map);
#endif
