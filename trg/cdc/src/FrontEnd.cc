//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : FrontEnd.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES
#define TRGCDCFrontEnd_INLINE_DEFINE_HERE

#include "trg/cdc/FrontEnd.h"

using namespace std;

namespace Belle2 {

TRGCDCFrontEnd::TRGCDCFrontEnd(const std::string & name,
				       const TRGClock & clock)
    : TRGBoard(name, clock) {
}

TRGCDCFrontEnd::~TRGCDCFrontEnd() {
}

} // namespace Belle2
