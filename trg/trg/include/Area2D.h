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

#ifndef TRGArea2D_FLAG_
#define TRGArea2D_FLAG_

#include "trg/trg/Point2D.h"

#ifdef TRGCDC_SHORT_NAMES
#define TRGArea2D TRGArea2D
#endif

namespace Belle2 {

/// A class to represent an 2D area.
class TRGArea2D {

  public:
    /// Contructor.
    TRGArea2D(const TRGPoint2D & leftBottom,
		  const TRGPoint2D & rightUpper);

    /// Destructor
    virtual ~TRGArea2D();

  public:
    /// returns true if give point is in the area.
    bool inArea(const TRGPoint2D & x) const;

    /// returns cross-points.
    void cross(const TRGPoint2D & x0,
	       const TRGPoint2D & x1,
	       unsigned & nFound,
	       TRGPoint2D crossPoint[2]) const;

  private:
    TRGPoint2D _c[2];
};

inline
bool
TRGArea2D::inArea(const TRGPoint2D & x) const {
    if ((x.x() >= _c[0].x()) && (x.x() <= _c[1].x()))
	if ((x.y() >= _c[0].y()) && (x.y() <= _c[1].y()))
	    return true;
    return false;
}

}

#endif
