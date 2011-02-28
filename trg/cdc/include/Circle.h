//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Circle.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a circle
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCCircle_FLAG_
#define TRGCDCCircle_FLAG_

#include "trg/trg/Point2D.h"
#include "trg/cdc/TrackBase.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCCircle TRGCDCCircle
#endif

namespace Belle2 {

class TRGCDCHoughPlane;

/// A class to represent a circle.
class TRGCDCCircle : public TRGCDCTrackBase {

  public:

    /// Constructor with a circle center, assuming the origin is on a circle.
    TRGCDCCircle(const TRGPoint2D & center,
                 float charge,
                 const TRGCDCHoughPlane & plane);

    /// Destructor
    virtual ~TRGCDCCircle();

  public:

    /// returns the circle center.
    const TRGPoint2D & center(void) const;

    /// returns Pt.
    float pt(void) const;

    /// returns radius.
    float radius(void) const;

    /// dumps debug information.
    void dump(const std::string & message = std::string(""),
              const std::string & prefix = std::string("")) const;

  private:

    /// Position of the circle center.
    TRGPoint2D _center;

    /// Radius.
    float _radius;

    /// Hough plane.
    const TRGCDCHoughPlane * _plane;
};

//-----------------------------------------------------------------------------

inline
float
TRGCDCCircle::pt(void) const {
    const double ConstantAlpha = 222.376063; // for 1.5T
    return _radius / ConstantAlpha;
}

inline
const TRGPoint2D &
TRGCDCCircle::center(void) const {
    return _center;
}

inline
float
TRGCDCCircle::radius(void) const {
    return _radius;
}

} // namespace Belle2

#endif /* TRGCDCCircle_FLAG_ */
