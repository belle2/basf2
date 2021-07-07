/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <limits>
#include <iostream>
#include <cmath>
#include "trg/trg/Clock.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Utilities.h"

using namespace std;

namespace Belle2_GDL {

  //...This should be moved to GDL module...
  const Belle2::TRGClock GDLSystemClock("GDL system clock", 0, 125.000);
}

namespace Belle2 {

  TRGClock::TRGClock(const string& name,
                     double offset,
                     double frequency)
    : _name(name),
      _source(0),
      _multi(1),
      _div(1),
      _offset(offset),
      _frequency(frequency),
      _cycle(1000 / frequency),
//     _min(int(TRGTime::min() / frequency)),
//     _max(int(TRGTime::max() / frequency)),
//     _min(numeric_limits<int>::min() + 100),
//     _max(numeric_limits<int>::max() - 100),
      _min(numeric_limits<int>::min() / 16),
      _max(numeric_limits<int>::max() / 16),
      _clockCounter(0)
  {

    if (this != & Belle2_GDL::GDLSystemClock) {
      if (Belle2_GDL::GDLSystemClock.minTiming() > minTiming())
        _min = int((Belle2_GDL::GDLSystemClock.minTiming() - _offset)
                   / _cycle);
      if (Belle2_GDL::GDLSystemClock.maxTiming() < maxTiming())
        _max = int((Belle2_GDL::GDLSystemClock.maxTiming() - _offset)
                   / _cycle);
    }
  }

  TRGClock::TRGClock(const string& name,
                     const TRGClock& source,
                     unsigned multiplicationFactor,
                     unsigned divisionFactor)
    : _name(name),
      _source(& source),
      _multi(multiplicationFactor),
      _div(divisionFactor),
      _offset(source._offset),
      _frequency(source._frequency *
                 double(multiplicationFactor) /
                 double(divisionFactor)),
      _cycle(source._cycle /
             double(multiplicationFactor) *
             double(divisionFactor)),
      _min(source._min * int(multiplicationFactor) / int(divisionFactor)),
      _max(source._max * int(multiplicationFactor) / int(divisionFactor)),
      _clockCounter(0)
  {
  }

  TRGClock::~TRGClock()
  {
    if (_clockCounter)
      delete _clockCounter;
  }

  void
  TRGClock::dump(const string&,
                 const string& pre) const
  {
    cout << pre << _name;
    if (_source) {
      cout << ":sync'd to " << _source->name() << endl;
      cout << pre << "    multiplication factor=" << _multi << endl;
      cout << pre << "    division factor      =" << _div << endl;
    } else {
      cout << endl;
    }

    cout << pre << "    offset   :" << _offset << endl
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
  TRGClock::position(double t) const
  {
#ifdef TRG_DEBUG
    if ((t < minTiming()) || (t > maxTiming()))
      cout << "TRGClock::unit(" << _name
           << ") !!! out of time window : min=" << minTiming()
           << ",max=" << maxTiming() << ",given value=" << t << endl;
//    cout << "t,offset,unit=" << t << "," << _offset << "," << int((t
//    - _offset) / _cycle) << endl;
#endif
    return floor((t - _offset) / _cycle);
  }

  double
  TRGClock::absoluteTime(int t) const
  {
    return double(t) * _cycle + _offset;
  }

  double
  TRGClock::overShoot(double t) const
  {
    return (t - _offset) - _cycle * double(position(t));
  }

  TRGTime
  TRGClock::minTRGTime(bool edge) const
  {
    TRGTime a(minTiming(), edge, * this, _name + "_min");
    return a;
  }

  TRGTime
  TRGClock::maxTRGTime(bool edge) const
  {
    TRGTime a(maxTiming(), edge, * this, _name + "_max");
    return a;
  }

  double
  TRGClock::phase(double a) const
  {
//  return overShoot(a) / _cycle * 360;
    const double pos = (a - _offset) / _cycle;
    const double pos0 = double(int(pos));
    const double dif = pos - pos0;
    // std::cout << "a,offset,pos,pos0,dif=" << a << "," << _offset << ","
    //        << pos << "," << pos0 << "," << dif << std::endl;
    return dif * 360;
  }

  const TRGSignalVector&
  TRGClock::clockCounter(void) const
  {

    if (_clockCounter)
      return * _clockCounter;

    _clockCounter = new TRGSignalVector(name() + " counter", * this);

    //...5 bit clock counter...
    unsigned cicle = 2;
    for (int i = 0; i < 5; i++) {
      TRGSignal s("tmp", * this);

      //...From 0 to 1280*5...
//      const int maxN = 1280 * 5;
      const int maxN = 400;
      for (int j = 0; j < maxN; j++) {
        if (((j - cicle / 2) % cicle) == 0)
          s.set(j, j + cicle / 2);
      }

      //...Append this bit...
      s.name("ClockCounterBit" + TRGUtilities::itostring(i));
      (* _clockCounter) += s;

      //...Double cicle...
      cicle *= 2;
    }

    return * _clockCounter;
  }

} // namespace Belle2
