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

#include "obj.hpp"
#include "time.hpp"
#include "view.hpp"
#include <nx/nx.hpp>
#include <iostream>
#include <string>

namespace nl {
obj::obj(node n) {
    x = n["x"];
    y = n["y"];
    z = n["z"];
    zm = n["zM"];
    zid = std::stoi(n.name());
    rx = n["rx"];
    ry = n["ry"];
    cx = n["cx"];
    cy = n["cy"];
    flow = n["flow"];
    flip = n["f"].get_bool();
    spr = nx::map["Obj"][n["oS"] + ".img"][n["l0"]][n["l1"]][n["l2"]];
    if (flow & 1 && !cx) { cx = 1000; }
    if (flow & 2 && !cy) { cy = 1000; }
}
void obj::render() {
    auto flags = sprite::relative;
    if (flip) { flags |= sprite::flipped; }
    auto dx = x;
    auto dy = y;
    if (flow & 1) {
        dx += static_cast<int>(rx * 5 * time::delta_total);
        flags |= sprite::tilex;
    }
    if (flow & 2) {
        dy += static_cast<int>(ry * 5 * time::delta_total);
        flags |= sprite::tiley;
    }
    spr.draw(dx, dy, flags, cx, cy);
}
}
