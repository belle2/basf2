//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : GDLTime.cc
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a signal timing in the trigger system.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trigger/gdl/GDLClock.h"
#include "trigger/gdl/GDLTime.h"
#include "trigger/gdl/GDLSignal.h"

using namespace std;

namespace Belle2 {

// double
// GDLTime::min(void) {
//     return Belle2_GDL::GDLSystemClock.minTiming();
// }

// double
// GDLTime::max(void) {
//     return Belle2_GDL::GDLSystemClock.maxTiming();
// }

// GDLTime
// GDLTime::minGDLTime(bool edge) {
//     GDLTime a(GDLTime::min(), edge, "timeMin");
//     return a;
// }

// GDLTime
// GDLTime::maxGDLTime(bool edge) {
//     GDLTime a(GDLTime::max(), edge, "timeMax");
//     return a;
// }

// GDLTime
// GDLTime::minGDLTime(bool edge, const GDLClock & clock) {
//     GDLTime a(GDLTime::min(), edge, clock, "timeMin");
//     return a;
// }

// GDLTime
// GDLTime::maxGDLTime(bool edge, const GDLClock & clock) {
//     GDLTime a(GDLTime::max(), edge, clock, "timeMax");
//     return a;
// }

// GDLTime::GDLTime(double timing, bool edge, const std::string & name) :
//     _time(timing),
//     _edge(edge),
//     _clock(0),
//     _name(name) {
// }

GDLTime::GDLTime(const GDLTime & t) :
    _time(t._time),
    _edge(t._edge),
    _clock(t._clock),
    _name(t._name) {
}

GDLTime::GDLTime(double timing,
		 bool edge,
		 const GDLClock & clock,
		 const std::string & name) :
    _time(clock.time(timing)),
    _edge(edge),
    _clock(& clock),
    _name(name) {
}

GDLTime::~GDLTime() {
}

GDLSignal
GDLTime::operator&(const GDLTime & left) const {
    GDLSignal t0(* this);
    GDLSignal t1(left);
    return t0 & t1;
}

GDLSignal
GDLTime::operator&(const GDLSignal & left) const {
    GDLSignal t0(* this);
    return t0 & left;
}

GDLSignal
GDLTime::operator|(const GDLTime & left) const {
    GDLSignal t0(* this);
    GDLSignal t1(left);
    return t0 | t1;
}

GDLSignal
GDLTime::operator|(const GDLSignal & left) const {
    GDLSignal t0(* this);
    return t0 | left;
}

void
GDLTime:: dump(const std::string & msg,
	       const std::string & pre) const {
    cout << pre << "time(clock):";

    if (_edge)
	cout << " o ";
    else
	cout << " . ";

    cout << _time;
    if (msg.find("name") != string::npos ||
	msg.find("detail") != string::npos)
	cout << "(" << _name << ")";
    cout << endl;
}

bool
GDLTime::sortByTime(const GDLTime & a, const GDLTime & b) {
    if (a.time() < b.time()) {
	return true;
    }
    else if (a.time() == b.time()) {
	if (a.edge() & (! b.edge()))
	    return true;
	else if (a.edge() == b.edge())
	    return true;
    }
    return false;
}

} // namespace Belle2
