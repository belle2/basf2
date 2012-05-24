//-----------------------------------------------------------------------------
// $Id: TRungeFitter.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TRungeFitter.cc
// Section  : Tracking
// Owner    : Kenji Inami
// Email    : inami@bmail.kek.jp
//-----------------------------------------------------------------------------
// Description : A class to fit a TTrackBase object to a Runge Kutta track
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.6  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.5  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.4  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.3  2001/12/13 15:31:59  katayama
// MDST_OBS
//
// Revision 1.2  2001/09/18 05:59:05  katayama
// Fixes for CC
//
// Revision 1.1  2001/08/02 07:04:17  yiwasaki
// RK fitter from K.Inami
//
//-----------------------------------------------------------------------------
#include <cfloat>
#include <ctime>
#include <iostream>




#include "tracking/modules/trasan/TRungeFitter.h"
#include "tracking/modules/trasan/TRunge.h"
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
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrackBase.h"

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

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

  TRungeFitter::TRungeFitter(const std::string& name)
    : TFitter(name),
      _sag(true), _propagation(1), _tof(false)
  {
  }
  TRungeFitter::TRungeFitter(const std::string& name,
                             bool m_sag, int m_prop, bool m_tof)
    : TFitter(name),
      _sag(m_sag), _propagation(m_prop), _tof(m_tof)
  {
  }

  TRungeFitter::~TRungeFitter()
  {
  }

  void TRungeFitter::sag(bool _in)
  {
    _sag = _in;
  }
  void TRungeFitter::propagation(int _in)
  {
    _propagation = _in;
  }
  void TRungeFitter::tof(bool _in)
  {
    _tof = _in;
  }
  int TRungeFitter::fit(TTrackBase& tb) const
  {
    return fit(tb, 0);
  }

  int TRungeFitter::fit(TTrackBase& tb, float t0Offset) const
  {

    // std::cout<<"TRungeFitter::fit  start"<<std::endl;

    //...Type check...
    if (tb.objectType() != Runge) return TFitUnavailable;
    TRunge& t = (TRunge&) tb;

    //...Already fitted ?...
    if (t.fitted()) return TFitAlreadyFitted;

    //...Count # of hits...
    AList<TLink> cores = t.cores();
    unsigned nCores = cores.length();
    unsigned nStereoCores = TLink::nStereoHits(cores);

    //...Check # of hits...
    if ((nStereoCores < 2) || (nCores - nStereoCores < 3))
      return TFitErrorFewHits;

    //...Move pivot to ORIGIN...
    const HepGeom::Point3D<double> pivot_bak = t.pivot();
    t.pivot(ORIGIN);

    //...Setup...
    CLHEP::HepVector a(5), da(5);
    a = t.a();
    CLHEP::HepVector dDda(5);
    CLHEP::HepVector dchi2da(5);
    CLHEP::HepSymMatrix d2chi2d2a(5, 0);
    const CLHEP::HepSymMatrix zero5(5, 0);
    double chi2;
    double chi2Old = DBL_MAX;
    int err = 0;

    double factor = 0.1;
    CLHEP::HepVector beta(5);
    CLHEP::HepSymMatrix alpha(5, 0);
    CLHEP::HepSymMatrix alpha2(5, 0);

    double(*d)[5] = new double[nCores][5];
    //  double (*d2)[5]=new double[nCores][5];
    CLHEP::HepVector ea(5);

    float tof;
    HepGeom::Vector3D<double> p;
    unsigned i;

    double distance;
    double eDistance;

    // ea... init
    const double ea_init = 0.000001;
    ea[0] = ea_init;  //dr
    ea[1] = ea_init;  //phi0
    ea[2] = ea_init;  //kappa
    ea[3] = ea_init;  //dz
    ea[4] = ea_init;  //tanl

    // std::cout<<"TRF ::"<<a[0]<<","<<a[1]<<","<<a[2]<<","<<a[3]<<","<<a[4]<<std::endl;

    //long int lclock0=clock()/1000;
    //long int lclock=lclock0;

    CLHEP::HepVector def_a(t.a());
    CLHEP::HepVector ta(def_a);

    //...Fitting loop...
    unsigned nTrial = 0;
    while (nTrial < 100) {

      //...Set up...
      chi2 = 0;
      for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0;
      d2chi2d2a = zero5;

      def_a = t.a();

      //#### loop for shifted helix parameter set ####
      for (unsigned j = 0; j < 5; j++) {
        ta = def_a;
        ta[j] += ea[j];
        t.a(ta);
        //...Loop with hits...
        i = 0;
        // std::cout<<"TRF:: cores="<<cores.length()<<std::endl;
        while (TLink* l = cores[i++]) {
          //...Cal. closest points...
          t.approach(*l, tof, p, _sag);
          const Point3D& onTrack = l->positionOnTrack();
          const Point3D& onWire = l->positionOnWire();
          d[i - 1][j] = (onTrack - onWire).mag();
          // std::cout<<"TRF:: i="<<i<<std::endl;
        }//end of loop with hits
        //lclock=clock()/1000;
        // std::cout<<"TRF  clock="<<lclock-lclock0<<std::endl;
        //lclock0=lclock;
      }
      /*
      for(int j=0;j<5;j++){
        ta=def_a;
        ta[j]-=ea[j];
        t.a(ta);
        //...Loop with hits...
        float tof_dummy;
        HepGeom::Vector3D<double> p_dummy;
        unsigned i=0;
        while(TLink* l = cores[i++]){
      //...Cal. closest points...
      t.approach(*l,tof_dummy,p_dummy,_sag);
      const Point3D& onTrack=l->positionOnTrack();
      const Point3D& onWire=l->positionOnWire();
      d2[i-1][j]=(onTrack-onWire).mag();
        }//end of loop with hits
      }
      */
      t.a(def_a);

      //#### original parameter set  and  calc. chi2 ####
      //...Loop with hits...
      i = 0;
      while (TLink* l = cores[i++]) {
        const Belle2::TRGCDCWireHit& h = *l->hit();

        //...Cal. closest points...
        if (t.approach(*l, tof, p, _sag) < 0) {
          std::cout << "trasan:TRF::  bad wire" << std::endl;
          continue;
        }
        const Point3D& onTrack = l->positionOnTrack();
        const Point3D& onWire = l->positionOnWire();
        unsigned leftRight = CellHitRight;
        if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;

        //...Obtain drift distance and its error...
        if ((t0Offset == 0.) && (_propagation == 0) && (! _tof)) {
          //...No correction...
          distance = l->drift(leftRight);
          eDistance = h.dDrift(leftRight);
        } else {
          //...T0 and propagation corrections...
          int wire = h.wire().id();
          int side = leftRight;
          if (side == 0) side = -1;
          float tp[3] = {p.x(), p.y(), p.z()};
          float x[3] = {onWire.x(), onWire.y(), onWire.z()};
//cnv float time = h.reccdc()->m_tdc + t0Offset - tof;
          float time = 0;
          float dist;
          float edist;
          int prop = _propagation;
          calcdc_driftdist_(& prop,
                            & wire,
                            & side,
                            tp,
                            x,
                            & time,
                            & dist,
                            & edist);
          distance = (double) dist;
          eDistance = (double) edist;
        }
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        const double d0 = (onTrack - onWire).mag();
        //if(d0>2) std::cout<<"TRF:: strange dist.  d0="<<d0<<" x="<<distance
        //       <<" ex="<<eDistance<<std::endl;
        double dDistance = d0 - distance;

        //...dDda...
        for (int j = 0; j < 5; j++) {
          dDda[j] = (d[i - 1][j] - d0) / ea[j];
          //if(dDda[j]==0) std::cout<<"TRF:: dDda=0 j="<<j<<" ea="<<ea[j]<<std::endl;
        }
        //      for(int j=0;j<5;j++) dDda[j]=(d[i-1][j]-d2[i-1][j])/ea[j]/2.;
        //...Chi2 related...
        dchi2da += (dDistance / eDistance2) * dDda;
        d2chi2d2a += vT_times_v(dDda) / eDistance2;
        double pChi2 = dDistance * dDistance / eDistance2;
        chi2 += pChi2;
        //if(!(pChi2<0 || pChi2>=0)){
        //  std::cout<<"TRF::  pChi2="<<pChi2<<" X="<<d0<<" fx="<<distance
        //      <<" ex="<<eDistance<<std::endl;
        //}

        //...Store results...
        l->update(onTrack, onWire, leftRight, pChi2);
      }//end of loop with hits

      //...Check condition...
      double change = chi2Old - chi2;

      if (0 <= change && change < 0.01) break;
      if (change < 0.) {
        factor *= 100;
        a += da;  //recover
        if (-0.01 < change) {
          d2chi2d2a = alpha;
          chi2 = chi2Old;
          break;
        }
      } else if (change > 0.) {
        chi2Old = chi2;
        factor *= 0.1;
        alpha = d2chi2d2a;
        beta = dchi2da;
      } else {
        std::cout << "trasan:TRF::  bad chi2 = " << chi2 << std::endl;
        break;  // protection for nan
      }
      alpha2 = alpha;
      for (unsigned j = 0; j < 5; j++) alpha2[j][j] *= (1 + factor);
      //...Cal. helix parameters for next loop...
      da = solve(alpha2, beta);

      //lclock=clock()/1000;
      // std::cout<<"TRF "<<nTrial<<": "
      //  <<"cl="<<lclock-lclock0<<": "
      //  <<a[0]<<","<<a[1]<<","<<a[2]<<","<<a[3]<<","<<a[4]<<" "
      //  <<chi2<<"/"<<nCores<<":"<<factor
      //  <<" :da "<<da[0]<<","<<da[1]<<","<<da[2]<<","<<da[3]<<","<<da[4]<<std::endl;
      //lclock0=lclock;

      a -= da;
      t.a(a);
      //ea = 0.0001*da;
      for (i = 0; i < 5; i++) {
        ea[i] = 0.0001 * abs(da[i]);
        if (ea[i] > ea_init) ea[i] = ea_init;
        if (ea[i] < 1.0e-10) ea[i] = 1.0e-10;
      }
      ++nTrial;
    }
    // std::cout<<"TRungeFitter:: nTrial="<<nTrial<<std::endl;

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(5, 0);
    unsigned dim;
    dim = 5;
    Ea = d2chi2d2a.inverse(err);

    //...Store information...
    if (! err) {
      t.a(a);
      t.Ea(Ea);
      t._fitted = true;
    } else {
      err = TFitFailed;
    }

    t._ndf = nCores - dim;
    t._chi2 = chi2;

    //...Recover pivot...
    t.pivot(pivot_bak);

    delete [] d;
    //  delete [] d2;

    return err;
  }

} // namespace Belle

