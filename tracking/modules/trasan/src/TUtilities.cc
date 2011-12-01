//-----------------------------------------------------------------------------
// $Id: TUtilities.cc 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TUtilities.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : Definitions of utility functions
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/11/24 01:33:13  yiwasaki
// addition of debug utilitiese
//
//-----------------------------------------------------------------------------


#include <math.h>
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/TUtilities.h"

namespace Belle {

  const float PI2 = 2. * M_PI;

  const TPoint2D Origin(0, 0);

  float
  PositiveRadian(float a)
  {
    while (a < 0)
      a += PI2;
    return fmod(a, PI2);
  }

  bool
  InRangeRadian(float a, float b, float c)
  {
    float phi0 = PositiveRadian(a);
    float phi1 = PositiveRadian(b);
    const float phi = PositiveRadian(c);

    if (phi1 < phi0) {
      float tmp = phi1;
      phi1 = phi0;
      phi0 = tmp;
    }

    const float dPhi = phi1 - phi0;
    bool ok = (phi > phi0) && (phi < phi1);
    if (dPhi > M_PI)
      ok = ! ok;

    return ok;
  }

  float
  DistanceRadian(float a, float b)
  {
    const float phi0 = PositiveRadian(a);
    const float phi1 = PositiveRadian(b);
    const float dPhi = fabs(phi1 - phi0);
    if (dPhi > M_PI)
      return PI2 - dPhi;
    return dPhi;
  }

} // namespace Belle

