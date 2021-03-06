//# Math.h
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
 * Math.h
 *
 *  Created on: Sep 5, 2015
 *      Author: dmehring
 */

#ifndef KEPLER_MATH_H_
#define KEPLER_MATH_H_

#include <Math/Types.h>

namespace kepler {

// are x and y relatively close to each other?
bool near(PrecType x, PrecType y, PrecType epsilon);

// are x and y absolutely close to each other?
bool nearAbs(PrecType x, PrecType y, PrecType epsilon);

}

#endif
