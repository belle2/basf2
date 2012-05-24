//-----------------------------------------------------------------------------
// $Id: THoughTransformationCircle.cc 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : THoughTransformationCircle.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent circle Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.4  2005/03/11 03:57:51  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.3  2004/04/23 09:48:20  yiwasaki
// Trasan 3.12 : curlVersion=2 is default
//
// Revision 1.2  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.1  2004/02/18 04:07:26  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------


#include <math.h>
#include "tracking/modules/trasan/THoughTransformationCircle.h"
#include "tracking/modules/trasan/TPoint2D.h"

namespace Belle {

  THoughTransformationCircle::THoughTransformationCircle(
    const std::string& name)
    : THoughTransformation(name)
  {
  }

  THoughTransformationCircle::~THoughTransformationCircle()
  {
  }

  float
  THoughTransformationCircle::y(float xReal, float yReal, float x) const
  {
//    return log10((xReal * xReal + yReal * yReal) /
//  (2 * xReal * cos(x) + 2 * yReal * sin(x)));
    const float r = (xReal * xReal + yReal * yReal) /
                    (2 * xReal * cos(x) + 2 * yReal * sin(x));
    if (r > 0)
      return log10(r);
    else
      return 0;
  }

  TPoint2D
  THoughTransformationCircle::circleCenter(const TPoint2D& p) const
  {
//    return TPoint2D(pow(10, p.y()), p.x());
    return TPoint2D(p.x(), pow(10, p.y()));
  }

  TPoint2D
  THoughTransformationCircle::convert(const TPoint2D& p) const
  {
    return TPoint2D(p.x(), log10(p.y()));
  }

  bool
  THoughTransformationCircle::diverge(float xReal,
                                      float yReal,
                                      float x0,
                                      float x1) const
  {
    static const float PI2 = 2 * M_PI;

    //...Divergence points...
    float d0 = atan(- xReal / yReal);
    if (d0 < 0) d0 += PI2;
    float d1 = d0 + M_PI;
    if (d1 > PI2) d1 -= PI2;

    //...In region ?...
    if ((x0 < d0) && (d0 < x1)) return true;
    if ((x0 < d1) && (d1 < x1)) return true;

    return false;
  }

  bool
  THoughTransformationCircle::positiveDiverge(float xReal,
                                              float yReal,
                                              float x0,
                                              float x1) const
  {
    return diverge(xReal, yReal, x0, x1);
  }

  bool
  THoughTransformationCircle::negativeDiverge(float xReal,
                                              float yReal,
                                              float x0,
                                              float x1) const
  {
    return diverge(xReal, yReal, x0, x1);
  }

} // namespace Belle

