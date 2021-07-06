/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include "trg/trg/Clock.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"

using namespace std;

namespace Belle2 {

  TRGTime::TRGTime(const TRGTime& t) :
    _time(t._time),
    _edge(t._edge),
    _clock(t._clock),
    _name(t._name)
  {
  }

  TRGTime::TRGTime(double timing,
                   bool edge,
                   const TRGClock& clock,
                   const std::string& name) :
    _time(clock.position(timing)),
    _edge(edge),
    _clock(& clock),
    _name(name)
  {
  }

  TRGTime::TRGTime(int timing,
                   bool edge,
                   const TRGClock& clock,
                   const std::string& name) :
    _time(timing),
    _edge(edge),
    _clock(& clock),
    _name(name)
  {
  }

  TRGTime::~TRGTime()
  {
  }

  TRGSignal
  TRGTime::operator&(const TRGTime& left) const
  {
    TRGSignal t0(* this);
    TRGSignal t1(left);
    return t0 & t1;
  }

  TRGSignal
  TRGTime::operator&(const TRGSignal& left) const
  {
    TRGSignal t0(* this);
    return t0 & left;
  }

  TRGSignal
  TRGTime::operator|(const TRGTime& left) const
  {
    TRGSignal t0(* this);
    TRGSignal t1(left);
    return t0 | t1;
  }

  TRGSignal
  TRGTime::operator|(const TRGSignal& left) const
  {
    TRGSignal t0(* this);
    return t0 | left;
  }

  void
  TRGTime:: dump(const std::string& msg,
                 const std::string& pre) const
  {
    cout << pre << "time(clock):";

    if (_edge)
      cout << " o ";
    else
      cout << " . ";

    cout << _time;
    cout << " abs=" << _clock->absoluteTime(_time);
    if (msg.find("name") != string::npos ||
        msg.find("detail") != string::npos) {
      cout << " (" << _name << ")";
    }
    cout << endl;
  }

  bool
  TRGTime::sortByTime(const TRGTime& a, const TRGTime& b)
  {
    if (a.time() < b.time()) {
      return true;
    } else if (a.time() == b.time()) {
      if (a.edge() & (! b.edge()))
        return true;
      else if (a.edge() == b.edge())
        return true;
    }
    return false;
  }

  const TRGClock&
  TRGTime::clock(const TRGClock& c)
  {
    const double t = _clock->absoluteTime(_time);
    _clock = & c;
    _time = c.position(t);
    return * _clock;
  }

} // namespace Belle2
