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
// Filename : DisplayDrawingArea.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to display CDC trigger objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifdef TRGCDC_DISPLAY

#define TRGCDC_SHORT_NAMES

#include <iostream>
#include <pangomm/init.h>
#include "trg/cdc/DisplayDrawingAreaRphi.h"

using namespace std;

namespace Belle2 {

  TRGCDCDisplayDrawingArea::TRGCDCDisplayDrawingArea(TRGCDCDisplay& w,
                                                     int size,
                                                     double outerR)
    : _w(w),
      _scale(double(size) / outerR / 2),
      _axial(true),
      _stereo(false),
      _wireName(false),
      _oldCDC(false)
  {
  }

  TRGCDCDisplayDrawingArea::~TRGCDCDisplayDrawingArea()
  {
  }

} // namespace Belle2

#endif
