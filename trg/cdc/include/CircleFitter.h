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

#include "CLHEP/Geometry/Point3D.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCCFitter TRGCDCCircleFitter
#endif

namespace Belle2 {

/// A class to fit a TTrackBase object to a circle.
  class TRGCDCCircleFitter : public TRGCDCFitter {

  public:
    /// Constructor.
    explicit TRGCDCCircleFitter(const std::string& name);

    /// Destructor
    virtual ~TRGCDCCircleFitter();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

  public:// Modifiers
    /// Fitter
    virtual int fit(TRGCDCTrackBase&) const override;

  private:
    /// charge
    mutable double _charge;
    /// radius
    mutable double _radius;
    /// center
    mutable HepGeom::Point3D<double> _center;
  };

//-----------------------------------------------------------------------------


} // namespace Belle2

#endif
