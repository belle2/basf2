//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGBoard.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a trigger board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trg/trg/Board.h"
#include "trg/trg/Clock.h"

using namespace std;

namespace Belle2 {

TRGBoard::TRGBoard(const std::string & name, const TRGClock & clock) :
    _name(name),
    _clock(& clock) {
}

TRGBoard::~TRGBoard() {
}

void
TRGBoard::dump(const string &, const string & pre) const {
    cout << pre << _name << "(" << _clock->name() << ")" << endl;
}

} // namespace Belle2
