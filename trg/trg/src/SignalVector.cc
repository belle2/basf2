//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGSignalVector.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bundle of digitized signals.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

// #include <algorithm>
// #include <functional>
#include <iostream>
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"
#include "trg/trg/SignalVector.h"

using namespace std;

namespace Belle2 {

TRGSignalVector::TRGSignalVector() : _name("unknown") {
}

TRGSignalVector::TRGSignalVector(const string & name) : _name(name) {
}

TRGSignalVector::TRGSignalVector(const TRGSignalVector & t) :
    std::vector<const TRGSignal *>(),
    _name("CopyOf" + t._name) {
}

TRGSignalVector::TRGSignalVector(const TRGSignal & t) :
    _name("VectorOf" + t.name()) {
    _signals.push_back(& t);
}

TRGSignalVector::~TRGSignalVector() {
}

void
TRGSignalVector::dump(const string & msg,
                const string & pre) const {
    cout << pre << _name << ":" << _signals.size() << " signal(s)" << endl;
    for (unsigned i = 0; i < size(); i++)
	(* this)[i]->dump(msg, "    " + pre);
    if (msg.find("detail") != string::npos)
	for (unsigned i = 0; i < _signals.size(); i++)
	    _signals[i]->dump(msg, "    " + pre);
}

// TRGSignalVector
// TRGSignalVector::operator+(const TRGSignalVector & left) const {
//     TRGSignalVector t;
//     t.push_back(this);
//     t.push_back(& left);
//     return t;
// }

TRGSignalVector &
TRGSignalVector::operator+=(const TRGSignal & left) {
    _signals.push_back(& left);
    return * this;
}

bool
TRGSignalVector::active(void) const {
    for (unsigned i = 0; i < size(); i++)
	if ((* this)[i]->active())
	    return true;
    return false;
}

} // namespace Belle2
