//-----------------------------------------------------------------------------
// $Id: THoughTransformationCircleGeneral.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : THoughTransformationCircleGeneral.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent general circle Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.1  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
//-----------------------------------------------------------------------------

#ifndef THoughTransformationCircleGeneral_FLAG_
#define THoughTransformationCircleGeneral_FLAG_


#include "tracking/modules/trasan/THoughTransformation.h"

namespace Belle {

/// A class to represent circle Hough transformation.
  class THoughTransformationCircleGeneral : public THoughTransformation {

  public:
    /// Contructor.
    THoughTransformationCircleGeneral(const std::string& name);

    /// Destructor
    virtual ~THoughTransformationCircleGeneral();

  public:// Selectors
    /// returns Y coordinate in a Hough parameter plane.
    virtual float y(float xReal, float yReal, float x) const;

    /// returns Point2D(phi, r) of a circle in real plane.
    virtual TPoint2D circleCenter(const TPoint2D&) const;

    /// returns radius in real plane.
    float circleRadius(const TPoint2D&) const;

    /// converts Point2D(r, phi) in real plane into Point2D(r, phi) in Hough plane.
    virtual TPoint2D convert(const TPoint2D&) const;

    /// returns true if Y diverges in given region.
    virtual bool diverge(float xReal, float yReal, float x0, float x1) const;

    /// returns true if Y diverges in given region.
    virtual bool positiveDiverge(float xReal, float yReal, float x0, float x1)
    const;

    /// returns true if Y diverges in given region.
    virtual bool negativeDiverge(float xReal, float yReal, float x0, float x1)
    const;

  public:// Modifiers
    /// sets R.
    float r(float);

  private:
    float _r;
  };

#ifdef THoughTransformationCircleGeneral_NO_INLINE
#define inline
#else
#undef inline
#define THoughTransformationCircleGeneral_INLINE_DEFINE_HERE
#endif

#ifdef THoughTransformationCircleGeneral_INLINE_DEFINE_HERE

  inline
  float
  THoughTransformationCircleGeneral::r(float a)
  {
    return _r = a;
  }

  inline
  bool
  THoughTransformationCircleGeneral::diverge(float, float, float, float) const
  {
    return false;
  }

  inline
  bool
  THoughTransformationCircleGeneral::positiveDiverge(float,
                                                     float,
                                                     float,
                                                     float) const
  {
    return false;
  }

  inline
  bool
  THoughTransformationCircleGeneral::negativeDiverge(float,
                                                     float,
                                                     float,
                                                     float) const
  {
    return false;
  }

#endif

#undef inline

} // namespace Belle

#endif
