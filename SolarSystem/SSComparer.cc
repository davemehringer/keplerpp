//# SSComparer.cc
//# Copyright (C) 2016 David Mehringer
//# 
//#     This program is free software: you can redistribute it and/or modify
//#     it under the terms of the GNU General Public License as published by
//#     the Free Software Foundation, either version 3 of the License, or
//#     (at your option) any later version.
//# 
//#     This program is distributed in the hope that it will be useful,
//#     but WITHOUT ANY WARRANTY; without even the implied warranty of
//#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#     GNU General Public License for more details.
//# 
//#     You should have received a copy of the GNU General Public License
//#     along with this program.  If not, see <http://www.gnu.org/licenses/>.
//# 
/*
 * SSComparer.cc
 *
 *  Created on: Sep 28, 2015
 *      Author: dmehring
 */

#include <SolarSystem/SSComparer.h>

#include <SolarSystem/JPLHorizonsRetriever.h>
#include <Physics/NBodySystem.h>

#include <iomanip>

namespace kepler {

void compareStateVectors(const NBodySystem& system) {
    auto n = system.getNumberOfBodies();
    Vvector x(n), v(n);
    system.getXV(x, v);
    JPLHorizonsRetriever jhr(JPLHorizonsRetriever::VECTORS);
    auto bodies = system.getBodies();
    auto t = system.getTime(DAY);
    for (const auto& b: bodies) {
        jhr.add(b.name, "sun", t, JPLHorizonsRetriever::ECLIPTIC);
    }
    jhr.retrieve();
    NBodySystem jplSystem(jhr.getBodies(), system.getTime(SECOND), SECOND);
    auto jbodies = jplSystem.getBodies();
    auto xiter = begin(x);
    auto viter = begin(v);
    int w = 14;
    for (const auto& jb: jbodies) {
        cout << jb.name << endl;
        const auto& jx = jb.x;
        auto d = *xiter - jx;
        cout << "diff x "<< setprecision(8) << setw(w) << fixed  << d[0] << " y " << setw(w) << d[1] << " z "
            << setw(w) << d[2] << endl;
        const auto& jv = jb.v;
        auto dv = *viter - jv;
        cout << "diff v "<< setprecision(8) << setw(w) << fixed  << dv[0] << " y " << setw(w) << dv[1] << " z "
                    << setw(w) << dv[2] << endl;
        ++xiter;
        ++viter;
    }
}



}
