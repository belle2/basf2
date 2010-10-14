//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CDCTriggerWireHitMC.h
// Section  : CDC Trigger
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a MC wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "trigger/cdc/CDCTriggerWireHitMC.h"

namespace Belle2 {

CDCTriggerWireHitMC::CDCTriggerWireHitMC(const CDCTriggerWire * w,
			     const CDCTriggerWireHit * wh)
: _wire(w),
  _hit(wh) {
}

CDCTriggerWireHitMC::~CDCTriggerWireHitMC() {
}

} // namespace Belle2

