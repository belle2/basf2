/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//-----------------------------------------------------------------------------

#define TRGCDC_SHORT_NAMES

#include "trg/cdc/Fitter.h"
#include "trg/cdc/TrackBase.h"

namespace Belle2 {

  TRGCDCFitter::TRGCDCFitter(const std::string& name) : _name(name)
  {
  }

  TRGCDCFitter::~TRGCDCFitter()
  {
  }

  void
  TRGCDCFitter::fitDone(TCTBase& t) const
  {
    t._fitted = true;
  }

  void
  TRGCDCFitter::dump(const std::string&, const std::string&) const
  {
  }

} // namespace Belle2

