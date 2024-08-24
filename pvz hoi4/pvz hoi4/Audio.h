#ifndef AUDIO_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define AUDIO_H

extern std::unordered_map<std::string, std::unordered_map<std::string, sf::Music*>> audios;
void initializeAudios(HINSTANCE hInstance);
#endif
