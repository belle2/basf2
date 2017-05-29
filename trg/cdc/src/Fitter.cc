//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Fitter.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object.
//-----------------------------------------------------------------------------
// $Log$
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

