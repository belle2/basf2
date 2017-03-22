//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGChannel.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a trigger board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trg/trg/Channel.h"
#include "trg/trg/Board.h"

using namespace std;

namespace Belle2 {

  TRGChannel::TRGChannel(const std::string& name,
                         const TRGBoard& sender,
                         const TRGBoard& receiver)
    : _name(name),
      _sender(sender),
      _receiver(receiver),
      _data(0)
  {
  }

  TRGChannel::~TRGChannel()
  {
  }

} // namespace Belle2
