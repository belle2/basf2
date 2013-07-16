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

TRGBoard::TRGBoard(const std::string & name,
		   const TRGClock & systemClock,
		   const TRGClock & dataClock,
		   const TRGClock & userClockInput,
		   const TRGClock & userClockOutput)
    : _name(name),
      _clockSystem(& systemClock),
      _clockData(& dataClock),
      _clockUserInput(& userClockInput),
      _clockUserOutput(& userClockOutput),
      _inputChannels(0),
      _outputChannels(0) {
}

TRGBoard::~TRGBoard() {
}

void
TRGBoard::dump(const string & msg, const string & pre) const {
    cout << pre << _name;
    if (msg.find("detail") != string::npos) {
	cout << endl;
	cout << pre << "[System clock:" << _clockSystem->name() << "]" << endl;
	cout << pre << "[Data clock:" << _clockData->name() << "]" << endl;
	cout << pre << "[Input user clock:" << _clockUserInput->name() << "]"
	     << endl;
	cout << pre << "[Output user clock:" << _clockUserOutput->name()
	     << "]";
    }
    cout << endl;
}

} // namespace Belle2
