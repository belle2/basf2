//-----------------------------------------------------------------------------
// $Id: TConformalFinder.cc 11152 2010-04-28 01:24:38Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TConformalFinder.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the conformal method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.120  2005/11/03 23:20:11  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.119  2005/03/11 03:57:50  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.118  2005/01/14 00:55:39  katayama
// uninitialized variable
//
// Revision 1.117  2004/03/26 06:07:03  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.116  2004/02/18 04:07:25  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.115  2003/12/25 13:03:16  yiwasaki
// minor fixes
//
// Revision 1.114  2003/12/25 12:03:32  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.113  2003/12/19 07:36:02  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.112  2003/09/10 01:18:30  yiwasaki
// Trasan for small cell CDC
//
// Revision 1.111  2002/02/26 22:38:59  yiwasaki
// bug fixes in debug mode
//
// Revision 1.110  2002/02/20 01:32:36  katayama
// std::
//
// Revision 1.109  2002/02/13 21:59:57  yiwasaki
// Trasan 3.03 : T0 reset test mode added
//
// Revision 1.108  2001/12/23 09:58:46  katayama
// removed Strings.h
//
// Revision 1.107  2001/12/19 02:59:45  katayama
// Uss find,istring
//
// Revision 1.106  2001/12/05 12:35:17  katayama
// For gcc-3
//
// Revision 1.105  2001/06/18 00:16:37  yiwasaki
// Trasan 3.01 : perfect segment finder option, segv in term on linux fixed
//
// Revision 1.104  2001/04/27 01:28:39  yiwasaki
// Trasan 3.00 : official release, nothing changed from 3.00 RC6
//
// Revision 1.103  2001/04/25 02:35:59  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.102  2001/04/11 23:20:24  yiwasaki
// Trasan 3.00 RC3 : a bug in stereo mode 1 and 2 is fixed
//
// Revision 1.101  2001/04/11 01:09:09  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.99  2001/02/07 22:25:02  yiwasaki
// Trasan 2.28 : conf minor change in shared memory
//
// Revision 1.98  2001/02/05 05:48:56  yiwasaki
// Trasan 2.26 : bug fix for data processing
//
// Revision 1.97  2001/02/01 06:15:47  yiwasaki
// Trasan 2.25 : conf bug fix for chisq=0
//
// Revision 1.96  2001/01/30 04:54:13  yiwasaki
// Trasan 2.21 release : bug fixes
//
// Revision 1.95  2001/01/29 09:27:44  yiwasaki
// Trasan 2.20 release : 2D & cosmic tracks are output, curler decision is made by dr and dz
//
// Revision 1.94  2000/10/05 23:54:21  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.93  2000/04/15 13:40:40  katayama
// Add/remove const so that they compile
//
// Revision 1.92  2000/04/14 05:20:45  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.91  2000/04/13 02:53:37  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.90  2000/04/11 13:05:42  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.89  2000/04/04 12:14:16  yiwasaki
// Trasan 2.00RC27 : bad point rejection in conf., association check for dE/dx
//
// Revision 1.88  2000/04/04 07:40:05  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.87  2000/03/31 07:21:11  yiwasaki
// PM finder updates from J.Tanaka, trkmgr bug fixes
//
// Revision 1.86  2000/03/24 10:22:54  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.85  2000/03/21 07:01:26  yiwasaki
// tmp updates
//
// Revision 1.84  2000/03/17 11:01:38  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.83  2000/03/01 04:51:07  yiwasaki
// Trasan 2.00RC14 : stereo bug fix, curl updates
//
// Revision 1.82  2000/02/29 07:16:15  yiwasaki
// Trasan 2.00RC13 : default stereo param. changed
//
// Revision 1.81  2000/02/28 01:58:57  yiwasaki
// Trasan 2.00RC11 : curl updates only
//
// Revision 1.80  2000/02/25 12:55:40  yiwasaki
// Trasan 2.00RC10 : stereo improved
//
// Revision 1.79  2000/02/25 08:09:55  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.78  2000/02/25 00:14:24  yiwasaki
// Trasan 2.00RC8 : robust fitter bug fix
//
// Revision 1.77  2000/02/24 06:19:28  yiwasaki
// Trasan 2.00RC7 : bug fix again
//
// Revision 1.76  2000/02/24 00:30:50  yiwasaki
// Trasan 2.00RC6 : quality2D bug fix
//
// Revision 1.75  2000/02/23 08:45:05  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.74  2000/02/18 04:33:21  yiwasaki
// bug fix
//
// Revision 1.73  2000/02/17 13:24:19  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.72  2000/02/15 13:46:40  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.71  2000/02/10 13:11:38  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.70  2000/02/09 03:27:39  yiwasaki
// Trasan 1.68l : curl and new conf stereo updated
//
// Revision 1.69  2000/02/03 06:18:02  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.68  2000/02/01 11:24:42  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.67  2000/01/30 08:17:05  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.66  2000/01/28 06:30:21  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.65  2000/01/23 08:23:04  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.64  2000/01/19 01:33:31  yiwasaki
// Trasan 1.67f : new conf modified
//
// Revision 1.62  1999/11/19 09:13:07  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.61  1999/10/30 10:12:16  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.60  1999/10/21 15:45:14  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.59  1999/09/21 02:01:33  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.57  1999/08/25 06:25:49  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.56  1999/08/04 01:01:41  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.55  1999/07/27 08:01:37  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.54  1999/07/23 04:47:23  yiwasaki
// comment out debug option
//
// Revision 1.53  1999/07/23 03:53:23  yiwasaki
// Trasan 1.57b release : minor changes only
//
// Revision 1.52  1999/07/09 01:47:20  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.51  1999/06/26 07:05:43  yiwasaki
// Trasan 1.47a release : hit and tracking efficiency improved
//
// Revision 1.50  1999/06/15 06:33:41  yiwasaki
// Trasan 1.43 release : minor changes in TRGCDCClust and TBuilder
//
// Revision 1.49  1999/06/14 12:40:21  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.48  1999/06/10 09:44:51  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.47  1999/06/10 00:27:27  yiwasaki
// Trasan 1.39 release : TTrack::approach bug fix
//
// Revision 1.46  1999/05/18 04:44:29  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.45  1999/03/21 15:45:40  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.44  1999/03/11 23:27:13  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.43  1999/03/09 06:29:23  yiwasaki
// Trasan 1.21 release : conformal finder bug fix
//
// Revision 1.42  1999/02/03 06:23:07  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.40  1999/01/11 03:09:08  yiwasaki
// Trasan 1.11 release
//
// Revision 1.39  1999/01/11 03:03:10  yiwasaki
// Fitters added
//
// Revision 1.38  1998/11/27 08:15:30  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.37  1998/11/12 12:27:25  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.36  1998/11/11 07:26:45  yiwasaki
// Trasan 1.1 beta 9 release : more protections for negative sqrt and zero division
//
// Revision 1.35  1998/11/10 09:09:05  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.34  1998/10/13 04:04:45  yiwasaki
// Trasan 1.1 beta 7 release : memory leak fixed by J.Tanaka, TCurlFinderParameters.h added by J.Tanaka
//
// Revision 1.33  1998/10/09 03:01:06  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.32  1998/09/28 16:52:10  yiwasaki
// TBuilderCosmic added
//
// Revision 1.31  1998/09/24 22:56:37  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.30  1998/08/12 16:32:47  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.29  1998/08/03 15:01:00  yiwasaki
// Trasan 1.07 release : cluster finder from S.Suzuki-san added
//
// Revision 1.28  1998/07/29 04:34:50  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.25  1998/07/06 15:48:47  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.24  1998/07/02 09:04:38  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.23  1998/06/29 02:15:07  yiwasaki
// Trasan 1 release candidate 1
//
// Revision 1.22  1998/06/24 06:55:01  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.21  1998/06/21 18:38:19  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.20  1998/06/17 20:22:58  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.19  1998/06/17 20:12:36  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.18  1998/06/15 09:58:14  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.17  1998/06/15 03:34:17  yiwasaki
// Trasan 1 beta 3.1, compiler error fixed
//
// Revision 1.16  1998/06/14 11:09:50  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.15  1998/06/11 12:25:15  yiwasaki
// TConformalLink removed
//
// Revision 1.14  1998/06/11 08:14:06  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.13  1998/06/08 14:37:52  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.12  1998/06/03 17:17:37  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TSegment::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.11  1998/05/26 05:10:17  yiwasaki
// cvs repair
//
// Revision 1.10  1998/05/22 18:20:56  yiwasaki
// Range moved to com-cdc
//
// Revision 1.9  1998/05/22 08:23:18  yiwasaki
// Trasan 1 alpha 4 release, TSegment added, TConformalLink no longer used
//
// Revision 1.8  1998/05/18 08:08:53  yiwasaki
// preparation for alpha 3
//
// Revision 1.7  1998/05/11 10:16:56  yiwasaki
// TTrack::assign -> TTrack::assert, WireHitUsedMask is set in TRGCDCWireHit
//
// Revision 1.6  1998/05/08 09:45:43  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.5  1998/04/23 17:21:37  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.4  1998/04/16 16:49:33  yiwasaki
// minor changes
//
// Revision 1.3  1998/04/14 01:04:48  yiwasaki
// TRGCDCWireHitMC added
//
// Revision 1.2  1998/04/10 09:36:27  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:50:14  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------




#include <algorithm>
#include <string.h>
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
#include "tracking/modules/trasan/Strings.h"

#include "trg/cdc/TRGCDC.h"

#include "trg/cdc/TrackMC.h"
#include "trg/cdc/WireHitMC.h"
#include "tracking/modules/trasan/TTrackManager.h"
#include "tracking/modules/trasan/TConformalFinder.h"
#include "tracking/modules/trasan/TConformalFinder0.h"
#include "tracking/modules/trasan/THistogram.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TUtilities.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifdef TRASAN_WINDOW_GTK
#include "tracking/modules/trasan/Trasan.h"
#endif

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  std::string
  TConformalFinder::version(void) const
  {
    return "3.06";
  }

  TConformalFinder::TConformalFinder(unsigned fastFinder,
                                     unsigned slowFinder,
                                     unsigned perfectSegmentFinding,
                                     bool useSmallCells,
                                     float maxSigma,
                                     float maxSigmaStereo,
                                     float salvageLevel,
                                     unsigned minNLinksForSegment,
                                     unsigned minNCoreLinks,
                                     unsigned minNSegments,
                                     unsigned salvageLoadWidth,
                                     unsigned stereoMode,
                                     unsigned stereoLoadWidth,
                                     float szSegmentDistance,
                                     float szLinkDistance,
                                     unsigned fittingFlag)
    : _fastFinder(fastFinder),
      _slowFinder(slowFinder),
      _perfectSegmentFinding(perfectSegmentFinding),
      _useSmallCells(useSmallCells),
      _maxSigma2(maxSigma),
      _minNLinksForSegment(minNLinksForSegment),
      _minNCoreLinks(minNCoreLinks),
      _minNSegments(minNSegments),
      _salvageLoadWidth(salvageLoadWidth),
      _stereoMode(stereoMode),
      _stereoLoadWidth(stereoLoadWidth),

      _doT0Reset(false),
      _T0ResetDone(false),
      _segmentSeparation(4),
      _minNLinksForSegmentInRefine(3),
      //  _maxNLinksForSegment((Belle2::TRGCDC::getTRGCDC()->versionCDC()=="superb")?16:8),
      _maxNLinksForSegment(8),
      _maxWidthForSegment(4),
      _minUsedFractionSlow2D(0.5),
      _appendLoad(2),
      _builder("conformal builder",
               maxSigma,
               maxSigmaStereo,
               salvageLevel,
               szSegmentDistance,
               szLinkDistance,
               fittingFlag),
      _s(0)
#ifdef TRASAN_WINDOW
      , _rphiWindow("rphi window")
#endif
  {
    _linkMaxDistance[0] = 0.02;
    _linkMaxDistance[1] = 0.025;
    _linkMaxDistance[2] = 0.025;
    _linkMinDirAngle[0] = 0.98;
    _linkMinDirAngle[1] = 0.97;
    _linkMinDirAngle[2] = 0.97;



//     if(Belle2::TRGCDC::getTRGCDC()->versionCDC()=="superb") {
//       _maxNLinksForSegment=16;
//     }

  }

  TConformalFinder::~TConformalFinder()
  {
  }

  void
  TConformalFinder::dump(const std::string& msg, const std::string& pre) const
  {
    if (msg.find("state") != std::string::npos) {
      std::cout << pre;
      TFinderBase::dump(msg);
      std::cout << pre;
      std::cout << "#axial=" << _hits[0].length();
      std::cout << ",#stereo=" << _hits[1].length();
    }
    if (msg.find("summary") != std::string::npos || msg.find("detail") != std::string::npos) {
      struct summary s;
      //    bzero((char *) & s, sizeof(struct summary));
      memset((char*) & s, 0, sizeof(struct summary));
      for (int i = 0; i < 0; i++) {
//cnv     int size;
        struct summary& r = s;
        s._nEvents += r._nEvents;
        s._nTracksFast3D += r._nTracksFast3D;
        s._nTracksSlow3D += r._nTracksSlow3D;
        s._nTracksFast2D += r._nTracksFast2D;
        s._nTracksSlow2D += r._nTracksSlow2D;
        s._nTracksFast2DBadQuality += r._nTracksFast2DBadQuality;
        s._nTracksSlow2DBadQuality += r._nTracksSlow2DBadQuality;
      }

      std::cout << pre
                << "all events : " << s._nEvents << std::endl;
      std::cout << pre
                << "fast 3D tracks              : "
                << s._nTracksFast3D << std::endl;
      std::cout << pre
                << "fast 2D tracks(line failed) : "
                << s._nTracksFast2D << std::endl;
      std::cout << pre
                << "fast 2D tracks(3D failed)   : "
                << s._nTracksFast2DBadQuality << std::endl;
      std::cout << pre
                << "slow 3D tracks              : "
                << s._nTracksSlow3D << std::endl;
      std::cout << pre
                << "slow 2D tracks(line failed) : "
                << s._nTracksSlow2D << std::endl;
      std::cout << pre
                << "slow 2D tracks(3D failed)   : "
                << s._nTracksSlow2DBadQuality << std::endl;
    }
  }

  void
  TConformalFinder::clear(void)
  {
    for (unsigned i = 0; i < 3; i++) {
      if (i == 2)
        HepAListDeleteAll(_allHits[i]);
      else
        _allHits[i].removeAll();
      _hits[i].removeAll();
      _unused[i].removeAll();
    }
    for (unsigned i = 0; i < 2; i++) {
      for (unsigned j = 0; j < 6; j++) {
        HepAListDeleteAll(_allSegments[i][j]);
        _allUnused[i][j].removeAll();
      }
    }
    _2DTracks.removeAll();
    _3DTracks.removeAll();
  }

  void
  TConformalFinder::selectGoodHits(void)
  {
    const bool ignoreSmallCell =
      (Belle2::TRGCDC::getTRGCDC()->version() == "small cell") &&
      (! _useSmallCells);

    for (unsigned i = 0; i < 2; i++) {
      unsigned n = _allHits[i].length();
      for (unsigned j = 0; j < n; j++) {

        //...Ignore small cells...
        if (ignoreSmallCell && _allHits[i][j]->wire()->layerId() < 3) {
          _unused[i].append(_allHits[i][j]);
//    std::cout << "ignoring !!!" << std::endl;
        }

        //...Normal cells...
        else {
          unsigned state = _allHits[i][j]->hit()->state();
          if ((state & CellHitIsolated) && (state & CellHitContinuous))
            _hits[i].append(_allHits[i][j]);
          else
            _unused[i].append(_allHits[i][j]);
        }
      }
    }
    _hits[2].append(_hits[0]);
    _hits[2].append(_hits[1]);
    _unused[2].append(_unused[0]);
    _unused[2].append(_unused[1]);

#ifdef TRASAN_WINDOW
    _rphiWindow.clear();
    _rphiWindow.skip(false);
    _rphiWindow.skipAllWindow(false);
    _rphiWindow.append(_allHits[2]);
    _rphiWindow.append(_hits[2], leda_pink);
//     displayStatus("Conf::selectGoodHits ... results");
//     _rphiWindow.wait();
#endif
#ifdef TRASAN_WINDOW_GTK
    TWindowGTKConformal& w = Trasan::getTrasan()->w();
    w.clear();
//  w.skip(false);
    w.stage("Conformal Finder : good hit selection");
    w.append(_allHits[2], Gdk::Color("grey"));
    w.append(_hits[2], Gdk::Color("green"));
    w.run();
#endif
  }

  void
  TConformalFinder::findSegments(void)
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
    unsigned nSuperLayers = cdc.nSuperLayers();
    unsigned maxNLinks = _maxNLinksForSegment;
    unsigned maxWidth = _maxWidthForSegment;
    //...Create lists of links for each super layer...
    AList<TLink> links[9];
    unsigned n = _hits[2].length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * _hits[2][i];
      links[l.wire()->superLayerId()].append(l);
    }

    //...Create phi hists...
    std::vector<THistogram*> hist;
    //    THistogram * hist[11];
    for (unsigned i = 0; i < nSuperLayers; ++i) {
      unsigned nw = (*cdc.superLayer(i))[0]->nCells();
      if (cdc.versionCDC() == "small cell" && i == 0) {
        nw = 64;
      }
      hist.push_back(new THistogram(nw));
    }
//     hist[0] = new THistogram(64);
//     hist[1] = new THistogram(80);
//     hist[2] = new THistogram(96);
//     hist[3] = new THistogram(128);
//     hist[4] = new THistogram(144);
//     hist[5] = new THistogram(160);
//     hist[6] = new THistogram(192);
//     hist[7] = new THistogram(208);
//     hist[8] = new THistogram(240);
//     hist[9] = new THistogram(256);
//     hist[10] = new THistogram(288);
    for (unsigned i = 0; i < nSuperLayers; i++) {
      unsigned superLayerId = i / 2;
      unsigned id = i % 2;
      hist[i]->fillPhi(links[i]);
//cnv   if(cdc.versionCDC()=="superb") {
//    maxNLinks = cdc.superLayer(i)->length() * 2;
//    maxWidth = cdc.superLayer(i)->length();
//  }

      //...Segment finding...
      AList<TSegment> tmp = hist[i]->segments();

      //...Remove bad segments...
      unsigned n = tmp.length();
      if (id == 0) {
        AList<TSegment> bad;
        for (unsigned j = 0; j < n; j++) {
          TSegment* s = tmp[j];
          if ((unsigned) s->links().length() < _minNLinksForSegment) {
            //      s->dump("detail","<min");
            bad.append(s);
          } else if ((unsigned) s->links().length() > maxNLinks) {
            //      s->dump("detail",">max");
            bad.append(s);
          } else if (TLink::width(s->links()) > maxWidth) {
            //      s->dump("detail",">width");
            bad.append(s);
          }
        }
        tmp.remove(bad);
        for (unsigned j = 0; j < (unsigned) bad.length(); j++) {
          _unused[id].append(bad[j]->links());
          _unused[2].append(bad[j]->links());
        }
        HepAListDeleteAll(bad);
      }

      //...Classify segments...
//    if (n > 1) {
//        for (unsigned k = 0; k < (n - 1); k++) {
//      TSegment & s0 = * tmp[k];
//      bool ok = true;
//      for (unsigned l = k + 1; l < n; l++) {
//          TSegment & s1 = * tmp[l];
//          int distance =
//        abs(InnerMost(s0.links())->wire()->localIdDifference(
//            * InnerMost(s1.links())->wire()));
//          if (distance < _segmentSeparation) {
//        s0.state(s0.state() | TSegmentCrowd);
//        s1.state(s1.state() | TSegmentCrowd);
//          }
//      }
//        }
//    }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... # of good segments=" << tmp.length()
                << std::endl;
#endif

      //...Store them...
      _allSegments[id][superLayerId].append(tmp);
      _allUnused[id][superLayerId] = _allSegments[id][superLayerId];
      //  delete hist[i];
    }

    for (std::vector<THistogram*>::iterator it = hist.begin();
         it != hist.end(); ++it) {
      delete *it;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... segment finding finished" << std::endl;
#endif
  }

  void
  TConformalFinder::linkSegments(unsigned level)
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
//cnv  unsigned nSuperLayers = cdc.nSuperLayers();

    //...Clear old links...
    unsigned superLayer = cdc.nAxialSuperLayers();
    if (superLayer > 0) {
      while (--superLayer) {
        AList<TSegment> & segments = _allUnused[0][superLayer];
        unsigned n = segments.length();
        for (unsigned i = 0; i < n; i++) {
          TSegment& base = * segments[i];
#ifdef TRASAN_DEBUG
          if (base.tracks().length()) {
            std::cout << "TConformalFinder::linkSegments !!! segment has ";
            std::cout << "an assignment to track(s)" << std::endl;
          }
#endif
          base.innerLinks().removeAll();
          base.outerLinks().removeAll();
        }
      }
    }

    //...Super layer loop...
    superLayer = cdc.nAxialSuperLayers();
    if (superLayer > 0) {
      while (--superLayer) {
        AList<TSegment> & segments = _allUnused[0][superLayer];
        AList<TSegment> & targets = _allUnused[0][superLayer - 1];
        AList<TSegment> & targets2 = _allUnused[0][superLayer - 2];
        unsigned n = segments.length();
        for (unsigned i = 0; i < n; i++) {
          TSegment& base = * segments[i];
#ifdef TRASAN_DEBUG
          if (base.tracks().length()) {
            std::cout << "TConformalFinder::linkSegments !!! segment has ";
            std::cout << "an assignment to track(s)" << std::endl;
          }
#endif

          const HepGeom::Point3D<double> & p = base.position();
          Vector3D v = base.direction();
          if (base.outerLinks().length() == 1)
            v = p - OuterMostUniqueLink(base)->position();
//        if (base.outerLinks().length() == 1)
//      v = p - base.outerLinks()[0]->position();
//        if (base.outerLinks().length() == 1)
//      v = direction(base);

          AList<TSegment> & candidates = base.innerLinks();
          TSegment* best = link(base, p, v, targets, candidates, level);
          if ((best == NULL) && (level > 0) && (superLayer > 1)) {
            best = link(base, p, v, targets2, candidates, level);
          }
          if (best) candidates.insert(best);

          unsigned m = candidates.length();
          for (unsigned j = 0; j < m; j++)
            candidates[j]->outerLinks().append(base);
        }
      }
    }

#ifdef TRASAN_WINDOW
    // _rphiWindow.skipAllWindow(false);
    displayStatus("Conf::linkSegments ... results");
    _rphiWindow.wait();
#endif
#ifdef TRASAN_WINDOW_GTK
    TWindowGTKConformal& w = Trasan::getTrasan()->w();
    w.clear();
//  w.skip(false);
    w.stage("Conformal Finder : segment links");
    w.append(_allHits[2], Gdk::Color("grey"));
//  w.append(_hits[2], Gdk::Color("green"));
    for (unsigned i = 0; i < Belle2::TRGCDC::getTRGCDC()->nSuperLayers(); i++) {
      unsigned superLayerId = i / 2;
      unsigned id = i % 2;
//  w.append(_allSegments[id][superLayerId], Gdk::Color("green"));
      w.append(_allUnused[id][superLayerId], Gdk::Color("green"));
    }
    w.run();
#endif
  }

  void
  TConformalFinder::resolveSegments(AList<TTrack> & trackCandidates) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... resolving assignments" << std::endl;
#endif

    //...Pick up segments which has multiple assignments...
    AList<TSegment> multi;
    unsigned n = trackCandidates.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * trackCandidates[i];
      AList<TSegment> & segments = t.segments();
      unsigned nS = segments.length();
      for (unsigned j = 0; j < nS; j++) {
        if (segments[j]->tracks().length() > 1)
          multi.append(segments[j]);
      }
    }
    multi.purge();

    //...Resolve assignments...
    n = multi.length();
    for (unsigned i = 0; i < n; i++) {
      TSegment& s = * multi[i];
      const AList<TTrack> & tracks = s.tracks();
      unsigned nT = tracks.length();

      //...Check TLink overlap...
      AList<TLink> multiLinks;
      const AList<TLink> & links = s.links();
      unsigned nL = links.length();
      for (unsigned j = 0; j < nL; j++) {
        TLink& l = * links[i];
        bool overlap = false;
        for (unsigned k = 0; k < nT; k++) {
          TTrack& t = * tracks[k];
          if (t.links().hasMember(l))
            overlap = true;
        }
        multiLinks.append(l);
      }
      multiLinks.purge();

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    segment : ";
      s.dump("hits sort flag");
      std::cout << "        # of assigned tracks = " << nT << std::endl;
      std::cout << "        # of overlap TLinks = " << multiLinks.length();
      std::cout << std::endl;
#endif
      //...Select the closest...
      nL = multiLinks.length();
      for (unsigned j = 0; j < nL; j++) {
        TLink& l = * multiLinks[j];

        float bestDiff = 999999999.;
        TTrack* best = NULL;
        for (unsigned k = 0; k < nT; k++) {
          TTrack& t = * tracks[k];
          t.approach(l);
          float distance = (l.positionOnWire() - l.positionOnTrack())
                           .mag();
          float diff = fabs(distance - l.hit()->drift());
          if (diff < bestDiff) {
            bestDiff = diff;
            best = & t;
          }
        }

        for (unsigned k = 0; k < nT; k++) {
          TTrack& t = * tracks[k];
          if (& t == best) continue;
          t.remove(l);
        }

#ifdef TRASAN_DEBUG_DETAIL
        {
          std::cout << "            " << l.wire()->name() << " -> ";
          std::cout << best->name() << std::endl;
        }
#endif
      }
    }
  }

  AList<TSegment>
  TConformalFinder::removeBadSegments(TTrack& t) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "removeBadSegments";
    EnterStage(stage);
    std::cout << Tab() << "removing bad segments:#used seg="
              << t.segments().length() << std::endl;
    for (unsigned i = 0; i < (unsigned) t.segments().length(); i++)
      t.segments()[i]->dump("hits sort flag", Tab(+1));
#endif

    const AList<TSegment> & segments = t.segments();
    AList<TSegment> bads;
    unsigned used = 0;
    TSegment* innerMost;
    unsigned n = segments.length();
    for (unsigned i = 0; i < n; i++) {
      TSegment& s = * segments[i];
      AList<TLink> links = Links(s, t);
//  unsigned nLinks = links.length();

      unsigned nCores = TLink::cores(links).length();
      unsigned nCoresSegment = s.nCores();
//    if (nCores < _minNCoreLinks) {
//  if (nCores < (nCoresSegment / 2)) {
      if ((nCores < _minNCoreLinks) && (nCores < ((nCoresSegment + 1) / 2))) {
        bads.append(s);
        continue;
      }

      unsigned id = segments[i]->superLayerId();
      used |= (1 << id);
      if (! id) innerMost = & s;
    }

//      //...Check used super layers...
//      n = segments.length();
//      for (unsigned i = 0; i < n; i++)
//    if ((used & 0x155) == 0x101)
//        bads.append(innerMost);

    if (! bads.length()) {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return bads;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "bad segments:#=" << bads.length() << std::endl;
#endif

    n = bads.length();
    for (unsigned i = 0; i < n; i++) {
      TSegment& s = * bads[i];

#ifdef TRASAN_DEBUG_DETAIL
      AList<TLink> links = Links(s, t);
      unsigned nLinks = links.length();
      unsigned nCores = TLink::cores(links).length();
      std::cout << Tab() << "#used links=" << nLinks
                << ",#used cores=" << nCores << std::endl;
      s.dump("hits sort flag", Tab(+1));
#endif

      s.tracks().remove(t);
      t.segments().remove(s);
      t.remove(s.links());
    }

    //...Refit...
    t.fit();

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... refitting" << std::endl;
    t.dump("detail", "2nd> ");
    LeaveStage(stage);
#endif

    return bads;
  }

  TSegment*
#ifdef TRASAN_DEBUG_DETAIL
  TConformalFinder::link(const TSegment& base,
                         const HepGeom::Point3D<double> & p,
                         const Vector3D& v,
                         const AList<TSegment> & candidates,
                         AList<TSegment> & alternatives,
                         unsigned level) const
  {
    std::cout << "    link : base = " << std::endl;
    base.dump("vector hits mc sort", "->  ");
    std::cout << "    p=" << p << ", v=" << v.unit() << std::endl;
#else
  TConformalFinder::link(const TSegment&,
                         const HepGeom::Point3D<double> & p,
                         const Vector3D& v,
                         const AList<TSegment> & candidates,
                         AList<TSegment> & alternatives,
                         unsigned level) const
  {
#endif

    //...Parameters...
    //static const float maxDistance = 0.02;
    //static const float minDirAngle = 0.97;

    //...Candidate loop...
    unsigned n = candidates.length();
//  float maxAngle = -999.;
//  float minDistance = _linkMaxDistance[level];
    float maxDirAngle = _linkMinDirAngle[level];
    TSegment* best = NULL;
    for (unsigned j = 0; j < n; j++) {
      TSegment& current = * candidates[j];

#ifdef TRASAN_DEBUG_DETAIL
      current.dump("vector hits mc sort", "    ");
      std::cout << "        dist,dirAngle,angle=" << current.distance(p, v);
      std::cout << "," << ((current.position() - p).unit()).dot(v.unit());
      std::cout << "," << v.dot(current.direction()) << std::endl;
#endif

      float distance = current.distance(p, v);
      if (distance > _linkMaxDistance[level]) continue;

      Vector3D dir = (current.position() - p);
      if (dir.x() > M_PI)        dir.setX(dir.x() - PI2);
      else if (dir.x() < - M_PI) dir.setX(PI2 + dir.x());

      float dirAngle = dir.unit().dot(v.unit());
      if (dirAngle < _linkMinDirAngle[level]) continue;

//    if (distance < minDistance) {
//        minDistance = distance;
//        best = & current;
//    }
      if (dirAngle > maxDirAngle) {
        maxDirAngle = dirAngle;
        best = & current;
      }
      alternatives.append(current);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    # of best + alternatives = " << alternatives.length() << std::endl;
    if (best) {
      std::cout << "        Best is ";
      best->dump("hits");
    }
#endif

    alternatives.remove(best);
    if (best) return best;
    return NULL;
  }

  void
  TConformalFinder::deleteTrack(TTrack& t) const
  {
    const AList<TSegment> & segments = t.segments();
    unsigned n = segments.length();
    for (unsigned i = 0; i < n; i++) {
      TSegment& s = * segments[i];
      s.tracks().remove(t);
    }

    delete & t;
  }

  void
  TConformalFinder::removeUsedSegments(const AList<TTrack> & tracks)
  {
    unsigned n = tracks.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * tracks[i];
      AList<TSegment> & segments = t.segments();
      AList<TSegment> toBeRemoved;
      unsigned nS = segments.length();
      for (unsigned j = 0; j < nS; j++) {
        TSegment& s = * segments[j];
        unsigned sId = s.superLayerId();
        unsigned as = sId % 2;
        unsigned id = sId / 2;

        //...Check used links...
        AList<TLink> links = Links(s, t);
        if (links.length() == 0) {
          s.tracks().remove(t);
          toBeRemoved.append(s);
#ifdef TRASAN_DEBUG
          std::cout << "!!! why this happends???" << std::endl;
          std::cout << "    no used link" << std::endl;
#endif
          continue;
        }

        //...Remove from lists...
        _allUnused[as][id].remove(s);

        //...Remove incoming links...
        const AList<TSegment> & outers = s.outerLinks();
        unsigned nO = outers.length();
//        std::cout << " >>> removing outer links" << std::endl;
//        s.dump("hits", "    To ");
        for (unsigned i = 0; i < nO; i++) {
          outers[i]->innerLinks().remove(s);
//      outers[i]->dump("hits", "    From ");
        }

        //...Remaining hits...
        AList<TLink> unused = s.links();
        unused.remove(links);
        s.remove(unused);
        unsigned nUnused = unused.length();

        //...Create new segment if too many remaining links...
        if (nUnused < _minNLinksForSegment) {
          for (unsigned k = 0; k < nUnused; k++) {
            _unused[as].append(unused[k]);
            _unused[2].append(unused[k]);
          }
        } else {
          TSegment* ss = new TSegment(unused);
          AList<TSegment> newSegments = ss->split();
          if (newSegments.length() == 0) {
            ss->solveDualHits();
            newSegments.append(ss);
            for (unsigned k = 0; k < nO; k++)
              outers[k]->innerLinks().append(ss);
          } else {
            delete ss;
          }
          _allUnused[as][id].append(newSegments);
          _allSegments[as][id].append(newSegments);
        }
      }
      segments.remove(toBeRemoved);
    }
  }

  void
  TConformalFinder::updateTLinks(AList<TTrack> & tracks)
  {
    unsigned n = tracks.length();
    for (unsigned i = 0; i < n; i++) {
      const AList<TLink> & links = tracks[i]->links();
      unsigned nL = links.length();
      for (unsigned j = 0; j < nL; j++) {
        TLink& l = * links[j];
        tracks[i]->approach(l);
      }
    }
  }

  int
  TConformalFinder::doit(const CAList<Belle2::TRGCDCWireHit> & axial,
                         const CAList<Belle2::TRGCDCWireHit> & stereo,
                         AList<TTrack> & tracks,
                         AList<TTrack> & tracks2D)
  {

    //...For debug...
    if (debugLevel() > 1)
      std::cout << name() << " ... processing"
                << " axial=" << axial.length()
                << ",stereo=" << stereo.length()
                << ",tracks=" << tracks.length()
                << std::endl;

#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage0 = "Conf";
    EnterStage(stage0);
#endif

    static bool first = true;
    if (first) {
      first = false;
//cnv int size;
      _s = 0;
    }

    //...Create TLinks with conformal position...
    TConformalFinder0::conformalTransformation(ORIGIN, axial, _allHits[0]);
    TConformalFinder0::conformalTransformation(ORIGIN, stereo, _allHits[1]);
    _allHits[2].append(_allHits[0]);
    _allHits[2].append(_allHits[1]);

    //...Select good hits...
    selectGoodHits();

    //...Segment finding...
    if (_perfectSegmentFinding)
      findSegmentsPerfect();
    else
      findSegments();

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage0);
    const std::string stage1 = "Conf::fastFinding";
    EnterStage(stage1);
#endif

    //...Fast finding...
    unsigned level = 0;
    _T0ResetDone = false;
    if (_fastFinder) {
      linkSegments(level);
      fastFinding2D(level);
      updateTLinks(_2DTracks);

      //...T0 reset here...
      if (_doT0Reset) {
        std::cout
            << "TConformalFinder ... T0 reset is done" << std::endl;
        _T0ResetDone = true;
        return 0;
      }

      fastFinding3D(level);
      updateTLinks(_2DTracks);
      updateTLinks(_3DTracks);

#ifdef TRASAN_WINDOW
      _rphiWindow.skip(false);
      displayTracks(_2DTracks, _allUnused, "all 2D after fast level 0");
      displayTracks(_3DTracks, _allUnused, "all 3D after fast level 0");
#endif
#ifdef TRASAN_WINDOW_GTK
      TWindowGTKConformal& w = Trasan::getTrasan()->w();
      w.clear();
//  w.skip(false);
      w.stage("Conformal Finder : fast finding (level 0) finished");
      w.append(_allHits[2], Gdk::Color("grey"));
      w.append(_hits[2], Gdk::Color("pink"));
      w.append(_2DTracks, Gdk::Color("red"));
      w.append(_3DTracks, Gdk::Color("green"));
      w.run();
#endif

      //...Fast finding again...
      level = 1;
      linkSegments(level);
      fastFinding2D(level);
      updateTLinks(_2DTracks);

#ifdef TRASAN_WINDOW
      _rphiWindow.skip(false);
#endif
      fastFinding3D(level);
      updateTLinks(_2DTracks);
      updateTLinks(_3DTracks);

#ifdef TRASAN_WINDOW
      _rphiWindow.skip(false);
      displayTracks(_2DTracks, _allUnused, "all 2D after fast level 1");
      displayTracks(_3DTracks, _allUnused, "all 3D after fast level 1");
#endif
#ifdef TRASAN_WINDOW_GTK
      w.clear();
//  w.skip(false);
      w.stage("Conformal Finder : fast finding (level 1) finished");
      w.append(_allHits[2], Gdk::Color("grey"));
      w.append(_hits[2], Gdk::Color("pink"));
      w.append(_2DTracks, Gdk::Color("red"));
      w.append(_3DTracks, Gdk::Color("green"));
      w.run();
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#3D tracks=" << _3DTracks.length() << ",#2D tracks="
              << _2DTracks.length() << std::endl;
    LeaveStage(stage1);
    const std::string stage2 = "Conf::fastFinding";
    EnterStage(stage2);
#endif

    //...Slow finding...
    if (_slowFinder) {
      level = 2;
      linkSegments(level);
      slowFinding2D(level);
      updateTLinks(_2DTracks);
#ifdef TRASAN_WINDOW
      _rphiWindow.skip(false);
//  _rphiWindow.skipAllWindow(false);
#endif
      fastFinding3D(level);
      updateTLinks(_2DTracks);
      updateTLinks(_3DTracks);

#ifdef TRASAN_WINDOW
      _rphiWindow.skip(false);
//  _rphiWindow.skipAllWindow(false);
      displayTracks(_2DTracks, _allUnused, "all 2D after slow level 2");
      displayTracks(_3DTracks, _allUnused, "all 3D after slow level 2");
#endif
#ifdef TRASAN_WINDOW_GTK
      TWindowGTKConformal& w = Trasan::getTrasan()->w();
      w.clear();
//  w.skip(false);
      w.stage("Conformal Finder : slow finding finished");
      w.append(_allHits[2], Gdk::Color("grey"));
      w.append(_hits[2], Gdk::Color("pink"));
      w.append(_2DTracks, Gdk::Color("red"));
      w.append(_3DTracks, Gdk::Color("green"));
      w.run();
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#3D tracks=" << _3DTracks.length() << ",#2D tracks="
              << _2DTracks.length() << std::endl;
    LeaveStage(stage2);
    const std::string stage3 = "Conf::termination";
    EnterStage(stage3);
#endif

    //...Mask hits with bad chisq...
    TTrackManager::maskBadHits(_3DTracks, _maxSigma2);

    //...Termination...
    tracks = _3DTracks;
    tracks2D = _2DTracks;
    ++_s->_nEvents;
    unsigned n3 = _3DTracks.length();
    for (unsigned i = 0; i < n3; i++)
      if (_3DTracks[i]->finder() & TrackFastFinder)
        ++_s->_nTracksFast3D;
      else if (_3DTracks[i]->finder() & TrackSlowFinder)
        ++_s->_nTracksSlow3D;
    unsigned n2 = _2DTracks.length();
    for (unsigned i = 0; i < n2; i++)
      if (_2DTracks[i]->finder() & TrackFastFinder)
        ++_s->_nTracksFast2D;
      else if (_2DTracks[i]->finder() & TrackSlowFinder)
        ++_s->_nTracksSlow2D;

    if (debugLevel() > 1) {
      std::cout << name() << " ... # 3D tracks = " << _3DTracks.length()
                << ", # 2D tracks = " << _2DTracks.length() << std::endl;
    }

    //...For debug...
    if (debugLevel() > 1)
      std::cout << name() << " ... processed"
                << " axial=" << axial.length()
                << ",stereo=" << stereo.length()
                << ",tracks=" << tracks.length()
                << std::endl;

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage3);
#endif

    return 0;
  }

  void

  TConformalFinder::fastFinding3D(unsigned level)
  {

#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "ConformalFastFinding3D level"
                              + itostring(level);
    EnterStage(stage);
#endif

    AList<TTrack> tracks3D;
    AList<TTrack> touched;
    AList<TSegment> bads;
    unsigned n = _2DTracks.length();
    for (unsigned i = 0; i < n; i++) {
      const TTrack& t = * _2DTracks[i];

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "==> fast 3D building : " << t.name() << std::endl;
      t.dump("hits sort flag", "    2D track hits=");
#endif
      AList<TSegment> segments = stereoSegments(t);
      AList<TSegment> badSegments;
      if (_stereoMode == 2)
        badSegments = stereoSegmentsFromBadHits(t);

#ifdef TRASAN_WINDOW
      displayStatus("Conf::fast3D ... seed");
      _rphiWindow.append(segments, leda_blue);
      _rphiWindow.append(badSegments, leda_red);
      _rphiWindow.oneShot(t, leda_green);
#endif

      //...Save a 2D track...
      TTrack* s = NULL;
      if (_stereoMode)
        s = _builder.buildStereoNew(t, segments, badSegments);
      else
        s = _builder.buildStereo(t, segments);
      HepAListDeleteAll(badSegments);

      if (! s) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "... 3D failure" << std::endl;
#endif
#ifdef TRASAN_WINDOW
        displayStatus("Conf::fastd3D ... 3D failed");
        _rphiWindow.append(segments, leda_blue);
        _rphiWindow.oneShot(t, leda_red);
#endif
        continue;
      }

      //...Quality check...
      if (! TTrackManager::goodTrack(* s)) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "... 3D failure (bad quality)" << std::endl;
#endif
#ifdef TRASAN_WINDOW
        displayStatus("Conf::fastd3D ... 3D failed (bad quality)");
        _rphiWindow.append(segments, leda_blue);
        _rphiWindow.oneShot(* s, leda_red);
#endif
        if (s->finder() & TrackFastFinder)
          ++_s->_nTracksFast2DBadQuality;
        else if (s->finder() & TrackSlowFinder)
          ++_s->_nTracksSlow2DBadQuality;

        deleteTrack(* s);
        continue;
      }

      //...New name...
      s->name(t.name() + "-3D");

#ifdef TRASAN_WINDOW
      displayStatus("Conf::fastd3D ... 3D ok");
      _rphiWindow.append(segments, leda_blue);
      _rphiWindow.oneShot(* s, leda_green);
#endif

      //...Salvage by segments...
      salvage(* s, 3, bads);
      tracks3D.append(s);
      touched.append(_2DTracks[i]);
      s->assign(CellHitConformalFinder);
      s->quality(0);

      //...Segment...
      static AList<TTrack> tmp;
      tmp.removeAll();
      tmp.append(s);
      removeUsedSegments(tmp);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... 3D finished : " << s->name() << std::endl;
      s->dump("detail", "    ");
#endif
#ifdef TRASAN_WINDOW
      displayStatus("Conf::fastd3D ... finished");
      _rphiWindow.oneShot(* s, leda_green);
#endif
    }

    // resolveSegments(tracks3D);
    _3DTracks.append(tracks3D);
    _2DTracks.remove(tracks3D);
    _2DTracks.remove(touched);
    for (unsigned i = 0; i < (unsigned) touched.length(); i++) {
      deleteTrack(* touched[i]);
      // saved[i]->fit();
    }

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

#ifdef TRASAN_WINDOW
  void
  TConformalFinder::displayStatus(const std::string& m) const
  {
    _rphiWindow.clear();
    _rphiWindow.text(m);
    _rphiWindow.append(_allHits[2], leda_pink);
    _rphiWindow.append(_hits[2], leda_cyan);
    _rphiWindow.append(_unused[2]);
    displayAppendSegments(_allSegments, leda_grey1);
    displayAppendSegments(_allUnused, leda_orange);
  }

  void
  TConformalFinder::displayAppendSegments(const AList<TSegment> a[2][6],
                                          leda_color c) const
  {
    for (unsigned i = 0; i < 2; i++) {
      for (unsigned j = 0; j < 6; j++) {
        const AList<TSegment> & segments = a[i][j];
        unsigned n = segments.length();
        for (unsigned k = 0; k < n; k++) {
          _rphiWindow.append(* segments[k], c);
        }
      }
    }
  }

  void
  TConformalFinder::displayTracks(const AList<TTrack> & l,
                                  const AList<TSegment> segments[2][6],
                                  const std::string& c) const
  {
    segments[0][0].length();
    displayStatus("Conf::display ... " + c);
    for (unsigned i = 0; i < (unsigned) l.length(); i++)
      _rphiWindow.append(* l[i], leda_green);
    _rphiWindow.wait();
  }
#endif

#ifdef TRASAN_WINDOW_GTK
  void
  TConformalFinder::displayStatus(const std::string& m) const
  {
//    _cw.clear();
//     _cw.text(m);
//     _cw.append(_allHits[2], leda_pink);
//     _cw.append(_hits[2], leda_cyan);
//     _cw.append(_unused[2]);
//     displayAppendSegments(_allSegments, leda_grey1);
//     displayAppendSegments(_allUnused, leda_orange);
  }
#endif

  bool
  TConformalFinder::quality2D(TTrack& t, unsigned level) const
  {
#ifdef TRASAN_WINDOW
    std::string s = "Conf::quality2D ... level " + itostring(level);
#endif

    //...Check # of segments(superlayers)...
    // unsigned nSuperLayers = NSuperLayers(t.links(), 2);
    unsigned nSuperLayers = TLink::nSuperLayers(t.links());
    if (nSuperLayers < _minNSegments) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... quality check : level=" << level << " : bad" << std::endl;
      std::cout << "    reason : # segments(superlayer) =" << nSuperLayers;
      std::cout << " < " << _minNSegments << std::endl;
#endif
#ifdef TRASAN_WINDOW
      s += " rejected because of few segments(superlayers)";
      displayStatus(s);
      _rphiWindow.oneShot(t, leda_red);
#endif
      return false;
    }
    if (level == 0) {
#ifdef TRASAN_WINDOW
      s += " ok : # of used segments(superlayers) = ";
      s += itostring(nSuperLayers);
      s += " > " + itostring(_minNSegments);
      displayStatus(s);
      _rphiWindow.oneShot(t, leda_green);
#endif

      return true;
    }

    //...Check superlayer usage...
    unsigned n3 = TLink::nSuperLayers(t.links(), 3);

    //...Check super layer usage...
    if (n3 != nSuperLayers) {
      unsigned sl = TLink::superLayer(t.links(), 2);
      // unsigned sl = TLink::superLayer(t.links(), 3);
      if (sl == 0) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "... quality check : level = " << level << " : bad";
        std::cout << std::endl;
        std::cout << "    reason : super layer pattern(n2) = 0 " << std::endl;
#endif
#ifdef TRASAN_WINDOW
        s += " rejected because of bad super-layer pattern(n2=0)";
        displayStatus(s);
        _rphiWindow.oneShot(t, leda_red);
#endif
        return false;
      }
      unsigned fl = 0;
      while ((sl & (1 << (fl * 2))) == 0) ++fl;
      bool empty = false;
      for (unsigned i = fl + 1; i < 6; i++) {
        bool thisLayer = (sl & (1 << (i * 2)));
        if (thisLayer && empty) {

#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... quality check : level = " << level;
          std::cout << " : bad" << std::endl;
          std::cout << "    reason : super layer pattern = ";
          for (unsigned j = 0; j < 6; j++) std::cout << (sl >> (j * 2)) % 2;
          std::cout << std::endl;
#endif
#ifdef TRASAN_WINDOW
          s += " rejected because of bad super-layer pattern";
          displayStatus(s);
          _rphiWindow.oneShot(t, leda_red);
#endif
          return false;
        }
        if (! thisLayer) empty = true;
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... quality check : level = " << level;
    std::cout << " : ok, super layer pattern = " << std::endl;
    std::string ptn;
    unsigned sl = TLink::superLayer(t.links(), 2);
    for (unsigned j = 0; j < 6; j++) {
      unsigned k = (sl >> (j * 2)) % 2;
      std::cout << k;
      if (k) ptn += "1";
      else   ptn += "0";
    }
    std::cout << std::endl;
#endif
#ifdef TRASAN_WINDOW
    s += " ok : super layer ptn = " + ptn;
    displayStatus(s);
    _rphiWindow.oneShot(t, leda_green);
#endif

    return true;
  }

  void

  TConformalFinder::fastFinding2D(unsigned level)
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
//cnv  unsigned nSuperLayers = cdc.nSuperLayers();

#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "2D level" + itostring(level);
    EnterStage(stage);
#endif

    AList<TSegment> * segments = & _allUnused[0][0];

    unsigned idBase = _2DTracks.length() + _3DTracks.length();
    unsigned nTrial = 0;
    unsigned seedLayer = 6;
    while ((seedLayer--) > 2) {

      //...Seed loop...
      AList<TTrack> trackCandidates;
      unsigned n = segments[seedLayer].length();
      for (unsigned i = 0; i < n; i++) {
        TSegment& seed = * segments[seedLayer][i];
        std::string name = "f" + itostring(nTrial + idBase);

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "super layer " << seedLayer << "," << i
                  << std::endl;
        seed.dump("link", Tab());
#endif

        //...Check uniqueness...
        AList<TSegment> seeds;
        if (NUniqueLinks(seed) > 1) {
          seeds = UniqueLinks(seed);
        } else if (NMajorLinks(seed) > 1) {
          seeds = MajorLinks(seed);
        } else {
          continue;
        }
        seeds.append(seed);

        //...Refine...
refine:

#ifdef TRASAN_WINDOW
        displayStatus("Conf::fast2D ... seed segments");
        _rphiWindow.oneShot(seeds, leda_green);
#endif
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "seed layer = " << seedLayer << "," << name
                  << std::endl;
#endif

        //...Try to build a track...
        TTrack* t = _builder.buildRphi(seeds);
        ++nTrial;

        //...Track check...
        if (! t) continue;
        t->name(name);
        bool ok = quality2D(* t, 0);
        if (! ok) {
          deleteTrack(* t);
          continue;
        }

        //...Bad segment rejection...
        AList<TSegment> bads = removeBadSegments(* t);
        if (bads.length()) {
          ok = quality2D(* t, 1);
          if (! ok) {
            seeds = refineSegments(* t);
            if ((unsigned) seeds.length() >= _minNSegments) {
              deleteTrack(* t);
              goto refine;
            }
            deleteTrack(* t);
            continue;
          }
        }

        //...Salvage by segments...
        salvage(* t, 1, bads);
        refineLinks(* t, 3);
        ok = quality2D(* t, 2);
        if (! ok) {
          deleteTrack(* t);
          continue;
        }

        //...Append segments...
        if (TLink::nSuperLayers(t->links()) < cdc.nAxialSuperLayers()) {
          t = expand(* t);
        }

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "2D finished:" << t->name() << std::endl;
        t->dump("hits sort flag pull", Tab(+1));
#endif
#ifdef TRASAN_WINDOW
        displayStatus("Conf::fast2D ... finished");
        _rphiWindow.oneShot(* t, leda_green);
#endif

        t->finder(TrackFastFinder);
        t->quality(TrackQuality2D);
        t->fitting(TrackFitGlobal);
        trackCandidates.append(t);
      }

      //...Resolve multi-track candidates...
      resolveSegments(trackCandidates);

      //...Remove used segments...
      removeUsedSegments(trackCandidates);
      _2DTracks.append(trackCandidates);
    }

    resolveHits(_2DTracks);

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  const Belle2::TRGCDCWire*
  TConformalFinder::conformal2Wire(const HepGeom::Point3D<double> & p)
  {
    std::cout << "p = " << p << std::endl;
    float r = sqrt(4. / p.mag2());
    float phi = p.phi();
    return Belle2::TRGCDC::getTRGCDC()->wire(r, phi);
  }

  AList<TSegment>
  TConformalFinder::pickUpSegmentsInConformal(float phi[12],
                                              float loadWidth,
                                              unsigned axialStereoSwitch) const
  {
    AList<TSegment> outList;
    HepGeom::Point3D<double> center(1., 1., 0.);
    center.setPhi(phi[0]);
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();

    //...Search for segments...
    for (unsigned sl = 0; sl < 2; sl++) {
      if (sl == 0) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        if (!(axialStereoSwitch & 2)) continue;
      }
      for (unsigned i = 0; i < 6; i++) {
        const AList<TSegment> & list = _allUnused[sl][i];
        unsigned n = list.length();
        for (unsigned j = 0; j < n; j++) {
          TSegment& s = * list[j];

          const HepGeom::Point3D<double> & p = s.position();
          if (center.dot(p) < 0.) continue;

          bool found = false;
          const AList<TLink> & inners = s.inners();
          unsigned m = inners.length();
          for (unsigned k = 0; k < m; k++) {
            float dPhi = phi[i * 2 + sl] -
                         inners[k]->position().phi();
            dPhi = fabs(fmod(dPhi, PI2));
            if (dPhi > (PI2 - dPhi)) dPhi = PI2 - dPhi;
            if (dPhi < cdc.cellWidth(i * 2 + sl) * loadWidth) {
              outList.append(s);
              found = true;
              break;
            }
          }

          if (found) continue;
          const AList<TLink> & outers = s.outers();
          m = outers.length();
          for (unsigned k = 0; k < m; k++) {
            float dPhi = phi[i * 2 + sl + 1] -
                         outers[k]->position().phi();
            dPhi = fabs(fmod(dPhi, PI2));
            if (dPhi > (PI2 - dPhi)) dPhi = PI2 - dPhi;
            if (dPhi < cdc.cellWidth(i * 2 + sl) * loadWidth) {
              outList.append(s);
              break;
            }
          }
        }
      }
    }

    return outList;
  }

  AList<TLink>
  TConformalFinder::pickUpLinksInConformal(float phi[12],
                                           float loadWidth,
                                           unsigned axialStereoSwitch) const
  {

    HepGeom::Point3D<double> center(1., 1., 0.);
    center.setPhi(phi[0]);

    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    AList<TLink> outList;
    unsigned nBad = _unused[2].length();
    for (unsigned i = 0; i < nBad; i++) {
      unsigned sl = _unused[2][i]->wire()->superLayerId();
      unsigned as = sl % 2;
      if (as == 0) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        if (!(axialStereoSwitch & 2)) continue;
      }

      const HepGeom::Point3D<double> & p = _unused[2][i]->position();
      if (center.dot(p) < 0.) continue;

      float dPhi = phi[sl] - _unused[2][i]->position().phi();
      dPhi = fabs(fmod(dPhi, PI2));
      if (dPhi > (PI2 - dPhi)) dPhi = PI2 - dPhi;
      if (dPhi < cdc.cellWidth(sl) * loadWidth) {
        outList.append(_unused[2][i]);
        continue;
      }
      dPhi = phi[sl + 1] - _unused[2][i]->position().phi();
      dPhi = fabs(fmod(dPhi, PI2));
      if (dPhi > (PI2 - dPhi)) dPhi = PI2 - dPhi;
      if (dPhi < cdc.cellWidth(sl) * loadWidth)
        outList.append(_unused[2][i]);
    }
    return outList;
  }

  int
  TConformalFinder::crossPointsInConformal(const AList<TSegment> & inList,
                                           HepGeom::Point3D<double> points[12]) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
//cnv    unsigned nSuperLayers = cdc.nSuperLayers();

    //...Parameters...
    static const float confRadius2[] = {4. / (8.3 *  8.3),
                                        4. / (16.9 * 16.9),
                                        4. / (21.7 * 21.7),
                                        4. / (31.3 * 31.3),
                                        4. / (36.1 * 36.1),
                                        4. / (44.1 * 44.1),
                                        4. / (50.5 * 50.5),
                                        4. / (58.5 * 58.5),
                                        4. / (64.9 * 64.9),
                                        4. / (72.9 * 72.9),
                                        4. / (79.3 * 79.3),
                                        4. / (87.4 * 87.4)
                                       };
    static std::vector<double> confRadius2vec;
    static bool init(true);
    if (init) {
      if (cdc.versionCDC() == "superb") {
        {
        }
      } else {
        for (int i = 0; i < 12; ++i) {
          confRadius2vec.push_back(confRadius2[i]);
        }
      }
      init = false;
//cnv      int ii=0;
      for (std::vector<double>::const_iterator it = confRadius2vec.begin();
           it != confRadius2vec.end(); ++it) {
//cnv   std::cout << "confRadius2[" << ii++ << "]=" << confRadius2[ii]
//         << " " << *it << std::endl;
      }
    }
    //...Get conformal points from segments as seeds...
    AList<TLink> forLine;
    HepGeom::Point3D<double> center;
    unsigned n = inList.length();
    for (unsigned i = 0; i < n; i++) {
      const HepGeom::Point3D<double> & p = inList[i]->position();
      forLine.append(new TLink(NULL, NULL, p));
      center += p;
    }
    center *= 1. / float(n);

    //...Make a line in the conformal plane...
    TLine line(forLine);
    int err = line.fit();
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "crossPoints ... failed due to line fit failure" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return -1;
    }

    //...Calculate points...
    for (unsigned i = 0; i < confRadius2vec.size(); i++) {
      Point3D p[2];
      float c = - line.a() * line.b();
      float d = 1. + line.a() * line.a();
      float e = sqrt(confRadius2vec[i] * d - line.b() * line.b());
      p[0].setX((c + e) / d);
      p[0].setY(line.a() * p[0].x() + line.b());
      p[1].setX((c - e) / d);
      p[1].setY(line.a() * p[1].x() + line.b());

      float mag0 = (center - p[0]).mag();
      float mag1 = (center - p[1]).mag();

      if (mag0 < mag1) points[i] = p[0];
      else             points[i] = p[1];
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "0,1  = " << p[0] << ", " << p[1] << std::endl;
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... center is : " << center << std::endl;
    std::cout << "... cross points are : " << std::endl;
    for (unsigned i = 0; i < confRadius2vec.size(); i++) {
      std::cout << "    " << i << " : " << points[i] << std::endl;
    }
#endif
#ifdef TRASAN_WINDOW
    displayStatus("Conf::crossPoints ... line to salvage for conf plane");
    _rphiWindow.append(inList, leda_green);
    _rphiWindow.oneShot(line, leda_blue);
#endif

    HepAListDeleteAll(forLine);
    return 0;
  }

  AList<TSegment>
  TConformalFinder::stereoSegments(const TTrack& t) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
//cnv  unsigned nSuperLayers = cdc.nSuperLayers();

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... finding stereo segments" << std::endl;
#endif

    const AList<TSegment> & bases = (const AList<TSegment> &) t.segments();
    AList<TSegment> seeds;
    unsigned n = bases.length();
    if (n == 0) return seeds;

    //...Calculate cross points in the conformal plane...
    TPoint2D points[12];
    int err(0);
#if defined(BELLE_DEBUG)
    try {
#endif
      err = crossPointsBySuperLayer(t, points);
      if (err) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "... stereo segment finding failed" << std::endl;
#endif
        return seeds;
      }
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "TConformalFinder::stereoSegments::helix is invalid" << std::endl;
      return seeds;
    }
#endif

    //...Pick up segments...
    // return pickUpSegments(points, float(_stereoLoadWidth), 2);
    AList<TSegment> list = pickUpSegments(points, float(_stereoLoadWidth), 2);

    //...Save direction of a segment of axial layers...
    TPoint2D dir[6];
    for (unsigned i = 0; i < n; i++) {
      const TSegment& s = * bases[i];
      unsigned sl = s.superLayerId() / 2;
      dir[sl] = TPoint2D(s.direction());
    }

    //...Cal. direction if empty...
    const int n6 = cdc.nAxialSuperLayers();
    for (unsigned i = 0; i < (unsigned) n6; i++) {
      if (dir[i].mag() < .5) {
        unsigned j = i;
        while ((j < (unsigned) n6) && (dir[j].mag() < .5))
          ++j;
        if (j > (unsigned)(n6 - 1)) j = n6 - 1;
        if (dir[j].mag() < .5) {
          j = i;
          while ((j > 0) && (dir[j].mag() < .5))
            --j;
        }
        dir[i] = dir[j];
      }
    }

    //...Remove bad segments...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... direction :" << std::endl;
    for (unsigned i = 0; i < unsigned(n6); i++)
      std::cout << "        " << i << " : " << dir[i] << std::endl;
    std::cout << "    ... direction cos :" << std::endl;
#endif
    AList<TSegment> badList;
    unsigned nL = list.length();
    for (unsigned i = 0; i < nL; i++) {
      TSegment& s = * list[i];
      unsigned sl = s.superLayerId() / 2;
      TPoint2D sDir = s.direction();
      if (dir[sl].dot(sDir) < 0.7) badList.append(s);
      if (dir[sl + 1].dot(sDir) < 0.7) badList.append(s);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "        " << dir[sl].dot(sDir) << ", ";
      std::cout << dir[sl + 1].dot(sDir) << " : ";
      s.dump("hits sort");
#endif
    }

    //...Width cut...
//      for (unsigned i = 0; i < nL; i++) {
//    TSegment & s = * list[i];
//    unsigned width = s.width();
//    if (width > 2) badList.append(s);
//      }
    list.remove(badList);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... bad segments :" << std::endl;
    for (unsigned i = 0; i < (unsigned) badList.length(); i++)
      badList[i]->dump("hits sort", "        ");
#endif

    return list;
  }

  Vector3D
  TConformalFinder::direction(const TSegment& segment) const
  {
    AList<TLink> forLine;
    const TSegment* s = & segment;
    while (1) {
      if (s->outerLinks().length() != 1) break;
      const TSegment* o = s->outerLinks()[0];
      const HepGeom::Point3D<double> & p = o->position();
      forLine.append(new TLink(NULL, NULL, p));
      s = o;
    }

    if (forLine.length() == 0)
      return segment.direction();
    else if (forLine.length() < 2) {
      HepAListDeleteAll(forLine);
      return segment.direction();
    }

    //...Make a line in the conformal plane...
    TLine line(forLine);
    int err = line.fit();
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "direction ... failed due to line fit failure" << std::endl;
#endif
      HepAListDeleteAll(forLine);
      return segment.direction();
    }

    HepAListDeleteAll(forLine);
    Vector3D tmp(-1., -(line.a() + line.b()), 0.);
    tmp.unit();
    return tmp;
  }


  void
  TConformalFinder::salvage(TTrack& track,
                            unsigned asSwitch,
                            const AList<TSegment> & bads) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "ConfSalvage";
    EnterStage(stage);
    std::cout << Tab() << "salvaging in real plane" << std::endl;
#endif

    //...Calculate cross points...
    TPoint2D points[12];
#if defined(BELLE_DEBUG)
    try {
#endif
      int err = crossPointsBySuperLayer(track, points);
      if (err) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "salvaging failed in calculation of intersection"
                  << std::endl;
        LeaveStage(stage);
#endif
        return;
      }
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "TConformalFinder::salvage::helix is invalid"
                << std::endl;
      return;
    }
#endif

    //...Pick up segments...
    AList<TSegment> toBeChecked = pickUpSegments(points,
                                                 float(_salvageLoadWidth),
                                                 asSwitch);
    toBeChecked.remove(bads);
    toBeChecked.remove(track.segments());
    toBeChecked = trackSide(track, toBeChecked);

    //...Pick up links...
    AList<TLink> links;
    unsigned nB = toBeChecked.length();
    for (unsigned i = 0; i < nB; i++) {
      const AList<TLink> & tmp = toBeChecked[i]->links();
      unsigned nL = tmp.length();
      for (unsigned j = 0; j < nL; j++) {
        if (tmp[j]->hit()->track()) continue;
        links.append(tmp[j]);
      }
    }

    //...Search in bad hits...
    AList<TLink> all = pickUpLinks(points, float(_salvageLoadWidth), asSwitch);
    all = trackSide(track, all);
    all.remove(track.links());
    links.append(all);

#ifdef TRASAN_WINDOW
    AList<TLink> tmp = links;
#endif

    //...Ask builder...
    _builder.TBuilder::salvage(track, links);

    //...Check used segments...
//  const AList<TLink> & usedLinks = track.links();
    for (unsigned i = 0; i < nB; i++) {
      TSegment& segment = * toBeChecked[i];
      AList<TLink> used = Links(segment, track);
      if (used.length() == 0) continue;

      track.segments().append(segment);
      segment.tracks().append(track);
    }

#ifdef TRASAN_WINDOW
    displayStatus("Conf::salvage ... results");
    TTrackBase y(tmp);
    _rphiWindow.append(y, leda_red);
    _rphiWindow.append(toBeChecked, leda_red);
    _rphiWindow.oneShot(track, leda_green);
#endif

    //...Termination...
#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif

    return;
  }

  AList<TSegment>
  TConformalFinder::pickUpSegments(const TPoint2D x[12],
                                   float loadWidth,
                                   unsigned axialStereoSwitch) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
//cnv    unsigned nSuperLayers = cdc.nSuperLayers();
    static const TPoint2D O(0., 0.);
    AList<TSegment> outList;

    //...Search for segments...
    for (unsigned sl = 0; sl < 2; sl++) {
      unsigned nMax = cdc.nAxialSuperLayers();
      if (sl == 0) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        nMax = cdc.nStereoSuperLayers();
        if (!(axialStereoSwitch & 2)) continue;
      }
      for (unsigned i = 0; i < nMax; i++) {
        if (x[i * 2 + sl] == O) continue;
        float a = cdc.cellWidth(i * 2 + sl) * loadWidth;
        float phi0 = x[i * 2 + sl].phi();
        float phi1 = x[i * 2 + sl + 1].phi();
        float phiC = (phi0 + phi1) / 2.;
        float phiD = fabs(phi0 - phiC);
        if (phiD > M_PI_2) {
          phiC += M_PI;
          phiD = M_PI - phiD;
        }
        phiD += a;
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "    pickUpSegments ... phi0,1,C,D=" << phi0 << ",";
        std::cout << phi1 << "," << phiC << "," << phiD << std::endl;
#endif

        const AList<TSegment> & list = _allUnused[sl][i];
        unsigned n = list.length();
        for (unsigned j = 0; j < n; j++) {
          TSegment& s = * list[j];

#ifdef TRASAN_DEBUG_DETAIL
          s.dump("hits sort", "        ");
#endif

          bool found = false;
          const AList<TLink> & inners = s.inners();
          unsigned m = inners.length();
          for (unsigned k = 0; k < m; k++) {
            float phi = inners[k]->position().phi();
            if (phi < 0.) phi += PI2;
            float dPhi = fabs(phi - phiC);
            if (dPhi > M_PI) dPhi = PI2 - dPhi;
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << "            " << inners[k]->wire()->name();
            std::cout << ", phi,dPhi=" << phi << "," << dPhi << std::endl;
#endif
            if (dPhi < phiD) {
              outList.append(s);
              found = true;
              break;
            }
          }

          if (found) continue;
          const AList<TLink> & outers = s.outers();
          m = outers.length();
          for (unsigned k = 0; k < m; k++) {
            float phi = outers[k]->position().phi();
            if (phi < 0.) phi += PI2;
            float dPhi = fabs(phi - phiC);
            if (dPhi > M_PI) dPhi = PI2 - dPhi;
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << "            " << outers[k]->wire()->name();
            std::cout << ", phi,dPhi=" << phi << "," << dPhi << std::endl;
#endif
            if (dPhi < phiD) {
              outList.append(s);
              found = true;
              break;
            }
          }
        }
      }
    }

    return outList;
  }

  AList<TLink>
  TConformalFinder::pickUpLinks(const TPoint2D x[12],
                                float loadWidth,
                                unsigned axialStereoSwitch) const
  {

    static const TPoint2D O(0., 0.);
    AList<TLink> outList;

    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    unsigned nBad = _unused[2].length();
    for (unsigned i = 0; i < nBad; i++) {
      unsigned sl = _unused[2][i]->wire()->superLayerId();
      unsigned as = sl % 2;
      if (as == 0) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        if (!(axialStereoSwitch & 2)) continue;
      }

      float a = cdc.cellWidth(sl) * loadWidth;
      float phi0 = x[sl].phi();
      float phi1 = x[sl + 1].phi();
      float phi = _unused[2][i]->position().phi();
      if (phi < 0.) phi += PI2;
      if (phi1 < phi0) {
        phi1 = phi0;
        phi0 = x[sl + 1].phi();
      }
      float dPhi = phi1 - phi0;
      if (dPhi < M_PI) {
        phi0 -= a;
        phi1 += a;
        if (phi > phi0 && phi < phi1) outList.append(_unused[2][i]);
      } else {
        phi0 += a;
        phi1 -= a;
        if (phi < phi0 || phi > phi1) outList.append(_unused[2][i]);
      }
    }
    return outList;
  }

  void

  TConformalFinder::slowFinding2D(unsigned level)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "ConformalSlowFinding2D level"
                              + itostring(level);
    EnterStage(stage);
#endif

    AList<TSegment> * segments = & _allUnused[0][0];

    unsigned id = _2DTracks.length() + _3DTracks.length();
    unsigned seedLayer = 6;
    while (seedLayer--) {

      //...Seed loop...
      AList<TSegment> tmp = segments[seedLayer];
      unsigned n = tmp.length();
      for (unsigned i = 0; i < n; i++) {
        AList<TTrack> trackCandidates;
        TSegment& current = * tmp[i];
        if (current.links().length() < 3) continue;
        if (current.innerLinks().length() != 1) continue;
        if (current.innerLinks()[0]->links().length() < 3) continue;
        AList<TSegment> seeds;
        seeds.append(current);
        seeds.append(current.innerLinks()[0]);

        std::string name = "s" + itostring(id);

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "==> slow building : " << name << std::endl;
#endif

        //...Try to build a track...
        TTrack* t = expand(seeds);
        if (t) {
          AList<TSegment> bads;
          t->name(name);
          salvage(* t, 1, bads);
          if (! trackQuality(* t)) {
            deleteTrack(* t);
            continue;
          }
          t->finder(TrackSlowFinder);
          t->quality(TrackQuality2D);
          t->fitting(TrackFitGlobal);
          trackCandidates.append(t);

#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... 2D finished : " << t->name() << std::endl;
          t->dump("hits sort flag pull", "    ");
#endif
#ifdef TRASAN_WINDOW
          displayStatus("Conf::expand ... finished");
          _rphiWindow.oneShot(* t, leda_green);
#endif
          removeUsedSegments(trackCandidates);
          _2DTracks.append(trackCandidates);
          id = _2DTracks.length() + _3DTracks.length();
        }
      }

      //...Resolve multi-track candidates...
      // resolveSegments(trackCandidates);

      //...Remove used segments...
    }

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  TTrack*
  TConformalFinder::expand(AList<TSegment> & seeds) const
  {
#ifdef TRASAN_WINDOW
    displayStatus("Conf::expand ... seeds");
    _rphiWindow.oneShot(seeds, leda_green);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... expand : # of seeds = " << seeds.length() << std::endl;
#endif
    TTrack* t = NULL;
#if defined(BELLE_DEBUG)
    try {
#endif
      if (seeds.length() > 2) {
        TTrack* t = _builder.buildRphi(seeds);
        if (t) {
          if (! trackQuality(* t)) {
            deleteTrack(* t);
            t = NULL;
          }
        }
      }
      if (t == NULL) {
        AList<TLink> links = Links(seeds);
        TCircle c(links);
        c.fit();
        t = new TTrack(c);
        t->fit();
        t->segments().append(seeds);
      }
      return expand(* t);
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "TConformalFinder::helix is invalid" << std::endl;
      return NULL;
    }
#endif
  }

  TTrack*
  TConformalFinder::expand(TTrack& ti) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
    unsigned nSuperLayers = cdc.nSuperLayers();
#ifdef TRASAN_WINDOW
    displayStatus("Conf::expand ... seed track");
    _rphiWindow.oneShot(ti, leda_green);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... expand : by a track" << std::endl;
#endif

    TTrack* t = & ti;
    AList<TSegment> seeds = t->segments();
    unsigned nSegments = seeds.length();
    unsigned nCores = t->nCores();
//  unsigned nTrial = 0;
    while (1) {

      //...Target superlayer...
      unsigned sl = TLink::superLayer(t->cores());
      unsigned inner;
      unsigned outer;
      targetSuperLayer(sl, inner, outer);
      unsigned target = inner;
      if (target == nSuperLayers)
        target = outer;
      if (target == nSuperLayers)
        break;

      //...Get target segments...
      AList<TSegment> segments = targetSegments(* t, target);
      if (segments.length() == 0) {
        if (inner > 0) target = inner - 2;
        if (target == (nSuperLayers - 2)) target = outer + 2;
        if (target == (nSuperLayers + 2)) break;
        segments = targetSegments(* t, target);
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... expand : segments to be checked = ";
      std::cout << segments.length() << std::endl;
#endif
#ifdef TRASAN_WINDOW
      displayStatus("Conf::expand ... candidate segments");
      _rphiWindow.append(segments, leda_blue);
      _rphiWindow.oneShot(* t, leda_green);
#endif

      //...Create candidate tracks...
      unsigned n = segments.length();
      AList<TTrack> tracks;
      for (unsigned i = 0; i < n; i++) {
        AList<TSegment> seed0 = seeds;
        seed0.append(segments[i]);
        TTrack* t0 = _builder.buildRphi(seed0);
        if (! t0) continue;
        if (((unsigned) t0->segments().length() <= nSegments) ||
            (t0->nCores() <= nCores)               ||
            (TLink::superLayer(t0->cores()) <= sl)) {
          deleteTrack(* t0);
          continue;
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... expand : candidate deleted" << std::endl;
#endif
        }
        tracks.append(t0);
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... expand : target superlayer = " << target;
      std::cout << ", # of track candidates = " << tracks.length()
                << std::endl;
#endif

      //...Expand by links...
      if (tracks.length() == 0) {
        AList<TLink> links = targetLinks(* t, target);
        _builder.TBuilder::salvage(* t, links);
        unsigned newSl = TLink::superLayer(t->cores());
        if (newSl & (1 << (target))) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... expand : links to be appended = ";
          std::cout << links.length() << std::endl;
#endif
#ifdef TRASAN_WINDOW
          displayStatus("Conf::expand ... candidate links");
          _rphiWindow.append(links, leda_blue);
          _rphiWindow.oneShot(* t, leda_green);
#endif
          TTrack* t0 = new TTrack(* t);
          if (! t0) break;
          if ((t0->nCores() <= nCores) ||
              (TLink::superLayer(t0->cores()) <= sl)) {
            deleteTrack(* t0);
            break;
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << "... expand : candidate deleted" << std::endl;
#endif
          }
          tracks.append(t0);
        } else
          break;
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... expand : # of track candidates = " << tracks.length()
                << std::endl;
#endif

      //...Select best...
      unsigned nTracks = tracks.length();
      if (nTracks == 0) break;
      TTrack* tNew = tracks[0];
      unsigned nBestCores = tNew->nCores();
      for (unsigned i = 1; i < nTracks; i++) {
        if (tracks[i]->nCores() > nBestCores) {
          deleteTrack(* tNew);
          tNew = tracks[i];
          nBestCores = tNew->nCores();
        } else {
          deleteTrack(* tracks[i]);
        }
      }
      nSegments = tNew->segments().length();
      nCores = nBestCores;
      seeds = tNew->segments();

      //...Quality check...
      if (! trackQuality(* tNew)) {
        deleteTrack(* tNew);
        break;
      }
      deleteTrack(* t);
      t = tNew;
    }

#ifdef TRASAN_DEBUG_DETAIL
    if (t == & ti) std::cout << "... expand : failed" << std::endl;
    else std::cout << "...  expand : track expanded" << std::endl;
#endif
#ifdef TRASAN_WINDOW
    displayStatus("Conf::expand ... finished");
    _rphiWindow.oneShot(* t, leda_green);
#endif

    return t;
  }

  void
  TConformalFinder::targetSuperLayer(unsigned sl,
                                     unsigned& inner,
                                     unsigned& outer) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
    unsigned nSuperLayers = cdc.nSuperLayers();
    inner = nSuperLayers;
    outer = nSuperLayers;
    bool innerFound = false;
    bool outerFound = false;
    for (unsigned i = 0; i < cdc.nAxialSuperLayers(); i++) {
      if (! innerFound) {
        if (sl & (1 << (i * 2))) innerFound = true;
        else inner = i * 2;
      }
      if (! outerFound) {
        if (sl & (1 << ((nSuperLayers - 1) - i * 2))) outerFound = true;
        else outer = (nSuperLayers - 1) - i * 2;
      }
    }
  }

  AList<TSegment>
  TConformalFinder::targetSegments(const TTrack& t, unsigned sl) const
  {
    AList<TSegment> targets;

    //...Calculate cross points...
    TPoint2D points[12];
    int err = crossPointsBySuperLayer(t, points);
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... no target found" << std::endl;
#endif
      return targets;
    }

    //...Pick up segments...
    AList<TSegment> toBeChecked = pickUpSegments(points, 3, 1);
    unsigned n = toBeChecked.length();
    for (unsigned i = 0; i < n; i++) {
      if (toBeChecked[i]->superLayerId() == sl)
        targets.append(toBeChecked[i]);
    }

    return targets;
  }

  AList<TLink>
  TConformalFinder::targetLinks(const TTrack& t, unsigned sl) const
  {
    AList<TLink> targets;

    //...Calculate cross points...
    TPoint2D points[12];
    int err = crossPointsBySuperLayer(t, points);
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... no target found" << std::endl;
#endif
      return targets;
    }

    //...Pick up segments...
    AList<TLink> toBeChecked = pickUpLinks(points, 3, 1);
    unsigned n = toBeChecked.length();
    for (unsigned i = 0; i < n; i++) {
      if (toBeChecked[i]->wire()->superLayerId() == sl)
        targets.append(toBeChecked[i]);
    }

    return targets;
  }

  AList<TSegment>
  TConformalFinder::refineSegments(const TTrack& t) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
//cnv  unsigned nSuperLayers = cdc.nSuperLayers();
    const AList<TSegment> & original = t.segments();
    AList<TSegment> outList;
    unsigned n = original.length();
    for (unsigned i = 0; i < n; i++) {
      AList<TLink> tmp = Links(* original[i], t);
      if ((unsigned) tmp.length() >= _minNLinksForSegmentInRefine)
        outList.append(original[i]);
    }
    unsigned sl = SuperLayer(outList);
    unsigned nCMax = 0;
    unsigned nStart = 0;
    unsigned nC = 0;
    unsigned nS = 0;
    for (unsigned i = 0; i < cdc.nAxialSuperLayers(); i++) {
      if (sl & (1 << (i * 2))) {
        ++nC;
        if (nC == 1) nS = i;
        if (nC > nCMax) {
          nCMax = nC;
          nStart = nS;
        }
      } else {
        nC = 0;
      }
    }
    nStart *= 2;
    nCMax *= 2;

    outList.removeAll();
    if (nCMax >= _minNSegments) {
      for (unsigned i = 0; i < n; i++) {
        unsigned id = original[i]->superLayerId();
        if ((id >= nStart) && (id < nStart + nCMax))
          outList.append(original[i]);
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    const unsigned nSuperLayers = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
    std::cout << "... refine segments : orignal sl = ";
    Belle2::TRGUtilities::bitDisplay(sl, nSuperLayers, 0);
    std::cout << ", output sl = ";
    Belle2::TRGUtilities::bitDisplay(SuperLayer(outList), nSuperLayers, 0);
    std::cout << std::endl;
#endif

    return outList;
  }

  bool
  TConformalFinder::trackQuality(const TTrack& t) const
  {
    unsigned n3 = TLink::nSuperLayers(t.links(), 3);

#ifdef TRASAN_WINDOW
    unsigned n1 = NSuperLayers(t.links());
    std::cout << "... trackQuality : n1,n3,nMissing=" << n1 << "," << n3;
    std::cout << "," << NMissingAxialSuperLayers(t.links()) << std::endl;
#endif
#ifdef TRASAN_WINDOW
    displayStatus("trackQuality");
    if ((n3 < 2) || (NMissingAxialSuperLayers(t.links()) > 1))
      _rphiWindow.oneShot(t, leda_red);
    else
      _rphiWindow.oneShot(t, leda_green);
#endif
    if (n3 < 2) return false;
    if (TLink::nMissingAxialSuperLayers(t.links()) > 1) return false;

    return true;
  }

  void
  TConformalFinder::refineLinks(TTrack& t, unsigned minN) const
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
    unsigned nSuperLayers = cdc.nSuperLayers();
    const AList<TLink> & links = t.links();
    AList<TLink> *sl = new AList<TLink> [nSuperLayers];
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++)
      sl[links[i]->wire()->superLayerId()].append(links[i]);
    AList<TLink> toBeRemoved;
    for (unsigned i = 0; i < nSuperLayers; i++)
      if ((unsigned) sl[i].length() < minN)
        toBeRemoved.append(sl[i]);
    t.remove(toBeRemoved);
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... refining links : removed links = " << toBeRemoved.length();
    std::cout << std::endl;
#endif
    delete [] sl;
  }

  AList<TLink>
  TConformalFinder::trackSide(const TTrack& t, const AList<TLink> & a) const
  {
    static const TPoint2D o(0., 0.);
    TPoint2D c = t.center();
    TPoint2D co = - c;

    AList<TLink> tmp;
    unsigned n = a.length();
    for (unsigned i = 0; i < n; i++) {
      TPoint2D x = a[i]->wire()->xyPosition();
      if (co.cross(x - c) * t.charge() < 0.)
        tmp.append(a[i]);
    }
    return tmp;
  }

  AList<TSegment>
  TConformalFinder::trackSide(const TTrack& t,
                              const AList<TSegment> & a) const
  {
    static const TPoint2D o(0., 0.);
    TPoint2D c = t.center();
    TPoint2D co = - c;

    AList<TSegment> tmp;
    unsigned n = a.length();
    for (unsigned i = 0; i < n; i++) {
      const AList<TLink> & b = a[i]->links();
      bool bad = false;
      unsigned nB = b.length();
      for (unsigned j = 0; j < nB; j++) {
        TPoint2D x = b[j]->wire()->xyPosition();
        if (co.cross(x - c) * t.charge() > 0.) {
          bad = true;
          break;
        }
      }
      if (bad) continue;
      tmp.append(a[i]);
    }
    return tmp;
  }

  void
  TConformalFinder::resolveHits(AList<TTrack> & tracks) const
  {
    unsigned nTracks = tracks.length();
    if (nTracks < 2) return;

    for (unsigned i = 0; i < nTracks - 1; i++) {
      TTrack& t0 = * tracks[i];
      const AList<TLink> & links0 = t0.links();
      unsigned n0 = links0.length();
      AList<TLink> remove0;
      for (unsigned j = i + 1; j < nTracks; j++) {
        TTrack& t1 = * tracks[j];
        const AList<TLink> & links1 = t1.links();
//      unsigned n1 = links1.length();
        AList<TLink> remove1;

        //...Check overlap...
        for (unsigned k = 0; k < n0; k++) {
          TLink& l = * links0[k];

          //...Decide which is better...
          if (links1.hasMember(l)) {
            TLink l0(NULL, l.hit());
            TLink l1(NULL, l.hit());
            t0.approach(l0);
            t1.approach(l1);
            if (l0.pull() > l1.pull())
              remove0.append(l);
            else
              remove1.append(l);
          }
        }

        if (remove1.length()) {
          t1.remove(remove1);
          t1.fit();
        }
      }

      if (remove0.length()) {
        t0.remove(remove0);
        t0.fit();
      }
    }
  }

  AList<TSegment>
  TConformalFinder::stereoSegmentsFromBadHits(const TTrack& t) const
  {

    // no_superlyr
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
//cnv    unsigned nSuperLayers = cdc.nSuperLayers();

    AList<TSegment> output;
    for (unsigned i = 0; i < cdc.nStereoSuperLayers(); i++)
      output.append(new TSegment());

    //...Calculate cross points...
    TPoint2D points[12];
    int err = crossPointsBySuperLayer(t, points);
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... conf::stereoSegBadHits : "
                << "error in calculation of intersection" << std::endl;
#endif
      return output;
    }

    static const TPoint2D O(0., 0.);
    unsigned nBad = _unused[2].length();
    for (unsigned i = 0; i < nBad; i++) {
      unsigned sl = _unused[2][i]->wire()->superLayerId();
      unsigned as = sl % 2;
      if (as == 0) continue;

      float a = cdc.cellWidth(sl) * _salvageLoadWidth;
      float phi0 = points[sl].phi();
      float phi1 = points[sl + 1].phi();
      float phi = _unused[2][i]->position().phi();
      if (phi < 0.) phi += PI2;
      if (phi1 < phi0) {
        phi1 = phi0;
        phi0 = points[sl + 1].phi();
      }
      float dPhi = phi1 - phi0;
      if (dPhi < M_PI) {
        phi0 -= a;
        phi1 += a;
        if (phi > phi0 && phi < phi1)
          output[sl / 2]->append(* _unused[2][i]);
      } else {
        phi0 += a;
        phi1 -= a;
        if (phi < phi0 || phi > phi1)
          output[sl / 2]->append(* _unused[2][i]);
      }
    }

    return output;
  }

  void
  TConformalFinder::findSegmentsPerfect(void)
  {
    // no_superlyr
    const Belle2::TRGCDC& cdc(*Belle2::TRGCDC::getTRGCDC());
    unsigned nSuperLayers = cdc.nSuperLayers();

    //...Create lists of links for each super layer...
    AList<TLink> *links = new AList<TLink>[nSuperLayers];
    unsigned nHits = _hits[2].length();
    for (unsigned i = 0; i < nHits; i++) {
      TLink& l = * _hits[2][i];
      links[l.wire()->superLayerId()].append(l);
    }

    //...MC tracks...
    const unsigned nHep = Belle2::TRGCDCTrackMC::list().size();

    //...Loop over each super layer...
    for (unsigned i = 0; i < nSuperLayers; i++) {
      unsigned superLayerId = i / 2;
      unsigned id = i % 2;

      //...Counters...
      AList<TLink> ** seeds;
      if (NULL == (seeds =
                     (AList<TLink> **) malloc(nHep * sizeof(AList<TLink> *)))) {
//    perror("$Id: TConformalFinder.cc 11152 2010-04-28 01:24:38Z yiwasaki $:seeds:malloc");
        exit(1);
      }
      for (unsigned j = 0; j < nHep; j++)
        seeds[j] = NULL;

      //...Hit loop...
      unsigned n = links[i].length();
      for (unsigned j = 0; j < n; j++) {
        TLink& l = * links[i][j];
        const Belle2::TRGCDCWireHitMC* mc = l.hit()->mc();
        if (! l.hit()->mc()) {
          std::cout << "TConformalFinder::findSegmentsPerfect !!! "
                    << "no MC info. found ... aborted" << std::endl;
          return;
        }
        const unsigned id = mc->hep()->id();

        if (! seeds[id])
          seeds[id] = new AList<TLink>();
        seeds[id]->append(l);
      }

      //...Create segments...
      AList<TSegment> segments;
      for (unsigned j = 0; j < nHep; j++) {
        if (! seeds[j]) continue;

        const unsigned length = seeds[j]->length();
        if (length < _minNLinksForSegment) continue;
        if (length > _maxNLinksForSegment) continue;
        if (TLink::width(* seeds[j]) > _maxWidthForSegment) continue;

        TSegment& s = * new TSegment();
        s.append(* seeds[j]);
        segments.append(s);
      }
      _allSegments[id][superLayerId].append(segments);
      _allUnused[id][superLayerId] = _allSegments[id][superLayerId];

      //...Clear...
      for (unsigned j = 0; j < nHep; j++) {
        if (seeds[j])
          delete seeds[j];
      }
      free(seeds);
    }
    delete [] links;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "... segment finding(perfect) finished" << std::endl;
#endif
  }

} // namespace Belle
