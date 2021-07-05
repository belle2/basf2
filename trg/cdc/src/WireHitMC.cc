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
      _hit(wh),
      _distance(), // 2019/07/31 by ytlai
      _energy(),
      _hep(),
      _leftRight()
  {
  }

  TRGCDCWireHitMC::~TRGCDCWireHitMC()
  {
  }

} // namespace Belle2

