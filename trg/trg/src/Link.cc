//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGLink.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a trigger board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGLink_INLINE_DEFINE_HERE
#define TRIGGER_SHORT_NAMES

#include <iostream>
#include "trg/trg/Link.h"
#include "trg/trg/Clock.h"

using namespace std;

namespace Belle2 {

TRGLink::TRGLink(const std::string & name, const TRGClock & clock) :
    _name(name),
    _clock(& clock) {
}

TRGLink::~TRGLink() {
}

} // namespace Belle2
