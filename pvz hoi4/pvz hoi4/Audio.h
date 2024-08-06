#ifndef AUDIO_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define AUDIO_H

std::unique_ptr<sf::Music> loadMusicFromResource(HINSTANCE hInstance, int resourceId);
extern std::map<std::string, std::map<std::string, std::unique_ptr<sf::Music>>> audios;
void initializeAudios(HINSTANCE hInstance);
#endif
