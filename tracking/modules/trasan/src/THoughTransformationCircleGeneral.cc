//-----------------------------------------------------------------------------
// $Id: THoughTransformationCircleGeneral.cc 9932 2006-11-12 14:26:53Z katayama $
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
// Revision 1.3  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.2  2005/04/19 00:12:15  yiwasaki
// Compiler error removed
//
// Revision 1.1  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
//-----------------------------------------------------------------------------


#include <cmath>
#include "tracking/modules/trasan/THoughTransformationCircleGeneral.h"
#include "tracking/modules/trasan/TPoint2D.h"

namespace Belle {

  THoughTransformationCircleGeneral::THoughTransformationCircleGeneral(
    const std::string& name)
    : THoughTransformation(name),
      _r(0)
  {
  }

  THoughTransformationCircleGeneral::~THoughTransformationCircleGeneral()
  {
  }

  float
  THoughTransformationCircleGeneral::y(float xReal,
                                       float yReal,
                                       float x) const
  {
    const float a = _r * _r
                    - 2 * _r * (xReal * cos(x) + yReal * sin(x))
                    + xReal * xReal + yReal * yReal;
    if (a > 0) return sqrt(a);
//    if (a > 0) return log10(sqrt(a));
    return 0;
  }

  TPoint2D
  THoughTransformationCircleGeneral::circleCenter(const TPoint2D& p) const
  {
    return TPoint2D(p.x(), _r);
  }

  float
  THoughTransformationCircleGeneral::circleRadius(const TPoint2D& p) const
  {
    return p.y();
  }

  TPoint2D
  THoughTransformationCircleGeneral::convert(const TPoint2D& p) const
  {
    return TPoint2D(p.x(), log10(p.y()) - _r);
  }

} // namespace Belle

