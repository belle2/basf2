//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGOpticalLink.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a trigger board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trg/trg/OpticalLink.h"
#include "trg/trg/Clock.h"

using namespace std;

namespace Belle2 {

  TRGOpticalLink::TRGOpticalLink(const std::string& name,
                                 const TRGClock& clock) :
    _name(name),
    _clock(& clock)
  {
  }

  TRGOpticalLink::~TRGOpticalLink()
  {
  }

} // namespace Belle2
