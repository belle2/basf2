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
// Filename : WireHit.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a wire hit in CDC.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <stdlib.h>
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"

using namespace std;

namespace Belle2 {

  vector<TRGCDCWireHit*> TRGCDCWireHit::_all;

  TRGCDCWireHit::TRGCDCWireHit(const TRGCDCWire& w,
                               unsigned indexCDCHit,
                               unsigned indexCDCSimHit,
                               unsigned indexMCParticle,
                               float driftLeft,
                               float driftLeftError,
                               float driftRight,
                               float driftRightError,
                               int mcLRflag,
                               float fudgeFactor)
    :        TCCHit((TRGCDCCell&) w,
                    indexCDCHit,
                    indexCDCSimHit,
                    indexMCParticle,
                    driftLeft,
                    driftLeftError,
                    driftRight,
                    driftRightError,
                    mcLRflag,
                    fudgeFactor),
             _mc(), //2019/07/31 by ytlai
             _iCDCSimHit(indexCDCSimHit)
  {
  }

  TRGCDCWireHit::~TRGCDCWireHit()
  {
  }

  const TRGCDCWire&
  TRGCDCWireHit::wire(void) const
  {
    return dynamic_cast<const TRGCDCWire&>(cell());
  }

  int
  TRGCDCWireHit::sortByWireId(const TRGCDCWireHit** a,
                              const TRGCDCWireHit** b)
  {
    if ((* a)->cell().id() > (* b)->cell().id())
      return 1;
    else if ((* a)->cell().id() == (* b)->cell().id())
      return 0;
    else
      return -1;
  }

  void
  TRGCDCWireHit::removeAll(void)
  {
    while (_all.size())
      delete _all.back();
  }

  void*
  TRGCDCWireHit::operator new(size_t size)
  {
    void* p = malloc(size);
    _all.push_back(static_cast<TRGCDCWireHit*>(p));

//     cout << ">---------------------" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
//  cout << "> " << i << " " << _all[i] << endl;

    return p;
  }

  void
  TRGCDCWireHit::operator delete(void* t)
  {
    for (vector<TRGCDCWireHit*>::iterator it = _all.begin();
         it != _all.end();
         ++it) {
      if ((* it) == static_cast<TRGCDCWireHit*>(t)) {
        _all.erase(it);
        break;
      }
    }
    free(t);

//     cout << "<---------------------" << endl;
//     cout << "==> " << t << " erased" << endl;
//     for (unsigned i = 0; i < _all.size(); i++)
//  cout << "< " << i << " " << _all[i] << endl;
  }

} // namespace Belle2
