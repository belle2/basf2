//-----------------------------------------------------------------------------
// $Id: T3DLine.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : T3DLine.h
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a 3D line in tracking
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.6  2002/01/03 11:04:57  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.5  2001/12/23 09:58:53  katayama
// removed Strings.h
//
// Revision 1.4  2001/12/14 02:54:45  katayama
// For gcc-3.0
//
// Revision 1.3  2001/04/11 01:10:00  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.2  1999/10/30 10:12:33  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1999/10/21 15:45:17  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
//
//-----------------------------------------------------------------------------

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#ifndef T3DLine_FLAG_
#define T3DLine_FLAG_
#include "CLHEP/Geometry/Vector3D.h"

#define Line3D  32
// This must be writen in TTrackBase.h


#include <string>

#define HEP_SHORT_NAMES
#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

#include "tracking/modules/trasan/THelix.h"
#include "tracking/modules/trasan/TTrackBase.h"

namespace Belle {

  typedef HepGeom::Point3D<double>  Point3D;
  class T3DLineFitter;
  class TTrack;
  class TLink;

/// A class to represent a track in tracking.
  class T3DLine : public TTrackBase {

  public:
    /// Constructors
    T3DLine();
    T3DLine(const TTrack&);
    T3DLine(const T3DLine&);

    /// Destructor
    virtual ~T3DLine();

  public:// Extractors
    /// Track parameters
    double dr(void) const;
    double phi0(void) const;
    double dz(void) const;
    double tanl(void) const;

    double cosPhi0(void) const;
    double sinPhi0(void) const;

    CLHEP::HepVector a(void) const;   //dr,phi0,dz,tanl

    /// returns error matrix
    const CLHEP::HepSymMatrix& Ea(void) const;

    /// pivot position
    const HepGeom::Point3D<double>  & pivot(void) const;

    /// approximated helix class
    THelix helix(void) const;

    /// returns NDF
    unsigned ndf(void) const;

    /// returns chi2.
    double chi2(void) const;

    /// returns reduced-chi2
    double reducedchi2(void) const;

    /// returns object type
    unsigned objectType(void) const;

  public:// Executors

  public:// Utilities
    /// returns position on 3D line
    HepGeom::Point3D<double>  x(double) const;

    /// returns 3D line component    x(t)=x0 + t * k
    HepGeom::Point3D<double>  x0(void) const;
    HepGeom::Vector3D<double> k(void) const;

    /// calculates the closest approach to a wire in real space. Results are stored in TLink. Return value is negative if error happened.
    int approach(TLink&, bool sagCorrection = true) const;

    /// caluculate closest points between a line and this track
    int approach_line(const HepGeom::Point3D<double> &, const HepGeom::Vector3D<double> &,
                      HepGeom::Point3D<double> & onLine, HepGeom::Point3D<double> & onTrack) const;

    /// caluculate closest point between a point and this track
    int approach_point(const HepGeom::Point3D<double> &, HepGeom::Point3D<double> & onTrack) const;

  public:// Modifiers
    /// set new pivot
    const HepGeom::Point3D<double>  & pivot(const HepGeom::Point3D<double> &);

    /// set track parameters,pivot
    void set(const HepGeom::Point3D<double> &, double t_dr, double t_phi0, double t_dz, double t_tanl);

    /// set track parameters
    CLHEP::HepVector a(const CLHEP::HepVector&);

    /// set error matrix
    const CLHEP::HepSymMatrix& Ea(const CLHEP::HepSymMatrix&);

  private:
    HepGeom::Point3D<double>  _pivot;
    // Updated when fitted
    double _dr;
    double _phi0;
    double _dz;
    double _tanl;

    CLHEP::HepSymMatrix _Ea;

    double _chi2;
    unsigned _ndf;

    //Cashe
    double _cos_phi0;
    double _sin_phi0;

    static const T3DLineFitter _fitter;

    friend class T3DLineFitter;
  };

  inline unsigned T3DLine::objectType(void) const
  {
    return Line3D;
  }

} // namespace Belle

#endif /* T3DLine_FLAG_ */
