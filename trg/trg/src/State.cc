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

const unsigned
TRGState::_su = sizeof(unsigned);

const unsigned
TRGState::_bsu = 8 * sizeof(unsigned);

TRGState::TRGState(unsigned bitSize)
    : _size(bitSize),
      _n(0),
      _state(0) {
    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    if (_n)
	_state = (unsigned *) calloc(_n, _su);
}

TRGState::TRGState(unsigned bitSize, const bool * const s)
    : _size(bitSize),
      _n(0),
      _state(0) {

    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    _state = (unsigned *) calloc(_n, _su);

    for (unsigned i = 0; i < _size; i++) {
	const unsigned wp = i / _bsu;
	const unsigned bp = i % _bsu;
	if (s[i])
	    _state[wp] |= (1 << bp);
	else
	    _state[wp] &= ~(1 << bp);
    }
}

TRGState::TRGState(vector<bool> states)
    : _size(0),
      _n(0),
      _state(0) {

    _size = states.size();
    _n = _size / _bsu;
    if (_size % _bsu) ++_n;
    _state = (unsigned *) calloc(_n, _su);

    for (unsigned i = 0; i < _size; i++) {
	const unsigned wp = i / _bsu;
	const unsigned bp = i % _bsu;
	if (states[i])
	    _state[wp] |= (1 << bp);
	else
	    _state[wp] &= ~(1 << bp);

// 	cout << "size,given,states=" << _size << "," << states[i] << ","
// 	     << _state[wp] << endl;

    }
}

TRGState::~TRGState() {
    if (_state)
	free(_state);
}

void
TRGState::dump(const string & msg,
	       const string & pre) const {
//    if (msg.find("bin") != string::npos) {
	cout << pre << "size=" << _size << ",";
	for (unsigned i = 0; i < _size; i++) {
	    const unsigned j = _size - i - 1;
	    if ((j % 8) == 7)
		cout << "_";
	    if ((* this)[j])
		cout << "1";
	    else
		cout << "0";
	}
	cout << dec << endl;
//    }
//  else {
// 	cout << pre << "size=" << _size << ",0x";
// 	bool first = true;
// 	for (unsigned i = 0; i < _n; i++) {
// 	    const unsigned j = _n - i - 1;
// 	    if (((j % 4) == 3) && (! first))
// 		cout << "_";
// 	    cout << hex << _state[j];
// 	    first = false;
// 	}
// 	cout << dec << endl;

// 	cout << pre << "size=" << _size << ",0x";
// 	bool first = true;
// 	for (unsigned i = 0; i < _n; i++) {
// 	    const unsigned j = _n - i - 1;
// 	    if (! first)
// 		cout << "_";
// 	    cout << hex << _state[j];
// 	    cout << "(";
// 	    for (unsigned k = 0; k < 4; k++) {
// 		unsigned c = ((_state[j] >> (4 - k + 1) * 8) & 0xff);
// 		if (c < 0x10)
// 		    cout << ".";
// 		cout << c;
// 	    }
// 	    cout << ")";
// 	    first = false;
// 	}
// 	cout << dec << endl;
//    }
}

TRGState &
TRGState::operator+=(const TRGState & a) {

    //...Check final size...
    const unsigned sizeNew = _size + a.size();
    const unsigned sizeMax = _n * _bsu;
    const unsigned oldSize = _size;

    //...Necessary to extend...
    if (sizeNew > sizeMax) {
	unsigned nNew = sizeNew / _bsu;
	if (sizeNew % _bsu) ++nNew;
	unsigned * tmp = (unsigned *) calloc(nNew, _su);
	for (unsigned i = 0; i < _n; i++) {
	    tmp[i] = _state[i];
	}
	if (_state)
	    free(_state);
	_state = tmp;
	_n = nNew;
    }
    _size = sizeNew;

    for (unsigned i = 0; i < a.size(); i++) {
	unsigned j = i + oldSize;
	const unsigned wp = j / _bsu;
	const unsigned bp = j % _bsu;
	if (a[i])
	    _state[wp] |= (1 << bp);
	else
	    _state[wp] &= ~(1 << bp);

// 	cout << "size,given,states=" << _size << "," << states[i] << ","
// 	     << _state[wp] << endl;

    }

    return (* this);
}

TRGState
TRGState::subset(unsigned s, unsigned n) const {
    vector<bool> stack;
    for (unsigned i = s; i < s + n; i++) {
	if ((* this)[i])
	    stack.push_back(true);
	else
	    stack.push_back(false);
    }
    return TRGState(stack);
}

TRGState &
TRGState::operator=(const TRGState & a) {
    if (_state)
	free(_state);

    _size = a._size;
    _n = a._n;
    _state = (unsigned *) calloc(_n, _su);
    for (unsigned i = 0; i < _n; i++) {
	_state[i] = a._state[i];
    }

    return * this;
}

bool
TRGState::operator<(const TRGState & a) const {
    const unsigned long long n0(* this);
    const unsigned long long n1(a);
    if (n0 < n1)
	return true;
    return false;
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
