//-----------------------------------------------------------------------------
// $Id: TTrackMC.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TTrackMC.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to have MC information of TTrack.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.36  2004/04/08 22:02:47  yiwasaki
// MC quality check added
//
// Revision 1.35  2003/12/19 07:36:03  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.34  2002/02/22 06:37:43  katayama
// Use __sparc
//
// Revision 1.33  2002/02/21 23:49:16  katayama
// For -ansi and other warning flags
//
// Revision 1.32  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.31  2001/12/19 02:59:48  katayama
// Uss find,istring
//
// Revision 1.30  2001/05/07 20:51:19  yiwasaki
// <float.h> included for linux
//
// Revision 1.29  2001/04/11 01:09:12  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.27  2001/01/30 20:51:50  yiwasaki
// Trasan 2.22 : bug fixes
//
// Revision 1.26  2001/01/17 06:27:56  yiwasaki
// hists in sakura modified
//
// Revision 1.25  2001/01/11 04:40:23  yiwasaki
// minor changes
//
// Revision 1.24  2000/04/13 02:53:42  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.23  2000/04/11 13:05:48  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.22  1999/10/30 10:12:25  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.21  1999/09/10 09:19:58  yiwasaki
// Trasan 1.61b release : new parameters added, TTrackMC bug fixed
//
// Revision 1.20  1999/06/24 11:27:43  itoh
// Fixes for DEC Alpha
//
// Revision 1.19  1999/04/09 11:36:55  yiwasaki
// Trasan 1.32 release : TCosmicFitter minor change
//
// Revision 1.18  1999/02/03 06:23:12  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.17  1999/01/25 03:16:12  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.16  1998/12/16 12:00:35  katayama
// 0 div again
//
// Revision 1.15  1998/12/04 15:11:02  yiwasaki
// TRGCDC creation timing changed, zero-division protection for TTrackMC
//
// Revision 1.14  1998/11/27 08:15:36  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.13  1998/11/15 04:32:54  katayama
// Protect acos
//
// Revision 1.12  1998/11/12 12:27:31  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.11  1998/11/11 13:36:26  katayama
// Protectoin again, again
//
// Revision 1.10  1998/11/11 13:30:59  katayama
// Protectoin again
//
// Revision 1.9  1998/11/11 11:04:09  yiwasaki
// protection again
//
// Revision 1.8  1998/11/11 07:26:49  yiwasaki
// Trasan 1.1 beta 9 release : more protections for negative sqrt and zero division
//
// Revision 1.7  1998/11/10 09:09:12  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.6  1998/09/28 16:11:13  yiwasaki
// fitter with cathode added
//
// Revision 1.5  1998/09/28 14:54:12  yiwasaki
// MC tables, TUpdater, oichan added
//
// Revision 1.4  1998/09/24 22:56:41  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.3  1998/08/31 05:15:55  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.2  1998/08/13 01:53:00  yiwasaki
// Missing symbols added, link option added
//
// Revision 1.1  1998/08/12 16:32:52  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
//-----------------------------------------------------------------------------
#include <string>
#include <exception>
#include <cfloat>


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

#define TRG_SHORT_NAMES

#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"

#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/Trasan.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TTrackMC.h"
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

namespace Belle {

#if defined(__alpha)
#define DBL_MIN 2.2250738585072014E-308
#define FLT_MIN 1.175494351E-38F
#endif

  TTrackMC::TTrackMC(const TTrack& t)
    : _state(0),
      _quality(0),
      _t(t),
      _hep(0),
      _hepID(-1),
      _wireFraction(-999.),
      _wireFractionHEP(-999.),
      _charge(false),
      _ptFraction(-999.),
      _pzFraction(-999.),
      _ptResidual(-999.),
      _pzResidual(-999.),
      _ptPull(-999.),
      _pzPull(-999.)
  {
  }

  TTrackMC::~TTrackMC()
  {
  }

  void
  TTrackMC::update(void)
  {
    _state = 0;
    _quality = 0;

    //...Prepare counters...
//cnv    unsigned nHep = Belle2::TRGCDCTrackMC::list().length();
    unsigned nHep = 0;
//  unsigned nTrk = Trasan::getTrasan()->tracks().length();
    unsigned* N1 = (unsigned*) malloc(nHep * sizeof(unsigned));
    float* F1 = (float*) malloc(nHep * sizeof(float));
//cnv    unsigned N2 = 0;
    for (unsigned i = 0; i < nHep; i++) {
      N1[i] = 0;
      F1[i] = 0.;
    }

    //...Prepare for fraction F1...
//  const AList<TLink> & cores = _t.cores();
    const AList<TLink> & cores = _t.finalHits();
    unsigned nCores = cores.length();
    for (unsigned i = 0; i < nCores; i++) {
//cnv TLink * t = cores[i];
//cnv   if(t && t->hit() && t->hit()->mc() && t->hit()->mc()->hep() ) {
//    int hepID = t->hit()->mc()->hep()->id();
//    ++N1[hepID];
//  }
    }

    //...Calculate fraction F1 and find the best HEP...
    int bestHep = -1;
    float bestF1 = 0.;
    for (unsigned i = 0; i < nHep; i++) {
      if (nCores) F1[i] = (float) N1[i] / (float) nCores;
      if (F1[i] > bestF1) {
        bestHep = i;
        bestF1 = F1[i];
      }
    }

    //...Check HEP...
    float F2 = 0.;
    Belle2::TRGCDCTrackMC* hep = 0;
    if (bestHep != -1) {
//cnv   hep = Belle2::TRGCDCTrackMC::list()[bestHep];
//  unsigned nAll = 0;
//  for (unsigned i = 0; i < (unsigned) hep->hits().length(); i++) {
//      const TRGCDCWireHit * hit = hep->hits()[i]->hit();
//      if (! hit) continue;
//      if (hit->state() & WireHitInvalidForFit) continue;

//      ++nAll;
//      if (hit->track() == & _t) ++N2;
//  }

//  //...Calculate fraction F2...
//  if (nAll) F2 = (float) N2 / (float) nAll;
    }

    //...Store results...
    _hepID = bestHep;
    _hep = hep;
    _wireFraction = bestF1;
    _wireFractionHEP = F2;

    //...Compare charge and momentum...
    compare();

    //...Classification...
    classify();

    //...Termination...
    free(N1);
    free(F1);
  }

  void
  TTrackMC::dump(const std::string& msg, const std::string& pre) const
  {
    std::cout << pre << msg;
    std::cout << _t.name() << ":";
    std::cout << "state=" << _state << ":";
    if (_quality & TTrackGood) std::cout << "good   :";
    else if (_quality & TTrackGhost) std::cout << "ghost  :";
    else if (_quality & TTrackBad) std::cout << "bad    :";
    else if (_quality & TTrackCharge) std::cout << "bad    :";
    else if (_quality & TTrackGarbage) std::cout << "garbage:";
    else std::cout << "classification error:";
    Belle2::TRGUtil::bitDisplay(_quality, 23, 15); std::cout << ":";
    std::cout << _hepID << ":";
    std::cout << _wireFraction << "," << _wireFractionHEP << ":";
    std::cout << _ptFraction << "," << _pzFraction;
    std::cout << std::endl;
  }

  void
  TTrackMC::compare(void)
  {
    if (! _hep) return;

    //...Get charge of HEP particle(This part should be done by LUCHARGE)...
//cnv    int id = _hep->pType();
    int id = 0;
    int aId = abs(id);
    if (aId == 11 || aId == 13 || aId == 15) id *= -1;

    //...Compare charge...
    if ((int) _t.charge() * id > 0) _charge = true;

    //...Get hep mom. at the inner-most hit...
//  AList<TLink> list = _t.cores();
    AList<TLink> list = _t.finalHits();
    unsigned n = list.length();
    bool found = false;
    HepGeom::Vector3D<double> pHep;
    HepGeom::Vector3D<double> vHep;
    for (unsigned i = 0; i < n; i++) {
      TLink* inner = TLink::innerMost(list);
      if (inner->hit()->mc()->hep() == _hep) {
        pHep = inner->hit()->mc()->momentum();
        vHep = inner->hit()->mc()->entrance();
        found = true;
        break;
      }
      list.remove(inner);
    }
    THelix hHep = THelix(vHep, pHep, copysign(1., id));
#if defined(BELLE_DEBUG)
    try {
#endif
      hHep.pivot(_t.helix().pivot());
      pHep = hHep.momentum();
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "TTrackMC::compare:exception has been thrown:" << e << std::endl;
      found = false;
    }
#endif
    //...For debug...
    if (! found) {
      std::cout << "TTrackMC::compare !!! something wrong with mc hits"
                << std::endl;

      //...For debug...
//    list = _t.cores();
//    for (unsigned i = 0; i < list.length(); i++) {
//        TLink * inner = innerMost(list);
//        std::cout << i << " " << inner << ":" << inner->hit()->mc()->hep();
//        std::cout << " " << _hep << std::endl;
//        if (inner->hit()->mc()->hep() == _hep) {
//      pHep = inner->hit()->mc()->momentum();
//      break;
//        }
//        list.remove(inner);
//    }
//    TLink * t = 0;
//    t->hit();
      //...For debug end...

      return;
    }

    //...Fill caches...
    _residual = _t.p() - pHep;
//cnv    _cosOpen = pHep.unit().dot(_t.p().unit());
    _cosOpen = 0;

    //...Compare pt...
    double pt = _t.pt();
    double ptHep = sqrt(pHep.x() * pHep.x() + pHep.y() * pHep.y());
    _ptResidual = pt - ptHep;
    const THelix& h = _t.helix();
    CLHEP::HepVector dPt(5, 0);
    dPt[2] = - 1. / (h.kappa() * h.kappa());
    double ptError2 = h.Ea().similarity(dPt);
    if (ptError2 < 0.0) {
      std::cout << h.kappa() << " " << h.Ea() << " dPt=" << dPt << std::endl;
    }
    double ptError = (ptError2 > 0.) ? sqrt(ptError2) : (DBL_MIN);
    _ptPull = (ptError2 > 0.) ? (_ptResidual) / ptError : (FLT_MAX);
    _ptFraction = (fabs(ptHep) > (FLT_MIN)) ? _ptResidual / ptHep : 0.0;

    //...Compare pz...
    double pz = _t.pz();
    double pzHep = pHep.z();
    _pzResidual = pz - pzHep;
    CLHEP::HepVector dPz(5, 0);
    dPz[2] = - h.tanl() / (h.kappa() * h.kappa());
    dPz[4] = 1. / h.kappa();
    double pzError2 = h.Ea().similarity(dPz);
    if (pzError2 < 0.0) {
      std::cout << h.kappa() << " " << h.Ea() << " dPz=" << dPz << std::endl;
    }
    double pzError = (pzError2 > 0.) ? sqrt(pzError2) : (DBL_MIN);
    _pzPull = (pzError2 > 0.) ? (_pzResidual) / pzError : (FLT_MAX);
    _pzFraction = (abs(pzHep) > FLT_MIN) ? (_pzResidual / pzHep) : (FLT_MAX);
  }

  void
  TTrackMC::classify(void)
  {
    _state |= TTrackClassified;
    _quality = TTrackGarbage;

    //...HEP matching...
    if (! _hep) return;
    _quality |= TTrackHep;
    if (fabs(_ptFraction) < .1) _quality |= TTrackPt;
    if (fabs(_pzFraction) < .1) _quality |= TTrackPz;

    if ((fabs(_ptFraction) < 0.1) &&
        (fabs(_pzFraction) < 0.1) &&
        (_cosOpen > 0.99))
      _quality |= TTrackMatchingLoose;
    if ((fabs(_ptFraction) < 0.02) &&
        (fabs(_pzFraction) < 0.02) &&
        (_cosOpen > 0.998))
      _quality |= TTrackMatchingTight;

//     float momResidual = sqrt(_ptResidual * _ptResidual +
//            _pzResidual * _pzResidual);
//     if ((momResidual < 0.100) && (_cosOpen > 0.99))
//  _quality |= TTrackMatchingLoose;
//     if ((momResidual < 0.020) && (_cosOpen > 0.998))
//  _quality |= TTrackMatchingTight;

    //...Wire fraction...
    if (_wireFraction < 0.8) return;
    _quality |= TTrackWire;
    _quality |= TTrackCharge;

    //...Charge matching...
    if (! _charge) return;
    _quality |= TTrackBad;

    //...Momentum matching...
    if (_quality & TTrackMatchingLoose)
      _quality |= TTrackGhost;

    //...TTrackGood is set by Trasan after checking uniqueness...
  }

  std::string
  TTrackMC::qualityString(void) const
  {
    return TrackMCQualityString(_quality);
  }

  std::string
  TrackMCStatus(unsigned quality)
  {
    //...This is a local function to hide from user...

    std::string matching;
    if (quality & TTrackHep) {
      if (quality & TTrackMatchingTight) matching += "tight";
      else if (quality & TTrackMatchingLoose) matching += "loose";
      else matching = "bad";
    }
    return TrackMCQualityString(quality) + " " + matching;
  }

  std::string
  TrackMCStatus(const TTrackMC& m)
  {
    return TrackMCStatus(m.quality());
  }

  std::string
  TrackMCStatus(const reccdc_mctrk& m)
  {
    return TrackMCStatus(m.m_quality);
  }

  std::string
  TrackMCQualityString(unsigned quality)
  {
    if (quality & TTrackGood) return std::string("Good");
    else if (quality & TTrackGhost) return std::string("Ghost");
    else if (quality & TTrackBad) return std::string("Bad");
    else if (quality & TTrackCharge) return std::string("Charge");
    else if (quality & TTrackGarbage) return std::string("Garbage");
    return std::string("Unknown");
  }

} // namespace Belle

