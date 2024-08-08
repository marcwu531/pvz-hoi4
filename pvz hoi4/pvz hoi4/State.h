#ifndef STATE_H    // To make sure you don't declare the function more than once by including the header multiple times.
#define STATE_H

extern std::map<std::string, std::map<std::string, std::function<int()>>> state_int;
extern std::map<std::string, std::map<std::string, std::function<std::string()>>> state_rgba;
extern std::string clicking_state;
extern std::string flag;
extern std::map<std::string, std::vector<std::string>> Regions;

#endif