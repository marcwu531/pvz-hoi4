#include <string>
#include <unordered_map>
#include <functional>
#include "State.hpp"
#include "State.h"

#define DEFINE_STATE_INT(region, state) \
    {#state, { \
        {"sx", []() { return State::region::state::sx; }}, \
        {"lx", []() { return State::region::state::lx; }}, \
        {"sy", []() { return State::region::state::sy; }}, \
        {"ly", []() { return State::region::state::ly; }} \
    }}

#define DEFINE_STATE_RGBA(region, state) \
    {#state, { \
        {"RGBA", []() { return State::region::state::RGBA(); }} \
    }}

std::unordered_map<std::string, std::unordered_map<std::string, std::function<int()>>> state_int = {
    DEFINE_STATE_INT(Taiwan, Taipei),
    DEFINE_STATE_INT(Taiwan, NewTaipei),
    DEFINE_STATE_INT(Taiwan, Taoyuan),
    DEFINE_STATE_INT(Taiwan, Taichung),
    DEFINE_STATE_INT(Taiwan, Tainan),
    DEFINE_STATE_INT(Taiwan, Kaohsiung),
    DEFINE_STATE_INT(Taiwan, Keelung),
    DEFINE_STATE_INT(Taiwan, Hsinchu_City),
    DEFINE_STATE_INT(Taiwan, Hsinchu_County),
    DEFINE_STATE_INT(Taiwan, Miaoli),
    DEFINE_STATE_INT(Taiwan, Changhua),
    DEFINE_STATE_INT(Taiwan, Nantou),
    DEFINE_STATE_INT(Taiwan, Yunlin),
    DEFINE_STATE_INT(Taiwan, Chiayi_City),
    DEFINE_STATE_INT(Taiwan, Chiayi_County),
    DEFINE_STATE_INT(Taiwan, Pingtung),
    DEFINE_STATE_INT(Taiwan, Yilan),
    DEFINE_STATE_INT(Taiwan, Hualien),
    DEFINE_STATE_INT(Taiwan, Taitung)
};

std::unordered_map<std::string, std::unordered_map<std::string, std::function<std::string()>>> state_rgba = {
    DEFINE_STATE_RGBA(Taiwan, Taipei),
    DEFINE_STATE_RGBA(Taiwan, NewTaipei),
    DEFINE_STATE_RGBA(Taiwan, Taoyuan),
    DEFINE_STATE_RGBA(Taiwan, Taichung),
    DEFINE_STATE_RGBA(Taiwan, Tainan),
    DEFINE_STATE_RGBA(Taiwan, Kaohsiung),
    DEFINE_STATE_RGBA(Taiwan, Keelung),
    DEFINE_STATE_RGBA(Taiwan, Hsinchu_City),
    DEFINE_STATE_RGBA(Taiwan, Hsinchu_County),
    DEFINE_STATE_RGBA(Taiwan, Miaoli),
    DEFINE_STATE_RGBA(Taiwan, Changhua),
    DEFINE_STATE_RGBA(Taiwan, Nantou),
    DEFINE_STATE_RGBA(Taiwan, Yunlin),
    DEFINE_STATE_RGBA(Taiwan, Chiayi_City),
    DEFINE_STATE_RGBA(Taiwan, Chiayi_County),
    DEFINE_STATE_RGBA(Taiwan, Pingtung),
    DEFINE_STATE_RGBA(Taiwan, Yilan),
    DEFINE_STATE_RGBA(Taiwan, Hualien),
    DEFINE_STATE_RGBA(Taiwan, Taitung)
};

#undef DEFINE_STATE_INT
#undef DEFINE_STATE_RGBA

std::string clicking_state;
std::string flag;

std::unordered_map<std::string, std::vector<std::string>> Regions = {
    {"Taiwan", {
        "Taipei", "NewTaipei", "Taoyuan", "Taichung", "Tainan", "Kaohsiung",
        "Keelung", "Hsinchu_City", "Hsinchu_County", "Miaoli", "Changhua",
        "Nantou", "Yunlin", "Chiayi_City", "Chiayi_County", "Pingtung",
        "Yilan", "Hualien", "Taitung"
    }}
};