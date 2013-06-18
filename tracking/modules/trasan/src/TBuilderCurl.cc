//-----------------------------------------------------------------------------
// $Id: TBuilderCurl.cc 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TBuilderCurl.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a curl track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.31  2005/03/11 03:57:47  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.30  2004/02/18 04:07:25  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.29  2003/12/25 12:03:31  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.28  2001/12/23 09:58:44  katayama
// removed Strings.h
//
// Revision 1.27  2001/12/19 02:59:43  katayama
// Uss find,istring
//
// Revision 1.26  2001/04/12 07:10:56  yiwasaki
// Trasan 3.00 RC4 : new stereo code for curl
//
// Revision 1.25  2001/04/11 01:09:08  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.24  2001/03/16 11:19:01  katayama
// for gcc 2.95.2
//
// Revision 1.23  2000/10/10 06:04:22  yiwasaki
// Trasan 2.11 : curl builder, helix fitter, perfect finder
//
// Revision 1.22  2000/10/05 23:54:20  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.21  2000/09/27 07:45:15  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.20  2000/04/13 02:53:36  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.19  2000/04/07 05:45:16  yiwasaki
// Trasan 2.00rc28 : curl finder bad point rejection, table clear, chisq info
//
// Revision 1.18  2000/02/28 01:58:56  yiwasaki
// Trasan 2.00RC11 : curl updates only
//
// Revision 1.17  2000/02/15 13:46:39  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.16  2000/02/08 07:53:41  katayama
// Bug fix from Tanaka san
//
// Revision 1.15  2000/02/01 11:24:41  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.14  1999/11/19 09:13:05  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.13  1999/10/30 10:12:10  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.12  1999/10/15 04:28:03  yiwasaki
// TWindow is hidden, curl finder parameter
//
// Revision 1.11  1999/08/25 06:25:48  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.10  1999/07/01 08:15:22  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.9  1999/06/14 05:51:02  yiwasaki
// Trasan 1.41 release : curl finder updates
//
// Revision 1.8  1999/04/02 09:15:21  yiwasaki
// Trasan 1.29 release : tighter cuts for curl finder by J.Tanaka
//
// Revision 1.7  1999/03/15 07:57:09  yiwasaki
// Trasan 1.27 release : curl finder update
//
// Revision 1.6  1999/03/12 06:56:42  yiwasaki
// Trasan 1.25 release : curl finder updated, salvage 2 is default, minor bug fixes
//
// Revision 1.5  1999/03/10 12:55:01  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.4  1999/01/11 03:03:07  yiwasaki
// Fitters added
//
// Revision 1.3  1998/11/10 09:08:57  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.2  1998/10/06 02:30:03  yiwasaki
// Trasan 1.1 beta 3 relase : only minor change
//
// Revision 1.1  1998/09/29 01:24:20  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
//-----------------------------------------------------------------------------

#include <memory>




#include "tracking/modules/trasan/TBuilderCurl.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/Lpav.h"
//cnv #include "tracking/modules/trasan/TSvdFinder.h"
//cnv #include "tracking/modules/trasan/TSvdHit.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TRobustLineFitter.h"
#include "trg/cdc/TRGCDC.h"

#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

// Following 3 parameters : (0,0,0) is best!
// Other cases are for the development.
#define DEBUG_CURL_DUMP 0
#define DEBUG_CURL_GNUPLOT 0
#define DEBUG_CURL_MC 0

#if (DEBUG_CURL_GNUPLOT+DEBUG_CURL_MC)
#include "trg/cdc/WireHitMC.h"
#include "tracking/modules/trasan/TTrackHEP.h"
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
#include <algo.h>
#endif

#if (DEBUG_CURL_DUMP+DEBUG_CURL_GNUPLOT+DEBUG_CURL_MC)
#include "tables/belletdf.h"
static int debugMcFlag = 1;
#endif

#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
TWindow szc("szc");
#endif

namespace Belle {

  bool TBuilderCurl::ms_smallcell(false);
  //ho  bool TBuilderCurl::ms_superb(false);
  bool TBuilderCurl::ms_superb(true);

  TBuilderCurl::TBuilderCurl(const std::string& name)
    : TBuilder0(name),
      _fitter("TBuilderCurl Fitter")
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::TBuilderCurl" << std::endl;
#endif
#if 0
    if (m_param.ON_CORRECTION) {
      _fitter.sag(true);
      _fitter.propagation(true);
      _fitter.tof(true);
    }
#endif

  }

  TBuilderCurl::~TBuilderCurl()
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::~TBuilderCurl" << std::endl;
#endif
    if (m_param.SVD_RECONSTRUCTION) {
      //delete m_svdFinder;
//cnv    delete m_svdAssociator;
    }
  }

  void
  TBuilderCurl::setParam(const TCurlFinderParameter& p)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setParam" << std::endl;
#endif
    m_param.Z_CUT = p.Z_CUT;
    m_param.Z_DIFF_FOR_LAST_ATTEND = p.Z_DIFF_FOR_LAST_ATTEND;
    m_param.SELECTOR_MAX_SIGMA = p.SELECTOR_MAX_SIGMA;
    m_param.SELECTOR_MAX_IMPACT = p.SELECTOR_MAX_IMPACT;
    m_param.SELECTOR_STRANGE_PZ = p.SELECTOR_STRANGE_PZ;
    m_param.SELECTOR_REPLACE_DZ = p.SELECTOR_REPLACE_DZ;
    m_param.SVD_RECONSTRUCTION = p.SVD_RECONSTRUCTION;
    m_param.MIN_SVD_ELECTRONS = p.MIN_SVD_ELECTRONS;
    m_param.ON_CORRECTION = p.ON_CORRECTION;
    m_param.CURL_VERSION = p.CURL_VERSION;
#if 1
    if (m_param.ON_CORRECTION) {
      _fitter.sag(true);
      _fitter.propagation(true);
      _fitter.tof(true);
    }
#endif
    if (m_param.SVD_RECONSTRUCTION) {
      //m_svdFinder = new TSvdFinder(-1.*(m_param.MIN_SVD_ELECTRONS),
      //       m_param.MIN_SVD_ELECTRONS);
//cnv     m_svdAssociator = new TSvdAssociator(-1.*(m_param.MIN_SVD_ELECTRONS),
//           m_param.MIN_SVD_ELECTRONS);
    }
  }

// Stereo Finder For Curl Tracks : by jtanaka == from here ==

  void
  TBuilderCurl::resetTHelixFit(THelixFitter* fit) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::resetHelixFit" << std::endl;
#endif
    fit->fit2D(false);
    fit->sag(false);
    fit->propagation(false);
    fit->tof(false);
    fit->freeT0(false);
  }

//.............................................
//...............Main Part.....................
//.............................................

bool
TBuilderCurl::buildStereo(TTrack & track, double &, double &) const {

#if defined(HO_DEBUG)
    std::cout << " TBuilderCurl::buildStereo 0" << std::endl;
#endif
    if (!(m_param.SVD_RECONSTRUCTION))return false;

// #if 0
//   double q = 1.;
//   if(track.helix().kappa()<0.)q = -1.;
//   double h[5], chisq;
//   if(m_svdFinder->recTrk(track.helix().center().x(),
//       track.helix().center().y(),
//       fabs(track.helix().radius()),
//       q,
//       0.2,
//       h[0], h[1], h[2], h[3], h[4], chisq)){
// // #if 0
// //     std::cout << "SVD = "
// //    << h[0] << " "
// //    << h[1] << " "
// //    << h[2] << " "
// //    << h[3] << " "
// //    << h[4];
// //     std::cout << ", chi2 = " << chisq << " pt=" << 1./h[2] << std::endl;
// // #endif
//     dZ   = h[3];
//     tanL = h[4];
//     return true;
//   }else{
//     // std::cout << "SVD Fail....." << std::endl;
//     return false;
//   }
// #else
    THelix th(track.helix());
    th.pivot(HepGeom::Point3D<double> (0., 0., 0.));
//cnv   AList<TSvdHit> cand;
//   if(m_svdAssociator->recTrk(th,dZ,tanL,0.5,-1.0,cand,0.5)){
//     // std::cout << "SVD " << dZ << " " << tanL << std::endl;
//     return true;
//   }else{
//     // std::cout << "SVD..." << std::endl;
//     return false;
//   }
// #endif

    return false;
}

  TTrack*
  TBuilderCurl::buildStereo(TTrack&, const AList<TLink> &) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::buildStereo 1" << std::endl;
#endif
    return NULL;
  }

  TTrack*
  TBuilderCurl::buildStereo(TTrack& track,
                            const AList<TLink> & stereoList,
                            const AList<TLink> & allStereoList) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::buildStereo 2" << std::endl;
#endif
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "BuildCurlStereo";
    EnterStage(stage);
#endif

#if (DEBUG_CURL_DUMP+DEBUG_CURL_GNUPLOT+DEBUG_CURL_MC)
    Belle_event_Manager& evtMgr = Belle_event_Manager::get_manager();
    debugMcFlag = 1;
    if (evtMgr.count() != 0 &&
        evtMgr[0].ExpMC() != 2)debugMcFlag = 0;// not MC
#endif

    AList<TLink> list = stereoList;
    unsigned nList = list.length();

    //...gets stereo wires from track.links
    for (unsigned i = 0, size = track.links().length(); i < size; ++i) {
      unsigned superID = (track.links())[i]->wire()->superLayerId();
      if (superID == 1 || superID == 3 || superID == 5 ||
          superID == 7 || superID == 9) {
        int ok = 1;
        for (unsigned j = 0; j < nList; ++j) {
          TLink* l = list[j];
          if (l->hit()->wire().id() == (track.links())[i]->hit()->wire().id()) {
            ok = 0; break;
          }
        }
        if (ok == 1)list.append(((track.links())[i]));
      }
      // set LR 2
      (track.links())[i]->leftRight(2);
    }
    for (unsigned i = 0, size = list.length(); i < size; ++i) {
      track._links.remove(list[i]);
      // set LR 2
      list[i]->leftRight(2);
    }
    if (list.length() < 2 ||
        list.length() + track.nLinks() < 5) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "!!! Too few hits : #stereo=" << list.length()
                << ",#(axial)" << track.nLinks() << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }
#ifdef TRASAN_DEBUG_DETAIL
    unsigned debug_stereo_counter1 = 0;
    for (unsigned i = 0; i < unsigned(track.links().length()); ++i) {
      unsigned superID = (track.links())[i]->hit()->wire().superLayerId();
      if (superID == 1 || superID == 3 ||
          superID == 5 || superID == 7 ||
          superID == 9)++debug_stereo_counter1;
    }
    std::cout << Tab() << "Fitted Track:"
              << ", A# = " << track.links().length() - debug_stereo_counter1
              << ", S# = " << debug_stereo_counter1 << "(==0)"
              << ", A+S# = " << track.links().length()
              << ", Cand Stereo Wires # = " << list.length() << std::endl;
    double debugChg = -1.;
    if (track.charge() > 0.)debugChg = 1.;
    if (debugChg > 0.)std::cout << Tab() << "... Positive Track" << std::endl;
    else std::cout << Tab() << "... Negative Track" << std::endl;
#endif
#ifdef TRASAN_WINDOW
    szc.clear();
    szc.skip(false);
    szc.mode(2);
    szc.appendSz(track, list, leda_brown);
#endif

    //...calculates a circle.
    double xc2D;
    double yc2D;
    double r2D;
    AList<TLink> tmpAxialList = track.links();
    bool err2D = fitWDD(xc2D, yc2D, r2D, tmpAxialList);

    //...using SVD
    double dZSVD, tanLSVD;
    bool initWithSVD = buildStereo(track, dZSVD, tanLSVD);

    //...sets arc and z pairs of each stereo hit wire.
    setArcZ(track, list);

    AList<TLink> removeList;
    for (unsigned i = 0, size = list.length(); i < size; ++i) {
      if (list[i]->arcZ(0).x() == -999. && list[i]->arcZ(1).x() == -999.)removeList.append(list[i]);
    }
    list.remove(removeList);
    if (list.length() < 2 ||
        list.length() + track.nLinks() < 5) { // stereo >=2 && axial >= 3
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail:few wires which can be set Arc-Z # = "
                << list.length() << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... Cand Stereo Wires which can be set Arc-Z # = "
              << list.length() << std::endl;
#if DEBUG_CURL_DUMP
    plotArcZ(list, 0., 0., 0);
#endif
#endif

// #if 0
//   //...separates
//   AList<TLink> layer[18];
//   for(unsigned i = 0, size = list.length(); i < size; ++i){
//     layer[list[i]->wire().layer()->axialStereoLayerId()].append(*list[i]);
//   }

// #if DEBUG_CURL_DUMP
//   for(int i=0;i<18;++i){
//     if(layer[i].length())
//       std::cout << Tab() << "(TBuilderCurl)  Cand Stereo Wires which can be set Arc-Z # = "
//     << layer[i].length()
//     << " on " << i << " Layer." << std::endl;
//   }
// #endif
// #endif

    // makes a line.
    AList<TLink> goodL;
    AList<Point3D> goodP;
    double minChi2 = 9999.;
    double goodA   = 9999.;
    double goodB   = 9999.;
    makeLine(track, list, allStereoList, goodL,
             minChi2, goodA, goodB, goodP);
    HepAListDeleteAll(goodP);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "a = " << goodA << ", b = " << goodB
              << " (after makeLine-function)" << std::endl;
#endif

    // refits
    static TRobustLineFitter rfitter("Can you work well?");
    TLine newLine0(goodL);

    if (rfitter.fit(newLine0) != 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail:linefitting...fail." << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

#ifdef TRASAN_WINDOW
    szc.append(newLine0, leda_green);
    szc.wait();
#endif

#if DEBUG_CURL_DUMP
    std::cout << Tab() << "a = " << newLine0.a() << ", b = " << newLine0.b()
              << " (after robustline-fit)" << std::endl;
#endif

    //...appends at last chance
//  unsigned nGoodL = goodL.length();
    list.remove(goodL);
    AList<TLink> goodL2;
    if (m_param.CURL_VERSION == 0) { // default
      if (fabs(newLine0.b()) < 10.) {
        appendPoints(list, goodL2,
                     newLine0.a(), newLine0.b(), track, m_param.Z_DIFF_FOR_LAST_ATTEND);
      } else { // in case of bad result of robustLineFitter. (2001/04/04)
        appendPoints(list, goodL2,
                     goodA, goodB, track, m_param.Z_DIFF_FOR_LAST_ATTEND);
      }
    } else {
      // same with b20010409_2122 at least
      appendPoints(list, goodL2,
                   newLine0.a(), newLine0.b(), track, m_param.Z_DIFF_FOR_LAST_ATTEND);
    }
    goodL.append(goodL2);
    TLine newLine(goodL);

    //...refits
    if (rfitter.fit(newLine) != 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail:appending and re-fitting...fail."
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

#ifdef TRASAN_WINDOW
    szc.append(newLine, leda_blue);
    szc.wait();
#endif

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "a = " << newLine.a() << ", b = " << newLine.b()
              << " (after last-append + re-robustline-fit)" << std::endl;
#endif

    //...makes 3D tracks
    const AList<TLink> &good = newLine.links();
    track.TTrackBase::append(good);
    if (!check(track)) {
#if DEBUG_CURL_DUMP
      std::cout << Tab() << "... Fail:checking wire numbers...fail."
                << std::endl;
#endif
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...sets initial values
    CLHEP::HepVector a = track.helix().a();
    if (err2D) {
      double tmpA[3];
      double tmpQ = track.charge() > 0. ? 1. : -1.;
      tmpA[1] = fmod(atan2(tmpQ * yc2D,
                           tmpQ * xc2D)
                     + 4. * M_PI,
                     2. * M_PI);
      tmpA[2] = tmpQ * THelix::ConstantAlpha / r2D;
      tmpA[0] = xc2D / cos(tmpA[1]) - tmpQ * r2D;
      // std::cout << yc2D/sin(tmpA[1])-tmpQ*r2D << std::endl;
      // std::cout << a[0] << " -0- " << tmpA[0] << std::endl;
      // std::cout << a[1] << " -1- " << tmpA[1] << std::endl;
      // std::cout << a[2] << " -2- " << tmpA[2] << std::endl;
      a[0] = tmpA[0];
      a[1] = tmpA[1];
      a[2] = tmpA[2];
    }
    a[3] = newLine.b();
    a[4] = newLine.a();
    if (initWithSVD) { // use SVD initial values if possible.
      a[3] = dZSVD;
      a[4] = tanLSVD;
    }

    if (m_param.CURL_VERSION == 0) {
      if (fabs(a[3]) > 10. && fabs(goodB) < 10.) { // 50cm --> 10cm @ 2001/04/04
        // use initial values when results of RobustFit is bad.
        a[3] = goodB;
        a[4] = goodA;
      }
    } else {
      if (fabs(a[3]) > 50. && fabs(goodB) < 50.) {
        a[3] = goodB;
        a[4] = goodA;
        track.TTrackBase::remove(goodL2);
      }
    }

    track._helix->a(a);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... Created Line: y = " << newLine.a()
              << " * x + " << newLine.b()
              << ", size = " << good.length() << std::endl;
#if DEBUG_CURL_DUMP
    plotArcZ(const_cast< AList<TLink>& >(good), newLine.a(), newLine.b(), 0);
#endif
#endif

    /* std::cout << "1st : "
         << track.helix().a()[0] << " " << track.helix().a()[1] << " "
         << track.helix().a()[2] << " " << track.helix().a()[3] << " "
         << track.helix().a()[4] << std::endl; */

#if 1
    if (m_param.ON_CORRECTION) {
      _fitter.sag();
      _fitter.propagation();
      _fitter.tof();
    }
#endif

    //...fits
    AList<TLink> bad;
    int err = _fitter.fit(track);
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail fitting(0)...error code = " << err
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    } else if (fabs(track.helix().a()[3]) > fabs(a[3]) &&
               (fabs(track.helix().a()[3]) > 50.  || // 50cm
                fabs(track.helix().a()[2]) > 100. || // 0.01GeV
                fabs(track.helix().a()[2]) < 0.1)) { // 10GeV
      // in strange case, set "correction" of wires OFF
      // and then fit with the initial values.
      if (m_param.ON_CORRECTION) {
        _fitter.sag();
        _fitter.propagation();
        _fitter.tof();
        track._helix->a(a);
        // std::cout << "ON --> OFF" << std::endl;
      } else {
#ifdef TRASAN_DEBUG_DETAIL
        LeaveStage(stage);
#endif
        return NULL;
      }
    }

    /* std::cout << "2nd : "
         << track.helix().a()[0] << " " << track.helix().a()[1] << " "
         << track.helix().a()[2] << " " << track.helix().a()[3] << " "
         << track.helix().a()[4] << std::endl; */

    track.refine(bad, m_param.SELECTOR_MAX_SIGMA * 100.);
    if (!check(track)) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail checking(1)..." << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }
    err = _fitter.fit(track);
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail fitting(1)...error code = " << err
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    track.refine(bad, m_param.SELECTOR_MAX_SIGMA * 10.);
    if (!check(track)) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "...  Fail checking(2)..." << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }
    err = _fitter.fit(track);
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail fitting(2)...error code = " << err
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    track.refine(bad, m_param.SELECTOR_MAX_SIGMA);
    if (!check(track)) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail checking(3)..." << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    err = _fitter.fit(track);
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail fitting(3)...error code = " << err
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

// #if 0
//   for(int i=0;i<track.links().length();++i){
//     if(track.links()[i]->pull() > 36.){
//       std::cout << track.links()[i]->wire()->id()
//     << " :+: " << track.links()[i]->pull() << std::endl;
//     }
//     if(track.links()[i]->hit()->state() & CellHitInvalidForFit)
//       std::cout << "Not Valid For Fit!" << std::endl;
//     //if(!(track.links()[i]->hit()->state() & CellHitFittingValid))
//     // std::cout << "No-Valid For Fit!" << std::endl;
//   }
// #endif

    track.refine(bad, m_param.SELECTOR_MAX_SIGMA);
    if (!check(track)) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail checking(4)..." << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    err = _fitter.fit(track);
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Fail fitting(4)...error code = " << err
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

// #if 0
//   for(int i=0;i<track.links().length();++i){
//     if(track.links()[i]->pull() > 36.){
//       std::cout << track.links()[i]->wire()->id()
//     << " :*: " << track.links()[i]->pull() << std::endl;
//     }
//     if(track.links()[i]->hit()->state() & CellHitInvalidForFit)
//       std::cout << "Not Valid For Fit!" << std::endl;
//     //if(!(track.links()[i]->hit()->state() & CellHitFittingValid))
//     // std::cout << "No-Valid For Fit!" << std::endl;
//   }
// #endif

    /* std::cout << "3rd : "
         << track.helix().a()[0] << " " << track.helix().a()[1] << " "
         << track.helix().a()[2] << " " << track.helix().a()[3] << " "
         << track.helix().a()[4] << std::endl; */

    //...tests
    if (track.nLinks() < 5) { // axial + stereo >= 5
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Success fitting, but pre-selection...fail."
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }
    if (fabs(track.impact()) > m_param.SELECTOR_MAX_IMPACT ||
        track.pt() < 0.005 || // Pt >= 5MeV
        fabs(track.pz()) > m_param.SELECTOR_STRANGE_PZ) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "... Success fitting, but selection...fail."
                << std::endl;
      std::cout << Tab() << "    impact = " << track.impact() << std::endl;
      std::cout << Tab() << "    pt     = " << track.pt() << std::endl;
      std::cout << Tab() << "    pz     = " << track.pz() << std::endl;
      std::cout << Tab() << "    dz     = " << track.helix().a()[3]
                << std::endl;
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...replaces init helix if dz is bad.
    if (fabs(track.helix().a()[3]) > m_param.SELECTOR_REPLACE_DZ &&
        fabs(a[3]) < fabs(track.helix().a()[3])) {
      track._helix->a(a);
    }
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "... Success Build Stereo!!!" << std::endl;
    LeaveStage(stage);
#endif
    return & track;
  }

//.............................................
//.............Set Arc and Z Part..............
//.............................................

  void
  TBuilderCurl::setArcZ(TTrack& track, AList<TLink> &list) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setArcZ 0" << std::endl;
#endif
    if (track.nLinks() < 4) {
      track.stereoHitForCurl(list);
      return;
    }

    AList<TLink> alayer[6];
    AList<TLink> slayer[5];
    for (unsigned i = 0, size = track.nLinks(); i < size; ++i) {
      unsigned id = (track.links())[i]->wire()->superLayerId();
      //ho
//iw      if( id<0 || id>8 ) {
      if (id>8 ) {
	std::cout << "invalid id=" << id <<" in TBuilderCurl::setArcZ" << std::endl;	
	exit(-1);
      }
      if (id == 0)alayer[0].append((track.links())[i]);
      else if (id ==  2)alayer[1].append((track.links())[i]);
      else if (id ==  4)alayer[2].append((track.links())[i]);
      else if (id ==  6)alayer[3].append((track.links())[i]);
      else if (id ==  8)alayer[4].append((track.links())[i]);
      else if (id == 10)alayer[5].append((track.links())[i]);
    }

    for (unsigned i = 0, size = list.length(); i < size; ++i) {
      unsigned id = list[i]->wire()->superLayerId();
      if (id == 1)slayer[0].append(list[i]);
      else if (id == 3)slayer[1].append(list[i]);
      else if (id == 5)slayer[2].append(list[i]);
      else if (id == 7)slayer[3].append(list[i]);
      else if (id == 9)slayer[4].append(list[i]);
    }
// #if 0
//   std::cout << "Stereo = "
//        << slayer[0].length() << " "
//        << slayer[1].length() << " "
//        << slayer[2].length() << " "
//        << slayer[3].length() << " "
//        << slayer[4].length() << std::endl;
//   std::cout << "Axial  = "
//        << alayer[0].length() << " "
//        << alayer[1].length() << " "
//        << alayer[2].length() << " "
//        << alayer[3].length() << " "
//        << alayer[4].length() << " "
//        << alayer[5].length() << std::endl;
// #endif
    unsigned ip = 0;
    if (slayer[0].length() >= 1) {
      if (alayer[0].length() + alayer[1].length() >= 4) {
        setArcZ(track, slayer[0], alayer[0], alayer[1], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() >= 4) {
        setArcZ(track, slayer[0], alayer[0], alayer[1],
                alayer[2], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() >= 4) {
        setArcZ(track, slayer[0], alayer[0], alayer[1],
                alayer[2], alayer[3], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() >= 4) {
        setArcZ(track, slayer[0], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[0], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], alayer[5], ip);
      }
    }
    if (slayer[1].length() >= 1) {
      if (alayer[1].length() + alayer[2].length() >= 4) {
        setArcZ(track, slayer[1], alayer[1], alayer[2], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() >= 4) {
        setArcZ(track, slayer[1], alayer[0], alayer[1],
                alayer[2], alayer[3], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() >= 4) {
        setArcZ(track, slayer[1], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[1], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], alayer[5], ip);
      }
    }
    if (slayer[2].length() >= 1) {
      if (alayer[2].length() + alayer[3].length() >= 4) {
        setArcZ(track, slayer[2], alayer[2], alayer[3], ip);
      } else if (alayer[1].length() + alayer[2].length() +
                 alayer[3].length() + alayer[4].length() >= 4) {
        setArcZ(track, slayer[2], alayer[1], alayer[2],
                alayer[3], alayer[4], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[2], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], alayer[5], ip);
      }
    }
    if (slayer[3].length() >= 1) {
      if (alayer[3].length() + alayer[4].length() >= 4) {
        setArcZ(track, slayer[3], alayer[3], alayer[4], ip);
      } else if (alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[3], alayer[2], alayer[3],
                alayer[4], alayer[5], ip);
      } else if (alayer[1].length() + alayer[2].length() +
                 alayer[3].length() + alayer[4].length() +
                 alayer[5].length() >= 4) {
        setArcZ(track, slayer[3], alayer[1], alayer[2],
                alayer[3], alayer[4], alayer[5], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[3], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], alayer[5], ip);
      }
    }
    if (slayer[4].length() >= 1) {
      if (alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[4], alayer[4], alayer[5], ip);
      } else if (alayer[3].length() + alayer[4].length() +
                 alayer[5].length() >= 4) {
        setArcZ(track, slayer[4], alayer[3], alayer[4],
                alayer[5], ip);
      } else if (alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[4], alayer[2], alayer[3],
                alayer[4], alayer[5], ip);
      } else if (alayer[1].length() + alayer[2].length() +
                 alayer[3].length() + alayer[4].length() +
                 alayer[5].length() >= 4) {
        setArcZ(track, slayer[4], alayer[1], alayer[2],
                alayer[3], alayer[4], alayer[5], ip);
      } else if (alayer[0].length() + alayer[1].length() +
                 alayer[2].length() + alayer[3].length() +
                 alayer[4].length() + alayer[5].length() >= 4) {
        setArcZ(track, slayer[4], alayer[0], alayer[1],
                alayer[2], alayer[3], alayer[4], alayer[5], ip);
      }
    }
  }

//.............................................
//.............Append Points(last).............
//.............................................

  unsigned
  TBuilderCurl::appendPoints(AList<TLink> &list, AList<TLink> &line,
                             double a, double b, TTrack&, double z_cut) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::appendPoints" << std::endl;
#endif
    unsigned size = list.length();
    if (size == 0)return 0;
    unsigned counter(0);
    for (unsigned i = 0; i < size; ++i) {
      for (unsigned j = 0; j < 4; ++j) {
        if (j <= 1 && list[i]->arcZ(j).x() == -999.)continue;
        else if (j > 1 && list[i]->arcZ(j).x() == -999.)break;
        double y = a * list[i]->arcZ(j).x() + b;
        if (fabs(y - list[i]->arcZ(j).y()) < z_cut) {
          list[i]->position(list[i]->arcZ(j));
          line.append(list[i]);
          ++counter;
          break;
        }
      }
    }
    return counter;
  }

//.............................................
//..............Line Fit Part..................
//.............................................

  int
  doLineFit(AList<TLink> &points,
            double& m_a, double& m_b, double& chi2, double& nhits,
            int ipC = 0)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::doLineFit" << std::endl;
#endif
    m_a = m_b = nhits = 0.;
    chi2 = 1.e+10;
    unsigned n = points.length();
    double sum = double(n);
    double sumX = 0., sumY = 0., sumX2 = 0., sumXY = 0., sumY2 = 0.;
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * points[i];

      double x = l.position().x();
      double y = l.position().y();
      sumX  += x;
      sumY  += y;
      sumX2 += x * x;
      sumXY += x * y;
      sumY2 += y * y;
    }
    if (ipC != 0)sum += 1.0; // IP Constraint
    nhits = sum;

    double m_det = sum * sumX2 - sumX * sumX;
    if (m_det == 0. && sum != 2.) {
      return -1;
    } else if (m_det == 0. && sum == 2.) {
      double x0 = points[0]->position().x();
      double y0 = points[0]->position().y();
      double x1 = points[1]->position().x();
      double y1 = points[1]->position().y();
      if (x0 == x1)return -1;
      m_a = (y0 - y1) / (x0 - x1);
      m_b = -m_a * x1 + y1;
      chi2 = 0.;
      return 0;
    }
    chi2 = 0.;
    m_a = (sumXY * sum - sumX * sumY) / m_det;
    m_b = (sumX2 * sumY - sumX * sumXY) / m_det;

    for (unsigned i = 0; i < n; i++) {
      TLink& l = * points[i];

      double x = l.position().x();
      double y = l.position().y();
      double d =  y - (x * m_a + m_b);
      chi2 += d * d;
    }

    return 0;
  }

  void
  TBuilderCurl::fitLine(AList<TLink> &tmpLine, double& min_chi2,
                        double& good_a, double& good_b,
                        AList<TLink> &goodLine, AList<Point3D> &goodPosition,
                        int& overCounter) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::fitLine" << std::endl;
#endif
// #if 0
//   // OLD
//   unsigned size = tmpLine.length();
//   TLine line(tmpLine);
//   if(!(line.fit())){
//     double chi2 = line.chi2()/(static_cast<double>(size-2));
//     if(chi2 < min_chi2 && fabs(line.b()) < m_param.Z_CUT){
//       min_chi2 = chi2;
//       good_a   = line.a();
//       good_b   = line.b();
//       goodLine = tmpLine;
//       HepAListDeleteAll(goodPosition);
//       for(unsigned i=0;i<size;++i)
//  goodPosition.append(new Point3D(const_cast<Point3D&>(tmpLine[i]->position())));
//     }
//   }
//   ++overCounter;
// #else
    unsigned size = tmpLine.length();
    double ta, tb, tc, tn;
    if (!(doLineFit(tmpLine, ta, tb, tc, tn, 1)) && tn > 2.) { // with IP
      double chi2 = tc / (tn - 2.);
      if (chi2 < min_chi2 && fabs(tb) < m_param.Z_CUT) {
        min_chi2 = chi2;
        good_a   = ta;
        good_b   = tb;
        goodLine = tmpLine;
        HepAListDeleteAll(goodPosition);
        for (unsigned i = 0; i < size; ++i)
          goodPosition.append(new Point3D(const_cast<Point3D&>(tmpLine[i]->position())));
      }
    }
    ++overCounter;
// #endif
  }

//.............................................
//.................Check Part..................
//.............................................

  unsigned
  TBuilderCurl::check(const TTrack& track) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::check" << std::endl;
#endif
    unsigned nAhits(0), nShits(0);
    for (unsigned i = 0, size = track.nLinks(); i < size; ++i) {
      if (!(track.links()[i]->hit()->state() & CellHitFittingValid))continue;
      if (track.links()[i]->wire()->stereo())++nShits;
      else ++nAhits;
    }
    if (nAhits >= 3 && nShits >= 2)return 1; // hard coding
    return 0;
  }

//.............................................
//.............Checker(Debuger)................
//.............................................
// #if 0

// int
// checkBorder(AList<TLink> &layer0,
//      AList<TLink> &layer1,
//      AList<TLink> &layer2){
#if defined(HO_DEBUG)
  //  std::cout << " TBuilderCurl::checkBorder 0" << std::endl;
#endif
//   const TRGCDC &cdc = *TRGCDC::getTRGCDC();

//   AList<TLink> list = layer0;
//   list.append(layer1);
//   list.append(layer2);
//   int size = list.length();
//   if(size <= 1)return 0;
//   int layerId = list[0]->hit()->wire()->layerId();
//   int maxLocalId = 79;

//   if(ms_superb) {
//     const TRGCDCLayer &l=*cdc.layer(layerId);
//     maxLocalId = l.nCells() - 1;
//   } else {
//     if(layerId >= 15)maxLocalId = 127;
//     if(layerId >= 23)maxLocalId = 159;
//     if(layerId >= 32)maxLocalId = 207;
//     if(layerId >= 41)maxLocalId = 255;
//   }

//   int HId = (int)(0.8*(maxLocalId+1));
//   int LId = (int)(0.2*(maxLocalId+1));
//   int low  = 0;
//   int high = 0;
//   for(int i=0;i>size;++i){
//     if(list[i]->hit()->wire().localId() < (unsigned) LId)low = 1;
//     else if(list[i]->hit()->wire().localId() > (unsigned) HId)high = 1;
//     if(low == 1 && high == 1)return 1;
//   }
//   return 0;
// }

// int
// checkBorder(AList<TLink> &layer0,
//      AList<TLink> &layer1,
//      AList<TLink> &layer2,
//      AList<TLink> &layer3){
#if defined(HO_DEBUG)
  //  std::cout << " TBuilderCurl::checkBorder 1" << std::endl;
#endif
//   const TRGCDC &cdc = *TRGCDC::getTRGCDC();

//   AList<TLink> list = layer0;
//   list.append(layer1);
//   list.append(layer2);
//   list.append(layer3);
//   int size = list.length();
//   if(size <= 1)return 0;
//   int layerId = list[0]->hit()->wire().layerId();
//   int maxLocalId = 79;
//   if(ms_superb) {
//     const TRGCDCLayer &l=*cdc.layer(layerId);
//     maxLocalId = l.nCells() - 1;
//   } else {
//     if(layerId >= 15)maxLocalId = 127;
//     if(layerId >= 23)maxLocalId = 159;
//     if(layerId >= 32)maxLocalId = 207;
//     if(layerId >= 41)maxLocalId = 255;
//   }
//   int HId = (int)(0.8*(maxLocalId+1));
//   int LId = (int)(0.2*(maxLocalId+1));
//   int low  = 0;
//   int high = 0;
//   for(int i=0;i>size;++i){
//     if(list[i]->hit()->wire().localId() < (unsigned) LId)low = 1;
//     else if(list[i]->hit()->wire().localId() > (unsigned) HId)high = 1;
//     if(low == 1 && high == 1)return 1;
//   }
//   return 0;
// }
// #endif

  int
  TBuilderCurl::offsetBorder(TLink* l)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::offsetBorder" << std::endl;
#endif
    const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();

    int layerId = l->hit()->wire().layerId();
    if (ms_superb) {
      const Belle2::TRGCDCLayer& l = *cdc.layer(layerId);
      return l.nCells();
    } else {
      int maxLocalId = 79;
      if (layerId >= 15)maxLocalId = 127;
      if (layerId >= 23)maxLocalId = 159;
      if (layerId >= 32)maxLocalId = 207;
      if (layerId >= 41)maxLocalId = 255;
      return maxLocalId + 1;
    }
  }


  void
  TBuilderCurl::makeList(AList<TLink> &layer, AList<TLink> &list, double q, int border, int checkB, TLink* layer0)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::makeList" << std::endl;
#endif
    int n = layer.length();
    if (checkB == 0) {
      for (int i = 0; i < n; ++i) {
        if (q < 0.) {
          if ((int) layer[i]->hit()->wire().localId() >= border)
            list.append(layer[i]);
        } else {
          if ((int)layer[i]->hit()->wire().localId() <= border)
            list.append(layer[i]);
        }
      }
    } else {
      //difficult!! --> puls offset
      int offset = offsetBorder(layer0);
      if (border * 2 < offset)border += offset;
      for (int i = 0; i < n; ++i) {
        int lId = layer[i]->hit()->wire().localId();
        if (lId * 2 < offset)lId += offset;
        if (q < 0.) {
          if (lId >= border)list.append(layer[i]);
        } else {
          if (lId <= border)list.append(layer[i]);
        }
      }
    }
  }

  int
  TBuilderCurl::sortByLocalId(AList<TLink> &list) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::sortByLocalId" << std::endl;
#endif
  //ho    const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();

    int size = list.length();
    if (size <= 1)return 0;
    int layerId = list[0]->hit()->wire().layerId();
    int maxLocalId;
    if (ms_superb) {
      const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();
      const Belle2::TRGCDCLayer& l = *cdc.layer(layerId);
      maxLocalId = l.nCells() - 1;
    } else {
      if (layerId < 15)maxLocalId = 79;
      else if (layerId >= 15)maxLocalId = 127;
      else if (layerId >= 23)maxLocalId = 159;
      else if (layerId >= 32)maxLocalId = 207;
      else if (layerId >= 41)maxLocalId = 255;
    }
    int flag = 0;
    for (int i = 0; i < size; ++i) {
      if (list[i]->hit()->wire().localId() == 0 ||
          list[i]->hit()->wire().localId() == 1 ||
          list[i]->hit()->wire().localId() == (unsigned) maxLocalId - 1 ||
          list[i]->hit()->wire().localId() == (unsigned) maxLocalId) {
        flag = 1;
        break;
      }
    }
    if (flag == 0)return 0;
    int maxDif = (int)(0.5 * (maxLocalId + 1));
    AList<TLink> fList; //former
    AList<TLink> lList; //later
    for (int i = 0; i < size; ++i) {
      if ((int) list[i]->hit()->wire().localId() < maxDif)
        lList.append(list[i]);
      else
        fList.append(list[i]);
    }
    list.removeAll();
    list.append(fList);
    list.append(lList);
    if (fList.length() >= 1 &&
        lList.length() >= 1)return 1;
    else return 0;
  }

//..................................................
//..................................................
//....................MC............................
//..................................................
//..................................................

  TTrack*
  TBuilderCurl::buildStereoMC(TTrack&, const AList<TLink> &) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::buildStereoMC" << std::endl;
#endif
#if DEBUG_CURL_MC
    AList<TLink> list = stereoList;

    HepGeom::Point3D<double> center = track.helix().center();
    double r  = fabs(track.helix().curv());
    for (unsigned i = 0, size = list.length();
         i < size; ++i) {
      HepGeom::Point3D<double> point((list[i]->hit()->mc()->datcdc()->m_xin +
                                      list[i]->hit()->mc()->datcdc()->m_xout) * 0.5,
                                     (list[i]->hit()->mc()->datcdc()->m_yin +
                                      list[i]->hit()->mc()->datcdc()->m_yout) * 0.5,
                                     0.);
      double cosdPhi = - center.dot((point - center).unit()) / center.mag();
      double dPhi;
      if (fabs(cosdPhi) < 1.0) {
        dPhi = acos(cosdPhi);
      } else if (cosdPhi >= 1.0) {
        dPhi = 0.;
      } else {
        dPhi = M_PI;
      }
      list[i]->position(Point3D(r * dPhi,
                                (list[i]->hit()->mc()->datcdc()->m_zin +
                                 list[i]->hit()->mc()->datcdc()->m_zout) * 0.5,
                                0.));
      /* std::cout << list[i]->wire()->id() << ": "
      << point.x() << " "
      << point.y() << " "
      << (list[i]->hit()->mc()->datcdc()->m_zin+
      list[i]->hit()->mc()->datcdc()->m_zout)*0.5 << std::endl; */
    }
    // std::cout << "A# = " << track.links().length() << ", S# = " << list.length() << std::endl;
    double xc2D;
    double yc2D;
    double r2D;
    bool err2D = fitWDD(xc2D, yc2D, r2D, track.links()); // axial only

    TLine0 newLine(list);
    if (newLine.fit() != 0) return NULL;
    const AList<TLink> &good = newLine.links();
    track.append(good);
    CLHEP::HepVector a = track.helix().a();
    a[3] = newLine.b();
    a[4] = newLine.a();
    if (err2D) {
      double tmpA[3];
      double tmpQ = track.charge() > 0. ? 1. : -1.;
      tmpA[1] = fmod(atan2(tmpQ * yc2D,
                           tmpQ * xc2D)
                     + 4. * M_PI,
                     2. * M_PI);
      tmpA[2] = tmpQ * THelix::ConstantAlpha / r2D;
      tmpA[0] = xc2D / cos(tmpA[1]) - tmpQ * r2D;
      a[0] = tmpA[0];
      a[1] = tmpA[1];
      a[2] = tmpA[2];
    }
    track._helix->a(a);
// #if 0
//   std::cout << track.helix().a()[0] << " " << track.helix().a()[1] << " "
//        << track.helix().a()[2] << " " << track.helix().a()[3] << " "
//        << track.helix().a()[4] << std::endl;
// #endif
    //...fits
    AList<TLink> bad;
    int err = _fitter.fit(track);
    if (err < m_param.ERROR_FOR_HELIX_FIT)return NULL;
    track.refine(bad, m_param.SELECTOR_MAX_SIGMA * 100.);
    err = _fitter.fit(track);
    if (err < m_param.ERROR_FOR_HELIX_FIT)return NULL;
    track.refine(bad, m_param.SELECTOR_MAX_SIGMA * 10.);
    err = _fitter.fit(track);
    if (err < m_param.ERROR_FOR_HELIX_FIT)return NULL;
    track.refine(bad, m_param.SELECTOR_MAX_SIGMA);
    err = _fitter.fit(track);
    if (err < m_param.ERROR_FOR_HELIX_FIT)return NULL;
// #if 0
//   std::cout << track.helix().a()[0] << " " << track.helix().a()[1] << " "
//        << track.helix().a()[2] << " " << track.helix().a()[3] << " "
//        << track.helix().a()[4] << std::endl;
// #endif
    //track._helix->a(a); // re-write

    //...checks
    if (track.nLinks() < m_param.SELECTOR_NLINKS)return NULL;
    if (fabs(track.impact()) > m_param.SELECTOR_MAX_IMPACT ||
        track.pt() < m_param.SELECTOR_MIN_PT)return NULL;
    return & track;
#else
    return NULL;
#endif
  }

//..................................................
//....................Plot..........................
//..................................................

#if DEBUG_CURL_GNUPLOT
  void
  TBuilderCurl::plotArcZ(AList<TLink> &tmpLine,
                         double a, double b, const int flag) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::plotArcZ" << std::endl;
#endif
    //#if 1
    if (a == 9999. || b == 9999.) {
      a = 0.;
      b = 0.;
    }
    int nCounter = 0;
    double gmaxX = 0. , gminX = 0.;
    double gmaxY = 0. , gminY = 0.;
    FILE* gnuplot, *data;
    if ((data = fopen("you_can_remove_this.dat", "w")) != NULL) {
      for (int ii = 0; ii < tmpLine.length(); ii++) {
        ++nCounter;
        std::fprintf(data, "%lf, %lf\n",
                     tmpLine[ii]->position().x(),
                     tmpLine[ii]->position().y());
        if (flag)std::cout << " Wire ID = " << tmpLine[ii]->hit()->wire().id()
                             << " Arc = " << tmpLine[ii]->position().x()
                             << " Z = " << tmpLine[ii]->position().y();
        //if(flag && !debugMcFlag)std::cout << std::endl;
        //if(flag && debugMcFlag){
        // std::cout << " Z(true) = " << (tmpLine[ii]->hit()->mc()->datcdc()->m_zin+
        //        tmpLine[ii]->hit()->mc()->datcdc()->m_zout)*0.5;
        // std::cout << " HepTrackID = " << tmpLine[ii]->hit()->mc()->hep()->id() << std::endl;
        //}
        if (gmaxX < tmpLine[ii]->position().x())
          gmaxX = tmpLine[ii]->position().x();
        if (gminX > tmpLine[ii]->position().x())
          gminX = tmpLine[ii]->position().x();
        if (gmaxY < tmpLine[ii]->position().y())
          gmaxY = tmpLine[ii]->position().y();
        if (gminY > tmpLine[ii]->position().y())
          gminY = tmpLine[ii]->position().y();
      }
      fclose(data);
    }
    if ((data = fopen("you_can_remove_this.dat2", "w")) != NULL) {
// #if 0
//     if(debugMcFlag){
//       for(int ii=0;ii<tmpLine.length();ii++){
//  double z = (tmpLine[ii]->hit()->mc()->datcdc()->m_zin+
//        tmpLine[ii]->hit()->mc()->datcdc()->m_zout)*0.5;
//  if(tmpLine[ii]->arcZ(0).x() != -999.){
//    ++nCounter;
//   std::fprintf(data,"%lf, %lf\n",tmpLine[ii]->arcZ(0).x(),z);
//    if(gmaxX < tmpLine[ii]->arcZ(0).x())
//      gmaxX = tmpLine[ii]->arcZ(0).x();
//    if(gminX > tmpLine[ii]->arcZ(0).x())
//      gminX = tmpLine[ii]->arcZ(0).x();
//    if(gmaxY < z)
//      gmaxY = z;
//    if(gminY > z)
//      gminY = z;
//  }
//       }
//     }
// #endif
      fclose(data);
    }
    if ((data = fopen("you_can_remove_this.dat3", "w")) != NULL) {
      for (int ii = 0; ii < tmpLine.length(); ii++) {
        for (int jj = 0; jj < 4; ++jj) {
          if (tmpLine[ii]->arcZ(jj).x() != -999.) {
            ++nCounter;
            std::fprintf(data, "%lf, %lf\n",
                         tmpLine[ii]->arcZ(jj).x(),
                         tmpLine[ii]->arcZ(jj).y());
            if (gmaxX < tmpLine[ii]->arcZ(jj).x())
              gmaxX = tmpLine[ii]->arcZ(jj).x();
            if (gminX > tmpLine[ii]->arcZ(jj).x())
              gminX = tmpLine[ii]->arcZ(jj).x();
            if (gmaxY < tmpLine[ii]->arcZ(jj).y())
              gmaxY = tmpLine[ii]->arcZ(jj).y();
            if (gminY > tmpLine[ii]->arcZ(jj).y())
              gminY = tmpLine[ii]->arcZ(jj).y();
          } else {
            break;
          }
        }
      }
      fclose(data);
    }
    if (gmaxX < 0.)gmaxX = 0.; if (gminX > 0.)gminX = 0.;
    if (gmaxY < 0.)gmaxY = 0.; if (gminY > 0.)gminY = 0.;
    if (nCounter && (gnuplot = popen("gnuplot", "w")) != NULL) {
      std::fprintf(gnuplot, "set nokey \n");
      std::fprintf(gnuplot, "set size 0.721,1.0 \n");
      std::fprintf(gnuplot, "set xrange [%f:%f] \n", gminX, gmaxX);
      std::fprintf(gnuplot, "set yrange [%f:%f] \n", gminY, gmaxY);
      if (a == 0. && b == 0.) {
        std::fprintf(gnuplot, "plot \"you_can_remove_this.dat3\", \"you_can_remove_this.dat\", \"you_can_remove_this.dat2\" \n");
      } else {
        std::fprintf(gnuplot, "plot \"you_can_remove_this.dat3\", \"you_can_remove_this.dat\", \"you_can_remove_this.dat2\", %lf*x+%lf \n", a, b);
      }
      fflush(gnuplot);
      char tmp[8];
      gets(tmp);
      pclose(gnuplot);
    }
    return;
  }
#endif

//
//
//

  unsigned
  TBuilderCurl::findMaxLocalId(unsigned superLayerId)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::findMaxLocalId" << std::endl;
#endif
    const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();
//  const AList<Belle2::TRGCDCLayer> &sl=*cdc.superLayer(superLayerId);
    const std::vector<Belle2::TRGCDCLayer*> & sl = * cdc.superLayer(superLayerId);
    unsigned maxLocalId = 79;
    if (ms_superb) {
      maxLocalId = sl[0]->nCells() - 1;
      return maxLocalId;
    } else {
      if (superLayerId == 3)maxLocalId = 127;
      else if (superLayerId == 5)maxLocalId = 159;
      else if (superLayerId == 7)maxLocalId = 207;
      else if (superLayerId == 9)maxLocalId = 255;
      return maxLocalId;
    }
  }

  unsigned
  TBuilderCurl::isIsolation(unsigned localId,
                            unsigned maxLocalId,
                            unsigned layerId,
                            int lr,
                            const AList<TLink> &allStereoList)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::isIsolation" << std::endl;
#endif
    unsigned findId;
    if (lr == 1) { // R : ox, Dose a wire exist at "x"?
      findId = maxLocalId;
      if (localId != 0)findId = localId - 1;
    } else if (lr == -1) { // L : xo, Dose a wire exist at "x"?
      findId = 0;
      if (localId != maxLocalId)findId = localId + 1;
    } else {
      return 1;
    }
    unsigned isolate = 1;
    for (int i = 0; i < allStereoList.length(); ++i) {
      if (allStereoList[i]->wire()->layerId() == layerId &&
          allStereoList[i]->wire()->localId() == findId) {
        isolate = 0;
        break;
      }
    }
    return isolate;
  }

  void
  TBuilderCurl::findTwoHits(AList<TLink> &twoOnLayer,
                            const AList<TLink> &hitsOnLayer,
                            const AList<TLink> &allStereoList)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::findTwoHits" << std::endl;
#endif
  //...finds "two" seq. and isolated hits.
  //...and then sets LR for selected hits.
  if(hitsOnLayer.length() == 0 ||
     hitsOnLayer.length() > 3)return;
  twoOnLayer.removeAll();
  if(hitsOnLayer.length() == 1){
    if(hitsOnLayer[0]->wire()->superLayerId() == 1)return;
    unsigned maxLocalId = findMaxLocalId(hitsOnLayer[0]->wire()->superLayerId());
    unsigned R = isIsolation(hitsOnLayer[0]->wire()->localId(),
			     maxLocalId,
			     hitsOnLayer[0]->wire()->layerId(),1,allStereoList);
    unsigned L = isIsolation(hitsOnLayer[0]->wire()->localId(),
			     maxLocalId,
			     hitsOnLayer[0]->wire()->layerId(),-1,allStereoList);
    if(R == 1 && L == 0){
      unsigned nextLocalId = hitsOnLayer[0]->wire()->localIdForPlus()+1;
      L = isIsolation(nextLocalId,
		      maxLocalId,
		      hitsOnLayer[0]->wire()->layerId(),-1,allStereoList);
      if(L == 1){ // xuox
	hitsOnLayer[0]->leftRight(1); // R
	hitsOnLayer[0]->position(hitsOnLayer[0]->arcZ(1));
	twoOnLayer.append(hitsOnLayer[0]);
      }
    }else if(R == 0 && L == 1){
      unsigned nextLocalId = hitsOnLayer[0]->wire()->localIdForMinus()+1;
      R = isIsolation(nextLocalId,
		      maxLocalId,
		      hitsOnLayer[0]->wire()->layerId(),1,allStereoList);
      if(R == 1){ // xoux
	hitsOnLayer[0]->leftRight(0); // L
	hitsOnLayer[0]->position(hitsOnLayer[0]->arcZ(0));
	twoOnLayer.append(hitsOnLayer[0]);
      }
    }
  }
  if(hitsOnLayer.length() == 2){
    if(hitsOnLayer[0]->wire()->localIdForPlus()+1 ==
       (int) hitsOnLayer[1]->wire()->localId()){
      unsigned maxLocalId = findMaxLocalId(hitsOnLayer[0]->wire()->superLayerId());
      unsigned R = isIsolation(hitsOnLayer[0]->wire()->localId(),
			       maxLocalId,
			       hitsOnLayer[0]->wire()->layerId(),1,allStereoList);
      unsigned L = isIsolation(hitsOnLayer[1]->wire()->localId(),
			       maxLocalId,
			       hitsOnLayer[1]->wire()->layerId(),-1,allStereoList);
      if(R == 1 && L == 1){ // xoox
	hitsOnLayer[0]->leftRight(1); // R
	hitsOnLayer[0]->position(hitsOnLayer[0]->arcZ(1));
	hitsOnLayer[1]->leftRight(0); // L
	hitsOnLayer[1]->position(hitsOnLayer[1]->arcZ(0));
	twoOnLayer.append(hitsOnLayer[0]);
	twoOnLayer.append(hitsOnLayer[1]);
      }
    }
  }
  if(hitsOnLayer.length() == 3){
    if(hitsOnLayer[0]->wire()->localIdForPlus()+1 ==
       (int) hitsOnLayer[1]->wire()->localId() &&
       hitsOnLayer[1]->wire()->localIdForPlus()+1 !=
       (int) hitsOnLayer[2]->wire()->localId()){
      unsigned maxLocalId = findMaxLocalId(hitsOnLayer[0]->wire()->superLayerId());
      unsigned R = isIsolation(hitsOnLayer[0]->wire()->localId(),
			       maxLocalId,
			       hitsOnLayer[0]->wire()->layerId(),1,allStereoList);
      unsigned L = isIsolation(hitsOnLayer[1]->wire()->localId(),
			       maxLocalId,
			       hitsOnLayer[1]->wire()->layerId(),-1,allStereoList);
      if(R == 1 && L == 1){ // oxoox
	hitsOnLayer[0]->leftRight(1); // R
	hitsOnLayer[0]->position(hitsOnLayer[0]->arcZ(1));
	hitsOnLayer[1]->leftRight(0); // L
	hitsOnLayer[1]->position(hitsOnLayer[1]->arcZ(0));
	twoOnLayer.append(hitsOnLayer[0]);
	twoOnLayer.append(hitsOnLayer[1]);
      }
    }else if(hitsOnLayer[0]->wire()->localIdForPlus()+1 !=
	     (int) hitsOnLayer[1]->wire()->localId() &&
	     hitsOnLayer[1]->wire()->localIdForPlus()+1 ==
	     (int) hitsOnLayer[2]->wire()->localId()){
      unsigned maxLocalId = findMaxLocalId(hitsOnLayer[1]->wire()->superLayerId());
      unsigned R = isIsolation(hitsOnLayer[1]->wire()->localId(),
			       maxLocalId,
			       hitsOnLayer[1]->wire()->layerId(),1,allStereoList);
      unsigned L = isIsolation(hitsOnLayer[2]->wire()->localId(),
			       maxLocalId,
			       hitsOnLayer[2]->wire()->layerId(),-1,allStereoList);
      if(R == 1 && L == 1){ // xooxo
	hitsOnLayer[1]->leftRight(1); // R
	hitsOnLayer[1]->position(hitsOnLayer[1]->arcZ(1));
	hitsOnLayer[2]->leftRight(0); // L
	hitsOnLayer[2]->position(hitsOnLayer[2]->arcZ(0));
	twoOnLayer.append(hitsOnLayer[1]);
	twoOnLayer.append(hitsOnLayer[2]);
      }
    }
  }
  /* if(twoOnLayer.length() != 0){
     std::cout << "TWO " << twoOnLayer.length() << std::endl;
     } */
  }

  void
  TBuilderCurl::setLR(AList<TLink> &hitsOnLayer, unsigned LR)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setLR" << std::endl;
#endif
    // LR = 0 : L
    //    = 1 : R
    for (unsigned i = 0; i < (unsigned) hitsOnLayer.length(); ++i) {
      if (LR == 0) {
        hitsOnLayer[i]->leftRight(0); // L
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(0));
      } else {
        hitsOnLayer[i]->leftRight(1); // R
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(1));
      }
    }
  }

  bool
  TBuilderCurl::moveLR(AList<TLink> &hitsOnLayer)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::moveLR 0" << std::endl;
#endif
    unsigned nHits = hitsOnLayer.length();
    if (nHits == 0)return false;
    // ex) LLLL --> LLLR --> LLRR --> LRRR --> RRRR
    if (hitsOnLayer[nHits - 1]->leftRight() == 1)return false; // All R
    for (unsigned i = 0; i < nHits; ++i) {
      if (hitsOnLayer[i]->leftRight() == 0) { // L
        hitsOnLayer[i]->leftRight(1); // R
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(1));
        return true;
      }
    }
    return false;
  }

  void
  TBuilderCurl::selectGoodWires(const AList<TLink> &allWires,
                                AList<TLink> &goodWires)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::selectGoodWires" << std::endl;
#endif
    goodWires.removeAll();
    for (int i = 0; i < allWires.length(); ++i) {
      if (allWires[i]->position().x() != -999.) {
        goodWires.append(allWires[i]);
      }
    }
  }

  void
  TBuilderCurl::fitLine2(const AList<TLink> &tmpLine, double& min_chi2,
                         double& good_a, double& good_b,
                         AList<TLink> &goodLine, AList<Point3D> &goodPosition,
                         int& overCounter) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::fitLine2" << std::endl;
#endif
    AList<TLink> goodWires;
    selectGoodWires(tmpLine, goodWires);
    if (goodWires.length() >= 3)
      fitLine(goodWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
  }

  void
  TBuilderCurl::calVirtualCircle(const TLink& hit, const TTrack& track, const int LR,
                                 HepGeom::Point3D<double>  &center, double& radius)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::calVirtualCircle" << std::endl;
#endif
    if (abs(LR) != 1)return;
    double Q = track.charge();
    int isOuter = 1;
    if (Q > 0. && LR == 1)isOuter = -1; // Inner
    else if (Q < 0. && LR == -1)isOuter = -1; // Inner
    radius = fabs(track.radius());
    center = track.helix().center();
    HepGeom::Point3D<double>  wire = hit.wire()->xyPosition();
    center.setZ(0.);
    wire.setZ(0.);
    // new center(virtual)
    center = wire +
             (radius + ((double)isOuter) * (hit.hit()->drift())) * (center - wire).unit();
  }

  void
  TBuilderCurl::moveLR(AList<TLink> &hits,
                       const AList<TLink> &hitsOnLayerOrg,
                       const TTrack& track)
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::moveLR 1" << std::endl;
#endif
    AList<TLink> hitsOnLayer = hitsOnLayerOrg;
    hitsOnLayer.remove(hits);
    if (hitsOnLayer.length() == 0)return;

    unsigned nHits = hits.length();
    if (nHits == 0)return;

    //...finds "ref" from hits.
    // ex) LLLL|, LLL|R, LL|RR, L|RRR, |RRRR
    int LR = -1; // L
    TLink ref;
    if (hits[nHits - 1]->leftRight() == 1) { // All R
      LR = 1; // R
      ref = *hits[nHits - 1];
    }
    for (unsigned i = 0; i < nHits; ++i) {
      if (hits[i]->leftRight() == 0) { // L
        ref = *hits[i];
        break;
      }
    }

    HepGeom::Point3D<double>  center;
    double radius;
    calVirtualCircle(ref, track, LR, center, radius);

    double Q = track.charge();
    for (int i = 0; i < hitsOnLayer.length(); ++i) {
      int isOuter = 1;
      //ho      if ((hitsOnLayer[i]->wire()->xyPosition() - center).mag() - radius < 0.)isOuter = -1;
      if ((hitsOnLayer[i]->wire()->xyPosition() - center).perp() - radius < 0.)isOuter = -1;
      if (Q > 0. && isOuter == 1) {
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(0)); // L
        hitsOnLayer[i]->leftRight(0); // L
      } else if (Q > 0. && isOuter == -1) {
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(1)); // R
        hitsOnLayer[i]->leftRight(1); // R
      } else if (Q < 0. && isOuter == 1) {
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(1)); // R
        hitsOnLayer[i]->leftRight(1); // R
      } else if (Q < 0. && isOuter == -1) {
        hitsOnLayer[i]->position(hitsOnLayer[i]->arcZ(0)); // L
        hitsOnLayer[i]->leftRight(0); // L
      }
    }
  }

  void
  TBuilderCurl::makeLine(TTrack& track,
                         AList<TLink> &list, const AList<TLink> &allStereoList, AList<TLink> &goodLine,
                         double& min_chi2, double& good_a, double& good_b,
                         AList<Point3D> &goodPosition) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::makeLine" << std::endl;
#endif
    if (list.length() == 0)return;
    const Belle2::TRGCDC& cdc = *Belle2::TRGCDC::getTRGCDC();
    const unsigned nstereolayers = cdc.nStereoLayers();
    const unsigned nstereosuperlayers = cdc.nStereoSuperLayers();

    AList<TLink> *layer = new AList<TLink> [nstereolayers];
    AList<TLink> *layerOrg = new AList<TLink> [nstereolayers];
    for (unsigned i = 0, size = list.length(); i < size; ++i) {
      layer[list[i]->wire()->layer().axialStereoLayerId()].append(*list[i]);
      layerOrg[list[i]->wire()->layer().axialStereoLayerId()].append(*list[i]);
    }

    AList<TLink> *fixedWires = new AList<TLink> [nstereosuperlayers]; // each superlayer
    AList<TLink> *nonFixedWires = new AList<TLink> [nstereosuperlayers]; // each superlayer
    AList<TLink> allFixedWires;
    for (unsigned i = 0; i < nstereolayers; ++i) {
      if (layer[i].length()) {
        layer[i].sort(TLink::sortByWireId);
        sortByLocalId(layer[i]); // chiisai-jun but kyoukai fukin ha sukoshi kufuu shite iru.
        AList<TLink> tmp;
        findTwoHits(tmp, layer[i], allStereoList);
        if (tmp.length()) {
          layer[i].removeAll();
          allFixedWires.append(tmp);
          if (ms_superb) {
//    fixedWires[cdc.StereoSuperLayer(i)].append(tmp);
            fixedWires[cdc.axialStereoSuperLayerId(1, i)].append(tmp);
          } else {
            if (i < 3)fixedWires[0].append(tmp);
            else if (i < 6)fixedWires[1].append(tmp);
            else if (i < 10)fixedWires[2].append(tmp);
            else if (i < 14)fixedWires[3].append(tmp);
            else fixedWires[4].append(tmp);
          }
        } else {
          if (ms_superb) {
//    nonFixedWires[cdc.StereoSuperLayer(i)].append(layer[i]);
            nonFixedWires[cdc.axialStereoSuperLayerId(1, i)].append(layer[i]);
          } else {
            if (i < 3)nonFixedWires[0].append(layer[i]);
            else if (i < 6)nonFixedWires[1].append(layer[i]);
            else if (i < 10)nonFixedWires[2].append(layer[i]);
            else if (i < 14)nonFixedWires[3].append(layer[i]);
            else nonFixedWires[4].append(layer[i]);
          }
        }
      }
    }

#if DEBUG_CURL_DUMP
    std::cout << "(TBuilderCurl)    1st fixed & non-fixed wires selection:" << std::endl;
    std::cout << "(TBuilderCurl)    all fixed wires # = " << allFixedWires.length() << std::endl;
    std::cout << "(TBuilderCurl)    fixed wires # = ("
              << fixedWires[0].length() << ", "
              << fixedWires[1].length() << ", "
              << fixedWires[2].length() << ", "
              << fixedWires[3].length() << ", "
              << fixedWires[4].length() << ")" << std::endl;
    std::cout << "(TBuilderCurl)    non fixed wires # = ("
              << nonFixedWires[0].length() << ", "
              << nonFixedWires[1].length() << ", "
              << nonFixedWires[2].length() << ", "
              << nonFixedWires[3].length() << ", "
              << nonFixedWires[4].length() << ")" << std::endl;
// #if 0 /* in detail */
//   for(unsigned i=0;i<allFixedWires.length();++i)
//     std::cout << i << ": LocalID/LayerID = " << allFixedWires[i]->wire()->localId()
//   << "/" << allFixedWires[i]->wire()->layerId()
//   << ", LR = " << allFixedWires[i]->leftRight() << std::endl;
// #endif /* in detail */
#endif

    int createdLine = 0;
    int overCounter = 0;
    AList<TLink> goodWires;
#if 1 /* fastest finder */
    if (allFixedWires.length() >= 5) {
      selectGoodWires(allFixedWires, goodWires);
      if (goodWires.length() >= 5) {
        fitLine(goodWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
        if (fabs(good_b) < 10.)createdLine = 1;
      }
    }
#endif /* fastest finder */
#if 1 /* faster finder */
    if (createdLine == 0) {
      // Q > 0 : Outer = L, Inner = R
      // Q < 0 : Outer = R, Inner = L
      double Q = track.charge();
      unsigned isIncreased = 0;
      for (int sl = 0; sl < (int)nstereosuperlayers; ++sl) {
        if (fixedWires[sl].length()    >= 1 &&
            nonFixedWires[sl].length() >= 1) {
          isIncreased = 1;
          unsigned bestNCorrectLR = 0;
          double bestR;
          HepGeom::Point3D<double>  bestC;
          for (int i = 0; i < fixedWires[sl].length(); ++i) {
            int LR = fixedWires[sl][i]->leftRight() == 0 ? -1 : 1;
            HepGeom::Point3D<double>  center;
            double radius;
            calVirtualCircle(*fixedWires[sl][i], track, LR, center, radius);
            unsigned nCorrectLR = 0;
            for (int j = 0; j < fixedWires[sl].length(); ++j) {
              if (i != j) {
                int tmpIsOuter = 1;
		//ho                if ((fixedWires[sl][j]->wire()->xyPosition() - center).mag() - radius < 0.)tmpIsOuter = -1;
                if ((fixedWires[sl][j]->wire()->xyPosition() - center).perp() - radius < 0.)tmpIsOuter = -1;
                if (Q > 0. && tmpIsOuter == 1 && fixedWires[sl][j]->leftRight() == 0)++nCorrectLR;
                else if (Q > 0. && tmpIsOuter == -1 && fixedWires[sl][j]->leftRight() == 1)++nCorrectLR;
                else if (Q < 0. && tmpIsOuter ==  1 && fixedWires[sl][j]->leftRight() == 1)++nCorrectLR;
                else if (Q < 0. && tmpIsOuter == -1 && fixedWires[sl][j]->leftRight() == 0)++nCorrectLR;
              }
            }
            if (i == 0 || nCorrectLR > bestNCorrectLR) {
              bestNCorrectLR = nCorrectLR;
              bestR = radius;
              bestC = center;
            }
            if (bestNCorrectLR == (unsigned) fixedWires[sl].length() - 1)break;
          }
          for (int i = 0; i < nonFixedWires[sl].length(); ++i) {
            int isOuter = 1;
	    //ho            if ((nonFixedWires[sl][i]->wire()->xyPosition() - bestC).mag() - bestR < 0.)isOuter = -1;
            if ((nonFixedWires[sl][i]->wire()->xyPosition() - bestC).perp() - bestR < 0.)isOuter = -1;
            if (Q > 0. && isOuter == 1) {
              nonFixedWires[sl][i]->position(nonFixedWires[sl][i]->arcZ(0)); // L
              nonFixedWires[sl][i]->leftRight(0); // L
            } else if (Q > 0. && isOuter == -1) {
              nonFixedWires[sl][i]->position(nonFixedWires[sl][i]->arcZ(1)); // R
              nonFixedWires[sl][i]->leftRight(1); // R
            } else if (Q < 0. && isOuter == 1) {
              nonFixedWires[sl][i]->position(nonFixedWires[sl][i]->arcZ(1)); // R
              nonFixedWires[sl][i]->leftRight(1); // R
            } else if (Q < 0. && isOuter == -1) {
              nonFixedWires[sl][i]->position(nonFixedWires[sl][i]->arcZ(0)); // L
              nonFixedWires[sl][i]->leftRight(0); // L
            }
          }
          allFixedWires.append(nonFixedWires[sl]);
          fixedWires[sl].append(nonFixedWires[sl]);
          nonFixedWires[sl].removeAll();
        }
      }

#if DEBUG_CURL_DUMP
      std::cout << "(TBuilderCurl)    2nd fixed & non-fixed wires selection:" << std::endl;
      std::cout << "(TBuilderCurl)    all fixed wires # = " << allFixedWires.length() << std::endl;
      std::cout << "(TBuilderCurl)    fixed wires # = ("
                << fixedWires[0].length() << ", "
                << fixedWires[1].length() << ", "
                << fixedWires[2].length() << ", "
                << fixedWires[3].length() << ", "
                << fixedWires[4].length() << ")" << std::endl;
      std::cout << "(TBuilderCurl)    non fixed wires # = ("
                << nonFixedWires[0].length() << ", "
                << nonFixedWires[1].length() << ", "
                << nonFixedWires[2].length() << ", "
                << nonFixedWires[3].length() << ", "
                << nonFixedWires[4].length() << ")" << std::endl;
// #if 0 /* in detail */
//     for(unsigned i=0;i<allFixedWires.length();++i)
//       std::cout << i << ": LocalID/LayerID = " << allFixedWires[i]->wire()->localId()
//     << "/" << allFixedWires[i]->wire()->layerId()
//     << ", LR = " << allFixedWires[i]->leftRight() << std::endl;
// #endif /* in detail */
#endif

      if (isIncreased == 1 && allFixedWires.length() >= 5) {
        selectGoodWires(allFixedWires, goodWires);
        if (goodWires.length() >= 5) {
          fitLine(goodWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
          if (fabs(good_b) < 10.)createdLine = 1;
        }
      }
    }
#endif /* faster finder */
#if 1 /* slow finder */
    // 2000/1/27...very slow but unlike an infinity loop
    if (createdLine == 0) {
      // nonFixed Wires
      int* maxNonFixedLayerIndex = new int[nstereosuperlayers];
      // = { -1, -1, -1, -1, -1 };
      int* maxLength = new int [nstereosuperlayers];
      //= { 0, 0, 0, 0, 0 };
      for (int i = 0; i < (int)nstereosuperlayers; ++i) {
        maxNonFixedLayerIndex[i] = -1;
        maxLength[i] = 0;
      }
      for (int l = 0; l < (int)nstereolayers; ++l) {
        unsigned sl = 4; // superlayer id
        if (ms_superb) {
          sl = cdc.axialStereoSuperLayerId(1, l);
        } else {
          if (l < 3)sl = 0;
          else if (l < 6)sl = 1;
          else if (l < 10)sl = 2;
          else if (l < 14)sl = 3;
        }
        layer[l].remove(fixedWires[sl]);
        setLR(layer[l]); // set All L
        if (layer[l].length() > 0 && layer[l].length() > maxLength[sl]) {
          maxLength[sl] = layer[l].length();
          maxNonFixedLayerIndex[sl] = l;
        }
      }

      unsigned index = 0;
      unsigned* nonFixedSuperLayerIndex = new unsigned [nstereosuperlayers];
      //= { 0,0,0,0,0 };
      for (unsigned i = 0; i < nstereosuperlayers; ++i) nonFixedSuperLayerIndex[i] = 0;
      unsigned isIncreased = 0;
      for (unsigned i = 0; i < nstereosuperlayers; ++i) {
        allFixedWires.append(nonFixedWires[i]);
        if (nonFixedWires[i].length() > 0) {
          isIncreased = 1;
          nonFixedSuperLayerIndex[index] = i;
          ++index;
        }
      }

      if (isIncreased) {
        do {
          moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[0]]],
                 nonFixedWires[nonFixedSuperLayerIndex[0]],
                 track);
          if (index > 1) {
            setLR(nonFixedWires[nonFixedSuperLayerIndex[1]]);
            do {
              moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[1]]],
                     nonFixedWires[nonFixedSuperLayerIndex[1]],
                     track);
              if (index > 2) {
                setLR(nonFixedWires[nonFixedSuperLayerIndex[2]]);
                do {
                  moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[2]]],
                         nonFixedWires[nonFixedSuperLayerIndex[2]],
                         track);
                  if (index > 3) {
                    setLR(nonFixedWires[nonFixedSuperLayerIndex[3]]);
                    do {
                      moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[3]]],
                             nonFixedWires[nonFixedSuperLayerIndex[3]],
                             track);
                      if (index > 4) {
                        setLR(nonFixedWires[nonFixedSuperLayerIndex[4]]);
                        do {
                          moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[4]]],
                                 nonFixedWires[nonFixedSuperLayerIndex[4]],
                                 track);
                          fitLine2(allFixedWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
                          if (overCounter > 10000)goto kokohe;
                        } while (moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[4]]]));
                      } else {
                        fitLine2(allFixedWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
                        if (overCounter > 10000)goto kokohe;
                      }
                    } while (moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[3]]]));
                  } else {
                    fitLine2(allFixedWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
                    if (overCounter > 10000)goto kokohe;
                  }
                } while (moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[2]]]));
              } else {
                fitLine2(allFixedWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
                if (overCounter > 10000)goto kokohe;
              }
            } while (moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[1]]]));
          } else {
            fitLine2(allFixedWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
            if (overCounter > 10000)goto kokohe;
          }
        } while (moveLR(layer[maxNonFixedLayerIndex[nonFixedSuperLayerIndex[0]]]));
kokohe:;
      } else if (allFixedWires.length() >= 3) {
        fitLine2(allFixedWires, min_chi2, good_a, good_b, goodLine, goodPosition, overCounter);
      }
      delete [] nonFixedSuperLayerIndex;
      delete [] maxNonFixedLayerIndex;
      delete [] maxLength;
    }
#endif /* slow finder */
    for (unsigned i = 0, size = goodLine.length(); i < size; ++i) {
      goodLine[i]->position(*(goodPosition[i]));
    }
#if DEBUG_CURL_DUMP
    std::cout << "(TBuilderCurl)    make a line from All SuperLayers." << std::endl;
    plotArcZ(goodLine, good_a, good_b, 0);
#endif
    delete [] layer;
    delete [] layerOrg;
    delete [] fixedWires;
    delete [] nonFixedWires;
  }

  bool
  TBuilderCurl::fitWDD(double& xc, double& yc, double& r,
                       AList<TLink> &list) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::fitWDD" << std::endl;
#endif
    if (list.length() <= 3)return false;
    Lpav circle;
    // CDC
    for (int i = 0; i < list.length(); ++i) {
      circle.add_point(list[i]->wire()->xyPosition().x(),
                       list[i]->wire()->xyPosition().y(), 1.0);
    }
    circle.add_point(0., 0., 1.0); // IP Constraint
    if (circle.fit() < 0.0 || circle.kappa() == 0.0) return false;
    xc = circle.center()[0];
    yc = circle.center()[1];
    r  = circle.radius();
    const int maxIte = 2;
    for (int ite = 0; ite < maxIte; ++ite) {
      Lpav circle2;
      circle2.clear();
      // CDC
      for (int i = 0; i < list.length(); ++i) {
        double R = sqrt((list[i]->wire()->xyPosition().x() - xc) * (list[i]->wire()->xyPosition().x() - xc) +
                        (list[i]->wire()->xyPosition().y() - yc) * (list[i]->wire()->xyPosition().y() - yc));
        if (R == 0.)continue;
        double U = 1. / R;
        double dir = R > r ? -1. : 1.;
        double X = xc + (list[i]->wire()->xyPosition().x() - xc) * U * (R + dir * list[i]->hit()->drift());
        double Y = yc + (list[i]->wire()->xyPosition().y() - yc) * U * (R + dir * list[i]->hit()->drift());
        circle2.add_point(X, Y, 1.0);
      }
      circle2.add_point(0., 0., 1.0); // IP Constraint
      if (circle2.fit() < 0.0 || circle2.kappa() == 0.0) return false;
      xc = circle2.center()[0];
      yc = circle2.center()[1];
      r  = circle2.radius();
      // std::cout << xc << ", " << yc << " : " << r << std::endl;
    }
    return true;
  }

  int
  TBuilderCurl::stereoHit(double& xc, double& yc, double& r, double& q,
                          AList<TLink> & list) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::stereoHit" << std::endl;
#endif
    if (list.length() == 0)return -1;

    HepGeom::Point3D<double> center(xc, yc, 0.);
    HepGeom::Point3D<double> tmp(-999., -999., 0.);
    for (unsigned i = 0, size = list.length(); i < size; ++i) {
      Belle2::TRGCDCWireHit& h = *const_cast<Belle2::TRGCDCWireHit*>(list[i]->hit());
      Vector3D X = 0.5 * (h.wire().forwardPosition() +
                          h.wire().backwardPosition());
      Vector3D x     = Vector3D(X.x(), X.y(), 0.);
      Vector3D w     = x - center;
      Vector3D V     = h.wire().direction();
      Vector3D v     = Vector3D(V.x(), V.y(), 0.);
      double   vmag2 = v.mag2();
      double   vmag  = sqrt(vmag2);
      //...temporary
      for (unsigned j = 0; j < 4; ++j)
        list[i]->arcZ(tmp, j);

      //...stereo?
      if (vmag == 0.) continue;

      double drift = h.drift(CellHitLeft);
      double R[2] = {r + drift, r - drift};
      double wv = w.dot(v);
      double d2[2];
      d2[0] = vmag2 * R[0] * R[0] + (wv * wv - vmag2 * w.mag2()); //...= v^2*(r^2 - w^2*sin()^2)...outer
      d2[1] = vmag2 * R[1] * R[1] + (wv * wv - vmag2 * w.mag2()); //...= v^2*(r^2 - w^2*sin()^2)...inner

      //...No crossing in R/Phi plane...
      if (d2[0] < 0. && d2[1] < 0.) continue;

      bool ok_inner(true);
      bool ok_outer(true);
      double d[2] = { -1., -1.};
      //...outer
      if (d2[0] >= 0.) {
        d[0] = sqrt(d2[0]);
      } else {
        ok_outer = false;
      }
      if (d2[1] >= 0.) {
        d[1] = sqrt(d2[1]);
      } else {
        ok_inner = false;
      }

      //...Cal. length and z to crossing points...
      double l[2][2];
      double z[2][2];
      //...outer
      if (ok_outer) {
        l[0][0] = (- wv + d[0]) / vmag2; //...= (-wvcos()+d)/v/v = (-wcos() + (r^2 - w^2*sin()^2)^0.5)/v
        l[1][0] = (- wv - d[0]) / vmag2; //...= (-wvcos()+d)/v/v = (-wcos() - (r^2 - w^2*sin()^2)^0.5)/v
        z[0][0] = X.z() + l[0][0] * V.z();
        z[1][0] = X.z() + l[1][0] * V.z();
      }
      //...inner
      if (ok_inner) {
        l[0][1] = (- wv + d[1]) / vmag2; //...= (-wvcos()+d)/v/v = (-wcos() + (r^2 - w^2*sin()^2)^0.5)/v
        l[1][1] = (- wv - d[1]) / vmag2; //...= (-wvcos()+d)/v/v = (-wcos() - (r^2 - w^2*sin()^2)^0.5)/v
        z[0][1] = X.z() + l[0][1] * V.z();
        z[1][1] = X.z() + l[1][1] * V.z();
      }

      //...Cal. xy position of crossing points...
      Vector3D p[2][2];
      if (ok_outer) {
        p[0][0] = x + l[0][0] * v;
        p[1][0] = x + l[1][0] * v;
        Vector3D tmp_pc = p[0][0] - center;
        Vector3D pc0 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[0][0] -= drift / pc0.mag() * pc0;
        tmp_pc = p[1][0] - center;
        Vector3D pc1 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[1][0] -= drift / pc1.mag() * pc1;
      }
      if (ok_inner) {
        p[0][1] = x + l[0][1] * v;
        p[1][1] = x + l[1][1] * v;
        Vector3D tmp_pc = p[0][1] - center;
        Vector3D pc0 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[0][1] += drift / pc0.mag() * pc0;
        tmp_pc = p[1][1] - center;
        Vector3D pc1 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[1][1] += drift / pc1.mag() * pc1;
      }

      //...Check r-phi...
      bool ok_xy[2][2];
      if (ok_outer) {
        ok_xy[0][0] = true;
        ok_xy[1][0] = true;
      } else {
        ok_xy[0][0] = false;
        ok_xy[1][0] = false;
      }
      if (ok_inner) {
        ok_xy[0][1] = true;
        ok_xy[1][1] = true;
      } else {
        ok_xy[0][1] = false;
        ok_xy[1][1] = false;
      }
      if (ok_outer) {
        if (q * (center.x() * p[0][0].y() - center.y() * p[0][0].x())  < 0.)
          ok_xy[0][0] = false;
        if (q * (center.x() * p[1][0].y() - center.y() * p[1][0].x())  < 0.)
          ok_xy[1][0] = false;
      }
      if (ok_inner) {
        if (q * (center.x() * p[0][1].y() - center.y() * p[0][1].x())  < 0.)
          ok_xy[0][1] = false;
        if (q * (center.x() * p[1][1].y() - center.y() * p[1][1].x())  < 0.)
          ok_xy[1][1] = false;
      }
      if (!ok_inner && ok_outer && (!ok_xy[0][0]) && (!ok_xy[1][0])) {
        continue;
      }
      if (ok_inner && !ok_outer && (!ok_xy[0][1]) && (!ok_xy[1][1])) {
        continue;
      }

      //...Check z position...
      if (ok_xy[0][0]) {
        if (z[0][0] < h.wire().backwardPosition().z() ||
            z[0][0] > h.wire().forwardPosition().z()) ok_xy[0][0] = false;
      }
      if (ok_xy[1][0]) {
        if (z[1][0] < h.wire().backwardPosition().z() ||
            z[1][0] > h.wire().forwardPosition().z()) ok_xy[1][0] = false;
      }
      if (ok_xy[0][1]) {
        if (z[0][1] < h.wire().backwardPosition().z() ||
            z[0][1] > h.wire().forwardPosition().z()) ok_xy[0][1] = false;
      }
      if (ok_xy[1][1]) {
        if (z[1][1] < h.wire().backwardPosition().z() ||
            z[1][1] > h.wire().forwardPosition().z()) ok_xy[1][1] = false;
      }
      if ((!ok_xy[0][0]) && (!ok_xy[1][0]) &&
          (!ok_xy[0][1]) && (!ok_xy[1][1])) {
        continue;
      }
      double cosdPhi, dPhi;
      //unsigned index = 0;
      unsigned indexL = 0;
      unsigned indexR = 0;
      // std::cout << "Stereo " << std::endl;
      // Q > 0 : Outer = L, Inner = R
      // Q < 0 : Outer = R, Inner = L
      if (ok_xy[0][0]) {
        //...cal. arc length...
        cosdPhi = - center.dot((p[0][0] - center).unit()) / center.mag();
        if (fabs(cosdPhi) < 1.0) {
          dPhi = acos(cosdPhi);
        } else if (cosdPhi >= 1.0) {
          dPhi = 0.;
        } else {
          dPhi = M_PI;
        }
        //list[i]->arcZ(Point3D(r*dPhi, z[0][0], 0.), index);
        // std::cout << r*dPhi << ", " << z[0][0] << std::endl;
        //++index;
        if (q > 0) {
          // std::cout << "Outer L" << std::endl;
          if (indexL == 0)list[i]->arcZ(Point3D(r * dPhi, z[0][0], 0.), 0);
          else list[i]->arcZ(Point3D(r * dPhi, z[0][0], 0.), 2);
          ++indexL;
        } else {
          // std::cout << "Outer R" << std::endl;
          if (indexR == 0)list[i]->arcZ(Point3D(r * dPhi, z[0][0], 0.), 1);
          else list[i]->arcZ(Point3D(r * dPhi, z[0][0], 0.), 3);
          ++indexR;
        }
      }
      if (ok_xy[1][0]) {
        //...cal. arc length...
        cosdPhi = - center.dot((p[1][0] - center).unit()) / center.mag();
        if (fabs(cosdPhi) < 1.0) {
          dPhi = acos(cosdPhi);
        } else if (cosdPhi >= 1.0) {
          dPhi = 0.;
        } else {
          dPhi = M_PI;
        }
        //list[i]->arcZ(Point3D(r*dPhi, z[1][0], 0.), index);
        // std::cout << r*dPhi << ", " << z[1][0] << std::endl;
        //++index;
        if (q > 0) {
          // std::cout << "Outer L" << std::endl;
          if (indexL == 0)list[i]->arcZ(Point3D(r * dPhi, z[1][0], 0.), 0);
          else list[i]->arcZ(Point3D(r * dPhi, z[1][0], 0.), 2);
          ++indexL;
        } else {
          // std::cout << "Outer R" << std::endl;
          if (indexR == 0)list[i]->arcZ(Point3D(r * dPhi, z[1][0], 0.), 1);
          else list[i]->arcZ(Point3D(r * dPhi, z[1][0], 0.), 3);
          ++indexR;
        }
      }
      if (ok_xy[0][1]) {
        //...cal. arc length...
        cosdPhi = - center.dot((p[0][1] - center).unit()) / center.mag();
        if (fabs(cosdPhi) < 1.0) {
          dPhi = acos(cosdPhi);
        } else if (cosdPhi >= 1.0) {
          dPhi = 0.;
        } else {
          dPhi = M_PI;
        }
        //list[i]->arcZ(Point3D(r*dPhi, z[0][1], 0.), index);
        // std::cout << r*dPhi << ", " << z[0][1] << std::endl;
        //++index;
        if (q > 0) {
          // std::cout << "Inner R" << std::endl;
          if (indexR == 0)list[i]->arcZ(Point3D(r * dPhi, z[0][1], 0.), 1);
          else list[i]->arcZ(Point3D(r * dPhi, z[0][1], 0.), 3);
          ++indexR;
        } else {
          // std::cout << "Inner L" << std::endl;
          if (indexL == 0)list[i]->arcZ(Point3D(r * dPhi, z[0][1], 0.), 0);
          else list[i]->arcZ(Point3D(r * dPhi, z[0][1], 0.), 2);
          ++indexL;
        }
      }
      if (ok_xy[1][1]) {
        //...cal. arc length...
        cosdPhi = - center.dot((p[1][1] - center).unit()) / center.mag();
        if (fabs(cosdPhi) < 1.0) {
          dPhi = acos(cosdPhi);
        } else if (cosdPhi >= 1.0) {
          dPhi = 0.;
        } else {
          dPhi = M_PI;
        }
        //list[i]->arcZ(Point3D(r*dPhi, z[1][1], 0.), index);
        // std::cout << r*dPhi << ", " << z[1][1] << std::endl;
        //++index;
        if (q > 0) {
          // std::cout << "Inner R" << std::endl;
          if (indexR == 0)list[i]->arcZ(Point3D(r * dPhi, z[1][1], 0.), 1);
          else list[i]->arcZ(Point3D(r * dPhi, z[1][1], 0.), 3);
          ++indexR;
        } else {
          // std::cout << "Inner L" << std::endl;
          if (indexL == 0)list[i]->arcZ(Point3D(r * dPhi, z[1][1], 0.), 0);
          else list[i]->arcZ(Point3D(r * dPhi, z[1][1], 0.), 2);
          ++indexL;
        }
      }
    }
    return 0;
  }

  void
  TBuilderCurl::setArcZ(TTrack& track, AList<TLink> &slayer,
                        AList<TLink> &alayer0, AList<TLink> &alayer1,
                        unsigned) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setArcZ 1" << std::endl;
#endif
    AList<TLink> tmp = alayer0;
    tmp.append(alayer1);
    double xc, yc, r;
    if (fitWDD(xc, yc, r, tmp)) {
      double q = track.charge();
      stereoHit(xc, yc, r, q, slayer);
    }
  }

  void
  TBuilderCurl::setArcZ(TTrack& track, AList<TLink> &slayer,
                        AList<TLink> &alayer0, AList<TLink> &alayer1,
                        AList<TLink> &alayer2,
                        unsigned) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setArcZ 2" << std::endl;
#endif
    AList<TLink> tmp = alayer0;
    tmp.append(alayer1);
    tmp.append(alayer2);
    double xc, yc, r;
    if (fitWDD(xc, yc, r, tmp)) {
      double q = track.charge();
      stereoHit(xc, yc, r, q, slayer);
    }
  }

  void
  TBuilderCurl::setArcZ(TTrack& track, AList<TLink> &slayer,
                        AList<TLink> &alayer0, AList<TLink> &alayer1,
                        AList<TLink> &alayer2, AList<TLink> &alayer3,
                        unsigned) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setArcZ 3" << std::endl;
#endif
    AList<TLink> tmp = alayer0;
    tmp.append(alayer1);
    tmp.append(alayer2);
    tmp.append(alayer3);
    double xc, yc, r;
    if (fitWDD(xc, yc, r, tmp)) {
      double q = track.charge();
      stereoHit(xc, yc, r, q, slayer);
    }
  }

  void
  TBuilderCurl::setArcZ(TTrack& track, AList<TLink> &slayer,
                        AList<TLink> &alayer0, AList<TLink> &alayer1,
                        AList<TLink> &alayer2, AList<TLink> &alayer3,
                        AList<TLink> &alayer4,
                        unsigned) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setArcZ 4" << std::endl;
#endif
    AList<TLink> tmp = alayer0;
    tmp.append(alayer1);
    tmp.append(alayer2);
    tmp.append(alayer3);
    tmp.append(alayer4);
    double xc, yc, r;
    if (fitWDD(xc, yc, r, tmp)) {
      double q = track.charge();
      stereoHit(xc, yc, r, q, slayer);
    }
  }

  void
  TBuilderCurl::setArcZ(TTrack& track, AList<TLink> &slayer,
                        AList<TLink> &alayer0, AList<TLink> &alayer1,
                        AList<TLink> &alayer2, AList<TLink> &alayer3,
                        AList<TLink> &alayer4, AList<TLink> &alayer5,
                        unsigned) const
  {
#if defined(HO_DEBUG)
  std::cout << " TBuilderCurl::setArcZ 5" << std::endl;
#endif
    AList<TLink> tmp = alayer0;
    tmp.append(alayer1);
    tmp.append(alayer2);
    tmp.append(alayer3);
    tmp.append(alayer4);
    tmp.append(alayer5);
    double xc, yc, r;
    if (fitWDD(xc, yc, r, tmp)) {
      double q = track.charge();
      stereoHit(xc, yc, r, q, slayer);
    }
  }

#undef DEBUG_CURL_DUMP
#undef DEBUG_CURL_GNUPLOT
#undef DEBUG_CURL_MC

// End === Stereo Finder For Curl Tracks : by jtanaka ===

} // namespace Belle
