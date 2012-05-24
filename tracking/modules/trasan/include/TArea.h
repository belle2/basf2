//-----------------------------------------------------------------------------
// $Id: TArea.h 9932 2006-11-12 14:26:53Z katayama $
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
// Revision 1.1  2004/02/18 04:07:45  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#ifndef TArea_FLAG_
#define TArea_FLAG_


#include "tracking/modules/trasan/TPoint2D.h"

namespace Belle {

/// A class to represent an 2D area.
  class TArea {

  public:
    /// Contructor.
    TArea(const TPoint2D& leftBottom, const TPoint2D& rightUpper);

    /// Destructor
    virtual ~TArea();

  public:
    /// returns true if give point is in the area.
    bool inArea(const TPoint2D& x) const;

    /// returns cross-points.
    void cross(const TPoint2D& x0, const TPoint2D& x1,
               unsigned& nFound, TPoint2D crossPoint[2]) const;

  private:
    TPoint2D _c[2];
  };

  inline
  bool
  TArea::inArea(const TPoint2D& x) const
  {
    if ((x.x() >= _c[0].x()) && (x.x() <= _c[1].x()))
      if ((x.y() >= _c[0].y()) && (x.y() <= _c[1].y()))
        return true;
    return false;
  }

} // namespace Belle

#endif
