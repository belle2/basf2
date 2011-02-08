//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Merger.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a CDC front-end board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRGCDCMerger_INLINE_DEFINE_HERE
#define TRGCDC_SHORT_NAMES

#include "trg/cdc/Merger.h"

using namespace std;

namespace Belle2 {

TRGCDCMerger::TRGCDCMerger(const std::string & name,
                                   const TRGClock & clock)
    : TRGBoard(name, clock) {
}

TRGCDCMerger::~TRGCDCMerger() {
}

} // namespace Belle2
