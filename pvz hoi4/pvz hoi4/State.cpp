#include <string>
#include <unordered_map>
#include <functional>
#include "State.hpp"
#include "State.h"

std::unordered_map<std::string, std::unordered_map<std::string, std::function<int()>>> state_int = {
        {"Taipei", {
            {"r", []() { return State::Taipei::r; }},
            {"sx", []() { return State::Taipei::sx; }},
            {"lx", []() { return State::Taipei::lx; }},
            {"sy", []() { return State::Taipei::sy; }},
            {"ly", []() { return State::Taipei::ly; }}
        }},
        {"Hualian", {
            {"r", []() { return State::Hualian::r; }},
            {"sx", []() { return State::Hualian::sx; }},
            {"lx", []() { return State::Hualian::lx; }},
            {"sy", []() { return State::Hualian::sy; }},
            {"ly", []() { return State::Hualian::ly; }}
        }}
};

std::unordered_map<std::string, std::unordered_map<std::string, std::function<std::string()>>> state_rgba = {
        {"Taipei", {
            {"RGBA", []() { return State::Taipei::RGBA(); }}
        }},
        {"Hualian", {
            {"RGBA", []() { return State::Hualian::RGBA(); }}
        }}
};