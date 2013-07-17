//-----------------------------------------------------------------------------
// $Id: TPoint2D.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TPoint2D.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a point in 2D.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2005/04/18 23:42:05  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.10  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.9  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.8  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.7  2000/04/14 05:20:48  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.6  2000/04/11 13:05:56  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.5  2000/03/07 03:00:58  yiwasaki
// Trasan 2.00RC15 : only debug info. changed
//
// Revision 1.4  2000/02/25 12:55:44  yiwasaki
// Trasan 2.00RC10 : stereo improved
//
// Revision 1.3  2000/02/17 13:24:23  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.2  2000/02/15 13:46:50  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.1  1999/11/19 09:13:16  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
//
//-----------------------------------------------------------------------------

#ifndef TPOINT2D_FLAG_
#define TPOINT2D_FLAG_
#include "CLHEP/Geometry/Point3D.h"

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#include <cmath>
#define HEP_SHORT_NAMES
#include "CLHEP/Geometry/Vector3D.h"

namespace Belle {

  typedef HepGeom::Vector3D<double>  Vector3D;

//  class CLHEP::Hep3Vector;

/// A class to represent a point in 2D.
  class TPoint2D {

  public:
    /// Constructors
    TPoint2D();
    TPoint2D(double, double);
    TPoint2D(const HepGeom::Point3D<double>  &);
    TPoint2D(const Vector3D&);
    TPoint2D(const CLHEP::Hep3Vector&);

    /// Destructor
    virtual ~TPoint2D();

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
    double dot(const TPoint2D&) const;
    double cross(const TPoint2D&) const;
    TPoint2D unit(void) const;
    TPoint2D operator + (const TPoint2D&) const;
    TPoint2D operator - (const TPoint2D&) const;
    TPoint2D operator - () const;
    bool operator == (const TPoint2D&) const;

  private:
    double _p[2];
  };

  std::ostream&
  operator << (std::ostream&, const TPoint2D&);

//-----------------------------------------------------------------------------

#ifdef TPOINT2D_NO_INLINE
#define inline
#else
#undef inline
#define TPOINT2D_INLINE_DEFINE_HERE
#endif
#ifdef TPOINT2D_INLINE_DEFINE_HERE

  inline
  double
  TPoint2D::x(void) const
  {
    return _p[0];
  }

  inline
  double
  TPoint2D::y(void) const
  {
    return _p[1];
  }

  inline
  double
  TPoint2D::x(double a)
  {
    return _p[0] = a;
  }

  inline
  double
  TPoint2D::y(double a)
  {
    return _p[1] = a;
  }

  inline
  double
  TPoint2D::mag(void) const
  {
    return sqrt(_p[0] * _p[0] + _p[1] * _p[1]);
  }

  inline
  double
  TPoint2D::mag2(void) const
  {
    return _p[0] * _p[0] + _p[1] * _p[1];
  }

  inline
  double
  TPoint2D::phi(void) const
  {
    if (_p[0] == 0.0 && _p[1] == 0.0) return 0.;
    double a = atan2(_p[1], _p[0]);
    if (a > 0) return a;
    return a + 2. * M_PI;
  }

  inline
  double
  TPoint2D::dot(const TPoint2D& a) const
  {
    return _p[0] * a.x() + _p[1] * a.y();
  }

  inline
  double
  TPoint2D::cross(const TPoint2D& a) const
  {
    return _p[0] * a.y() - a.x() * _p[1];
  }

  inline
  TPoint2D
  TPoint2D::operator + (const TPoint2D& a) const
  {
    return TPoint2D(_p[0] + a.x(), _p[1] + a.y());
  }

  inline
  TPoint2D
  TPoint2D::operator - (const TPoint2D& a) const
  {
    return TPoint2D(_p[0] - a.x(), _p[1] - a.y());
  }

  inline
  TPoint2D
  TPoint2D::operator - () const
  {
    return TPoint2D(- _p[0], - _p[1]);
  }

  inline
  bool
  TPoint2D::operator == (const TPoint2D& a) const
  {
    if (a.x() == _p[0] && a.y() == _p[1]) return true;
    return false;
  }

  inline
  TPoint2D
  TPoint2D::unit(void) const
  {
    double sum2 = _p[0] * _p[0] + _p[1] * _p[1];
    if (sum2 == 0.) return TPoint2D(0., 0.);
    double sum = sqrt(sum2);
    return TPoint2D(_p[0] / sum, _p[1] / sum);
  }

#endif
#undef inline

} // namespace Belle

#endif /* TPOINT2D_FLAG_ */
