//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGClock.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a clock.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <limits>
#include <iostream>
#include "trg/trg/Clock.h"
#include "trg/trg/Time.h"

using namespace std;

namespace Belle2_GDL {

    //...This should be moved to GDL module...
    const Belle2::TRGClock GDLSystemClock(5.9, 125.000, "GDL system clock");
}

namespace Belle2 {

TRGClock::TRGClock(double offset,
                   double frequency,
                   const std::string & name) :
    _offset(offset),
    _frequency(frequency),
    _cycle(1000 / frequency),
//     _min(int(TRGTime::min() / frequency)),
//     _max(int(TRGTime::max() / frequency)),
//     _min(numeric_limits<int>::min() + 100),
//     _max(numeric_limits<int>::max() - 100),
    _min(numeric_limits<int>::min() / 16),
    _max(numeric_limits<int>::max() / 16),
    _name(name) {

    if (this != & Belle2_GDL::GDLSystemClock) {
        if (Belle2_GDL::GDLSystemClock.minTiming() > minTiming())
            _min = int(Belle2_GDL::GDLSystemClock.minTiming() / _cycle);
        if (Belle2_GDL::GDLSystemClock.maxTiming() < maxTiming())
            _max = int(Belle2_GDL::GDLSystemClock.maxTiming() / _cycle);
    }
}

TRGClock::~TRGClock() {
}

void
TRGClock::dump(const std::string & message,
               const std::string & pre) const {
    cout << pre << _name << ":" << endl
         << pre << "    offset   :" << _offset << endl
         << pre << "    freq(MHz):" << _frequency << endl
         << pre << "    cycle(ns):" << _cycle << endl
         << pre << "    min pos  :" << _min << endl
         << pre << "    max pos  :" << _max << endl
         << pre << "    min(ns)  :" << minTiming() << endl
         << pre << "    max(ns)  :" << maxTiming() << endl;

    cout << pre << "    numeric limit of int (min,max="
         << numeric_limits<int>::min() << ","
         << numeric_limits<int>::max() << ")" << endl;
}

int
TRGClock::time(double t) const {
    if ((t < minTiming()) || (t > maxTiming()))
        cout << "TRGClock::unit(" << _name
             << ") !!! out of time window : min=" << minTiming()
             << ",max=" << maxTiming() << ",given value=" << t << endl;

//     cout << "t,offset,unit=" << t << "," << _offset << "," << int((t - _offset) / _cycle) << endl;

    return int((t - _offset) / _cycle);
}

TRGTime
TRGClock::minTRGTime(bool edge) const {
    TRGTime a(minTiming(), edge, * this, _name + "_min");
    return a;
}

TRGTime
TRGClock::maxTRGTime(bool edge) const {
    TRGTime a(maxTiming(), edge, * this, _name + "_max");
    return a;
}

} // namespace Belle2
