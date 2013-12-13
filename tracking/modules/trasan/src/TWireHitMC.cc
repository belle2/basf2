//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TWireHitMC.h
// Section  : CDC tracking trasan
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a MC wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "tracking/modules/trasan/TWireHitMC.h"

namespace Belle {

TWireHitMC::TWireHitMC(const TWire * w,
                       const TWireHit * wh)
  : _wire(w),
    _hit(wh) {
}

TWireHitMC::~TWireHitMC() {
}

} // namespace Belle
