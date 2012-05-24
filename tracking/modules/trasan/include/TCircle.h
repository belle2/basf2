//-----------------------------------------------------------------------------
// $Id: TCircle.h 10660 2008-09-25 04:27:48Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TCircle.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a circle in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.25  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.24  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.23  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.22  2004/02/18 04:07:45  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.21  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.20  2001/12/23 09:58:55  katayama
// removed Strings.h
//
// Revision 1.19  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.18  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.17  2001/04/11 01:10:02  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.16  2000/10/05 23:54:30  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.15  1999/11/23 10:30:21  yiwasaki
// ALPHA in TRGCDCUtil is replaced by THelix::ConstantAlpha
//
// Revision 1.14  1999/10/30 10:12:45  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.13  1999/03/11 23:27:25  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.12  1999/01/11 03:03:24  yiwasaki
// Fitters added
//
// Revision 1.11  1998/09/29 01:24:33  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.10  1998/07/29 04:35:15  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.8  1998/07/06 15:48:56  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.7  1998/06/14 11:09:56  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.6  1998/06/11 08:15:43  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.5  1998/05/08 09:47:05  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.4  1998/04/23 17:24:59  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.3  1998/04/16 16:51:07  yiwasaki
// minor changes
//
// Revision 1.2  1998/04/10 09:38:18  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:51:13  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef TCircle_FLAG_
#define TCircle_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES

#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TCircleFitter.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/Lpav.h"

namespace Belle {

/// A class to represent a circle in tracking.
  class TCircle : public TTrackBase {

  public:
    /// Constructor.
    TCircle(const TCircle&);
    TCircle(const AList<TLink> & links);
    TCircle(const TTrack& track);
    TCircle(float r, float phi, float charge = 0);
    TCircle(const TPoint2D& center, float radius, float charge = 0);

    /// Destructor
    virtual ~TCircle();

  public:// Selectors
    /// returns type.
    virtual unsigned objectType(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns position of center.
    const HepGeom::Point3D<double>  & center(void) const;

    /// returns radius.
    double radius(void) const;

    /// returns Pt.
    double pt(void) const;

    /// returns impact parameter to the origin.
    double impact(void) const;

    /// returns charge.
    double charge(void) const;

    /// returns weight of TLink in order to fit and make a circle.
    double weight(const TLink& l) const;

  public:// Modifiers
    /// fits itself. Error was happened if return value is not zero.
    // int fitx(void);

    /// fits itself only for curl finder. Error was happened if return value is not zero.
    int fitForCurl(int ipConst = 0);

    double charge(double charge);

    /// sets circle properties.
    void property(double charge, double radius, HepGeom::Point3D<double>  center);
    void property(double charge, double radius, TPoint2D& center);

  private:// Updated when fitted
    Lpav _circle;
    double _charge;
    double _radius;
    HepGeom::Point3D<double>  _center;
    static const TCircleFitter _fitter;
  };

//-----------------------------------------------------------------------------

#ifdef TCircle_NO_INLINE
#define inline
#else
#undef inline
#define TCircle_INLINE_DEFINE_HERE
#endif

#ifdef TCircle_INLINE_DEFINE_HERE

  inline
  const HepGeom::Point3D<double>  &
  TCircle::center(void) const
  {
#ifdef TRASAN_DEBUG
//     if (! _fitted) std::cout << "TCircle::center !!! fit not performed : "
//           << "center=" << _center << std::endl;
#endif
    return _center;
  }

  inline
  double
  TCircle::radius(void) const
  {
#ifdef TRASAN_DEBUG
//     if (! _fitted) std::cout << "TCircle::radius !!! fit not performed : "
//           << "radius=" << _radius << std::endl;
#endif
    return _radius;
  }

  inline
  double
  TCircle::pt(void) const
  {
#ifdef TRASAN_DEBUG
//     if (! _fitted) std::cout << "TCircle::pt !!! fit not performed"
//           << std::endl;
#endif
    return fabs(_radius) / THelix::ConstantAlpha;
  }

  inline
  double
  TCircle::impact(void) const
  {
#ifdef TRASAN_DEBUG
//     if (! _fitted) std::cout << "TCircle::impact !!! fit not performed"
//           << std::endl;
#endif
    return fabs(sqrt(_center.x() * _center.x() +
                     _center.y() * _center.y())
                - fabs(_radius));
  }

  inline
  double
  TCircle::charge(void) const
  {
#ifdef TRASAN_DEBUG
//     if (! _fitted) std::cout << "TCircle::charge !!! fit not performed"
//           << std::endl;
#endif
    return _charge;
  }

  inline
  void
  TCircle::property(double c, double r, HepGeom::Point3D<double>  e)
  {
    _charge = c;
    _radius = r;
    _center = e;
  }

  inline
  unsigned
  TCircle::objectType(void) const
  {
    return Circle;
  }

  inline
  double
  TCircle::charge(double a)
  {
    return _charge = a;
  }

  inline
  void
  TCircle::property(double c, double r, TPoint2D& e)
  {
    _charge = c;
    _radius = r;
    _center = HepGeom::Point3D<double> (e.x(), e.y(), 0);
  }

#endif

#undef inline

} // namespace Belle

#endif /* TCircle_FLAG_ */
