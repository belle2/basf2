//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : SegmentHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track segment hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"

using namespace std;

namespace Belle2 {

TRGCDCSegmentHit::TRGCDCSegmentHit(const TCSegment & w)
    : TCCHit(w) {
}

TRGCDCSegmentHit::~TRGCDCSegmentHit() {
}

int
TRGCDCSegmentHit::sortById(const TRGCDCSegmentHit ** a,
			   const TRGCDCSegmentHit ** b) {
    if ((* a)->cell().id() > (* b)->cell().id())
        return 1;
    else if ((* a)->cell().id() == (* b)->cell().id())
        return 0;
    else
        return -1;
}

void
TRGCDCSegmentHit::dump(const std::string & message,
		       const std::string & prefix) const {
    TCCHit::dump(message, prefix);
}

const TRGCDCSegment &
TRGCDCSegmentHit::segment(void) const {
    return dynamic_cast<const TRGCDCSegment &>(cell());
}

} // namespace Belle2
