//-----------------------------------------------------------------------------
// $Id: T3DLine.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : T3DLine.cc
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a 3D line in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2003/12/25 12:03:30  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.4  2000/04/11 13:05:37  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.3  2000/03/17 07:00:49  katayama
// Module function modified
//
// Revision 1.2  1999/12/14 15:12:22  yiwasaki
// Update from K.Inami
//
//
// inami ; debug : pivot calc. , cache at a(const CLHEP::HepVector &)
//
// Revision 1.1  1999/10/21 15:45:12  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/T3DLine.h"
#include "tracking/modules/trasan/T3DLineFitter.h"
#include "trg/cdc/Wire.h"
#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  const T3DLineFitter T3DLine::_fitter = T3DLineFitter("T3DLine Default fitter");

  T3DLine::T3DLine()
    : TTrackBase(),
      _pivot(ORIGIN),
      _dr(0),
      _phi0(0),
      _dz(0),
      _tanl(0),
      _Ea(CLHEP::HepSymMatrix(4, 0)),
      _chi2(0),
      _ndf(0),
      _cos_phi0(1),
      _sin_phi0(0)
  {

    //...Set a default fitter...
    fitter(& T3DLine::_fitter);

    _fitted = false;
    _fittedWithCathode = false;
  }

  T3DLine::T3DLine(const TTrack& a)
    : TTrackBase((TTrackBase&) a),
      _pivot(a.helix().pivot()),
      _dr(a.helix().dr()),
      _phi0(a.helix().phi0()),
      _dz(a.helix().dz()),
      _tanl(a.helix().tanl()),
      _Ea(CLHEP::HepSymMatrix(4, 0)),
      _chi2(0),
      _ndf(0)
  {

    _cos_phi0 = cos(_phi0);
    _sin_phi0 = sin(_phi0);

    //...Set a default fitter...
    fitter(& T3DLine::_fitter);

    _fitted = false;
    _fittedWithCathode = false;
  }

  T3DLine::~T3DLine()
  {
  }

  T3DLine::T3DLine(const T3DLine& a)
    : TTrackBase((TTrackBase&) a),
      _pivot(a.pivot()),
      _dr(a.dr()),
      _phi0(a.phi0()),
      _dz(a.dz()),
      _tanl(a.tanl()),
      _Ea(a.Ea()),
      _chi2(a.chi2()),
      _ndf(a.ndf())
  {

    _cos_phi0 = cos(_phi0);
    _sin_phi0 = sin(_phi0);

    //...Set a default fitter...
    fitter(& T3DLine::_fitter);

    _fitted = false;
    _fittedWithCathode = false;
  }

  double T3DLine::dr(void) const
  {
    return _dr;
  }

  double T3DLine::phi0(void) const
  {
    return _phi0;
  }

  double T3DLine::dz(void) const
  {
    return _dz;
  }

  double T3DLine::tanl(void) const
  {
    return _tanl;
  }

  double T3DLine::cosPhi0(void) const
  {
    return _cos_phi0;
  }

  double T3DLine::sinPhi0(void) const
  {
    return _sin_phi0;
  }

  const Point3D& T3DLine::pivot(void) const
  {
    return _pivot;
  }

  CLHEP::HepVector
  T3DLine::a(void) const
  {
    CLHEP::HepVector ta(4);
    ta[0] = _dr;
    ta[1] = _phi0;
    ta[2] = _dz;
    ta[3] = _tanl;
    return(ta);
  }

  const CLHEP::HepSymMatrix& T3DLine::Ea(void) const
  {
    return(_Ea);
  }

  THelix T3DLine::helix(void) const
  {
    CLHEP::HepVector a(5);
    a[0] = _dr; a[1] = _phi0; a[2] = 1e-10; a[3] = _dz; a[4] = _tanl;
    THelix _helix(_pivot, a);
    return _helix;
  }

  unsigned T3DLine::ndf(void) const
  {
    return _ndf;
  }

  double T3DLine::chi2(void) const
  {
    return _chi2;
  }

  double T3DLine::reducedchi2(void) const
  {
    if (_ndf == 0) {
      std::cout << "error at T3DLine::reducedchi2  ndf=0" << std::endl;
      return 0;
    }
    return (_chi2 / _ndf);
  }

  Point3D T3DLine::x(double t) const
  {
    double tx = _pivot.x() + _dr * _cos_phi0 - t * _sin_phi0;
    double ty = _pivot.y() + _dr * _sin_phi0 + t * _cos_phi0;
    double tz = _pivot.z() + _dz             + t * _tanl;
    HepGeom::Point3D<double> p(tx, ty, tz);
    return p;
  }

  Point3D T3DLine::x0(void) const
  {
    double tx = _pivot.x() + _dr * _cos_phi0;
    double ty = _pivot.y() + _dr * _sin_phi0;
    double tz = _pivot.z() + _dz;
    HepGeom::Point3D<double> p(tx, ty, tz);
    return p;
  }
  HepGeom::Vector3D<double> T3DLine::k(void) const
  {
    HepGeom::Point3D<double> p(-_sin_phi0, _cos_phi0, _tanl);
    return p;
  }

  const Point3D& T3DLine::pivot(const Point3D& newpivot)
  {
    double dr = (_pivot.x() - newpivot.x()) * _cos_phi0
                + (_pivot.y() - newpivot.y()) * _sin_phi0 + _dr;
    double dz = _pivot.z() - newpivot.z() + _dz
                + _tanl * ((_pivot.x() - newpivot.x()) * _sin_phi0
                           + (newpivot.y() - _pivot.y()) * _cos_phi0);
    _dr = dr;
    _dz = dz;
    _pivot = newpivot;
    return _pivot;
  }

  void T3DLine::set(const Point3D& t_pivot,
                    double t_dr, double t_phi0, double t_dz, double t_tanl)
  {

    _pivot = t_pivot;
    _dr = t_dr;
    _phi0 = t_phi0;
    _dz = t_dz;
    _tanl = t_tanl;
    _cos_phi0 = cos(_phi0);
    _sin_phi0 = sin(_phi0);
  }

  CLHEP::HepVector T3DLine::a(const CLHEP::HepVector& ta)
  {
    _dr = ta[0];
    _phi0 = ta[1];
    _dz = ta[2];
    _tanl = ta[3];
    _cos_phi0 = cos(_phi0);
    _sin_phi0 = sin(_phi0);
    return(ta);
  }

  const CLHEP::HepSymMatrix& T3DLine::Ea(const CLHEP::HepSymMatrix& tEa)
  {
    _Ea = tEa;
    return(_Ea);
  }

  int T3DLine::approach(TLink& l, bool doSagCorrection) const
  {

    const Belle2::TRGCDCWire& w = *l.wire();
    HepGeom::Point3D<double> xw = w.xyPosition();
    HepGeom::Point3D<double> wireBackwardPosition = w.backwardPosition();
    HepGeom::Vector3D<double> v = w.direction();

    HepGeom::Point3D<double> onWire, onTrack;

    if (approach_line(wireBackwardPosition, v, onWire, onTrack) < 0)
      return(-1);

    // onWire,onTrack filled

    if (!doSagCorrection) {
      l.positionOnWire(onWire);
      l.positionOnTrack(onTrack);
      return(0);        // no sag correction
    }
    // Sag correction
    //   loop for sag correction
//   double onWire_y = onWire.y();
//   double onWire_z = onWire.z();

    unsigned nTrial = 1;
    std::cout << "T3DLine::approach !!! sag correction is not implemented"
              << std::endl;
//   while(nTrial<100){
//     w.wirePosition(onWire_z,xw,wireBackwardPosition,(Vector3D&)v);
//     if(approach_line(wireBackwardPosition,v,onWire,onTrack)<0)
//       return(-1);
//     if(fabs(onWire_y - onWire.y())<0.0001) break;  // |dy|< 1 micron
//     onWire_y = onWire.y();
//     onWire_z = onWire.z();

//     nTrial++;
//   }

    l.positionOnWire(onWire);
    l.positionOnTrack(onTrack);
    return(nTrial);
  }

  int T3DLine::approach_line(const Point3D& w0, const HepGeom::Vector3D<double> & v,
                             Point3D& onLine, Point3D& onTrack) const
  {
    //  line = [w0] + s * [v]    -> [onLine]
    //  trk  = [x0] + t * [k]    -> [onTrack]
    //  if [v]//[k] then return(-1) error

    const HepGeom::Vector3D<double> k = this->k();
    const double v_k = v.dot(k);
    const double v_2 = v.mag2();
    const double k_2 = k.mag2();
    const double tk = v_k * v_k - v_2 * k_2;
    if (tk == 0) return(-1);

    const HepGeom::Vector3D<double> x0 = this->x0();
    const HepGeom::Vector3D<double> dx = x0 - w0;
    const double t = dx.dot(v_2 * k - v_k * v) / tk;
    const double s = dx.dot(v_k * k - k_2 * v) / tk;

    onLine = w0 + s * v;
    onTrack = x0 + t * k;
    return(0);
  }

  int T3DLine::approach_point(const Point3D& p0, Point3D& onTrack) const
  {
    //  trk  = [x0] + t * [k]    -> [onTrack]
    //  if [v]//[k] then return(-1) error

    const HepGeom::Vector3D<double> k = this->k();
    const HepGeom::Vector3D<double> x0 = this->x0();
    const HepGeom::Vector3D<double> dx = p0 - x0;
    const double t = dx.dot(k) / k.mag2();

    onTrack = x0 + t * k;
    return(0);
  }

} // namespace Belle

