#include <string>
#include <unordered_map>
#include <functional>
#include "State.hpp"
#include "State.h"

std::unordered_map<std::string, std::unordered_map<std::string, std::function<int()>>> state_int = {
        {"Taipei", {
            {"sx", []() { return State::Taipei::sx; }},
            {"lx", []() { return State::Taipei::lx; }},
            {"sy", []() { return State::Taipei::sy; }},
            {"ly", []() { return State::Taipei::ly; }}
        }},
        {"NewTaipei", {
            {"sx", []() { return State::NewTaipei::sx; }},
            {"lx", []() { return State::NewTaipei::lx; }},
            {"sy", []() { return State::NewTaipei::sy; }},
            {"ly", []() { return State::NewTaipei::ly; }}
        }},
    {"Taoyuan", {
            {"sx", []() { return State::Taoyuan::sx; }},
            {"lx", []() { return State::Taoyuan::lx; }},
            {"sy", []() { return State::Taoyuan::sy; }},
            {"ly", []() { return State::Taoyuan::ly; }}
        }},
    {"Keelung", {
            {"sx", []() { return State::Keelung::sx; }},
            {"lx", []() { return State::Keelung::lx; }},
            {"sy", []() { return State::Keelung::sy; }},
            {"ly", []() { return State::Keelung::ly; }}
        }}
};

std::unordered_map<std::string, std::unordered_map<std::string, std::function<std::string()>>> state_rgba = {
        {"Taipei", {
            {"RGBA", []() { return State::Taipei::RGBA(); }}
        }},
        {"NewTaipei", {
            {"RGBA", []() { return State::NewTaipei::RGBA(); }}
        }},
    {"Taoyuan", {
            {"RGBA", []() { return State::Taoyuan::RGBA(); }}
        }},
    {"Keelung", {
            {"RGBA", []() { return State::Keelung::RGBA(); }}
        }}
};

std::string clicking_state;

std::string States[4] = {"Taipei", "NewTaipei", "Taoyuan", "Keelung"};