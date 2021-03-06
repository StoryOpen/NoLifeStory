//////////////////////////////////////////////////////////////////////////////
// NoLifeClient - Part of the NoLifeStory project                           //
// Copyright © 2014 Peter Atashian                                          //
//                                                                          //
// This program is free software: you can redistribute it and/or modify     //
// it under the terms of the GNU Affero General Public License as           //
// published by the Free Software Foundation, either version 3 of the       //
// License, or (at your option) any later version.                          //
//                                                                          //
// This program is distributed in the hope that it will be useful,          //
// but WITHOUT ANY WARRANTY; without even the implied warranty of           //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            //
// GNU Affero General Public License for more details.                      //
//                                                                          //
// You should have received a copy of the GNU Affero General Public License //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.    //
//////////////////////////////////////////////////////////////////////////////

#include "map.hpp"
#include "config.hpp"
#include "layer.hpp"
#include "view.hpp"
#include "foothold.hpp"
#include "background.hpp"
#include "time.hpp"
#include "portal.hpp"
#include "sound.hpp"
#include "player.hpp"
#include "log.hpp"
#include <nx/nx.hpp>
#include <nx/node.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <iostream>

namespace nl {
namespace map {
std::vector<std::string> all_maps;
node map_node;
node current, next;
std::string next_portal;
std::string current_name;
bool old_style;
void load(std::string name, std::string port) {
    if (name.size() < 9) { name.insert(0, 9 - name.size(), '0'); }
    if (name == current_name) { return player::respawn(port); }
    auto m = old_style ? map_node[name + ".img"]
                       : map_node[std::string("Map") + name[0]][name + ".img"];
    // If the map is invalid just ignore it
    if (!m) { return; }
    // Some maps link to other maps. I have no idea why.
    if (m["info"]["link"]) { return load(m["info"]["link"], port); }
    next = m;
    next_portal = port;
}
void add_random(node n) {
    auto name = n.name();
    // Ignore anything which isn't obviously a map
    if (name.size() != 13) { return; }
    all_maps.emplace_back(name.substr(0, name.size() - 4));
}
void init_random() {
    if (old_style) {
        for (auto n : map_node) { add_random(n); }
    } else {
        for (auto i = 0; i <= 9; ++i) {
            for (auto n : map_node["Map" + std::to_string(i)]) { add_random(n); }
        }
    }
    if (all_maps.empty()) { throw std::runtime_error{"No maps found!"}; }
}
void load_random() {
    std::random_device rand;
    std::uniform_int_distribution<size_t> dist(0, all_maps.size() - 1);
    load(all_maps[dist(rand)], "sp");
}
void load_now() {
    current = next;
    current_name = current.name();
    current_name.erase(current_name.find(".img"));
    config::map = current_name;
    log << "Loading map " << current.name() << std::endl;
    for (auto n : current["info"]) {
        log << '\t' << n.name() << ": " << n.get_string() << std::endl;
    }
    time::reset();
    music::play();
    layer::load();
    background::load();
    foothold::load();
    portal::load();
    player::respawn(next_portal);
    view::reset();
}
void init() {
    map_node = nx::map["Map"];
    if (!map_node) { throw std::runtime_error{"Theres no maps!"}; }
    old_style = !map_node["Map0"];
    init_random();
    load_random(); // Just in case loading the config'd map fails
    load(config::map, "sp");
    if (!next) { throw std::runtime_error{"No map to load!"}; }
    load_now();
}
void update() {
    player::update();
    if (next != current) load_now();
}
void render() {
    for (auto & b : backgrounds) { b.render(); }
    layer::render();
    for (auto & p : portals) { p.render(); }
    for (auto & b : foregrounds) { b.render(); }
    foothold::draw_lines();
    view::draw_edges();
}
}
}
