//# StepperFactory.cc
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
 * StepperFactory.cc
 *
 *  Created on: Nov 8, 2015
 *      Author: dmehring
 */

#include <Integrator/StepperFactory.h>

namespace kepler {

BSStepperType StepperFactory::createBurlirschStoer(
    PrecType maxAbsErr, PrecType maxRelErr
) {
    return BSStepperType(
        new bulirsch_stoer<vector<PrecType>, PrecType>(maxAbsErr, maxRelErr)
    );
}

RKCashKarpStepperType StepperFactory::createRKCashKarp(
    PrecType maxAbsErr, PrecType maxRelErr
) {
    auto stepper = make_controlled(maxAbsErr, maxRelErr, runge_kutta_cash_karp54< vector<PrecType>, PrecType>());
    return RKCashKarpStepperType(
        new controlled_runge_kutta<runge_kutta_cash_karp54< vector<PrecType>, PrecType>>(stepper)
    );
}

RKFehlbergStepperType StepperFactory::createRKFehlberg(
    PrecType maxAbsErr, PrecType maxRelErr
) {
    auto stepper = make_controlled(maxAbsErr, maxRelErr, runge_kutta_fehlberg78<vector<PrecType>, PrecType>());
    return RKFehlbergStepperType(
        new controlled_runge_kutta<runge_kutta_fehlberg78< vector<PrecType>, PrecType>>(stepper)
    );
}

}
