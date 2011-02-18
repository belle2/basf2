//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGArea2D.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a 2D area.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "trg/trg/Area2D.h"

namespace Belle2 {

TRGArea2D::TRGArea2D(const TRGPoint2D & leftBottom,
                             const TRGPoint2D & rightUpper) {
    _c[0] = leftBottom;
    _c[1] = rightUpper;
}

TRGArea2D::~TRGArea2D() {
}

void
TRGArea2D::cross(const TRGPoint2D & x0,
                     const TRGPoint2D & x1,
                     unsigned & nFound,
                     TRGPoint2D crossPoint[2]) const {

    //...Parameters...
    const float xDiff = x1.x() - x0.x();
    const float yDiff = x1.y() - x0.y();
    const float a = yDiff / xDiff;
    const float b = x0.x() - a * x0.x();

    //...Find cross-points...
    nFound = 0;
    for (unsigned i = 0; i < 2; i++) {

        TRGPoint2D p(_c[i].x(), a * _c[i].x() + b);
        if (inArea(p))
            crossPoint[nFound++] = p;
        if (nFound == 2) return;

        TRGPoint2D q((_c[i].y() - b) / a, _c[i].y());
        if (inArea(q))
            crossPoint[nFound++] = q;
        if (nFound == 2) return;
    }
}

} // namespace Belle2

