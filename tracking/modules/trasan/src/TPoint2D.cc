//-----------------------------------------------------------------------------
// $Id: TPoint2D.cc 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TPoint2D.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a point in 2D.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.6  2002/01/03 11:04:56  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.5  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.4  2000/04/11 13:05:46  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.3  2000/02/25 12:55:41  yiwasaki
// Trasan 2.00RC10 : stereo improved
//
// Revision 1.2  2000/02/15 13:46:41  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.1  1999/11/19 09:13:09  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
//
//-----------------------------------------------------------------------------

#define TPOINT2D_INLINE_DEFINE_HERE


#include <iostream>
#include "tracking/modules/trasan/TPoint2D.h"
#include "CLHEP/Vector/ThreeVector.h"


namespace Belle {

  TPoint2D::TPoint2D()
  {
    _p[0] = 0.;
    _p[1] = 0.;
  }

  TPoint2D::TPoint2D(double x, double y)
  {
    _p[0] = x;
    _p[1] = y;
  }

  TPoint2D::TPoint2D(const HepGeom::Point3D<double>  & a)
  {
    _p[0] = a.x();
    _p[1] = a.y();
  }

  TPoint2D::TPoint2D(const HepGeom::Vector3D<double>  & a)
  {
    _p[0] = a.x();
    _p[1] = a.y();
  }

  TPoint2D::TPoint2D(const CLHEP::Hep3Vector& a)
  {
    _p[0] = a.x();
    _p[1] = a.y();
  }

  TPoint2D::~TPoint2D()
  {
  }

  std::ostream&
  operator << (std::ostream& s, const TPoint2D& a)
  {
    return s << "(" << a.x() << "," << a.y() << ")";
  }

} // namespace Belle

