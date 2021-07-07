/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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

