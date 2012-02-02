//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGBitStream.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a bit stream.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trg/trg/Utilities.h"
#include "trg/trg/BitStream.h"

using namespace std;

namespace Belle2 {

TRGBitStream::TRGBitStream() :
    _name("unknown"),
    _sizeMax(0),
    _size(0),
    _stream(0) {
}

TRGBitStream::TRGBitStream(int size, const std::string & name) :
    _name(name),
    _sizeMax(size),
    _size(0),
    _stream(0) {
}

TRGBitStream::TRGBitStream(const TRGBitStream & a) :
    _name("CopyOf" + a._name),
    _sizeMax(a._sizeMax),
    _size(a._size),
    _stream(0) {
    for (unsigned i = 0; i < a._stream.size(); i++)
	_stream.push_back(new unsigned(* a._stream[i]));
}

TRGBitStream::~TRGBitStream() {
    for (unsigned i = 0; i < _stream.size(); i++)
	delete _stream[i];
}

void
TRGBitStream::dump(const string & msg,
		   const string & pre) const {

    const string tab = "    ";

    cout << pre << _name << endl;
    for (unsigned i = 0; i < _stream.size(); i++) {
	cout << pre << tab;
	if (i == _stream.size() - 1) {
	    const unsigned last = _size % sizeof(unsigned);
	    if (last)
		cout << TRGUtilities::streamDisplay(* _stream[i], 0, last);
	}
	else {
	    cout << TRGUtilities::streamDisplay(* _stream[i], 0, 31);
	}
	cout << endl;
    }
}

void
TRGBitStream::append(bool a) {
    if (_sizeMax) {
	if (_size >= _sizeMax) {
	    cout << "TRGBitStream::append !!! stream is full :current size="
		 << _size << ",max size=" << _sizeMax << endl;
	    return;
	}
    }

    const unsigned storageSize = sizeof(unsigned) * 8 * _stream.size();
    ++_size;

    if (_size <= storageSize) {
	if (a) {
	    const unsigned position = _size % sizeof(unsigned);
	    unsigned & last = * _stream.back();
	    last |= (1 << position);

	    cout << "    posi,last=" << position << "," << last << endl;
	}
    }
    else {
	if (a)
	    _stream.push_back(new unsigned(1));
	else
	    _stream.push_back(new unsigned(0));
    }
}


} // namespace Belle2
