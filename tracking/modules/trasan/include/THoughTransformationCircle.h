//-----------------------------------------------------------------------------
// $Id: THoughTransformationCircle.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : THoughTransformationCircle.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent circle Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.4  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.3  2005/03/11 03:58:35  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.2  2004/04/23 09:48:24  yiwasaki
// Trasan 3.12 : curlVersion=2 is default
//
// Revision 1.1  2004/02/18 04:07:46  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#ifndef THoughTransformationCircle_FLAG_
#define THoughTransformationCircle_FLAG_


#include "tracking/modules/trasan/THoughTransformation.h"

namespace Belle {

/// A class to represent circle Hough transformation.
  class THoughTransformationCircle : public THoughTransformation {

  public:
    /// Contructor.
    THoughTransformationCircle(const std::string& name);

    /// Destructor
    virtual ~THoughTransformationCircle();

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
    virtual TPoint2D circleCenter(const TPoint2D&) const;

    /// converts Point2D(phi, r) in real plane into Point2D(phi, r) in Hough plane.
    virtual TPoint2D convert(const TPoint2D&) const;
  };

} // namespace Belle

#endif
