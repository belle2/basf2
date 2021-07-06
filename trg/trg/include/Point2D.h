/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

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
    /// Constructor
    TRGPoint2D();
    /// Constructor
    TRGPoint2D(double, double);
    /// Constructor
    // cppcheck-suppress noExplicitConstructor
    TRGPoint2D(const HepGeom::Point3D<double>&);
    /// Constructor
    explicit TRGPoint2D(const Vector3D&);
    /// Constructor
    explicit TRGPoint2D(const CLHEP::Hep3Vector&);

    /// Destructor
    virtual ~TRGPoint2D();

  public:// Selectors
    /// x of the point
    double x(void) const;
    /// y of the point
    double y(void) const;
    /// magnitude of the point
    double mag(void) const;
    /// magnitude square of the point
    double mag2(void) const;
    /// phi of the point
    double phi(void) const;

  public:// Modifiers
    /// x of the point
    double x(double);
    /// y of the point
    double y(double);

  public:// Operators
    /// inner product
    double dot(const TRGPoint2D&) const;
    /// outer product
    double cross(const TRGPoint2D&) const;
    /// unit vector
    TRGPoint2D unit(void) const;
    /// + operator
    TRGPoint2D operator + (const TRGPoint2D&) const;
    /// - operator
    TRGPoint2D operator - (const TRGPoint2D&) const;
    /// - operator
    TRGPoint2D operator - () const;
    /// equal operator
    bool operator == (const TRGPoint2D&) const;

  private:
    /// vector
    double _p[2];
  };

  /// ostrream operator
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
