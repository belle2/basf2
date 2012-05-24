//-----------------------------------------------------------------------------
// $Id: TRunge.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TRunge.h
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track using Runge Kutta method
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2003/12/25 12:04:44  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.4  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.3  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.2  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.1  2001/08/02 07:04:15  yiwasaki
// RK fitter from K.Inami
//
//
//-----------------------------------------------------------------------------

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#ifndef TRUNGE_FLAG_
#define TRUNGE_FLAG_
#include "CLHEP/Geometry/Point3D.h"
#include "CLHEP/Geometry/Vector3D.h"

#define Runge 64
// This must be writen in TTrackBase.h

#define HEP_SHORT_NAMES

#ifndef CLHEP_POINT3D_H
#include "CLHEP/Geometry/Point3D.h"
#endif

#include "tracking/modules/trasan/THelix.h"
#include "tracking/modules/trasan/Bfield.h"
#include "tracking/modules/trasan/TTrackBase.h"

namespace Belle {

  typedef HepGeom::Point3D<double>  Point3D;
  class TRungeFitter;
  class TTrack;
  class TLink;

#define TRunge_MAXstep 10000

/// A class to represent a track in tracking.
  class TRunge : public TTrackBase {

  public:
    /// Constructors
    TRunge();
    TRunge(const TTrack&);
    TRunge(const THelix&);
    TRunge(const TRunge&);

    /// Destructor
    ~TRunge();

  public:// General information
    /// returns object type
    unsigned objectType(void) const;

  public:// Extractors
    /// Track parameters (at pivot)
    double dr(void) const;
    double phi0(void) const;
    double kappa(void) const;
    double dz(void) const;
    double tanl(void) const;

    /// pivot position
    const HepGeom::Point3D<double> & pivot(void) const;

    /// returns helix parameter
    const CLHEP::HepVector& a(void) const;

    /// returns error matrix
    const CLHEP::HepSymMatrix& Ea(void) const;

    /// returns helix class
    THelix helix(void) const;


    /// returns NDF
    unsigned ndf(void) const;

    /// returns chi2.
    double chi2(void) const;

    /// returns reduced-chi2
    double reducedchi2(void) const;

    /// returns B field ID
    int BfieldID(void) const;

    /// returns step size
    double StepSize(void) const;

    /// return error parameters for fitting with step size control
    const double* Yscal(void) const;
    double Eps(void) const;
    double StepSizeMax(void) const;
    double StepSizeMin(void) const;

    /// return mass
    float Mass(void) const;

    /// return max flight length
    double MaxFlightLength(void) const;

  public:// Executors

  public:// Utilities
    /// calculates the closest approach to a wire in real space.
    ///  Results are stored in TLink. Return value is negative if error happened.
    int approach(TLink&, bool sagCorrection = true) const;
    int approach(TLink&, float& tof, HepGeom::Vector3D<double> & p,
                 bool sagCorrection = true) const;

    /// caluculate closest points between a line and this track
    int approach_line(const HepGeom::Point3D<double> &, const HepGeom::Vector3D<double> &,
                      HepGeom::Point3D<double> & onLine, HepGeom::Point3D<double> & onTrack) const;
    int approach_line(const HepGeom::Point3D<double> &, const HepGeom::Vector3D<double> &,
                      HepGeom::Point3D<double> & onLine, HepGeom::Point3D<double> & onTrack,
                      float& tof, HepGeom::Vector3D<double> & p) const;
    int approach_line(const HepGeom::Point3D<double> &, const HepGeom::Vector3D<double> &,
                      HepGeom::Point3D<double> & onLine, HepGeom::Point3D<double> & onTrack,
                      float& tof, HepGeom::Vector3D<double> & p, unsigned& stepNum) const;

    /// caluculate closest point between a point and this track
    int approach_point(const HepGeom::Point3D<double> &, HepGeom::Point3D<double> & onTrack) const;

  public:// Modifiers
    /// set new pivot
    const HepGeom::Point3D<double> & pivot(const HepGeom::Point3D<double> &);

    /// set helix parameter
    const CLHEP::HepVector& a(const CLHEP::HepVector&);

    /// set helix error matrix
    const CLHEP::HepSymMatrix& Ea(const CLHEP::HepSymMatrix&);

    /// set B field map ID
    int BfieldID(int);

    /// set step size to calc. trajectory
    double StepSize(double);

    /// set error parameters for fitting with step size control
    const double* Yscal(const double*);
    double Eps(double);
    double StepSizeMax(double);
    double StepSizeMin(double);

    /// set mass  for tof calc.
    float Mass(float);

    // set max flight length
    double MaxFlightLength(double);

  public:// utilities for local use
    /// make the trajectory in cache,   return the number of step
    unsigned Fly(void) const;
    unsigned Fly_SC(void) const; //fly with stepsize control

    /// propagate the track using 4th-order Runge-Kutta method
    void Propagate(double y[6], const double& step) const;
    void Function(const double y[6], double f[6]) const;
    // for propagate with quality check
    void Propagate1(const double y[6], const double dydx[6],
                    const double& step, double yout[6]) const;
    void Propagate_QC(double y[6], double dydx[6], const double& steptry,
                      const double& eps, const double yscal[6],
                      double& stepdid, double& stepnext) const;

    /// set first point (position, momentum)  s=0, phi=0
    void SetFirst(double y[6]) const;

    /// access to trajectory cache
    unsigned Nstep(void) const;
    int GetXP(unsigned stepNum, double y[6]) const;
    // y[6] = (x,y,z,px,py,pz)  output: error=-1
    int GetStep(unsigned stepNum, double& step) const;

    /// set flight length using wire hits
    double SetFlightLength(void);

  private:
    // track parameters   updated when fitted
    HepGeom::Point3D<double>  _pivot;
    CLHEP::HepVector _a;    //dr,phi0,kappa,dz,tanl
    CLHEP::HepSymMatrix _Ea;

    double _chi2;
    unsigned _ndf;

    static const TRungeFitter _fitter;

    friend class TRungeFitter;

    Bfield* _bfield;
    int _bfieldID;

    double _maxflightlength;

    double _stepSize; // default step size
    //if 0, step size will be made automatically
    double _yscal[6];
    double _eps;    //used for step size control
    double _stepSizeMax;
    double _stepSizeMin;

    float _mass;

  private:
    // caches
    float _mass2; //=_mass*_mass;
    int _charge;

    mutable double _y[TRunge_MAXstep][6]; //(x,y,z,px,py,pz)
    mutable double _h[TRunge_MAXstep];  //step size in each step
    mutable unsigned _Nstep;    //0:not cached the trajectory

  };

  inline unsigned TRunge::objectType(void) const
  {
    return Runge;
  }

} // namespace Belle

#endif /* TRUNGE_FLAG_ */
