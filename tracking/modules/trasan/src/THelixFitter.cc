//-----------------------------------------------------------------------------
// $Id: THelixFitter.cc 10693 2008-10-20 01:40:36Z hitoshi $
//-----------------------------------------------------------------------------
// Filename : THelixFitter.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a helix.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.42  2005/11/03 23:20:11  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.41  2005/03/11 03:57:50  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.40  2004/05/28 21:55:55  katayama
// Bug fixed by Kakuno san
//
// Revision 1.39  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.38  2002/02/26 22:38:59  yiwasaki
// bug fixes in debug mode
//
// Revision 1.37  2002/02/22 06:37:42  katayama
// Use __sparc
//
// Revision 1.36  2002/02/21 23:49:15  katayama
// For -ansi and other warning flags
//
// Revision 1.35  2001/12/23 09:58:47  katayama
// removed Strings.h
//
// Revision 1.34  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.33  2001/05/07 20:51:18  yiwasaki
// <float.h> included for linux
//
// Revision 1.32  2001/04/25 02:48:08  yiwasaki
// one line for test is removed
//
// Revision 1.31  2001/04/25 02:36:00  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.30  2001/04/18 01:29:24  yiwasaki
// helix fitter speed up by jt
//
// Revision 1.29  2001/04/12 07:10:56  yiwasaki
// Trasan 3.00 RC4 : new stereo code for curl
//
// Revision 1.28  2001/04/11 01:09:10  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.27  2000/12/25 06:53:51  yiwasaki
// update from H.Ozaki
//
// Revision 1.26  2000/10/10 06:04:24  yiwasaki
// Trasan 2.11 : curl builder, helix fitter, perfect finder
//
// Revision 1.25  2000/10/05 23:54:23  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.24  2000/04/11 13:05:44  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.23  2000/02/23 08:45:06  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.22  2000/02/17 06:02:16  yiwasaki
// track charge is determined by fitting
//
// Revision 1.21  1999/11/23 10:30:19  yiwasaki
// ALPHA in TRGCDCUtil is replaced by THelix::ConstantAlpha
//
// Revision 1.20  1999/11/19 09:13:08  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.19  1999/10/20 12:04:15  yiwasaki
// Trasan 1.64b : new T0 determination methode by H.Ozaki
//
// Revision 1.18  1999/09/13 05:57:55  yiwasaki
// Trasan 1.62b release : calcdc_sag2 -> calcdc_sag3
//
// Revision 1.17  1999/08/25 06:25:50  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.16  1999/07/23 03:53:24  yiwasaki
// Trasan 1.57b release : minor changes only
//
// Revision 1.15  1999/07/17 06:42:58  yiwasaki
// THelixFitter has tof correction, Trasan time has option for tof correction
//
// Revision 1.14  1999/07/15 08:43:20  yiwasaki
// Trasan 1.55b release : Curl finder # of hits protection, bug in TManager for MC, helix fitter # of hits protection, fast finder improved
//
// Revision 1.13  1999/07/01 08:15:24  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.12  1999/06/29 05:14:40  yiwasaki
// Trasan 1.49a release : minor change : Support for invalid hits in TRGCDC
//
// Revision 1.11  1999/06/17 01:39:18  yiwasaki
// Trasan 1.441 release : default mom. cut = 0. to recon. cosmics
//
// Revision 1.10  1999/06/16 08:29:51  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.9  1999/06/14 12:40:21  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.8  1999/06/10 09:44:51  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.7  1999/06/10 05:05:11  yiwasaki
// disable outputs
//
// Revision 1.6  1999/06/10 00:27:28  yiwasaki
// Trasan 1.39 release : TTrack::approach bug fix
//
// Revision 1.5  1999/03/12 13:11:57  yiwasaki
// Trasan 1.26 : bug fix in RECCDC_TRK output
//
// Revision 1.4  1999/03/11 23:27:14  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.3  1999/03/03 15:46:02  yiwasaki
// flag bug fix
//
// Revision 1.2  1999/03/03 09:14:50  yiwasaki
// TRGCDCWireHit::WireHitValid -> WireHitFindingValid, WireHitFittingValid flag is checked in THelixFitter::fit
//
// Revision 1.1  1999/01/11 03:03:11  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

/* for copysign */
#if defined(__sparc)
#  if defined(__EXTENSIONS__)
#    include <cmath>
#  else
#    define __EXTENSIONS__
#    include <cmath>
#    undef __EXTENSIONS__
#  endif
#elif defined(__GNUC__)
#  if defined(_XOPEN_SOURCE)
#    include <cmath>
#  else
#    define _XOPEN_SOURCE
#    include <cmath>
#    undef _XOPEN_SOURCE
#  endif
#endif

#define HEP_SHORT_NAMES
#include <cfloat>



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
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/DiagMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "tracking/modules/trasan/THelixFitter.h"
#include "tracking/modules/trasan/TTrack.h"
#ifdef TRASAN_DEBUG

#include "tracking/modules/trasan/TDebugUtilities.h"
#include "tracking/modules/trasan/TConformalFinder.h"
#include "trg/cdc/WireHitMC.h"
#endif

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

// speed up option by j.tanaka (2001/04/14)
#define OPTJT

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
  calcdc_driftdist3_(int*,
                     int*,
                     float[3],
                     float[3],
                     float*,
                     float[2],
                     float[2],
                     float[2]);

  extern "C"
  void
  calcdc_tof2_(int*, float*, float*, float*);

#ifdef TRASAN_DEBUG
// BelleHistogram * _nCall[8];
// BelleHistogram * _nTrial[8];
// BelleHistogram * _pull[2][2][8];
// BelleHistogram * _nTrialNegative;
// BelleHistogram * _nTrialPositive;
  bool first = true;
#endif

  //#define NTrailMax 100
#define Convergence 1.0e-5

  extern float
  TrasanTHelixFitterChisqMax;
  extern int
  TrasanTHelixFitterNtrialMax;

  THelixFitter::THelixFitter(const std::string& name)
    : TFitter(name),
      _fit2D(false),
      _freeT0(false),
      _sag(false),
      _propagation(false),
      _tof(false),
      _tanl(false),
      _pre_chi2(0.),
      _fitted_chi2(0.)
  {
  }

  THelixFitter::~THelixFitter()
  {
  }

#ifdef OPTJT
// speed up
  int
  THelixFitter::main(TTrackBase& b, float t0Offset,
                     double* pre_chi2, double* fitted_chi2) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "THlxFttr::main";
    EnterStage(stage);

//cnv     if (first) {
//  first = false;
//  extern BelleTupleManager * BASF_Histogram;
//  BelleTupleManager * m = BASF_Histogram;
//  _nCall[0] = m->histogram("HF nCall all", 1, 0., 1.);
//  _nCall[1] = m->histogram("HF nCall conf f2d l0", 1, 0., 1.);
//  _nCall[2] = m->histogram("HF nCall conf f3d l0", 1, 0., 1.);
//  _nCall[3] = m->histogram("HF nCall conf f2d l1", 1, 0., 1.);
//  _nCall[4] = m->histogram("HF nCall conf f3d l1", 1, 0., 1.);
//  _nCall[5] = m->histogram("HF nCall conf s2d", 1, 0., 1.);
//  _nCall[6] = m->histogram("HF nCall conf s3d", 1, 0., 1.);
//  _nCall[7] = m->histogram("HF nCall other", 1, 0., 1.);
//  _nTrial[0] = m->histogram("HF nTrial all", 100, 0., 100.);
//  _nTrial[1] = m->histogram("HF nTrial conf f2d l0", 100, 0., 100.);
//  _nTrial[2] = m->histogram("HF nTrial conf f3d l0", 100, 0., 100.);
//  _nTrial[3] = m->histogram("HF nTrial conf f2d l1", 100, 0., 100.);
//  _nTrial[4] = m->histogram("HF nTrial conf f3d l1", 100, 0., 100.);
//  _nTrial[5] = m->histogram("HF nTrial conf s2d", 100, 0., 100.);
//  _nTrial[6] = m->histogram("HF nTrial conf s3d", 100, 0., 100.);
//  _nTrial[7] = m->histogram("HF nTrial other", 100, 0., 100.);
//  _pull[0][0][0] = m->histogram("HF pull ax true all",
//              100, 0., 5000.);
//  _pull[0][0][2] = m->histogram("HF pull ax true conf f3d l0",
//              100, 0., 5000.);
//  _pull[0][0][4] = m->histogram("HF pull ax true conf f3d l1",
//              100, 0., 5000.);
//  _pull[0][0][6] = m->histogram("HF pull ax true conf s3d",
//              100, 0., 5000.);
//  _pull[0][0][7] = m->histogram("HF pull ax true other",
//              100, 0., 5000.);
//  _pull[1][0][0] = m->histogram("HF pull st true all",
//              100, 0., 5000.);
//  _pull[1][0][2] = m->histogram("HF pull st true conf f3d l0",
//              100, 0., 5000.);
//  _pull[1][0][4] = m->histogram("HF pull st true conf f3d l1",
//              100, 0., 5000.);
//  _pull[1][0][6] = m->histogram("HF pull st true conf s3d",
//              100, 0., 5000.);
//  _pull[1][0][7] = m->histogram("HF pull st true other",
//              100, 0., 5000.);
//  _pull[0][1][0] = m->histogram("HF pull ax wrong all",
//              100, 0., 5000.);
//  _pull[0][1][2] = m->histogram("HF pull ax wrong conf f3d l0",
//              100, 0., 5000.);
//  _pull[0][1][4] = m->histogram("HF pull ax wrong conf f3d l1",
//              100, 0., 5000.);
//  _pull[0][1][6] = m->histogram("HF pull ax wrong conf s3d",
//              100, 0., 5000.);
//  _pull[0][1][7] = m->histogram("HF pull ax wrong other",
//              100, 0., 5000.);
//  _pull[1][1][0] = m->histogram("HF pull st wrong all",
//              100, 0., 5000.);
//  _pull[1][1][2] = m->histogram("HF pull st wrong conf f3d l0",
//              100, 0., 5000.);
//  _pull[1][1][4] = m->histogram("HF pull st wrong conf f3d l1",
//              100, 0., 5000.);
//  _pull[1][1][6] = m->histogram("HF pull st wrong conf s3d",
//              100, 0., 5000.);
//  _pull[1][1][7] = m->histogram("HF pull st wrong other",
//              100, 0., 5000.);
//  _nTrialPositive = m->histogram("HF nTrial +", 100, 0., 100.);
//  _nTrialNegative = m->histogram("HF nTrial -", 100, 0., 100.);
//     }
// #ifdef TRASAN_DEBUG_DETAIL
// //     _nCall[0]->accumulate(.5);
// //     if (TConformalFinder::_stage == ConformalOutside)
// //   _nCall[7]->accumulate(.5);
// //     else if (TConformalFinder::_stage == ConformalFast2DLevel0)
// //   _nCall[1]->accumulate(.5);
// //     else if (TConformalFinder::_stage == ConformalFast3DLevel0)
// //   _nCall[2]->accumulate(.5);
// //     else if (TConformalFinder::_stage == ConformalFast2DLevel1)
// //   _nCall[3]->accumulate(.5);
// //     else if (TConformalFinder::_stage == ConformalFast3DLevel1)
// //   _nCall[4]->accumulate(.5);
// //     else if (TConformalFinder::_stage == ConformalSlow2D)
// //   _nCall[5]->accumulate(.5);
// //     else if (TConformalFinder::_stage == ConformalSlow3D)
// //   _nCall[6]->accumulate(.5);
// //     bool posi = true;
// #endif
// //  const TTrackHEP & hep = Links2HEP(b.links());
#endif

    //...Initialize
    _pre_chi2 = _fitted_chi2 = 0.;
    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    //...Type check...
    if (b.objectType() != Track) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return TFitUnavailable;
    }
    TTrack& t = (TTrack&) b;

    //...Already fitted ?...
    if (t.fitted()) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return TFitAlreadyFitted;
    }

    //...Count # of hits...
    AList<TLink> cores = t.cores();
    if (_fit2D) cores = TLink::axialHits(cores);
    unsigned nCores = cores.length();
    unsigned nStereoCores = TLink::nStereoHits(cores);

    //...2D or 3D...
    bool fitBy2D = _fit2D;
    if (! fitBy2D) fitBy2D = (! nStereoCores);

    //...Check # of hits...
    if (! fitBy2D) {
      if ((nStereoCores < 2) || (nCores - nStereoCores < 3)) {
#ifdef TRASAN_DEBUG
        LeaveStage(stage);
#endif
        return TFitErrorFewHits;
      }
    } else {
      if (nCores < 3) {
#ifdef TRASAN_DEBUG
        LeaveStage(stage);
#endif
        return TFitErrorFewHits;
      }
    }
    //...Setup...
    CLHEP::HepVector a(5), da(5);
#if defined(BELLE_DEBUG)
    try {
#endif
      a = t.helix().a();
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "THelixFitter::helix is invalid" << std::endl;
      return TFitFailed;
    }
#endif
    CLHEP::HepVector dxda(5);
    CLHEP::HepVector dyda(5);
    CLHEP::HepVector dzda(5);
    CLHEP::HepVector dDda(5);
    CLHEP::HepVector dchi2da(5);
    CLHEP::HepSymMatrix d2chi2d2a(5, 0);
    static const CLHEP::HepSymMatrix zero5(5, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    const double convergence = Convergence;
//  bool allAxial = true;
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;//jtanaka0715

    //...For bad hit rejection...(by JT, 2001/04/12)...
    int flagBad = 0;
    if (TrasanTHelixFitterChisqMax != 0.)
      flagBad = 1;
    AList<TLink> initBadWires;
    unsigned nInitBadWires = 0;
    CLHEP::HepVector initBadDchi2da(5);
    CLHEP::HepSymMatrix initBadD2chi2d2a(5, 0);
    for (unsigned j = 0; j < 5; ++j) initBadDchi2da[j] = 0.;
    double initBadChi2 = 0.;

    //...Initializing position for sag correction
    if (_sag) {
      unsigned i = 0;
      while (TLink* l = cores[i++]) {
        t.approach(*l);
      }
    }

//...Fitting loop...
    unsigned nTrial = 0;
    //    while (nTrial < NTrailMax) {
    while (nTrial < (unsigned) TrasanTHelixFitterNtrialMax) {

      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
      d2chi2d2a = zero5;

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = cores[i++]) {
        const Belle2::TRGCDCWireHit& h = * l->hit();

        //...Cal. closest points...
        t.approach(* l, _sag);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();
        unsigned leftRight = (onWire.cross(onTrack).z() < 0.)
                             ? CellHitLeft : CellHitRight;

        //...Obtain drift distance and its error...
        double distance;
        double eDistance;
        drift(t, * l, t0Offset, distance, eDistance);
        double inv_eDistance2 = 1. / (eDistance * eDistance);

        //...Residual...
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda(* l, t.helix(), dPhi, dxda, dyda, dzda);

        //...Chi2 related...
        // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
        // HepGeom::Vector3D<double> vw = h.wire().direction();
        double vw[3] = { h.wire().direction().x(),
                         h.wire().direction().y(),
                         h.wire().direction().z()
                       };
        double vwxy = vw[0] * vw[1];
        double vwyz = vw[1] * vw[2];
        double vwzx = vw[2] * vw[0];
        static CLHEP::HepVector nv(5, 0);
        dDda = (vmag > 0.)
               ? ((v.x() * (1. - vw[0] * vw[0]) -
                   v.y() * vwxy - v.z() * vwzx)
                  * dxda +
                  (v.y() * (1. - vw[1] * vw[1]) -
                   v.z() * vwyz - v.x() * vwxy)
                  * dyda +
                  (v.z() * (1. - vw[2] * vw[2]) -
                   v.x() * vwzx - v.y() * vwyz)
                  * dzda) / vmag
               : nv;
#ifdef TRASAN_DEBUG_DETAIL
        if (vmag <= 0.0) {
          std::cout  << Stage() << " !!! in fit " << onTrack << ", "
                     << onWire;
          h.dump();
        }
#endif
        double pChi2 = dDistance * dDistance * inv_eDistance2;

        //...Bad hit rejection...
        if (flagBad && nTrial == 0) {
          if (pChi2 > TrasanTHelixFitterChisqMax) {
            initBadWires.append(l);
            initBadDchi2da += (dDistance * inv_eDistance2) * dDda;
            initBadD2chi2d2a += vT_times_v(dDda) * inv_eDistance2;
            initBadChi2 += pChi2;
          }
        } else {
          dchi2da += (dDistance * inv_eDistance2) * dDda;
          d2chi2d2a += vT_times_v(dDda) * inv_eDistance2;
          chi2 += pChi2;

          //...Store results...
          l->update(onTrack, onWire, leftRight, pChi2);
        }

#ifdef TRASAN_DEBUG
//        if ((! fitBy2D) && (nTrial == 0)) {
//      unsigned as = 0;
//      if (l->hit()->wire().stereo()) as = 1;
//      unsigned mt = 0;
//      if (& hep != l->hit()->mc()->hep()) mt = 1;

//      _pull[as][mt][0]->accumulate(pChi2);
//      if (TConformalFinder::_stage == ConformalOutside)
//          _pull[as][mt][7]->accumulate(pChi2);
//      else if (TConformalFinder::_stage == ConformalFast3DLevel0)
//          _pull[as][mt][2]->accumulate(pChi2);
//      else if (TConformalFinder::_stage == ConformalFast3DLevel1)
//          _pull[as][mt][4]->accumulate(pChi2);
//      else if (TConformalFinder::_stage == ConformalSlow3D)
//          _pull[as][mt][6]->accumulate(pChi2);
//        }
#endif
      }

      //...Bad hit rejection...
      if (flagBad && nTrial == 0) {
        if ((initBadWires.length() == 1 || initBadWires.length() == 2) &&
            nCores >= 20 &&
            chi2 / (double)(nCores - initBadWires.length()) < 10.) {
          cores.remove(initBadWires);
          nInitBadWires = initBadWires.length();
        } else if (initBadWires.length() != 0) {
          dchi2da += initBadDchi2da;
          d2chi2d2a += initBadD2chi2d2a;
          chi2 += initBadChi2;
        }
      }

      //...Save chi2 information...
      if (nTrial == 0) {
        _pre_chi2 = chi2;
        _fitted_chi2 = chi2;
      } else {
        _fitted_chi2 = chi2;
      }

      //...Check condition...
      double change = chi2Old - chi2;
      if (fabs(change) < convergence) break;
      if (change < 0.) {
//          a += factor * da;
//          t._helix->a(a);
//          break;
        factor = 0.5;
      }
      chi2Old = chi2;

      //...Cal. helix parameters for next loop...
      if (fitBy2D) {
        f = dchi2da.sub(1, 3);
        e = d2chi2d2a.sub(1, 3);
        f = solve(e, f);
        da[0] = f[0];
        da[1] = f[1];
        da[2] = f[2];
        da[3] = 0.;
        da[4] = 0.;
      } else {
        da = solve(d2chi2d2a, dchi2da);
      }
      static const double kp_max = 1.e10;
      static const double kp_min = 1.e-30;
      const double kp_test = std::abs(a[2] - factor * da[2]);
      if (kp_test < kp_min || kp_test > kp_max) {
        err = 1;
        break;
      }

      a -= factor * da;
#if defined(BELLE_DEBUG)
      try {
#endif
        t._helix->a(a);
#if defined(BELLE_DEBUG)
      } catch (std::string& e) {
        std::cout << "THelixFitter::helix is invalid" << std::endl;
        err = 1;
        break;
      }
#endif
      ++nTrial;

      // jtanaka 001008
      //if( fabs(a[3]) > 200. ){
      // yiwasaki 001010
      if (fabs(a[3]) > 1000.) {
        // stop "fit" and return error.
        // std::cout << "Stop Fit... " << a << std::endl;
        err = 1;
        break;
      }
#ifdef TRASAN_DEBUG_DETAIL
      std::string name = "?";
      if (b.type() == Track) name = ((TTrack&) b).name();
      std::cout << Tab() << name << " fit" << nTrial - 1 << ":chi2=" << chi2
                << ":change=" << change << std::endl;
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
//     _nTrial[0]->accumulate(float(nTrial) + .5);
//     if (TConformalFinder::_stage == ConformalOutside)
//  _nTrial[7]->accumulate(float(nTrial) + .5);
//     else if (TConformalFinder::_stage == ConformalFast2DLevel0)
//  _nTrial[1]->accumulate(float(nTrial) + .5);
//     else if (TConformalFinder::_stage == ConformalFast3DLevel0)
//  _nTrial[2]->accumulate(float(nTrial) + .5);
//     else if (TConformalFinder::_stage == ConformalFast2DLevel1)
//  _nTrial[3]->accumulate(float(nTrial) + .5);
//     else if (TConformalFinder::_stage == ConformalFast3DLevel1)
//  _nTrial[4]->accumulate(float(nTrial) + .5);
//     else if (TConformalFinder::_stage == ConformalSlow2D)
//  _nTrial[5]->accumulate(float(nTrial) + .5);
//     else if (TConformalFinder::_stage == ConformalSlow3D)
//  _nTrial[6]->accumulate(float(nTrial) + .5);

//     if (posi) _nTrialPositive->accumulate((float) nTrial + .5);
//     else _nTrialNegative->accumulate((float) nTrial + .5);
#endif
    // if (!err) by nk
    if (!err) {

      //...Cal. error matrix...
      CLHEP::HepSymMatrix Ea(5, 0);
      unsigned dim;
      if (fitBy2D) {
        dim = 3;
        CLHEP::HepSymMatrix Eb(3, 0), Ec(3, 0);
#if defined(BELLE_DEBUG)
        try {
#endif
          Eb = d2chi2d2a.sub(1, 3);
          Ec = Eb.inverse(err);
          Ea[0][0] = Ec[0][0];
          Ea[0][1] = Ec[0][1];
          Ea[0][2] = Ec[0][2];
          Ea[1][1] = Ec[1][1];
          Ea[1][2] = Ec[1][2];
          Ea[2][2] = Ec[2][2];
#if defined(BELLE_DEBUG)
        } catch (...) {
          err = 1;
          std::cout << "Matrix inversion gave a floating exception" << std::endl;
        }
#endif
      } else {
        dim = 5;
#if defined(BELLE_DEBUG)
        try {
#endif
          Ea = d2chi2d2a.inverse(err);
#if defined(BELLE_DEBUG)
        } catch (...) {
          err = 1;
          std::cout << "Matrix inversion gave a floating exception" << std::endl;
        }
#endif
      }



      //...Store information...
      if (! err) {

        t._helix->a(a);
        t._helix->Ea(Ea);
        t._fitted = true;
      } else {
        err = TFitFailed;
      }

      t._charge = copysign(1., a[2]);
      t._ndf = nCores - dim - nInitBadWires;
      t._chi2 = chi2;

    } // if (!err) by nk
    else {
      err = TFitFailed;
    }

    //...Treatment for bad wires...
    if (nInitBadWires) {
      for (unsigned i = 0; i < nInitBadWires; i++) {
        TLink* l = initBadWires[i];
        t.approach(* l, _sag);
//      double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        unsigned leftRight = (onWire.cross(onTrack).z() < 0.)
                             ? CellHitLeft : CellHitRight;
        double distance;
        double eDistance;
        drift(t, * l, t0Offset, distance, eDistance);
        double inv_eDistance2 = 1. / (eDistance * eDistance);
        double dDistance = vmag - distance;
        double pChi2 = dDistance * dDistance * inv_eDistance2;
        l->update(onTrack, onWire, leftRight, pChi2);
      }
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "#rejected hits=" << nInitBadWires << std::endl;
#endif
    }

    if (pre_chi2) * pre_chi2 = _pre_chi2;
    if (fitted_chi2) * fitted_chi2 = _fitted_chi2;

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
    return err;
  }
#else
  int
  THelixFitter::main(TTrackBase& b, float t0Offset,
                     double* pre_chi2, double* fitted_chi2) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    if (first) {
      first = false;
//cnv   extern BelleTupleManager * BASF_Histogram;
//  BelleTupleManager * m = BASF_Histogram;
//  _nCall = m->histogram("HF nCall", 1, 0., 1.);
//  _nTrial = m->histogram("HF nTrial", 100, 0., 100.);
//  _nTrialPositive = m->histogram("HF nTrial +", 100, 0., 100.);
//  _nTrialNegative = m->histogram("HF nTrial -", 100, 0., 100.);
    }
//     _nCall->accumulate(1.);
    bool posi = true;
        << "        THelixFitter::fit ..." << std::endl;
#endif
    //...Initialize
    _pre_chi2 = _fitted_chi2 = 0.;
    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    //...Type check...
    if (b.objectType() != Track) return TFitUnavailable;
    TTrack& t = (TTrack&) b;

    //...Already fitted ?...
    if (t.fitted()) return TFitAlreadyFitted;

    //...Count # of hits...
    AList<TLink> cores = t.cores();
    if (_fit2D) cores = AxialHits(cores);
    unsigned nCores = cores.length();
    unsigned nStereoCores = NStereoHits(cores);

    //...2D or 3D...
    bool fitBy2D = _fit2D;
    if (! fitBy2D) fitBy2D = (! nStereoCores);

    //...Check # of hits...
    if (! fitBy2D) {
      if ((nStereoCores < 2) || (nCores - nStereoCores < 3))
        return TFitErrorFewHits;
    } else {
      if (nCores < 3) return TFitErrorFewHits;
    }

    //...Setup...
    CLHEP::HepVector a(5), da(5);
    a = t.helix().a();
    CLHEP::HepVector dxda(5);
    CLHEP::HepVector dyda(5);
    CLHEP::HepVector dzda(5);
    CLHEP::HepVector dDda(5);
    CLHEP::HepVector dchi2da(5);
    CLHEP::HepSymMatrix d2chi2d2a(5, 0);
    static const CLHEP::HepSymMatrix zero5(5, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    const double convergence = Convergence;
    bool allAxial = true;
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;//jtanaka0715

    int flagBad = 0; // 2001/04/12
    AList<TLink> initBadWires;
    unsigned nInitBadWires = 0;
    CLHEP::HepVector initBadDchi2da(5);
    CLHEP::HepSymMatrix initBadD2chi2d2a(5, 0);
    for (unsigned j = 0; j < 5; ++j) initBadDchi2da[j] = 0.;
    double initBadChi2 = 0.;

    //...Fitting loop...
    unsigned nTrial = 0;
    //    while (nTrial < NTrailMax) {
    while (nTrial < TrasanTHelixFitterNtrialMax) {

      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
      d2chi2d2a = zero5;

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = cores[i++]) {
        const Belle2::TRGCDCWireHit& h = * l->hit();

        //...Cal. closest points...
        t.approach(* l, _sag);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();
        unsigned leftRight = CellHitRight;
        if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;

        //...Obtain drift distance and its error...
        double distance;
        double eDistance;
        drift(t, * l, t0Offset, distance, eDistance);
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda(* l, t.helix(), dPhi, dxda, dyda, dzda);

        //...Chi2 related...
        // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
        HepGeom::Vector3D<double> vw = h.wire().direction();
        static CLHEP::HepVector nv(5, 0);
        dDda = (vmag > 0.)
               ? ((v.x() * (1. - vw.x() * vw.x()) -
                   v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
                  * dxda +
                  (v.y() * (1. - vw.y() * vw.y()) -
                   v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
                  * dyda +
                  (v.z() * (1. - vw.z() * vw.z()) -
                   v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
                  * dzda) / vmag
               : nv;
        if (vmag <= 0.0) {
          std::cout << "    in fit " << onTrack << ", " << onWire;
          h.dump();
        }

        double pChi2 = dDistance * dDistance / eDistance2;
        if (flagBad) { // 2001/04/12
          if (nTrial == 0 && pChi2 > 1500.) {
            initBadWires.append(l);
            initBadDchi2da += (dDistance / eDistance2) * dDda;
            initBadD2chi2d2a += vT_times_v(dDda) / eDistance2;
            initBadChi2 += pChi2;
          } else {
            dchi2da += (dDistance / eDistance2) * dDda;
            d2chi2d2a += vT_times_v(dDda) / eDistance2;
            chi2 += pChi2;
            //...Store results...
            l->update(onTrack, onWire, leftRight, pChi2);
          }
        } else {
          dchi2da += (dDistance / eDistance2) * dDda;
          d2chi2d2a += vT_times_v(dDda) / eDistance2;
          chi2 += pChi2;
          //...Store results...
          l->update(onTrack, onWire, leftRight, pChi2);
        }
      }

      if (flagBad) { // 2001/04/12
        if (nTrial == 0 &&
            (initBadWires.length() == 1 ||
             initBadWires.length() == 2) &&
            nCores >= 20 &&
            chi2 / (double)(nCores - initBadWires.length()) < 10.) {
          cores.remove(initBadWires);
          nInitBadWires = initBadWires.length();
        } else if (nTrial == 0 && initBadWires.length() != 0) {
          dchi2da += initBadDchi2da;
          d2chi2d2a += initBadD2chi2d2a;
          chi2 += initBadChi2;
        }
      }

      //...Save chi2 information...
      if (nTrial == 0) {
        _pre_chi2 = chi2;
        _fitted_chi2 = chi2;
      } else _fitted_chi2 = chi2;

      //...Check condition...
      double change = chi2Old - chi2;
      if (fabs(change) < convergence) break;
      if (change < 0.) {
//          a += factor * da;
//          t._helix->a(a);
//          break;
        factor = 0.5;
      }
      chi2Old = chi2;

      //...Cal. helix parameters for next loop...
      if (fitBy2D) {
        f = dchi2da.sub(1, 3);
        e = d2chi2d2a.sub(1, 3);
        f = solve(e, f);
        da[0] = f[0];
        da[1] = f[1];
        da[2] = f[2];
        da[3] = 0.;
        da[4] = 0.;
      } else {
        da = solve(d2chi2d2a, dchi2da);
      }
      static const double kp_max = 1.e10;
      static const double kp_min = 1.e-30;
      const double kp_test = std::abs(a[2] - factor * da[2]);
      if (kp_test < kp_min || kp_test > kp_max) {
        err = 1;
        break;
      }

      a -= factor * da;
      t._helix->a(a);
      ++nTrial;

      // jtanaka 001008
      //if( fabs(a[3]) > 200. ){
      // yiwasaki 001010
      if (fabs(a[3]) > 1000.) {
        // stop "fit" and return error.
        // std::cout << "Stop Fit... " << a << std::endl;
        err = 1;
        break;
      }
#ifdef TRASAN_DEBUG_DETAIL
          << "            fit " << nTrial - 1 << " : " << chi2 << " : "
          << change << std::endl;
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    _nTrial->accumulate((float) nTrial + .5);
    if (posi) _nTrialPositive->accumulate((float) nTrial + .5);
    else _nTrialNegative->accumulate((float) nTrial + .5);
#endif

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(5, 0);
    unsigned dim;
    if (fitBy2D) {
      dim = 3;
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
      dim = 5;
      Ea = d2chi2d2a.inverse(err);
    }

    //...Store information...
    if (! err) {
      t._helix->a(a);
      t._helix->Ea(Ea);
      t._fitted = true;
    } else {
      err = TFitFailed;
    }

    t._charge = copysign(1., a[2]);
    t._ndf = nCores - dim - nInitBadWires;
    t._chi2 = chi2;

    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    return err;
  }
#endif

#ifdef OPTJT
// speed up
  int
  THelixFitter::dxda(const TLink& link,
                     const THelix& h,
                     double dPhi,
                     CLHEP::HepVector& dxda,
                     CLHEP::HepVector& dyda,
                     CLHEP::HepVector& dzda) const
  {

    //...Setup...
    const Belle2::TRGCDCWire& w = * link.wire();
    const CLHEP::HepVector& a = h.a();
    const double dRho  = a[0];
    const double phi0  = a[1];
    const double kappa = a[2];
    const double rho   = THelix::ConstantAlpha / kappa;
    const double tanLambda = a[4];

    const double sinPhi0 = sin(phi0);
    const double cosPhi0 = cos(phi0);
    // double sinPhi0 = h.sinphi0();
    // double cosPhi0 = h.cosphi0();
    const double sinPhi0dPhi = sin(phi0 + dPhi);
    const double cosPhi0dPhi = cos(phi0 + dPhi);
    //Vector dphida(5);
    double dphida[5];

    //...Sag correction...
    HepGeom::Point3D<double> xw = w.xyPosition();
    HepGeom::Point3D<double> wireBackwardPosition = w.backwardPosition();
    Vector3D v = w.direction();
    if (_sag) {
      std::cout << "THelixFitter::dxda !!! sag correction is not implemented"
                << std::endl;
//         w.wirePosition(link.positionOnTrack().z(),
//                        xw,
//                        wireBackwardPosition,
//                        v);
    }

    //...Axial case...
    if (w.axial()) {
      const double d[3] = { h.center().x() - xw.x(),
                            h.center().y() - xw.y(),
                            h.center().z() - xw.z()
                          };
      const double inv_dmag2 = 1. / (d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);

      const double rho_per_kappa  = rho / kappa;
      const double dRho_plus_rho  = dRho + rho;
      const double rhoSinPhi0dPhi = rho * sinPhi0dPhi;
      const double rhoCosPhi0dPhi = rho * cosPhi0dPhi;
      const double rhoTanLambda   = rho * tanLambda;

      dphida[0] = (sinPhi0 * d[0] - cosPhi0 * d[1]) * inv_dmag2;
      dphida[1] = dRho_plus_rho * (cosPhi0 * d[0] + sinPhi0 * d[1]) * inv_dmag2 - 1.;
      dphida[2] = -rho_per_kappa * dphida[0];
      dphida[3] = 0.;
      dphida[4] = 0.;

      dxda[0] = cosPhi0 + rhoSinPhi0dPhi * dphida[0];
      dxda[1] = -dRho_plus_rho * sinPhi0 + rhoSinPhi0dPhi * (1. + dphida[1]);
      dxda[2] = -rho_per_kappa * (cosPhi0 - cosPhi0dPhi) + rhoSinPhi0dPhi * dphida[2];
      dxda[3] = 0.;
      dxda[4] = 0.;

      dyda[0] = sinPhi0 - rhoCosPhi0dPhi * dphida[0];
      dyda[1] = dRho_plus_rho * cosPhi0 - rhoCosPhi0dPhi * (1. + dphida[1]);
      dyda[2] = -rho_per_kappa * (sinPhi0 - sinPhi0dPhi) - rhoCosPhi0dPhi * dphida[2];
      dyda[3] = 0.;
      dyda[4] = 0.;

      dzda[0] = -rhoTanLambda * dphida[0];
      dzda[1] = -rhoTanLambda * dphida[1];
      dzda[2] = rho_per_kappa * tanLambda * dPhi - rhoTanLambda * dphida[2];
      dzda[3] = 1.;
      dzda[4] = -rho * dPhi;
    }

    //...Stereo case...
    else {
      const double v_dot_wireBackwardPosition = v.x() * wireBackwardPosition.x()
                                                +                                       v.y() * wireBackwardPosition.y()
                                                +                                       v.z() * wireBackwardPosition.z();
      const double c[3] = { w.backwardPosition().x() - v_dot_wireBackwardPosition* v.x(),
                            w.backwardPosition().y() - v_dot_wireBackwardPosition* v.y(),
                            w.backwardPosition().z() - v_dot_wireBackwardPosition* v.z()
                          };

      const double x[3] = { link.positionOnTrack().x(), link.positionOnTrack().y(), link.positionOnTrack().z() };
      const double x_minus_c[3] = { x[0] - c[0], x[1] - c[1], x[2] - c[2] };

      //Vector dxdphi(3);
      const double dxdphi[3] = { rho* sinPhi0dPhi, -rho* cosPhi0dPhi, -rho* tanLambda };

      //Vector d2xdphi2(3);
      const double d2xdphi2[3] = { -dxdphi[1], dxdphi[0], 0. };

      double dxdphi_dot_v = (dxdphi[0] * v.x() +
                             dxdphi[1] * v.y() +
                             dxdphi[2] * v.z());
      double x_dot_v = x[0] * v.x() + x[1] * v.y() + x[2] * v.z();
      double inv_dfdphi = -1. / ((dxdphi[0] - dxdphi_dot_v * v.x()) * dxdphi[0]
                                 + (dxdphi[1] - dxdphi_dot_v * v.y()) * dxdphi[1]
                                 + (dxdphi[2] - dxdphi_dot_v * v.z()) * dxdphi[2]
                                 + (x_minus_c[0] - x_dot_v * v.x()) * d2xdphi2[0]
                                 + (x_minus_c[1] - x_dot_v * v.y()) * d2xdphi2[1]);
      /* +(x_minus_c[2] - x_dot_v*v.z()) * d2xdphi2[2];  = 0. */


      const double rho_per_kappa  =  rho / kappa;
      const double dRho_plus_rho  =  dRho + rho;
      const double& rhoSinPhi0dPhi =  dxdphi[0];
      const double rhoCosPhi0dPhi = -dxdphi[1];
      const double rhoTanLambda   = -dxdphi[2];

      //dxda_phi, dyda_phi, dzda_phi : phi is fixed
      //Vector dxda_phi(5);
      double dxda_phi[5];
      dxda_phi[0] = cosPhi0;
      dxda_phi[1] = -dRho_plus_rho * sinPhi0 + rhoSinPhi0dPhi;
      dxda_phi[2] = -rho_per_kappa * (cosPhi0 - cosPhi0dPhi);
      dxda_phi[3] = 0.;
      dxda_phi[4] = 0.;

      //Vector dyda_phi(5);
      double dyda_phi[5];
      dyda_phi[0] = sinPhi0;
      dyda_phi[1] = dRho_plus_rho * cosPhi0 - rhoCosPhi0dPhi;
      dyda_phi[2] = -rho_per_kappa * (sinPhi0 - sinPhi0dPhi);
      dyda_phi[3] = 0.;
      dyda_phi[4] = 0.;

      //Vector dzda_phi(5);
      double dzda_phi[5];
      dzda_phi[0] = 0.;
      dzda_phi[1] = 0.;
      dzda_phi[2] = rho_per_kappa * tanLambda * dPhi;
      dzda_phi[3] = 1.;
      dzda_phi[4] = -rho * dPhi;

      //Vector d2xdphida(5);
      double d2xdphida[5];
      d2xdphida[0] = 0.;
      d2xdphida[1] = rhoCosPhi0dPhi;
      d2xdphida[2] = -rho_per_kappa * sinPhi0dPhi;
      d2xdphida[3] = 0.;
      d2xdphida[4] = 0.;

      //Vector d2ydphida(5);
      double d2ydphida[5];
      d2ydphida[0] = 0.;
      d2ydphida[1] = rhoSinPhi0dPhi;
      d2ydphida[2] = rho_per_kappa * cosPhi0dPhi;
      d2ydphida[3] = 0.;
      d2ydphida[4] = 0.;

      //Vector d2zdphida(5);
      double d2zdphida[5];
      d2zdphida[0] = 0.;
      d2zdphida[1] = 0.;
      d2zdphida[2] = rho_per_kappa * tanLambda;
      d2zdphida[3] = 0.;
      d2zdphida[4] = -rho;

      //Vector dfda(5);
      double dfda[5];
      for (int i = 0; i < 5; ++i) {
        double d_dot_v = (v.x() * dxda_phi[i] +
                          v.y() * dyda_phi[i] +
                          v.z() * dzda_phi[i]);
        dfda[i] = (- (dxda_phi[i] - d_dot_v * v.x()) * dxdphi[0]
                   - (dyda_phi[i] - d_dot_v * v.y()) * dxdphi[1]
                   - (dzda_phi[i] - d_dot_v * v.z()) * dxdphi[2]
                   - (x_minus_c[0] - x_dot_v * v.x()) * d2xdphida[i]
                   - (x_minus_c[1] - x_dot_v * v.y()) * d2ydphida[i]
                   - (x_minus_c[2] - x_dot_v * v.z()) * d2zdphida[i]);
        dphida[i] = -dfda[i] * inv_dfdphi;
      }

      dxda[0] = cosPhi0 + rhoSinPhi0dPhi * dphida[0];
      dxda[1] = -dRho_plus_rho * sinPhi0 + rhoSinPhi0dPhi * (1. + dphida[1]);
      dxda[2] = -rho_per_kappa * (cosPhi0 - cosPhi0dPhi) + rhoSinPhi0dPhi * dphida[2];
      dxda[3] = rhoSinPhi0dPhi * dphida[3];
      dxda[4] = rhoSinPhi0dPhi * dphida[4];

      dyda[0] = sinPhi0 - rhoCosPhi0dPhi * dphida[0];
      dyda[1] = dRho_plus_rho * cosPhi0 - rhoCosPhi0dPhi * (1. + dphida[1]);
      dyda[2] = -rho_per_kappa * (sinPhi0 - sinPhi0dPhi) - rhoCosPhi0dPhi * dphida[2];
      dyda[3] = -rhoCosPhi0dPhi * dphida[3];
      dyda[4] = -rhoCosPhi0dPhi * dphida[4];

      dzda[0] = -rhoTanLambda * dphida[0];
      dzda[1] = -rhoTanLambda * dphida[1];
      dzda[2] = rho_per_kappa * tanLambda * dPhi - rhoTanLambda * dphida[2];
      dzda[3] = 1. - rhoTanLambda * dphida[3];
      dzda[4] = -rho * dPhi - rhoTanLambda * dphida[4];
    }

    // std::cout << dxda << std::endl;
    // std::cout << dyda << std::endl;
    // std::cout << dzda << std::endl;

    return 0;
  }
#else
  int
  THelixFitter::dxda(const TLink& link,
                     const THelix& h,
                     double dPhi,
                     CLHEP::HepVector& dxda,
                     CLHEP::HepVector& dyda,
                     CLHEP::HepVector& dzda) const
  {

    //...Setup...
    const Belle2::TRGCDCWire& w = * link.wire();
    const CLHEP::HepVector& a = h.a();
    double dRho  = a[0];
    double phi0  = a[1];
    double kappa = a[2];
    double rho   = THelix::ConstantAlpha / kappa;
    double tanLambda = a[4];

    double sinPhi0 = sin(phi0);
    double cosPhi0 = cos(phi0);
    // double sinPhi0 = h.sinphi0();
    // double cosPhi0 = h.cosphi0();
    double sinPhi0dPhi = sin(phi0 + dPhi);
    double cosPhi0dPhi = cos(phi0 + dPhi);
    CLHEP::HepVector dphida(5);

    //...Sag correction...
    HepGeom::Point3D<double> xw = w.xyPosition();
    HepGeom::Point3D<double> wireBackwardPosition = w.backwardPosition();
    Vector3D v = w.direction();
    if (_sag)
      w.wirePosition(link.positionOnTrack().z(),
                     xw,
                     wireBackwardPosition,
                     v);

    //...Axial case...
    if (w.axial()) {
      Point3D d = h.center() - xw;
      double dmag2 = d.mag2();

      dphida[0] = (sinPhi0 * d.x() - cosPhi0 * d.y()) / dmag2;
      dphida[1] = (dRho + rho)    * (cosPhi0 * d.x() + sinPhi0 * d.y())
                  / dmag2 - 1.;
      dphida[2] = (- rho / kappa) * (sinPhi0 * d.x() - cosPhi0 * d.y())
                  / dmag2;
      dphida[3] = 0.;
      dphida[4] = 0.;
    }

    //...Stereo case...
    else {
      //temp  Point3D onTrack = h.x(dPhi);
      //temp
      HepGeom::Point3D<double> onTrack = link.positionOnTrack();
      //     << "ontrack =" << onTrack  << std::endl;
      //     << "ontrackp=" << onTrackp << std::endl;
      //temp

      CLHEP::HepVector c(3);
      c = Point3D(w.backwardPosition() - (v * wireBackwardPosition) * v);

      CLHEP::HepVector x(3);
      x = onTrack;

      CLHEP::HepVector dxdphi(3);
      dxdphi[0] =   rho * sinPhi0dPhi;
      dxdphi[1] = - rho * cosPhi0dPhi;
      dxdphi[2] = - rho * tanLambda;

      CLHEP::HepVector d2xdphi2(3);
      d2xdphi2[0] = rho * cosPhi0dPhi;
      d2xdphi2[1] = rho * sinPhi0dPhi;
      d2xdphi2[2] = 0.;

      double dxdphi_dot_v = (dxdphi[0] * v.x() +
                             dxdphi[1] * v.y() +
                             dxdphi[2] * v.z());
      double x_dot_v = x[0] * v.x() + x[1] * v.y() + x[2] * v.z();
      double dfdphi = - (dxdphi[0] - dxdphi_dot_v * v.x()) * dxdphi[0]
                      - (dxdphi[1] - dxdphi_dot_v * v.y()) * dxdphi[1]
                      - (dxdphi[2] - dxdphi_dot_v * v.z()) * dxdphi[2]
                      - (x[0] - c[0] - x_dot_v * v.x()) * d2xdphi2[0]
                      - (x[1] - c[1] - x_dot_v * v.y()) * d2xdphi2[1];
      /*  - (x[2] - c[2] - x_dot_v*v.z()) * d2xdphi2[2];  = 0. */


      //dxda_phi, dyda_phi, dzda_phi : phi is fixed
      CLHEP::HepVector dxda_phi(5);
      dxda_phi[0] = cosPhi0;
      dxda_phi[1] = - (dRho + rho) * sinPhi0 + rho * sinPhi0dPhi;
      dxda_phi[2] = - (rho / kappa) * (cosPhi0 - cosPhi0dPhi);
      dxda_phi[3] = 0.;
      dxda_phi[4] = 0.;

      CLHEP::HepVector dyda_phi(5);
      dyda_phi[0] = sinPhi0;
      dyda_phi[1] = (dRho + rho) * cosPhi0 - rho * cosPhi0dPhi;
      dyda_phi[2] = - (rho / kappa) * (sinPhi0 - sinPhi0dPhi);
      dyda_phi[3] = 0.;
      dyda_phi[4] = 0.;

      CLHEP::HepVector dzda_phi(5);
      dzda_phi[0] = 0.;
      dzda_phi[1] = 0.;
      dzda_phi[2] = (rho / kappa) * tanLambda * dPhi;
      dzda_phi[3] = 1.;
      dzda_phi[4] = - rho * dPhi;

      CLHEP::HepVector d2xdphida(5);
      d2xdphida[0] = 0.;
      d2xdphida[1] = rho * cosPhi0dPhi;
      d2xdphida[2] = - (rho / kappa) * sinPhi0dPhi;
      d2xdphida[3] = 0.;
      d2xdphida[4] = 0.;

      CLHEP::HepVector d2ydphida(5);
      d2ydphida[0] = 0.;
      d2ydphida[1] = rho * sinPhi0dPhi;
      d2ydphida[2] = (rho / kappa) * cosPhi0dPhi;
      d2ydphida[3] = 0.;
      d2ydphida[4] = 0.;

      CLHEP::HepVector d2zdphida(5);
      d2zdphida[0] = 0.;
      d2zdphida[1] = 0.;
      d2zdphida[2] = (rho / kappa) * tanLambda;
      d2zdphida[3] = 0.;
      d2zdphida[4] = - rho;

      CLHEP::HepVector dfda(5);
      for (int i = 0; i < 5; i++) {
        double d_dot_v = (v.x() * dxda_phi[i] +
                          v.y() * dyda_phi[i] +
                          v.z() * dzda_phi[i]);
        dfda[i] = (- (dxda_phi[i] - d_dot_v * v.x()) * dxdphi[0]
                   - (dyda_phi[i] - d_dot_v * v.y()) * dxdphi[1]
                   - (dzda_phi[i] - d_dot_v * v.z()) * dxdphi[2]
                   - (x[0] - c[0] - x_dot_v * v.x()) * d2xdphida[i]
                   - (x[1] - c[1] - x_dot_v * v.y()) * d2ydphida[i]
                   - (x[2] - c[2] - x_dot_v * v.z()) * d2zdphida[i]);
        dphida[i] = - dfda[i] / dfdphi;
      }
    }

    dxda[0] = cosPhi0 + rho * sinPhi0dPhi * dphida[0];
    dxda[1] = - (dRho + rho) * sinPhi0 + rho * sinPhi0dPhi * (1. + dphida[1]);
    dxda[2] = - rho / kappa * (cosPhi0 - cosPhi0dPhi)
              + rho * sinPhi0dPhi * dphida[2];
    dxda[3] = rho * sinPhi0dPhi * dphida[3];
    dxda[4] = rho * sinPhi0dPhi * dphida[4];

    dyda[0] = sinPhi0 - rho * cosPhi0dPhi * dphida[0];
    dyda[1] = (dRho + rho) * cosPhi0 - rho * cosPhi0dPhi * (1. + dphida[1]);
    dyda[2] = - rho / kappa * (sinPhi0 - sinPhi0dPhi)
              - rho * cosPhi0dPhi * dphida[2];
    dyda[3] = - rho * cosPhi0dPhi * dphida[3];
    dyda[4] = - rho * cosPhi0dPhi * dphida[4];

    dzda[0] = - rho * tanLambda * dphida[0];
    dzda[1] = - rho * tanLambda * dphida[1];
    dzda[2] = rho / kappa * tanLambda * dPhi - rho * tanLambda * dphida[2];
    dzda[3] = 1. - rho * tanLambda * dphida[3];
    dzda[4] = - rho * dPhi - rho * tanLambda * dphida[4];

    // std::cout << dxda << std::endl;
    // std::cout << dyda << std::endl;
    // std::cout << dzda << std::endl;

    return 0;
  }
#endif

  void
  THelixFitter::drift(const TTrack& t,
                      const TLink& l,
                      float t0Offset,
                      double& distance,
                      double& err) const
  {

    const Belle2::TRGCDCWireHit& h = * l.hit();
    const HepGeom::Point3D<double> & onTrack = l.positionOnTrack();
    const HepGeom::Point3D<double> & onWire = l.positionOnWire();
    unsigned leftRight = CellHitRight;
    if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;

    //...No correction...
    if ((t0Offset == 0.) && (! _propagation) && (! _tof)) {
      distance = h.drift(leftRight);
      err = h.dDrift(leftRight);
      return;
    }

    //...TOF correction...
    float tof = 0.;
    double dPhi = l.dPhi();
    while (dPhi >  M_PI) dPhi -= M_PI;
    while (dPhi < -M_PI) dPhi += M_PI;
    if (_tof) {
      int imass = 3;
      float tl = t.helix().a()[4];
      float f = sqrt(1. + tl * tl);
      float s = fabs(t.helix().curv() * dPhi) * f;
      float p = f / fabs(t.helix().a()[2]);
      calcdc_tof2_(& imass, & p, & s, & tof);
    }

    //...T0 and propagation corrections...
    int wire = h.wire().id();
    int side = leftRight;
    if (side == 0) side = -1;
    HepGeom::Vector3D<double> tp = t.helix().momentum(dPhi);
    float p[3] = {tp.x(), tp.y(), tp.z()};
    float x[3] = {onWire.x(), onWire.y(), onWire.z()};
//cnv    float time = h.reccdc()->m_tdc + t0Offset - tof;
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
    // std::cout << "at drift of THelixFitter" << std::endl;
    // std::cout << "(" << p[0] << ", " << p[1] << ", " << p[2] << ")" << std::endl;
    // std::cout << distance << "/" << err << std::endl;
    return;
  }

#ifdef OPTJT
//=====================================================================
  int
  THelixFitter::main(TTrackBase& b, float& tev, float& tev_err,
                     double* pre_chi2, double* fitted_chi2) const
  {
//=====================================================================
    //...Initialize
    _pre_chi2 = _fitted_chi2 = 0.;
    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    //...Type check...
    if (b.objectType() != Track) return TFitUnavailable;
    TTrack& t = (TTrack&) b;

    //...Already fitted ?...
    if (t.fitted()) return TFitAlreadyFitted;

    //...Count # of hits...
    AList<TLink> cores = t.cores();
    if (_fit2D) cores = TLink::axialHits(cores);
    unsigned nCores = cores.length();
    unsigned nStereoCores = TLink::nStereoHits(cores);

    //...2D or 3D...
    bool fitBy2D = _fit2D;
    if (! fitBy2D) fitBy2D = (! nStereoCores);

    //...Check # of hits...
    if (! fitBy2D) {
      if ((nStereoCores < 2) || (nCores - nStereoCores < 3))
        return TFitErrorFewHits;
    } else {
      if (nCores < 3) return TFitErrorFewHits;
    }

    //...Setup...
    CLHEP::HepVector a(6), da(6);
    CLHEP::HepVector a_5dim(5);
    for (unsigned j = 0; j < 5; j++) a[j] = t.helix().a()[j];
    a[5] = tev;
    CLHEP::HepVector dxda(5);
    CLHEP::HepVector dyda(5);
    CLHEP::HepVector dzda(5);
    CLHEP::HepVector dDda(6);
    CLHEP::HepVector dDda_5dim(5);
    CLHEP::HepVector dchi2da(6);
    CLHEP::HepSymMatrix d2chi2d2a(6, 0);
    static const CLHEP::HepSymMatrix zero6(6, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    //temp    const double convergence = Convergence;
    const double convergence = 1.0e-4;
//  bool allAxial = true;
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;//jtanaka0715

    //...Fitting loop...
    unsigned nTrial = 0;
    //    while (nTrial < NTrailMax) {
    while (nTrial < (unsigned) TrasanTHelixFitterNtrialMax) {

      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 6; j++) dchi2da[j] = 0.;
      d2chi2d2a = zero6;

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = cores[i++]) {
        const Belle2::TRGCDCWireHit& h = * l->hit();

        //...Cal. closest points...
        t.approach(* l, _sag);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();
        unsigned leftRight = (onWire.cross(onTrack).z() < 0.) ? CellHitLeft : CellHitRight;

        //...Obtain drift distance and its error...
        double distance;
        double eDistance;
        double dddt;
        drift(t, * l, tev, distance, eDistance, dddt);
        double inv_eDistance2 = 1. / (eDistance * eDistance);

        //...Residual...
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda(* l, t.helix(), dPhi, dxda, dyda, dzda);

        //...Chi2 related...
        // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
        double vw[3] = { h.wire().direction().x(),
                         h.wire().direction().y(),
                         h.wire().direction().z()
                       };
        double vwxy = vw[0] * vw[1];
        double vwyz = vw[1] * vw[2];
        double vwzx = vw[2] * vw[0];
        dDda_5dim = (vmag > 0.)
                    ? ((v.x() * (1. - vw[0] * vw[0]) -
                        v.y() * vwxy - v.z() * vwzx)
                       * dxda +
                       (v.y() * (1. - vw[1] * vw[1]) -
                        v.z() * vwyz - v.x() * vwxy)
                       * dyda +
                       (v.z() * (1. - vw[2] * vw[2]) -
                        v.x() * vwzx - v.y() * vwyz)
                       * dzda) / vmag
                    : CLHEP::HepVector(5, 0);
        if (vmag <= 0.0) {
          std::cout  << "    in fit " << onTrack << ", " << onWire;
          h.dump();
        }
        //      for (unsigned j = 0; j < 5; j++) dDda[j] = dDda_5dim[j];
        dDda[0] = dDda_5dim[0];
        dDda[1] = dDda_5dim[1];
        dDda[2] = dDda_5dim[2];
        dDda[3] = dDda_5dim[3];
        dDda[4] = dDda_5dim[4];
        dDda[5] = -dddt;

        dchi2da += (dDistance * inv_eDistance2) * dDda;
        d2chi2d2a += vT_times_v(dDda) * inv_eDistance2;
        double pChi2 = dDistance * dDistance * inv_eDistance2;
        chi2 += pChi2;

        //...Store results...
        l->update(onTrack, onWire, leftRight, pChi2);
      }

      //...Save chi2 information...
      if (nTrial == 0) {
        _pre_chi2 = chi2;
        _fitted_chi2 = chi2;
      } else _fitted_chi2 = chi2;

      //...Check condition...
      double change = chi2Old - chi2;
      if (fabs(change) < convergence) break;
      //temp
      factor = 1.0;
      //temp
      if (change < 0.) {
//          a += factor * da;
//          t._helix->a(a);
//          break;
        factor = 0.5;
      }
      chi2Old = chi2;

      //...Cal. helix parameters for next loop...
      if (fitBy2D) {
        f = dchi2da.sub(1, 4);
        e = d2chi2d2a.sub(1, 4);
        f = solve(e, f);
        da[0] = f[0];
        da[1] = f[1];
        da[2] = f[2];
        da[3] = f[3];
        da[4] = 0.;
        da[5] = 0.;
      } else {
        da = solve(d2chi2d2a, dchi2da);
      }
      static const double kp_max = 1.e10;
      static const double kp_min = 1.e-30;
      const double kp_test = std::abs(a[2] - factor * da[2]);
      if (kp_test < kp_min || kp_test > kp_max) {
        err = 1;
        break;
      }

      a -= factor * da;

      //  for (unsigned j = 0; j < 5; j++) a_5dim[j] = a[j];
      a_5dim[0] = a[0];
      a_5dim[1] = a[1];
      a_5dim[2] = a[2];
      a_5dim[3] = a[3];
      a_5dim[4] = a[4];
      t._helix->a(a_5dim);
      tev = a[5];
      //temp
      //  if(nTrial == 0)  << "initial chi2=" <<chi2 << std::endl;
      //temp
      ++nTrial;

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "fit " << nTrial - 1 << " : " << chi2 << " : " << change << std::endl;
#endif
    }

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(6, 0);
    unsigned dim;
    if (fitBy2D) {
      dim = 4;
      CLHEP::HepSymMatrix Eb(4, 0), Ec(4, 0);
      Eb = d2chi2d2a.sub(1, 4);
      Ec = Eb.inverse(err);
      Ea[0][0] = Ec[0][0];
      Ea[0][1] = Ec[0][1];
      Ea[0][2] = Ec[0][2];
      Ea[0][3] = Ec[0][3];
      Ea[1][1] = Ec[1][1];
      Ea[1][2] = Ec[1][2];
      Ea[1][3] = Ec[1][3];
      Ea[2][2] = Ec[2][2];
      Ea[2][3] = Ec[2][3];
      Ea[3][3] = Ec[3][3];
    } else {
      dim = 6;
      Ea = d2chi2d2a.inverse(err);
      // std::cout << "err flg=" << err << std::endl;
    }
    //
    // nk temp fix
    //
    if (!err && Ea[5][5] < 0) {
      std::cout << "Ea[5][5] is bad... " << Ea[5][5] << std::endl;
      return TFitFailed;
    }

    //...Store information...
    if (! err) {
      for (unsigned j = 0; j < 5; j++) a_5dim[j] = a[j];
      CLHEP::HepSymMatrix Ea_5dim(5, 0);
      Ea_5dim = Ea.sub(1, 5);
      t._helix->a(a_5dim);
      t._helix->Ea(Ea_5dim);
      tev = a[5];
      tev_err = sqrt(Ea[5][5]);
      //temp
      //   << "nTrial=" << nTrial << std::endl;
      //   << "chi2="   << chi2   << std::endl;
      //   << "tev,tev_err="<<tev<<" "<<tev_err<<std::endl;
      //temp

      t._fitted = true;
    } else {
      err = TFitFailed;
    }

    t._ndf = nCores - dim;
    t._chi2 = chi2;

    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    return err;
  }
#else
//=====================================================================
  int
  THelixFitter::main(TTrackBase& b, float& tev, float& tev_err,
                     double* pre_chi2, double* fitted_chi2) const
  {
//=====================================================================
    //...Initialize
    _pre_chi2 = _fitted_chi2 = 0.;
    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    //...Type check...
    if (b.objectType() != Track) return TFitUnavailable;
    TTrack& t = (TTrack&) b;

    //...Already fitted ?...
    if (t.fitted()) return TFitAlreadyFitted;

    //...Count # of hits...
    AList<TLink> cores = t.cores();
    if (_fit2D) cores = AxialHits(cores);
    unsigned nCores = cores.length();
    unsigned nStereoCores = NStereoHits(cores);

    //...2D or 3D...
    bool fitBy2D = _fit2D;
    if (! fitBy2D) fitBy2D = (! nStereoCores);

    //...Check # of hits...
    if (! fitBy2D) {
      if ((nStereoCores < 2) || (nCores - nStereoCores < 3))
        return TFitErrorFewHits;
    } else {
      if (nCores < 3) return TFitErrorFewHits;
    }

    //...Setup...
    CLHEP::HepVector a(6), da(6);
    CLHEP::HepVector a_5dim(5);
    for (unsigned j = 0; j < 5; j++) a[j] = t.helix().a()[j];
    a[5] = tev;
    CLHEP::HepVector dxda(5);
    CLHEP::HepVector dyda(5);
    CLHEP::HepVector dzda(5);
    CLHEP::HepVector dDda(6);
    CLHEP::HepVector dDda_5dim(5);
    CLHEP::HepVector dchi2da(6);
    CLHEP::HepSymMatrix d2chi2d2a(6, 0);
    static const CLHEP::HepSymMatrix zero6(6, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    //temp    const double convergence = Convergence;
    const double convergence = 1.0e-4;
    bool allAxial = true;
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;//jtanaka0715

    //...Fitting loop...
    unsigned nTrial = 0;
    //    while (nTrial < NTrailMax) {
    while (nTrial < TrasanTHelixFitterNtrialMax) {

      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 6; j++) dchi2da[j] = 0.;
      d2chi2d2a = zero6;

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = cores[i++]) {
        const Belle2::TRGCDCWireHit& h = * l->hit();

        //...Cal. closest points...
        t.approach(* l, _sag);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();
        unsigned leftRight = CellHitRight;
        if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;

        //...Obtain drift distance and its error...
        double distance;
        double eDistance;
        double dddt;
        drift(t, * l, tev, distance, eDistance, dddt);
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda(* l, t.helix(), dPhi, dxda, dyda, dzda);

        //...Chi2 related...
        // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
        HepGeom::Vector3D<double> vw = h.wire().direction();
        dDda_5dim = (vmag > 0.)
                    ? ((v.x() * (1. - vw.x() * vw.x()) -
                        v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
                       * dxda +
                       (v.y() * (1. - vw.y() * vw.y()) -
                        v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
                       * dyda +
                       (v.z() * (1. - vw.z() * vw.z()) -
                        v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
                       * dzda) / vmag
                    : CLHEP::HepVector(5, 0);
        if (vmag <= 0.0) {
          std::cout << "    in fit " << onTrack << ", " << onWire;
          h.dump();
        }
        //      for (unsigned j = 0; j < 5; j++) dDda[j] = dDda_5dim[j];
        dDda[0] = dDda_5dim[0];
        dDda[1] = dDda_5dim[1];
        dDda[2] = dDda_5dim[2];
        dDda[3] = dDda_5dim[3];
        dDda[4] = dDda_5dim[4];
        dDda[5] = -dddt;

        dchi2da += (dDistance / eDistance2) * dDda;
        d2chi2d2a += vT_times_v(dDda) / eDistance2;
        double pChi2 = dDistance * dDistance / eDistance2;
        chi2 += pChi2;

        //...Store results...
        l->update(onTrack, onWire, leftRight, pChi2);
      }

      //...Save chi2 information...
      if (nTrial == 0) {
        _pre_chi2 = chi2;
        _fitted_chi2 = chi2;
      } else _fitted_chi2 = chi2;

      //...Check condition...
      double change = chi2Old - chi2;
      if (fabs(change) < convergence) break;
      //temp
      factor = 1.0;
      //temp
      if (change < 0.) {
//          a += factor * da;
//          t._helix->a(a);
//          break;
        factor = 0.5;
      }
      chi2Old = chi2;

      //...Cal. helix parameters for next loop...
      if (fitBy2D) {
        f = dchi2da.sub(1, 4);
        e = d2chi2d2a.sub(1, 4);
        f = solve(e, f);
        da[0] = f[0];
        da[1] = f[1];
        da[2] = f[2];
        da[3] = f[3];
        da[4] = 0.;
        da[5] = 0.;
      } else {
        da = solve(d2chi2d2a, dchi2da);
      }
      static const double kp_max = 1.e10;
      static const double kp_min = 1.e-30;
      const double kp_test = std::abs(a[2] - factor * da[2]);
      if (kp_test < kp_min || kp_test > kp_max) {
        err = 1;
        break;
      }

      a -= factor * da;

      //  for (unsigned j = 0; j < 5; j++) a_5dim[j] = a[j];
      a_5dim[0] = a[0];
      a_5dim[1] = a[1];
      a_5dim[2] = a[2];
      a_5dim[3] = a[3];
      a_5dim[4] = a[4];
      t._helix->a(a_5dim);
      tev = a[5];
      //temp
      //  if(nTrial == 0)  << "initial chi2=" <<chi2 << std::endl;
      //temp
      ++nTrial;

#ifdef TRASAN_DEBUG_DETAIL
          << "fit " << nTrial - 1 << " : " << chi2 << " : " << change << std::endl;
#endif
    }

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(6, 0);
    unsigned dim;
    if (fitBy2D) {
      dim = 4;
      CLHEP::HepSymMatrix Eb(4, 0), Ec(4, 0);
      Eb = d2chi2d2a.sub(1, 4);
      Ec = Eb.inverse(err);
      Ea[0][0] = Ec[0][0];
      Ea[0][1] = Ec[0][1];
      Ea[0][2] = Ec[0][2];
      Ea[0][3] = Ec[0][3];
      Ea[1][1] = Ec[1][1];
      Ea[1][2] = Ec[1][2];
      Ea[1][3] = Ec[1][3];
      Ea[2][2] = Ec[2][2];
      Ea[2][3] = Ec[2][3];
      Ea[3][3] = Ec[3][3];
    } else {
      dim = 6;
      Ea = d2chi2d2a.inverse(err);
      // std::cout << "err flg=" << err << std::endl;
    }


    //...Store information...
    if (! err) {
      for (unsigned j = 0; j < 5; j++) a_5dim[j] = a[j];
      CLHEP::HepSymMatrix Ea_5dim(5, 0);
      Ea_5dim = Ea.sub(1, 5);
      t._helix->a(a_5dim);
      t._helix->Ea(Ea_5dim);
      tev = a[5];
      tev_err = sqrt(Ea[5][5]);
      //temp
      //   << "nTrial=" << nTrial << std::endl;
      //   << "chi2="   << chi2   << std::endl;
      //   << "tev,tev_err="<<tev<<" "<<tev_err<<std::endl;
      //temp

      t._fitted = true;
    } else {
      err = TFitFailed;
    }

    t._ndf = nCores - dim;
    t._chi2 = chi2;

    if (pre_chi2)*pre_chi2 = _pre_chi2;
    if (fitted_chi2)*fitted_chi2 = _fitted_chi2;

    return err;
  }
#endif

//=========================================
  void
  THelixFitter::drift(const TTrack& t,
                      const TLink& l,
                      float tev,
                      double& distance,
                      double& err,
                      double& dddt) const
  {
//=========================================

    const Belle2::TRGCDCWireHit& h = * l.hit();
    const HepGeom::Point3D<double> & onTrack = l.positionOnTrack();
    const HepGeom::Point3D<double> & onWire = l.positionOnWire();
    unsigned leftRight = CellHitRight;
    if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;

    //...No correction...
    if ((tev == 0.) && (! _propagation) && (! _tof)) {
      distance = h.drift(leftRight);
      err = h.dDrift(leftRight);
      //do not return here; need to cal. dddt
      //  return;
    }

    //...TOF correction...
    float tof = 0.;
    double dPhi = l.dPhi();
    while (dPhi >  M_PI) dPhi -= M_PI;
    while (dPhi < -M_PI) dPhi += M_PI;
    if (_tof) {
      int imass = 3;
      THelix th = t.helix();
      th.ignoreErrorMatrix();
      float tl = th.a()[4];
      float f = sqrt(1. + tl * tl);
      float s = std::abs(th.curv() * dPhi) * f;
      float p = f / std::abs(th.a()[2]);

      //      std::cout << "pivot= " << th.pivot() << std::endl;
      if (th.pivot().x() != 0. || th.pivot().y() != 0.) {
        HepGeom::Point3D<double>  vt = th.x(0.) - th.center();
        float vtx(vt.x()), vty(vt.y());
        HepGeom::Point3D<double>  vtp = th.x(0.);
        th.pivot(ORIGIN);
        HepGeom::Point3D<double>  v0 = th.x(0.) - th.center();
        float v0x(v0.x()), v0y(v0.y());
        float dfi = atan2(v0x * vty - v0y * vtx, v0x * vtx + v0y * vty);
        s += f * std::abs(th.curv() * dfi);
        //  std::cout << "s    = " << std::abs(th.curv()*dfi) << std::endl;
        //  float xt( vtp.x() ), yt( vtp.y() );
        //  std::cout << "s_lin= " << sqrt( xt*xt + yt*yt)    << std::endl;
      }

      calcdc_tof2_(& imass, & p, & s, & tof);
    }

    //...T0 and propagation corrections...
    int wire = h.wire().id();
    int side = leftRight;
    if (side == 0) side = -1;
    HepGeom::Vector3D<double> tp = t.helix().momentum(dPhi);
    float p[3] = {tp.x(), tp.y(), tp.z()};
    float x[3] = {onWire.x(), onWire.y(), onWire.z()};
//cnv    float time = h.reccdc()->m_tdc + tev - tof;
    float time = 0;
    //    float dist_p;
    //    float dist_m;
    float dist;
    float edist;
    int prop = _propagation;

    //    //calculate derivative w.r.t. time in blute force way; need update
    //    //in future to speed up
    //    float time_p = time + 0.1;
    //    calcdc_driftdist_(& prop,
    //              & wire,
    //              & side,
    //              p,
    //              x,
    //              & time_p,
    //              & dist_p,
    //              & edist);
    //
    //        float time_m = time - 0.1;
    //        calcdc_driftdist_(& prop,
    //              & wire,
    //                      & side,
    //              p,
    //              x,
    //              & time_m,
    //              & dist_m,
    //              & edist);
    ////dddt = (dist_p - dist_m)/0.2;
    //    dddt = (dist_p - dist_m)*5.;
    // std::cout << "side=" << side << std::endl;
    // std::cout << "dddt=" << dddt << std::endl;

    float dist2[2];
    float sigma_d2[2];
    float deriv2[2];
    float time_tmp = time;
    calcdc_driftdist3_(& prop,
                       & wire,
                       p,
                       x,
                       & time_tmp,
                       dist2,
                       sigma_d2,
                       deriv2);
    //n.b. input and output time are slightly different because of prop.
    //delay corr. in driftdist3.
    //    calcdc_driftdist_(& prop,
    //              & wire,
    //              & side,
    //              p,
    //              x,
    //              & time,
    //              & dist,
    //              & edist);

    if (side == -1) {
      // std::cout << " " << std::endl;
      // std::cout << dist << " " << dist2[0] << " " <<dist2[1] << std::endl;
      // std::cout << edist << " " << sigma_d2[0] << " " << sigma_d2[1] << std::endl;
      // std::cout << dddt  << " " << 0.001*deriv2[0] << std::endl;
      dist  = dist2[0];
      edist = sigma_d2[0];
      dddt = 0.001 * deriv2[0];
    } else if (side == 1) {
      // std::cout << " " << std::endl;
      // std::cout << dist << " " << dist2[1] << " " <<dist2[0] << std::endl;
      // std::cout << edist << " " << sigma_d2[1] << " " << sigma_d2[0] << std::endl;
      // std::cout << dddt  << " " << 0.001*deriv2[1] << std::endl;
      dist  = dist2[1];
      edist = sigma_d2[1];
      dddt = 0.001 * deriv2[1];
    }

    distance = (double) dist;
    // std::cout << "time,distance="<<time<<" "<<distance<<std::endl;
    err = (double) edist;
    return;
  }

} // namespace Belle
