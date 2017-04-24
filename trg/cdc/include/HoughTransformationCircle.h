//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughTransformationCircle.h
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent circle Hough transformation.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHoughTransformationCircle_FLAG_
#define TRGCDCHoughTransformationCircle_FLAG_

#include "trg/cdc/HoughTransformation.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCHTransformationCircle TRGCDCHoughTransformationCircle
#endif

namespace Belle2 {

/// A class to represent circle Hough transformation.
  class TRGCDCHoughTransformationCircle
    : public TRGCDCHoughTransformation {

  public:
    /// Contructor.
    TRGCDCHoughTransformationCircle(const std::string& name);

    /// Destructor
    virtual ~TRGCDCHoughTransformationCircle();

  public:// Selectors
    /// returns Y coordinate in a Hough parameter plane.
    virtual float y(float xReal, float yReal, float x) const;

    /// returns true if Y diverges in given region.
    virtual bool diverge(float xReal, float yReal, float x0, float x1) const;

    /// returns true if Y diverges in given region.
    virtual bool positiveDiverge(float xReal, float yReal, float x0, float x1)
    const;

    /// returns true if Y diverges in given region.
    virtual bool negativeDiverge(float xReal, float yReal, float x0, float x1)
    const;

    /// returns Point2D(phi, r) of a circle in real plane.
    virtual TRGPoint2D circleCenter(const TRGPoint2D&) const;

    /// converts Point2D(phi, r) in real plane into Point2D(phi, r) in Hough plane.
    virtual TRGPoint2D convert(const TRGPoint2D&) const;
  };

} // namespace Belle2

#endif
