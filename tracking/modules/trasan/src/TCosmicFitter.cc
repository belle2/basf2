//-----------------------------------------------------------------------------
// $Id: TCosmicFitter.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TCosmicFitter.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a helix.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.19  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.18  2001/12/23 09:58:46  katayama
// removed Strings.h
//
// Revision 1.17  2001/12/19 02:59:45  katayama
// Uss find,istring
//
// Revision 1.16  2001/05/07 20:51:18  yiwasaki
// <float.h> included for linux
//
// Revision 1.15  2001/04/11 01:09:09  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.14  2000/10/05 23:54:22  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.13  2000/04/13 02:53:38  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.12  2000/04/11 13:05:43  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.11  1999/11/23 10:30:17  yiwasaki
// ALPHA in TRGCDCUtil is replaced by THelix::ConstantAlpha
//
// Revision 1.10  1999/10/30 10:12:17  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.9  1999/09/13 05:57:55  yiwasaki
// Trasan 1.62b release : calcdc_sag2 -> calcdc_sag3
//
// Revision 1.8  1999/07/09 01:47:20  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.7  1999/06/14 11:41:53  yiwasaki
// bug in sag. correction fixed
//
// Revision 1.6  1999/04/10 00:19:03  katayama
// typo???
//
// Revision 1.5  1999/04/09 11:36:53  yiwasaki
// Trasan 1.32 release : TCosmicFitter minor change
//
// Revision 1.3  1999/04/07 06:14:10  yiwasaki
// Trasan 1.30 release : curl finder mask updated
//
// Revision 1.2  1999/03/29 03:20:13  katayama
// Use signal propagation delay
//
// Revision 1.1  1999/03/21 15:45:41  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.3  1999/03/03 15:46:02  yiwasaki
// flag bug fix
//
// Revision 1.2  1999/03/03 09:14:50  yiwasaki
// TRGCDCWireHit::WireHitValid -> WireHitFindingValid, WireHitFittingValid flag is checked in TCosmicFitter::fit
//
// Revision 1.1  1999/01/11 03:03:11  yiwasaki
// Fitters added
//
//-----------------------------------------------------------------------------

#define HEP_SHORT_NAMES


#include <float.h>
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
//#include "panther/panther.h"
//#include "panther/panther_manager.h"
#include "tracking/modules/trasan/TCosmicFitter.h"
#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

#define NTrailMax 100
#define Convergence 1.0e-5

  extern "C" void calcdc_sag3_(int*, float*, float[3], float*, float*, float*);
  extern "C" void calcdc_driftdist_(int*, int*, int*,
                                    float[3], float[3], float*, float*, float*);

  TCosmicFitter::TCosmicFitter(const std::string& name) : TFitter(name)
  {
  }

  TCosmicFitter::~TCosmicFitter()
  {
  }

  int
  TCosmicFitter::fit(TTrackBase& b) const
  {

    //...Already fitted ?...
    if (b.fitted()) return TFitAlreadyFitted;

    int err = fit(b, 0.);
    if (! err) b._fitted = true;

    return err;
  }

  int
  TCosmicFitter::fit(TTrackBase& b, float t0Offset) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TCosmicFitter::fit ..." << std::endl;
#endif

    //...Check # of hits...
    if (b.links().length() < 5) return TFitErrorFewHits;
    unsigned nValid = 0;
    for (unsigned i = 0; i < (unsigned) b.links().length(); i++) {
      unsigned state = b.links()[i]->hit()->state();
      if (state & CellHitInvalidForFit) continue;
      if (state & CellHitFittingValid) ++nValid;
    }
    if (nValid < 5) return TFitErrorFewHits;

    //...Type check...
    //    if (b.type() != Track) return TFitUnavailable;
    if (b.objectType() != Track) return TFitUnavailable;
    TTrack& t = (TTrack&) b;

    //...Setup...
    CLHEP::HepVector a(5), da(5);
    a = t.helix().a();
    CLHEP::HepVector dxda(5);
    CLHEP::HepVector dyda(5);
    CLHEP::HepVector dzda(5);
    CLHEP::HepVector dDda(5);
    CLHEP::HepVector dchi2da(5);
    CLHEP::HepSymMatrix d2chi2d2a(5, 0);
    double chi2;
    // double chi2Old = 10e99;
    double chi2Old = DBL_MAX;
    const double convergence = Convergence;
    bool allAxial = true;
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;//jtanaka0715

    CLHEP::HepVector maxDouble(5);
    for (unsigned i = 0; i < 5; i++) maxDouble[i] = (FLT_MAX);

    //...Fitting loop...
    unsigned nTrial = 0;
    while (nTrial < NTrailMax) {

      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
      d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = t.links()[i++]) {
        const Belle2::TRGCDCWireHit& h = * l->hit();

        //...Check state...
        if (h.state() & CellHitInvalidForFit) continue;
        if (!(h.state() & CellHitFittingValid)) continue;

        //...Check wire...
        if (! nTrial)
          if (h.wire().stereo()) allAxial = false;

        //...Cal. closest points...
        int doSagCorrection = 0;
        if (nTrial && !allAxial) doSagCorrection = 1;
        t.approach(* l, doSagCorrection);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();

#ifdef TRASAN_DEBUG_DETAIL
//      std::cout << "    in fit " << onTrack << ", " << onWire;
//      h.dump();
#endif

        //...Obtain drift distance and its error...
        unsigned leftRight = CellHitRight;
        if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;
        double distance = h.drift(leftRight);
        double eDistance = h.dDrift(leftRight);
        //...
        if (nTrial  && !allAxial) {
          int wire = h.wire().id();
          int side = leftRight;
          if (side == 0) side = -1;
          float x[3] = { onWire.x(), onWire.y(), onWire.z()};
          float p[3] = {t.p().x(), t.p().y(), t.p().z()};
//cnv               float time = l->hit()->reccdc()->m_tdc + t0Offset;
          float time = 0;
          float dist =  distance;
          float edist = eDistance;
          int doPropDelay = 1;
          calcdc_driftdist_(
            &doPropDelay, &wire, &side, p, x, &time, &dist, &edist);
          distance = (double) dist;
          eDistance = (double) edist;
        }
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda(*l, t.helix(), dPhi, dxda, dyda, dzda, doSagCorrection);

        //...Chi2 related...
        // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
        HepGeom::Vector3D<double> vw = h.wire().direction();
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
               : CLHEP::HepVector(5, 0);
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
      if (fabs(change) < convergence) break;
      //if (change < 0.) break;
      //jtanaka -- from traffs -- Ozaki-san added this part to traffs.
      if (change < 0.) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "chi2Old, chi2=" << chi2Old << " " << chi2 << std::endl;
#endif
        //change to the old value.
        a += factor * da;
        t._helix->a(a);

        chi2 = 0.;
        for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
        d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

        //...Loop with hits...
        unsigned i = 0;
        while (TLink* l = t.links()[i++]) {
          const Belle2::TRGCDCWireHit& h = * l->hit();

          //...Check state...
          if (h.state() & CellHitInvalidForFit) continue;
          if (!(h.state() & CellHitFittingValid)) continue;

          //...Check wire...
          if (! nTrial)
            if (h.wire().stereo()) allAxial = false;

          //...Cal. closest points...
          int doSagCorrection = 0;
          if (nTrial  && !allAxial) doSagCorrection = 1;
          t.approach(* l, doSagCorrection);
          double dPhi = l->dPhi();
          const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
          const HepGeom::Point3D<double> & onWire = l->positionOnWire();

#ifdef TRASAN_DEBUG_DETAIL
          // std::cout << "    in fit in case of change < 0. " << onTrack << ", " << onWire;
          // h.dump();
#endif

          //...Obtain drift distance and its error...
          unsigned leftRight = CellHitRight;
          if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;
          double distance = h.drift(leftRight);
          double eDistance = h.dDrift(leftRight);
          if (nTrial  && !allAxial) {
            int wire = h.wire().id();
            int side = leftRight;
            if (side == 0) side = -1;
            float x[3] = { onWire.x(), onWire.y(), onWire.z()};
            float p[3] = {t.p().x(), t.p().y(), t.p().z()};
//cnv               float time = l->hit()->reccdc()->m_tdc + t0Offset;
            float time = 0;
            float dist = distance;
            float edist = eDistance;
            int doPropDelay = 1; //
            calcdc_driftdist_(
              &doPropDelay, &wire, &side, p, x, &time, &dist, &edist);
            distance = (double) dist;
            eDistance = (double) edist;
          }
          double eDistance2 = eDistance * eDistance;

          //...Residual...
          Vector3D v = onTrack - onWire;
          double vmag = v.mag();
          double dDistance = vmag - distance;

          //...dxda...
          this->dxda(*l, t.helix(), dPhi, dxda, dyda, dzda, doSagCorrection);

          //...Chi2 related...
          //dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
          HepGeom::Vector3D<double> vw = h.wire().direction();
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
                 : CLHEP::HepVector(5, 0);
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
        //break;
        factor *= 0.75;
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "factor = " << factor << std::endl;
        std::cout << "chi2 = " << chi2 << std::endl;
#endif
        if (factor < 0.01)break;
      }

      chi2Old = chi2;

      //...Cal. helix parameters for next loop...
      if (allAxial) {
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

#ifdef TRASAN_DEBUG_DETAIL
      // std::cout << "    fit " << nTrial << " : " << da << std::endl;
      // std::cout << "        d2chi " << d2chi2d2a << std::endl;
      // std::cout << "        dchi2 " << dchi2da << std::endl;
#endif

      a -= factor * da;
      t._helix->a(a);
      ++nTrial;
    }

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(5, 0);
    unsigned dim;
    if (allAxial) {
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
    } else {
      err = TFitFailed;
    }

    t._ndf = nValid - dim;
    t._chi2 = chi2;
    // t._fitted = true;

    return err;
  }

// addition by matsu ( 1999/07/05 )
  int
  TCosmicFitter::fitWithCathode(TTrackBase& b, float t0Offset,
                                float windowSize , int SysCorr)
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TCosmicFitter::fitCathode ..." << std::endl;
#endif

    //...Already fitted ? ...
    if (b.fittedWithCathode()) return 0;

    //...Check # of his...
    if (b.nCores() < 5) return -1;

    //...Type check...
    if (b.objectType() != Track) return TFitUnavailable;
    TTrack& t = (TTrack&) b;

    //...for cathode ndf...
    int NusedCathode(0);

    //...Setup...
    unsigned nTrial = 0;
    CLHEP::HepVector a(5), da(5);
    a = t.helix().a();
    CLHEP::HepVector dxda(5);
    CLHEP::HepVector dyda(5);
    CLHEP::HepVector dzda(5);
    CLHEP::HepVector dDda(5);
    CLHEP::HepVector dDzda(5);  // for cathode z
    CLHEP::HepVector dchi2da(5);
    CLHEP::HepSymMatrix d2chi2d2a(5, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    const double convergence = Convergence;
    bool allAxial = true;
    int LayerStat(0); // layer status  axial:0 stereo:1 cathode:2
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;

//  const AList<Belle2::TRGCDCCatHit> & chits = t.catHits();

    CLHEP::HepVector maxDouble(5);
    for (unsigned i = 0; i < 5; i++) maxDouble[i] = (FLT_MAX);

    //...Fitting loop...
    while (nTrial < NTrailMax) {

      //...Set up...
      chi2 = 0.;
      NusedCathode = 0;
      for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
      d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

      //...Loop with hits...
      unsigned i = 0;
      AList<TLink> cores = t.cores();
      while (TLink* l = cores[i++]) {

        const Belle2::TRGCDCWireHit& h = * l->hit();

        // Check layer status ( cathode added )
        LayerStat = 0;
        if (h.wire().stereo()) LayerStat = 1;
        unsigned nlayer = h.wire().layerId();
        if (nlayer == 0 || nlayer == 1 || nlayer == 2) LayerStat = 2;

        //...Check wire...
        if (! nTrial)
          if (h.wire().stereo() || LayerStat == 2) allAxial = false;

        //...Cal. closest points...
        int doSagCorrection = 0;
        if (nTrial && !allAxial) doSagCorrection = 1;
        t.approach(* l, doSagCorrection);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();

#ifdef TRASAN_DEBUG_DETAIL
        // std::cout << "    in fitCathode " << onTrack << ", " << onWire;
        // h.dump();
#endif

        //...Obtain drift distance and its error...
        unsigned leftRight = CellHitRight;
        if (onWire.cross(onTrack).z() < 0.)     leftRight = CellHitLeft;
        double distance = h.drift(leftRight);
        double eDistance = h.dDrift(leftRight);
        //...
        if (nTrial  && !allAxial) {
          int wire = h.wire().id();
          int side = leftRight;
          if (side == 0) side = -1;
          float x[3] = { onWire.x(), onWire.y(), onWire.z()};
          float p[3] = {t.p().x(), t.p().y(), t.p().z()};
//cnv               float time = l->hit()->reccdc()->m_tdc + t0Offset;
          float time = 0;
          float dist =  distance;
          float edist = eDistance;
          int doPropDelay = 1;
          calcdc_driftdist_(
            &doPropDelay, &wire, &side, p, x, &time, &dist, &edist);
          distance = (double) dist;
          eDistance = (double) edist;
        }
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda(*l, t.helix(), dPhi, dxda, dyda, dzda , doSagCorrection);

        // ... Chi2 related ...
        double pChi2(0.);

        if (LayerStat == 0 || LayerStat == 1) {
          // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
          HepGeom::Vector3D<double> vw = h.wire().direction();
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
                 : CLHEP::HepVector(5, 0);
          if (vmag <= 0.0) {
            std::cout << "    in fit " << onTrack << ", " << onWire;
            h.dump();
          }
          dchi2da += (dDistance / eDistance2) * dDda;
          d2chi2d2a += vT_times_v(dDda) / eDistance2;
          double pChi2 = dDistance * dDistance / eDistance2;
          chi2 += pChi2;

        } else {


          dDda = (v.x() * dxda + v.y() * dyda) / v.perp();
          dchi2da += (dDistance / eDistance2) * dDda;
          d2chi2d2a += vT_times_v(dDda) / eDistance2;

          pChi2 = 0;
          pChi2 += (dDistance / eDistance) * (dDistance / eDistance) ;

          if (l->usecathode() >= 3) {

//cnv             Belle2::TRGCDCClust * mclust = l->getmclust();

//               double dDistanceZ(t.helix().x(dPhi).z());

//               if( SysCorr ){
//                 if( !nTrial ) {
//                   mclust->zcalc( atan( t.helix().tanl()) );
//                   mclust->esterz( atan( t.helix().tanl()) );
//                 }

//                 dDistanceZ -= mclust->zclustWithSysCorr();
//              } else {
//                 dDistanceZ -= mclust->zclust();
//              }

//              double eDistanceZ(mclust->erz());
//              if( !eDistanceZ ) eDistanceZ = 99999.;


//               double eDistance2Z = eDistanceZ * eDistanceZ;
//               double pChi2z = 0;
//               pChi2z = (dDistanceZ/eDistanceZ);
//               pChi2z *= pChi2z;

#ifdef TRASAN_DEBUG_DETAIL
//              std::cout << "dDistanceZ = " << dDistanceZ << std::endl;
#endif

//       //.... requirement for use of cluster
//       if( nTrial == 0 &&
//           fabs(dDistanceZ)< windowSize &&
//             mclust->chits().length() == 1
//           ) l->setusecathode(4 );

//       if( l->usecathode() == 4 ){
//                 NusedCathode++;
//                 dDzda = dzda ;
//                 dchi2da += (dDistanceZ/eDistance2Z) * dDzda;
//                 d2chi2d2a += vT_times_v(dDzda)/eDistance2Z;
//                 pChi2 +=  pChi2z ;

//             }
//           }

//         } // end Chi2 related

//         chi2 += pChi2;


//         //...Store results...
//         l->update(onTrack, onWire, leftRight, pChi2);


          } // TLink loop end

          //...Check condition...
          double change = chi2Old - chi2;
          //if(TRASAN_DEBUG_DETAIL>0)  std::cout << "chi2 change = " <<change << std::endl;

          if (fabs(change) < convergence) break;
          if (change < 0.) {

#ifdef TRASAN_DEBUG_DETAIL
            std::cout << "chi2Old, chi2=" << chi2Old << " " << chi2 << std::endl;
#endif

            NusedCathode = 0;
            //change to the old value.
            a += factor * da;
            t._helix->a(a);


            chi2 = 0.;
            for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
            d2chi2d2a = CLHEP::HepSymMatrix(5, 0);


            //...Loop with hits...
            unsigned i = 0;
            while (TLink* l = cores[i++]) {

              const Belle2::TRGCDCWireHit& h = * l->hit();

              // Check layer status ( cathode added )
              LayerStat = 0;
              if (h.wire().stereo()) LayerStat = 1;
              unsigned nlayer = h.wire().layerId();
              if (nlayer == 0 || nlayer == 1 || nlayer == 2) LayerStat = 2;

              //...Cal. closest points...
              int doSagCorrection = 0;
              if (nTrial  && !allAxial) doSagCorrection = 1;
              t.approach(* l , doSagCorrection);
              double dPhi = l->dPhi();
              const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
              const HepGeom::Point3D<double> & onWire = l->positionOnWire();

#ifdef TRASAN_DEBUG_DETAIL
              // std::cout << "    in fitCathode " << onTrack << ", " << onWire;
              // h.dump();
#endif

              //...Obtain drift distance and its error...
              unsigned leftRight = CellHitRight;
              if (onWire.cross(onTrack).z() < 0.)     leftRight = CellHitLeft;
              double distance = h.drift(leftRight);
              double eDistance = h.dDrift(leftRight);
              if (nTrial  && !allAxial) {
                int wire = h.wire().id();
                int side = leftRight;
                if (side == 0) side = -1;
                float x[3] = { onWire.x(), onWire.y(), onWire.z()};
                float p[3] = {t.p().x(), t.p().y(), t.p().z()};
//cnv               float time = l->hit()->reccdc()->m_tdc + t0Offset;
                float time = 0;
                float dist = distance;
                float edist = eDistance;
                int doPropDelay = 1; //
                calcdc_driftdist_(
                  &doPropDelay, &wire, &side, p, x, &time, &dist, &edist);
                distance = (double) dist;
                eDistance = (double) edist;
              }
              double eDistance2 = eDistance * eDistance;

              //...Residual...
              Vector3D v = onTrack - onWire;
              double vmag = v.mag();
              double dDistance = vmag - distance;

              //...dxda...
              this->dxda(* l, t.helix(), dPhi, dxda, dyda, dzda , doSagCorrection);

              // ... Chi2 related ...
              double pChi2(0.);


              if (LayerStat == 0 || LayerStat == 1) {
                // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
                HepGeom::Vector3D<double> vw = h.wire().direction();
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
                       : CLHEP::HepVector(5, 0);
                if (vmag <= 0.0) {
                  std::cout << "    in fit " << onTrack << ", " << onWire;
                  h.dump();
                }
                dchi2da += (dDistance / eDistance2) * dDda;
                d2chi2d2a += vT_times_v(dDda) / eDistance2;
                double pChi2 = dDistance * dDistance / eDistance2;
                chi2 += pChi2;

              } else {

//           dDda = ( v.x() * dxda + v.y() * dyda )/v.perp();
//           dchi2da += (dDistance/eDistance2) * dDda;
//           d2chi2d2a += vT_times_v(dDda)/eDistance2;

//           pChi2 = 0;
//           pChi2 += (dDistance/eDistance) * (dDistance/eDistance) ;

//           if( l->usecathode() == 4 ){

//             Belle2::TRGCDCClust * mclust = l->getmclust();

//             if( mclust ){
//                 NusedCathode++;

//               double dDistanceZ(t.helix().x(dPhi).z());
//               if( SysCorr ) dDistanceZ -= mclust->zclustWithSysCorr();
//               else          dDistanceZ -= mclust->zclust();

//               double eDistanceZ(99999.);
//               if( mclust->erz() != 0. ) eDistanceZ = mclust->erz();

//               double eDistance2Z = eDistanceZ * eDistanceZ;
//               double pChi2z = 0;
//               pChi2z = (dDistanceZ/eDistanceZ);
//               pChi2z *= pChi2z;

//                 dDzda = dzda ;
//                 dchi2da += (dDistanceZ/eDistance2Z) * dDzda;
//                 d2chi2d2a += vT_times_v(dDzda)/eDistance2Z;
//                 pChi2 +=  pChi2z ;
//             }

//            }

              } // end Chi2 related

              chi2 += pChi2;


              //...Store results...
              l->update(onTrack, onWire, leftRight, pChi2);

            }

            //break;

            factor *= 0.75;
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << "factor = " << factor << std::endl;
            std::cout << "chi2 = " << chi2 << std::endl;
#endif
            if (factor < 0.01)break;

          }

          chi2Old = chi2;

          //...Cal. helix parameters for next loop...
          if (allAxial) {
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

#ifdef TRASAN_DEBUG_DETAIL
          // std::cout << "    fit " << nTrial << " : " << da << std::endl;
          // std::cout << "        d2chi " << d2chi2d2a << std::endl;
          // std::cout << "        dchi2 " << dchi2da << std::endl;
#endif

          a -= da;
          t._helix->a(a);
          ++nTrial;
        }


        //...Cal. error matrix...
        CLHEP::HepSymMatrix Ea(5, 0);
        unsigned dim;
        if (allAxial) {
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
        } else {
          err = TFitFailed;
        }

        t._ndf = t.nCores() + NusedCathode - dim;

        t._chi2 = chi2;

        t._fittedWithCathode = true;

        return err;
      }
    }

    return err;
  }

// end of addition

  int
  TCosmicFitter::dxda(const TLink& link,
                      const THelix& h,
                      double dPhi,
                      CLHEP::HepVector& dxda,
                      CLHEP::HepVector& dyda,
                      CLHEP::HepVector& dzda,
                      int doSagCorrection) const
  {

    //...Setup...
    const Belle2::TRGCDCWire& w = * link.wire();
    CLHEP::HepVector a = h.a();
    double dRho  = a[0];
    double phi0  = a[1];
    double kappa = a[2];
    double rho   = THelix::ConstantAlpha / kappa;
    double tanLambda = a[4];

    double sinPhi0 = sin(phi0);
    double cosPhi0 = cos(phi0);
    double sinPhi0dPhi = sin(phi0 + dPhi);
    double cosPhi0dPhi = cos(phi0 + dPhi);
    CLHEP::HepVector dphida(5);
    //... sag correction
    HepGeom::Point3D<double> xw = w.xyPosition();
    HepGeom::Point3D<double>  wireBackwardPosition = w.backwardPosition();
    Vector3D v = w.direction();

    if (doSagCorrection) {
      int  wireID = w.id();
      float wirePosition[3];
      float wireZ = link.positionOnTrack().z();
      float dydz = 0;
      float ybSag = 0;
      float yfSag = 0;
      if (wireZ > w.backwardPosition().z() &&
          wireZ < w.forwardPosition().z()) {

        calcdc_sag3_(&wireID, &wireZ, wirePosition, &dydz, &ybSag, &yfSag);

        //... wire Position
        xw.setX((double) wirePosition[0]);
        xw.setY((double) wirePosition[1]);
        xw.setZ((double) wirePosition[2]);
        //...
        wireBackwardPosition.setY((double) ybSag);
        //...
        //...        v.setY((double) dydz);
        Vector3D v_aux(w.forwardPosition().x() - w.backwardPosition().x(),
                       yfSag - ybSag,
                       w.forwardPosition().z() - w.backwardPosition().z());
        v = v_aux.unit();
      }
    }

    //...Axial case...
    if (w.axial()) {
      //  Point3D d = h.center() - w.xyPosition();
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
      Point3D onTrack = h.x(dPhi);
      CLHEP::HepVector c(3);
      //c = Point3D(w.backwardPosition() - (v * w.backwardPosition()) * v);
      c = Point3D(wireBackwardPosition - (v * wireBackwardPosition) * v);

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

      double dxdphi_dot_v = dxdphi[0] * v.x() + dxdphi[1] * v.y() + dxdphi[2] * v.z();
      double x_dot_v      = x[0] * v.x() + x[1] * v.y() + x[2] * v.z();

      double dfdphi = - (dxdphi[0] - dxdphi_dot_v * v.x()) * dxdphi[0]
                      - (dxdphi[1] - dxdphi_dot_v * v.y()) * dxdphi[1]
                      - (dxdphi[2] - dxdphi_dot_v * v.z()) * dxdphi[2]
                      - (x[0] - c[0] - x_dot_v * v.x()) * d2xdphi2[0]
                      - (x[1] - c[1] - x_dot_v * v.y()) * d2xdphi2[1];
      /*  - (x[2] - c[2] - x_dot_v*v.z()) * d2xdphi2[2];  = 0. */


      //dxda_phi, dyda_phi, dzda_phi : phi is fixed
      CLHEP::HepVector dxda_phi(5);
      dxda_phi[0] =  cosPhi0;
      dxda_phi[1] = -(dRho + rho) * sinPhi0 + rho * sinPhi0dPhi;
      dxda_phi[2] = -(rho / kappa) * (cosPhi0 - cosPhi0dPhi);
      dxda_phi[3] =  0.;
      dxda_phi[4] =  0.;

      CLHEP::HepVector dyda_phi(5);
      dyda_phi[0] =  sinPhi0;
      dyda_phi[1] = (dRho + rho) * cosPhi0 - rho * cosPhi0dPhi;
      dyda_phi[2] = -(rho / kappa) * (sinPhi0 - sinPhi0dPhi);
      dyda_phi[3] =  0.;
      dyda_phi[4] =  0.;

      CLHEP::HepVector dzda_phi(5);
      dzda_phi[0] =  0.;
      dzda_phi[1] =  0.;
      dzda_phi[2] = (rho / kappa) * tanLambda * dPhi;
      dzda_phi[3] =  1.;
      dzda_phi[4] = -rho * dPhi;

      CLHEP::HepVector d2xdphida(5);
      d2xdphida[0] =  0.;
      d2xdphida[1] =  rho * cosPhi0dPhi;
      d2xdphida[2] = -(rho / kappa) * sinPhi0dPhi;
      d2xdphida[3] =  0.;
      d2xdphida[4] =  0.;

      CLHEP::HepVector d2ydphida(5);
      d2ydphida[0] = 0.;
      d2ydphida[1] = rho * sinPhi0dPhi;
      d2ydphida[2] = (rho / kappa) * cosPhi0dPhi;
      d2ydphida[3] = 0.;
      d2ydphida[4] = 0.;

      CLHEP::HepVector d2zdphida(5);
      d2zdphida[0] =  0.;
      d2zdphida[1] =  0.;
      d2zdphida[2] = (rho / kappa) * tanLambda;
      d2zdphida[3] =  0.;
      d2zdphida[4] = -rho;

      CLHEP::HepVector dfda(5);
      for (int i = 0; i < 5; i++) {
        double d_dot_v = v.x() * dxda_phi[i]
                         + v.y() * dyda_phi[i]
                         + v.z() * dzda_phi[i];
        dfda[i] = - (dxda_phi[i] - d_dot_v * v.x()) * dxdphi[0]
                  - (dyda_phi[i] - d_dot_v * v.y()) * dxdphi[1]
                  - (dzda_phi[i] - d_dot_v * v.z()) * dxdphi[2]
                  - (x[0] - c[0] - x_dot_v * v.x()) * d2xdphida[i]
                  - (x[1] - c[1] - x_dot_v * v.y()) * d2ydphida[i]
                  - (x[2] - c[2] - x_dot_v * v.z()) * d2zdphida[i];
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

    return 0;
  }

} // namespace Belle

