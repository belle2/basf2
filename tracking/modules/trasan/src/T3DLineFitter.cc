//-----------------------------------------------------------------------------
// $Id: T3DLineFitter.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : T3DLineFitter.cc
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a 3D line.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2005/03/11 03:57:45  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.10  2003/12/25 12:03:30  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.9  2001/12/23 09:58:43  katayama
// removed Strings.h
//
// Revision 1.8  2001/12/19 02:59:42  katayama
// Uss find,istring
//
// Revision 1.7  2001/05/07 20:51:18  yiwasaki
// <float.h> included for linux
//
// Revision 1.6  2001/04/11 01:09:07  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.5  2000/10/05 23:54:20  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.4  2000/04/11 13:05:37  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.3  2000/01/18 07:03:56  yiwasaki
// T3DLineFitter update from K.Inami
//
//
// 2000/01/04 inami ; update : 2D(r-phi) fitting option added
//
// Revision 1.2  1999/10/30 10:12:07  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1999/10/21 15:45:12  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
//
//-----------------------------------------------------------------------------

#include <iostream>
#include <cfloat>




#include "tracking/modules/trasan/T3DLineFitter.h"
#include "tracking/modules/trasan/T3DLine.h"
#define HEP_SHORT_NAMES
//#include "panther/panther.h"
#ifndef PANTHER_RECCDC_WIRHIT_
#define PANTHER_RECCDC_WIRHIT_
struct reccdc_wirhit {
  int m_panther_dummy_;
  int m_ID;
  float m_ddl;
  float m_ddr;
  float m_erddl;
  float m_erddr;
  float m_pChiSq;
  int m_lr;
  int m_stat;
  int m_geo;
  int m_dat;
  int m_trk;
  float m_tdc;
  float m_adc;
};
#else
struct reccdc_wirhit;
#endif
#ifndef PANTHER_GEOCDC_LAYER_
#define PANTHER_GEOCDC_LAYER_
struct geocdc_layer {
  int m_panther_dummy_;
  int m_ID;
  float m_slant;
  float m_r;
  float m_zf;
  float m_zb;
  float m_rcsiz1;
  float m_rcsiz2;
  float m_pcsiz;
  float m_div;
  float m_offset;
  float m_shift;
  int m_wirst;
  int m_sup;
  float m_fdist;
  float m_bdist;
  float m_ft;
  float m_bt;
  float m_fpin;
  float m_bpin;
  float m_ftwist;
  float m_btwist;
  float m_fxoff;
  float m_fyoff;
  float m_fzoff;
  float m_bxoff;
  float m_byoff;
  float m_bzoff;
};
#endif
#ifndef PANTHER_GEOCDC_WIRE_
#define PANTHER_GEOCDC_WIRE_
struct geocdc_wire {
  int m_panther_dummy_;
  int m_ID;
  float m_xwb;
  float m_ywb;
  float m_zwb;
  float m_xwf;
  float m_ywf;
  float m_zwf;
  float m_slant;
  int m_cell;
  int m_layer;
  int m_stat;
  int m_lyr;
};
#endif
#ifndef PANTHER_RECCDC_WIRHIT_
#define PANTHER_RECCDC_WIRHIT_
struct reccdc_wirhit {
  int m_panther_dummy_;
  int m_ID;
  float m_ddl;
  float m_ddr;
  float m_erddl;
  float m_erddr;
  float m_pChiSq;
  int m_lr;
  int m_stat;
  int m_geo;
  int m_dat;
  int m_trk;
  float m_tdc;
  float m_adc;
};
#endif
#ifndef PANTHER_DATRGCDC_MCWIRHIT_
#define PANTHER_DATRGCDC_MCWIRHIT_
struct datcdc_mcwirhit {
  int m_panther_dummy_;
  int m_ID;
  float m_xw;
  float m_yw;
  float m_zw;
  float m_dist;
  float m_chrg;
  float m_xin;
  float m_yin;
  float m_zin;
  float m_xout;
  float m_yout;
  float m_zout;
  int m_lr;
  int m_stat;
  int m_geo;
  int m_dat;
  int m_hep;
  int m_trk;
  float m_px;
  float m_py;
  float m_pz;
  float m_pid;
  float m_tof;
  float m_tdc;
};
#endif
#ifndef PANTHER_RECCDC_TRK_
#define PANTHER_RECCDC_TRK_
struct reccdc_trk {
  int m_panther_dummy_;
  int m_ID;
  float m_helix[5];
  float m_pivot[3];
  float m_error[15];
  float m_chiSq;
  float m_ndf;
  float m_fiTerm;
  int m_nhits;
  int m_nster;
  int m_nclus;
  int m_stat;
  float m_mass;
};
#endif
#ifndef PANTHER_RECCDC_TRK_ADD_
#define PANTHER_RECCDC_TRK_ADD_
struct reccdc_trk_add {
  int m_panther_dummy_;
  int m_ID;
  int m_quality;
  int m_kind;
  int m_mother;
  int m_daughter;
  int m_decision;
  float m_likelihood[3];
  int m_stat;
  int m_rectrk;
};
#endif
#ifndef PANTHER_RECCDC_MCTRK2HEP_
#define PANTHER_RECCDC_MCTRK2HEP_
struct reccdc_mctrk2hep {
  int m_panther_dummy_;
  int m_ID;
  int m_wir;
  int m_clust;
  int m_trk;
  int m_hep;
};
#endif
#ifndef PANTHER_RECCDC_MCTRK_
#define PANTHER_RECCDC_MCTRK_
struct reccdc_mctrk {
  int m_panther_dummy_;
  int m_ID;
  int m_hep;
  float m_wirFrac;
  float m_wirFracHep;
  int m_charge;
  float m_ptFrac;
  float m_pzFrac;
  int m_quality;
};
#endif
#ifndef PANTHER_RECCDC_TIMING_
#define PANTHER_RECCDC_TIMING_
struct reccdc_timing {
  int m_panther_dummy_;
  int m_ID;
  float m_time;
  int m_quality;
};
#endif
#include "CLHEP/Geometry/Vector3D.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

#define T3DLine2DFit 2

  extern "C"
  void
  calcdc_driftdist_(int*,
                    int*,
                    int*,
                    float[3],
                    float[3],
                    float*,
                    float*,
                    float*);

  extern "C"
  void
  calcdc_tof2_(int*, float*, float*, float*);

  T3DLineFitter::T3DLineFitter(const std::string& name)
    : TFitter(name),
      _sag(true), _propagation(1), _tof(false)
  {
  }
  T3DLineFitter::T3DLineFitter(const std::string& name,
                               bool m_sag, int m_prop, bool m_tof)
    : TFitter(name),
      _sag(m_sag), _propagation(m_prop), _tof(m_tof)
  {
  }

  T3DLineFitter::~T3DLineFitter()
  {
  }

  void T3DLineFitter::sag(bool _in)
  {
    _sag = _in;
  }
  void T3DLineFitter::propagation(int _in)
  {
    _propagation = _in;
  }
  void T3DLineFitter::tof(bool _in)
  {
    _tof = _in;
  }

  void T3DLineFitter::drift(const T3DLine& t, const TLink& l,
                            float t0Offset,
                            double& distance, double& err) const
  {

    const Belle2::TRGCDCWireHit& h = *l.hit();
    const Point3D& onTrack = l.positionOnTrack();
    const Point3D& onWire = l.positionOnWire();
    unsigned leftRight = CellHitRight;
    //  if (onWire.cross(onTrack).z() < 0) leftRight = CellHitLeft;
    if ((onWire.x()*onTrack.y() - onWire.y()*onTrack.x()) < 0) leftRight = CellHitLeft;

    //...No correction...
    if ((t0Offset == 0.) && (_propagation == 0) && (! _tof)) {
      distance = l.drift(leftRight);
      err = h.dDrift(leftRight);
      return;
    }

    //...TOF correction...
    //  momentum ???? or velocity -> assumued light velocity
//cnv    float tof = 0.;
//cnv    if (_tof) {
      //    double length = ((onTrack - t.x0())*t.k())/t.k().mag();
//cnv      double tl = t.tanl();
//cnv      double length = ((onTrack - t.x0()) * t.k()) / sqrt(1 + tl * tl);
//cnv      static const double Ic = 1 / 29.9792; //1/[cm/ns]
//cnv      tof = length * Ic;
//cnv    }

    //...T0 and propagation corrections...
//int wire = h.wire()->id();
    int wire = h.wire().id();
    int side = leftRight;
    if (side == 0) side = -1;
    float p[3] = {float( -t.sinPhi0()), float(t.cosPhi0()), float(t.tanl())};
    float x[3] = {float(onWire.x()), float(onWire.y()), float(onWire.z())};
//cnv  float time = h.reccdc()->m_tdc + t0Offset - tof;
    float time = 0;
    float dist;
    float edist;
    int prop = _propagation;
    calcdc_driftdist_(& prop,
                      & wire,
                      & side,
                      p,
                      x,
                      & time,
                      & dist,
                      & edist);
    distance = (double) dist;
    err = (double) edist;
    return;
  }

  int T3DLineFitter::fit(TTrackBase& tb) const
  {
    return fit(tb, 0);
  }

  int T3DLineFitter::fit(TTrackBase& tb, float t0Offset) const
  {

    // std::cout<<"T3DLineFitter::fit  start"<<std::endl;

    //...Type check...
    if (tb.objectType() != Line3D) return TFitUnavailable;
    T3DLine& t = (T3DLine&) tb;

    //...Already fitted ?...
    if (t.fitted()) return TFitAlreadyFitted;

    //...Count # of hits...
    AList<TLink> cores = t.cores();
    unsigned nCores = cores.length();
    unsigned nStereoCores = TLink::nStereoHits(cores);

    //...Check # of hits...
    bool flag2D = false;
    if ((nStereoCores == 0) && (nCores > 3)) flag2D = true;
    else if ((nStereoCores < 2) || (nCores - nStereoCores < 3))
      return TFitErrorFewHits;

    //...Move pivot to ORIGIN...
    const HepGeom::Point3D<double> pivot_bak = t.pivot();
    t.pivot(ORIGIN);

    //...Setup...
    CLHEP::HepVector a(4), da(4);
    a = t.a();
    CLHEP::HepVector dxda(4);
    CLHEP::HepVector dyda(4);
    CLHEP::HepVector dzda(4);
    CLHEP::HepVector dDda(4);
    CLHEP::HepVector dchi2da(4);
    CLHEP::HepSymMatrix d2chi2d2a(4, 0);
    static const CLHEP::HepSymMatrix zero4(4, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    double factor = 1.0;
    int err = 0;
    CLHEP::HepSymMatrix e(2, 0);
    CLHEP::HepVector f(2);

    //...Fitting loop...
    unsigned nTrial = 0;
    while (nTrial < 100) {

      //...Set up...
      chi2 = 0;
      for (unsigned j = 0; j < 4; j++) dchi2da[j] = 0;
      d2chi2d2a = zero4;

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = cores[i++]) {
        const Belle2::TRGCDCWireHit& h = *l->hit();

        //...Cal. closest points...
        t.approach(*l, _sag);
        const Point3D& onTrack = l->positionOnTrack();
        const Point3D& onWire = l->positionOnWire();
        unsigned leftRight = CellHitRight;
        if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;

        //...Obtain drift distance and its error...
        double distance;
        double eDistance;
        drift(t, * l, t0Offset, distance, eDistance);
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        HepGeom::Vector3D<double> v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        HepGeom::Vector3D<double> vw;
        //...dxda...
        this->dxda(*l, t, dxda, dyda, dzda, vw);

        //...Chi2 related...
        dDda = (vmag > 0.)
               ? ((v.x() * (1. - vw.x() * vw.x()) -
                   v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
                  * dxda +
                  (v.y() * (1. - vw.y() * vw.y()) -
                   v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
                  * dyda +
                  (v.z() * (1. - vw.z() * vw.z()) -
                   v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
                  * dzda) / vmag : CLHEP::HepVector(4, 0);
        if (vmag <= 0.0) {
          std::cout << "    in fit " << onTrack << ", " << onWire;
          h.dump();
        }
        dchi2da += (dDistance / eDistance2) * dDda;
        d2chi2d2a += vT_times_v(dDda) / eDistance2;
        double pChi2 = dDistance * dDistance / eDistance2;
        chi2 += pChi2;

        //...Store results...
        l->update(onTrack, onWire, leftRight, pChi2);
      }

      //...Check condition...
      double change = chi2Old - chi2;

      if (fabs(change) < 1.0e-5) break;
      if (change < 0.) {
        a += factor * da; //recover
        factor *= 0.1;
      } else {
        chi2Old = chi2;
        if (flag2D) {
          f = dchi2da.sub(1, 2);
          e = d2chi2d2a.sub(1, 2);
          f = solve(e, f);
          da[0] = f[0];
          da[1] = f[1];
          da[2] = 0;
          da[3] = 0;
        } else {
          //...Cal. helix parameters for next loop...
          da = solve(d2chi2d2a, dchi2da);
        }
      }
      a -= factor * da;
      t.a(a);
      ++nTrial;
    }

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(4, 0);
    unsigned dim;
    if (flag2D) {
      dim = 2;
      CLHEP::HepSymMatrix Eb(3, 0), Ec(3, 0);
      Eb = d2chi2d2a.sub(1, 3);
      Ec = Eb.inverse(err);
      Ea[0][0] = Ec[0][0];
      Ea[0][1] = Ec[0][1];
      Ea[0][2] = Ec[0][2];
      Ea[1][1] = Ec[1][1];
      Ea[1][2] = Ec[1][2];
      Ea[2][2] = Ec[2][2];
    } else {
      dim = 4;
      Ea = d2chi2d2a.inverse(err);
    }

    //...Store information...
    if (! err) {
      t.a(a);
      t.Ea(Ea);
      t._fitted = true;
      if (flag2D) err = T3DLine2DFit;
    } else {
      err = TFitFailed;
    }

    t._ndf = nCores - dim;
    t._chi2 = chi2;

    //...Recover pivot...
    t.pivot(pivot_bak);

    return err;
  }

  int T3DLineFitter::dxda(const TLink& l, const T3DLine& t,
                          CLHEP::HepVector& dxda, CLHEP::HepVector& dyda, CLHEP::HepVector& dzda,
                          HepGeom::Vector3D<double> & wireDirection) const
  {
    //   onTrack = x0 + t * k
    //   onWire  = w0 + s * wireDirection
    //...Setup...
    const Belle2::TRGCDCWire& w = *l.wire();
    const HepGeom::Vector3D<double> k = t.k();
    const double cosPhi0 = t.cosPhi0();
    const double sinPhi0 = t.sinPhi0();
    const double dr = t.dr();
    const Point3D& onWire = l.positionOnWire();
    // const Point3D& onTrack = l.positionOnTrack();
    // const HepGeom::Vector3D<double> u = onTrack - onWire;
    const double t_t = (onWire - t.x0()).dot(k) / k.mag2();

    //...Sag correction...
    HepGeom::Point3D<double> xw = w.xyPosition();
    HepGeom::Point3D<double> wireBackwardPosition = w.backwardPosition();
    wireDirection = w.direction();
    std::cout << "T3DLineFitter::dxda !!! sag correction is not implemented"
              << std::endl;
//   if (_sag) w.wirePosition(onWire.z(),
//         xw,
//         wireBackwardPosition,
//         (HepGeom::Vector3D<double> &) wireDirection);
    const HepGeom::Vector3D<double> & v = wireDirection;

    // onTrack = x0 + t * k
    // onWire  = w0 + s * v

    const double v_k = v.dot(k);
    const double tvk = v_k * v_k - k.mag2();
    if (tvk == 0) return(-1);

    const HepGeom::Vector3D<double> & dxdt_a = k;

    const HepGeom::Vector3D<double> dxda_t[4]
      = {HepGeom::Vector3D<double>(cosPhi0, sinPhi0, 0),
         HepGeom::Vector3D<double>(-dr* sinPhi0 - t_t* cosPhi0, dr* cosPhi0 - t_t* sinPhi0, 0),
         HepGeom::Vector3D<double>(0, 0, 1),
         HepGeom::Vector3D<double>(0, 0, t_t)
        };

    const HepGeom::Vector3D<double> dx0da[4]
      = {HepGeom::Vector3D<double>(cosPhi0, sinPhi0, 0),
         HepGeom::Vector3D<double>(-dr* sinPhi0, dr* cosPhi0, 0),
         HepGeom::Vector3D<double>(0, 0, 1),
         HepGeom::Vector3D<double>(0, 0, 0)
        };

    const HepGeom::Vector3D<double> dkda[4]
      = {HepGeom::Vector3D<double>(0, 0, 0),
         HepGeom::Vector3D<double>(-cosPhi0, -sinPhi0, 0),
         HepGeom::Vector3D<double>(0, 0, 0),
         HepGeom::Vector3D<double>(0, 0, 1)
        };

    const HepGeom::Vector3D<double> d = t.x0() - wireBackwardPosition;
    const HepGeom::Vector3D<double> kvkv = k - v_k * v;

    for (int i = 0; i < 4; i++) {
      const double v_dkda = v.dot(dkda[i]);

      const double dtda = dx0da[i].dot(kvkv) / tvk
                          + d.dot(dkda[i] - v_dkda * v) / tvk
                          - d.dot(kvkv) * 2 * (v_k * v_dkda - k.dot(dkda[i])) / (tvk * tvk);

      const HepGeom::Vector3D<double> dxda3D = dxda_t[i] + dtda * dxdt_a;

      dxda[i] = dxda3D.x();
      dyda[i] = dxda3D.y();
      dzda[i] = dxda3D.z();
    }

    return 0;
  }

} // namespace Belle

