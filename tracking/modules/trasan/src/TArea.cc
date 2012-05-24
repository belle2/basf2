//-----------------------------------------------------------------------------
// $Id: TArea.cc 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TArea.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent an 2D area.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.1  2004/02/18 04:07:25  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------


#include "tracking/modules/trasan/TArea.h"

namespace Belle {

  TArea::TArea(const TPoint2D& leftBottom, const TPoint2D& rightUpper)
  {
    _c[0] = leftBottom;
    _c[1] = rightUpper;
  }

  TArea::~TArea()
  {
  }

  void
  TArea::cross(const TPoint2D& x0,
               const TPoint2D& x1,
               unsigned& nFound,
               TPoint2D crossPoint[2]) const
  {

    //...Parameters...
    const float xDiff = x1.x() - x0.x();
    const float yDiff = x1.y() - x0.y();
    const float a = yDiff / xDiff;
    const float b = x0.x() - a * x0.x();

    //...Find cross-points...
    nFound = 0;
    for (unsigned i = 0; i < 2; i++) {

      TPoint2D p(_c[i].x(), a * _c[i].x() + b);
      if (inArea(p))
        crossPoint[nFound++] = p;
      if (nFound == 2) return;

      TPoint2D q((_c[i].y() - b) / a, _c[i].y());
      if (inArea(q))
        crossPoint[nFound++] = q;
      if (nFound == 2) return;
    }
  }

} // namespace Belle

