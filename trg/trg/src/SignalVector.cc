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

#define TRG_SHORT_NAMES

#include <string>
#include <cstring>
#include <algorithm>
#include <limits>
#include <iostream>
#include "trg/trg/Constants.h"
#include "trg/trg/Utilities.h"
#include "trg/trg/Clock.h"
#include "trg/trg/Signal.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/State.h"

using namespace std;

namespace Belle2 {

TRGSignalVector::TRGSignalVector() : _name("unknown") {
}

TRGSignalVector::TRGSignalVector(const string & name) : _name(name) {
}

TRGSignalVector::TRGSignalVector(const TRGSignalVector & t) :
    std::vector<TRGSignal>(),
    _name("CopyOf" + t._name) {
}

TRGSignalVector::TRGSignalVector(const TRGSignal & t) :
    _name("VectorOf" + t.name()) {
    push_back(t);
}

TRGSignalVector::~TRGSignalVector() {
}

void
TRGSignalVector::dump(const string & msg,
		      const string & pre) const {
    cout << pre << _name << ":" << size() << " signal(s)" << endl;
    for (unsigned i = 0; i < size(); i++)
	(* this)[i].dump(msg, "    " + pre);
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
    push_back(left);
    return * this;
}

bool
TRGSignalVector::active(void) const {
    for (unsigned i = 0; i < size(); i++)
	if ((* this)[i].active())
	    return true;
    return false;
}

std::vector<int>
TRGSignalVector::stateChanges(void) const {
    std::vector<int> list;
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++) {
	vector<int> a =(* this)[i].stateChanges();
	for (unsigned j = 0; j < a.size(); j++)
	    list.push_back(a[j]);
    }

    //...Sorting...
    std::sort(list.begin(), list.end());

    //...Remove duplicated clock...
    std::vector<int> list2;
    int last = numeric_limits<int>::min();
    for (unsigned i = 0; i < list.size(); i++) {
	const int j = list[i];
	if (j != last) {
	    list2.push_back(j);
	    last = j;
	}
    }
    return list2;
}

TRGState
TRGSignalVector::state(int clockPosition) const {
    std::vector<bool> list;
    const unsigned n = size();
    for (unsigned i = 0; i < n; i++)
	list.push_back((* this)[i].state(clockPosition));
    return TRGState(list);
}

} // namespace Belle2
