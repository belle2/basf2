//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : WireHitMC.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a MC wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "trg/cdc/WireHitMC.h"

namespace Belle2 {

  TRGCDCWireHitMC::TRGCDCWireHitMC(const TRGCDCWire* w,
                                   const TRGCDCWireHit* wh)
    : _wire(w),
      _hit(wh)
  {
  }

  TRGCDCWireHitMC::~TRGCDCWireHitMC()
  {
  }

} // namespace Belle2

