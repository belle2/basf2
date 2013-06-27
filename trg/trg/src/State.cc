//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGState.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a state of multi bits
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <algorithm>
#include <functional>
#include <iostream>
#include "trg/trg/State.h"

using namespace std;

namespace Belle2 {

TRGState::TRGState(unsigned bitSize)
    : _size(bitSize),
      _state(0) {
    unsigned n = _size / sizeof(unsigned);
    if (_size % sizeof(unsigned)) ++n;
    _state = (unsigned *) calloc(n, sizeof(unsigned));
}

TRGState::TRGState(vector<bool> states)
    : _size(0),
      _state(0) {

    _size = states.size();
    unsigned n = _size / sizeof(unsigned);
    if (_size % sizeof(unsigned)) ++n;
    _state = (unsigned *) calloc(n, sizeof(unsigned));

    for (unsigned i = 0; i < _size; i++) {
	const unsigned wp = i / sizeof(unsigned);
	const unsigned bp = i % sizeof(unsigned);
	if (states[i])
	    _state[wp] |= (1 << bp);
	else
	    _state[wp] &= ~(1 << bp);

// 	cout << "size,given,states=" << _size << "," << states[i] << ","
// 	     << _state[wp] << endl;

    }
}

TRGState::~TRGState() {
    free(_state);
}

void
TRGState::dump(const string & msg,
	       const string & pre) const {
    cout << pre << "size=" << _size << ",";
    unsigned n = _size / sizeof(unsigned);
    if (_size % sizeof(unsigned)) ++n;
    for (unsigned i = 0; i < n; i++) {
	cout << i << ":" << hex << _state[i] << dec;
	if (i < n - 1)
	    cout << ",";
    }
    cout << endl;
}

} // namespace Belle2

namespace std {

    ostream &
    operator<<(ostream & out, const Belle2::TRGState & s) {
	for (unsigned i = 0; i < s.size(); i++) {
	    const unsigned j = s.size() - i - 1;
	    if (s.active(j))
		out << "1";
	    else
		out << "0";
	}
	return out;
    }

}
