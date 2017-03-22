//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TRGPoint2D.h
// Section  : TRG
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a point in 2D.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGPoint2D_FLAG_
#define TRGPoint2D_FLAG_

#include <cmath>
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

namespace Belle2 {

  typedef HepGeom::Vector3D<double> Vector3D;

// class CLHEP::Hep3Vector;

/// A class to represent a point in 2D.
  class TRGPoint2D {

  public:
    /// Constructors
    TRGPoint2D();
    TRGPoint2D(double, double);
    TRGPoint2D(const HepGeom::Point3D<double>&);
    TRGPoint2D(const Vector3D&);
    TRGPoint2D(const CLHEP::Hep3Vector&);

    /// Destructor
    virtual ~TRGPoint2D();

  public:// Selectors
    double x(void) const;
    double y(void) const;
    double mag(void) const;
    double mag2(void) const;
    double phi(void) const;

  public:// Modifiers
    double x(double);
    double y(double);

  public:// Operators
    double dot(const TRGPoint2D&) const;
    double cross(const TRGPoint2D&) const;
    TRGPoint2D unit(void) const;
    TRGPoint2D operator + (const TRGPoint2D&) const;
    TRGPoint2D operator - (const TRGPoint2D&) const;
    TRGPoint2D operator - () const;
    bool operator == (const TRGPoint2D&) const;

  private:
    double _p[2];
  };

  std::ostream&
  operator << (std::ostream&, const TRGPoint2D&);

//-----------------------------------------------------------------------------

  inline
  double
  TRGPoint2D::x(void) const
  {
    return _p[0];
  }

  inline
  double
  TRGPoint2D::y(void) const
  {
    return _p[1];
  }

  inline
  double
  TRGPoint2D::x(double a)
  {
    return _p[0] = a;
  }

  inline
  double
  TRGPoint2D::y(double a)
  {
    return _p[1] = a;
  }

  inline
  double
  TRGPoint2D::mag(void) const
  {
    return sqrt(_p[0] * _p[0] + _p[1] * _p[1]);
  }

  inline
  double
  TRGPoint2D::mag2(void) const
  {
    return _p[0] * _p[0] + _p[1] * _p[1];
  }

  inline
  double
  TRGPoint2D::phi(void) const
  {
    if (_p[0] == 0.0 && _p[1] == 0.0) return 0.;
    double a = atan2(_p[1], _p[0]);
    if (a > 0) return a;
    return a + 2. * M_PI;
  }

  inline
  double
  TRGPoint2D::dot(const TRGPoint2D& a) const
  {
    return _p[0] * a.x() + _p[1] * a.y();
  }

  inline
  double
  TRGPoint2D::cross(const TRGPoint2D& a) const
  {
    return _p[0] * a.y() - a.x() * _p[1];
  }

  inline
  TRGPoint2D
  TRGPoint2D::operator + (const TRGPoint2D& a) const
  {
    return TRGPoint2D(_p[0] + a.x(), _p[1] + a.y());
  }

  inline
  TRGPoint2D
  TRGPoint2D::operator - (const TRGPoint2D& a) const
  {
    return TRGPoint2D(_p[0] - a.x(), _p[1] - a.y());
  }

  inline
  TRGPoint2D
  TRGPoint2D::operator - () const
  {
    return TRGPoint2D(- _p[0], - _p[1]);
  }

  inline
  bool
  TRGPoint2D::operator == (const TRGPoint2D& a) const
  {
    if (a.x() == _p[0] && a.y() == _p[1]) return true;
    return false;
  }

  inline
  TRGPoint2D
  TRGPoint2D::unit(void) const
  {
    double sum2 = _p[0] * _p[0] + _p[1] * _p[1];
    if (sum2 == 0.) return TRGPoint2D(0., 0.);
    double sum = sqrt(sum2);
    return TRGPoint2D(_p[0] / sum, _p[1] / sum);
  }

} // namespace Belle2k

#endif /* TRGPoint2D_FLAG_ */
