//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : CircleFitter.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a circle.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCCIRCLEFITTER_FLAG_
#define TRGCDCCIRCLEFITTER_FLAG_

#include "trg/cdc/Fitter.h"
#include "trg/cdc/Helix.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCCFitter TRGCDCCircleFitter
#endif

namespace Belle2 {

  class TLink;
  class TRGCDCCircle;

/// A class to fit a TTrackBase object to a circle.
  class TRGCDCCircleFitter : public TRGCDCFitter {

  public:
    /// Constructor.
    TRGCDCCircleFitter(const std::string& name);

    /// Destructor
    virtual ~TRGCDCCircleFitter();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    virtual int fit(TRGCDCTrackBase&) const;

  private:
    mutable double _charge;
    mutable double _radius;
    mutable HepGeom::Point3D<double> _center;
  };

//-----------------------------------------------------------------------------


} // namespace Belle2

#endif
