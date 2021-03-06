//# Symplectic.h
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

 *      Author: dmehring
 */

#ifndef KEPLER_symplectic_H
#define KEPLER_symplectic_H

#include <Integrator/Integrator.h>
#include <IO/IO.h>

#include <AccelerationCalculator/AccelerationCalculator.h>

#include <boost/numeric/odeint.hpp>

#include <Math/Vector.h>

using namespace boost::numeric::odeint;

namespace kepler {

using container_type = Vvector;

class Symplectic: public Integrator {
public:

    Symplectic() = delete;

    Symplectic(
        NBodySystem* system, AccelerationCalculator* accCalc,
        TimeStepManager *const &tsm,
        const IntegrationEnder *const& ie
    );

    virtual ~Symplectic();

    virtual void integrate();

protected:

    Vvector _ai, _af;

    virtual void _step() = 0;

    struct coord {
        coord(const vector<PrecType>& masses) : _m(masses) {}

        void operator()(const container_type &p , container_type &dqdt) const {
            auto iter = begin(dqdt);
            auto miter = begin(_m);
            for (const auto& pp: p) {
                pp.divide(*iter, *miter);
                //*iter = pp/(*miter);
                ++iter;
                ++miter;
            }
        }
        vector<PrecType> _m;
    };

    struct momentum {
        momentum(AccelerationCalculator* aCalc, const vector<PrecType>& masses
    ) : _aCalc(aCalc), _m(masses), _a(masses.size()) {}

        void operator()( const Vvector &q , Vvector &dpdt ) const {
            _aCalc->compute(_a, &q);
            auto i = 0;
            for (auto& pt: dpdt) {
                _a[i].times(pt, _m[i]);
                //pt = _m[i]*_a[i];
                ++i;
            }
        }
        AccelerationCalculator* _aCalc;
        vector<PrecType> _m;
        mutable Vvector _a;
    };

    pair<coord, momentum> _odeSystem;
    pair<Vvector, Vvector> _inState, _outState;

};
}

#endif
