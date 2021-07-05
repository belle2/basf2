/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
#include "trg/trg/Clock.h"
#include "trg/trg/Board.h"
#include "trg/trg/Channel.h"

using namespace std;

namespace Belle2 {

  TRGBoard::TRGBoard(const std::string& name,
                     const TRGClock& systemClock,
                     const TRGClock& dataClock,
                     const TRGClock& userClockInput,
                     const TRGClock& userClockOutput)
    : _name(name),
      _clockSystem(& systemClock),
      _clockData(& dataClock),
      _clockUserInput(& userClockInput),
      _clockUserOutput(& userClockOutput),
      _inputChannels(0),
      _outputChannels(0)
  {
  }

  TRGBoard::~TRGBoard()
  {
  }

  void
  TRGBoard::dump(const string& msg, const string& pre) const
  {
    cout << pre << _name;
    const string tab = pre + "    ";
    if (msg.find("detail") != string::npos) {
      cout << endl;
      cout << pre << "System clock:" << _clockSystem->name() << endl;
      cout << pre << "Data clock:" << _clockData->name() << endl;
      cout << pre << "Input user clock:" << _clockUserInput->name() << endl;
      cout << pre << "Output user clock:" << _clockUserOutput->name()
           << endl;
      cout << pre << "Input channels" << endl;
      for (unsigned i = 0; i < _inputChannels.size(); i++) {
        cout << tab << _inputChannels[i]->name() << endl;
      }
      cout << pre << "Output channels" << endl;
      for (unsigned i = 0; i < _outputChannels.size(); i++) {
        cout << tab << _outputChannels[i]->name() << endl;
      }
    }
    cout << endl;
  }

} // namespace Belle2
