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
// Filename : TRGOpticalLink.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a trigger board
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include <iostream>
#include "trg/trg/OpticalLink.h"
#include "trg/trg/Clock.h"

using namespace std;

namespace Belle2 {

  TRGOpticalLink::TRGOpticalLink(const std::string& name,
                                 const TRGClock& clock) :
    _name(name),
    _clock(& clock)
  {
  }

  TRGOpticalLink::~TRGOpticalLink()
  {
  }

} // namespace Belle2
