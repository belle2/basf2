//-----------------------------------------------------------------------------
// $Id: THoughTransformationLine.cc 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : THoughTransformationLine.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent line Hough transformation.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/03/11 03:57:51  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/04/23 09:51:44  yiwasaki
// New files for Hough finder
//
//-----------------------------------------------------------------------------


#include <math.h>
#include "tracking/modules/trasan/THoughTransformationLine.h"
#include "tracking/modules/trasan/TPoint2D.h"

namespace Belle {

  THoughTransformationLine::THoughTransformationLine(
    const std::string& name)
    : THoughTransformation(name)
  {
  }

  THoughTransformationLine::~THoughTransformationLine()
  {
  }

  float
  THoughTransformationLine::y(float xReal, float yReal, float x) const
  {
    return (xReal * cos(x) + yReal * sin(x));
  }

  bool
  THoughTransformationLine::diverge(float xReal,
                                    float yReal,
                                    float x0,
                                    float x1) const
  {
    return false;
  }

  bool
  THoughTransformationLine::positiveDiverge(float xReal,
                                            float yReal,
                                            float x0,
                                            float x1) const
  {
    return false;
  }

  bool
  THoughTransformationLine::negativeDiverge(float xReal,
                                            float yReal,
                                            float x0,
                                            float x1) const
  {
    return false;
  }

} // namespace Belle

