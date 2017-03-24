//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGPoint2D.cc
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a point in 2D.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#include "trg/trg/Point2D.h"

namespace Belle2 {

  TRGPoint2D::TRGPoint2D()
  {
    _p[0] = 0.;
    _p[1] = 0.;
  }

  TRGPoint2D::TRGPoint2D(double x, double y)
  {
    _p[0] = x;
    _p[1] = y;
  }

  TRGPoint2D::TRGPoint2D(const HepGeom::Point3D<double>&   a)
  {
    _p[0] = a.x();
    _p[1] = a.y();
  }

  TRGPoint2D::TRGPoint2D(const HepGeom::Vector3D<double>&   a)
  {
    _p[0] = a.x();
    _p[1] = a.y();
  }

  TRGPoint2D::TRGPoint2D(const CLHEP::Hep3Vector& a)
  {
    _p[0] = a.x();
    _p[1] = a.y();
  }

  TRGPoint2D::~TRGPoint2D()
  {
  }

  std::ostream&
  operator << (std::ostream& s, const TRGPoint2D& a)
  {
    return s << "(" << a.x() << "," << a.y() << ")";
  }

} // namespace Belle2

