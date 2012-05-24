#define OPTNK
//-----------------------------------------------------------------------------
// $Id: TTrack.cc 10688 2008-10-14 18:23:50Z hitoshi $
//-----------------------------------------------------------------------------
// Filename : TTrack.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to represent a track in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.97  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.96  2005/03/11 03:57:52  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.95  2003/12/25 13:03:16  yiwasaki
// minor fixes
//
// Revision 1.94  2003/12/25 12:03:35  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.93  2002/02/22 06:37:43  katayama
// Use __sparc
//
// Revision 1.92  2002/02/21 23:49:16  katayama
// For -ansi and other warning flags
//
// Revision 1.91  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.90  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.89  2001/05/07 20:51:19  yiwasaki
// <float.h> included for linux
//
// Revision 1.88  2001/04/25 02:36:00  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.86  2001/04/11 01:09:11  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.85  2001/02/07 21:18:26  yiwasaki
// Trasan 2.27 : pivot of 2D track is moved to (x,y,0)
//
// Revision 1.84  2001/01/30 04:54:14  yiwasaki
// Trasan 2.21 release : bug fixes
//
// Revision 1.83  2000/10/05 23:54:26  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.82  2000/08/31 23:51:47  yiwasaki
// Trasan 2.04 : pefect finder added
//
// Revision 1.81  2000/07/31 04:47:32  yiwasaki
// Minor changes
//
// Revision 1.80  2000/04/25 02:53:25  yiwasaki
// Trasan 2.00rc31 : Definition of nhits, nster, and ndf in RECCDC_WIRHIT
//
// Revision 1.79  2000/04/13 02:53:40  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.78  2000/04/11 13:05:47  katayama
// Added std:: to cout, cerr, std::endl etc.
//
// Revision 1.77  2000/04/07 14:24:39  yiwasaki
// blank output fixed
//
// Revision 1.76  2000/04/04 07:40:07  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.75  2000/03/31 08:33:23  yiwasaki
// Trasan 2.00RC24 : fix again
//
// Revision 1.74  2000/03/31 07:21:13  yiwasaki
// PM finder updates from J.Tanaka, trkmgr bug fixes
//
// Revision 1.73  2000/03/30 08:30:32  katayama
// mods for CC5.0
//
// Revision 1.72  2000/03/30 06:40:07  yiwasaki
// debug info. added
//
// Revision 1.71  2000/03/24 10:22:55  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.70  2000/03/23 13:27:54  yiwasaki
// Trasan 2.00RC18 : bug fixes
//
// Revision 1.69  2000/03/21 07:01:27  yiwasaki
// tmp updates
//
// Revision 1.68  2000/02/29 08:39:01  yiwasaki
// minor changes
//
// Revision 1.67  2000/02/25 12:55:42  yiwasaki
// Trasan 2.00RC10 : stereo improved
//
// Revision 1.66  2000/02/25 08:09:56  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.65  2000/02/24 06:19:29  yiwasaki
// Trasan 2.00RC7 : bug fix again
//
// Revision 1.64  2000/02/17 13:24:20  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.63  2000/02/15 13:46:44  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.62  2000/02/06 22:55:40  katayama
// Missing ifdef, missing type
//
// Revision 1.61  2000/02/01 11:24:43  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.60  2000/01/28 06:30:25  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.59  1999/11/23 10:30:19  yiwasaki
// ALPHA in TRGCDCUtil is replaced by THelix::ConstantAlpha
//
// Revision 1.58  1999/11/19 09:13:10  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.57  1999/10/30 10:12:23  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.56  1999/09/13 05:57:56  yiwasaki
// Trasan 1.62b release : calcdc_sag2 -> calcdc_sag3
//
// Revision 1.55  1999/07/12 13:41:14  yiwasaki
// Trasan 1.54a release : cpu improvement in TTrack::approach, TTrackManager::saveTables checks # of hits
//
// Revision 1.54  1999/07/09 01:47:21  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.53  1999/06/16 08:29:52  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.52  1999/06/14 05:51:04  yiwasaki
// Trasan 1.41 release : curl finder updates
//
// Revision 1.51  1999/06/10 09:44:52  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.50  1999/06/10 00:27:28  yiwasaki
// Trasan 1.39 release : TTrack::approach bug fix
//
// Revision 1.49  1999/06/09 15:09:53  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.48  1999/05/28 09:43:35  yiwasaki
// Trasan 1.35 beta : movePivot bug fix
//
// Revision 1.47  1999/05/28 07:11:11  yiwasaki
// Trasan 1.35 alpha release : cathdoe test version
//
// Revision 1.46  1999/05/26 05:03:48  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.45  1999/04/09 11:36:54  yiwasaki
// Trasan 1.32 release : TCosmicFitter minor change
//
// Revision 1.44  1999/04/09 09:26:54  yiwasaki
// Trasan 1.31 release : Cosmic fitter and curl finder updated
//
// Revision 1.43  1999/03/21 15:45:41  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.42  1999/03/11 23:27:18  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.41  1999/03/10 12:55:05  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.40  1999/02/09 06:23:56  yiwasaki
// Trasan 1.17 release : cathode codes updated by T.Matsumoto, FPE error fixed by J.Tanaka
//
// Revision 1.39  1999/02/06 11:04:18  yiwasaki
// new approach calculation
//
// Revision 1.38  1999/02/03 06:23:10  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.37  1999/01/20 01:02:46  yiwasaki
// Trasan 1.12 release : movePivot problem avoided temporary, new sakura
//
// Revision 1.36  1999/01/11 03:03:15  yiwasaki
// Fitters added
//
// Revision 1.35  1998/12/26 03:43:03  yiwasaki
// cathode updates from S.Suzuki
//
// Revision 1.34  1998/11/27 08:15:33  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.33  1998/11/16 06:43:52  yiwasaki
// curl finder bug fixed, vmag protection
//
// Revision 1.32  1998/11/16 00:32:36  katayama
// Somehow did not go in
//
// Revision 1.31  1998/11/12 12:27:28  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.30  1998/11/11 13:39:19  katayama
// Protectoin once more
//
// Revision 1.29  1998/11/11 11:04:08  yiwasaki
// protection again
//
// Revision 1.28  1998/11/11 07:26:47  yiwasaki
// Trasan 1.1 beta 9 release : more protections for negative sqrt and zero division
//
// Revision 1.27  1998/11/10 09:09:09  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.26  1998/10/09 03:01:08  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.25  1998/09/29 01:24:25  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.24  1998/09/28 16:11:11  yiwasaki
// fitter with cathode added
//
// Revision 1.23  1998/09/24 22:56:38  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.22  1998/08/31 05:15:50  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.21  1998/08/03 15:01:04  yiwasaki
// Trasan 1.07 release : cluster finder from S.Suzuki-san added
//
// Revision 1.20  1998/07/29 04:34:56  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.17  1998/07/06 15:48:51  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.16  1998/06/17 20:23:00  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.15  1998/06/17 20:12:38  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.14  1998/06/15 09:58:16  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.13  1998/06/14 11:09:52  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.12  1998/06/11 12:58:02  yiwasaki
// TTrack::fit bug fixed, pointted out by J.Tanaka
//
// Revision 1.11  1998/06/11 08:14:10  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.10  1998/06/08 14:37:54  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.9  1998/06/03 17:17:39  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TCluster::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.8  1998/05/26 05:10:18  yiwasaki
// cvs repair
//
// Revision 1.7  1998/05/24 14:59:59  yiwasaki
// Trasan 1 alpha 5 release, pivot is moved to the first hit
//
// Revision 1.6  1998/05/22 08:23:20  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.5  1998/05/11 10:16:57  yiwasaki
// TTrack::assign -> TTrack::assert, WireHitUsedMask is set in TRGCDCWireHit
//
// Revision 1.4  1998/05/08 09:45:44  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.3  1998/04/23 17:21:44  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.2  1998/04/16 16:49:34  yiwasaki
// minor changes
//
// Revision 1.1  1998/04/10 09:36:29  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
//-----------------------------------------------------------------------------

/* for isnan */
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
#include <cfloat>



#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/Strings.h"
#include "CLHEP/Matrix/Vector.h"
#include "CLHEP/Matrix/SymMatrix.h"
#include "CLHEP/Matrix/DiagMatrix.h"
#include "CLHEP/Matrix/Matrix.h"
#include "trg/cdc/TRGCDC.h"
#include "tracking/modules/trasan/TFinderBase.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TLink.h"

#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
//cnv #include "trg/cdc/Strip.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TUtilities.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
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
#ifndef PANTHER_RECTRK_
#define PANTHER_RECTRK_
struct rectrk {
  int m_panther_dummy_;
  int m_ID;
  int m_glob[5];
  int m_zero[5];
  int m_first[5];
  int m_last[5];
  int m_prekal;
};
#endif
#ifndef PANTHER_RECTRK_GLOBAL_
#define PANTHER_RECTRK_GLOBAL_
struct rectrk_global {
  int m_panther_dummy_;
  int m_ID;
  float m_mass;
  float m_chisq[3];
  int m_ndf[3];
  float m_pathl;
  float m_tof;
  float m_pathl_rd;
  float m_pathl_ab;
  int m_stat[2];
  int m_nhits[5];
  int m_hit[3];
};
#endif
#ifndef PANTHER_RECTRK_LOCALZ_
#define PANTHER_RECTRK_LOCALZ_
struct rectrk_localz {
  int m_panther_dummy_;
  int m_ID;
  float m_pivot[3];
  float m_helix[5];
  float m_error[15];
};
#endif
#ifndef PANTHER_RECTRK_LOCALF_
#define PANTHER_RECTRK_LOCALF_
struct rectrk_localf {
  int m_panther_dummy_;
  int m_ID;
  float m_pivot[3];
  float m_helix[5];
  float m_error[15];
};
#endif
#ifndef PANTHER_RECTRK_LOCALL_
#define PANTHER_RECTRK_LOCALL_
struct rectrk_locall {
  int m_panther_dummy_;
  int m_ID;
  float m_pivot[3];
  float m_helix[5];
  float m_error[15];
};
#endif
#ifndef PANTHER_RECTRK_LOCALZ_LOWP_
#define PANTHER_RECTRK_LOCALZ_LOWP_
struct rectrk_localz_lowp {
  int m_panther_dummy_;
  int m_ID;
  float m_pivot[3];
  float m_helix[5];
  float m_error[15];
};
#endif
#ifndef PANTHER_RECTRK_V0_
#define PANTHER_RECTRK_V0_
struct rectrk_v0 {
  int m_panther_dummy_;
  int m_ID;
  int m_kind;
  int m_dau[2];
  float m_vtx[3];
  float m_P[5];
  float m_fl;
  float m_dz;
  int m_type;
};
#endif
#ifndef PANTHER_RECTRK_V02_
#define PANTHER_RECTRK_V02_
struct rectrk_v02 {
  int m_panther_dummy_;
  int m_ID;
  int m_kind;
  int m_daut[2];
  float m_P[4];
  float m_vtx[3];
  float m_dz;
  float m_chisq;
  int m_type;
  int m_dau;
};
#endif
#ifndef PANTHER_RECTRK_V0_DAUGHTERS_
#define PANTHER_RECTRK_V0_DAUGHTERS_
struct rectrk_v0_daughters {
  int m_panther_dummy_;
  int m_ID;
  float m_helix_p[5];
  float m_helix_m[5];
  float m_error_p[15];
  float m_error_m[15];
};
#endif
#ifndef PANTHER_RECTRK_V0_DAUGHTERS_ADD_
#define PANTHER_RECTRK_V0_DAUGHTERS_ADD_
struct rectrk_v0_daughters_add {
  int m_panther_dummy_;
  int m_ID;
  int m_nhits_p[2];
  int m_hit_p;
  int m_nhits_m[2];
  int m_hit_m;
  int m_v0;
};
#endif
#ifndef PANTHER_RECTRK_PATHL_
#define PANTHER_RECTRK_PATHL_
struct rectrk_pathl {
  int m_panther_dummy_;
  int m_ID;
  int m_layer;
  int m_trk;
  float m_xin;
  float m_yin;
  float m_zin;
  float m_xout;
  float m_yout;
  float m_zout;
  float m_pathl;
  float m_px;
  float m_py;
  float m_pz;
  int m_status;
};
#endif
#ifndef PANTHER_RECCDC_SVD_TRK_
#define PANTHER_RECCDC_SVD_TRK_
struct reccdc_svd_trk {
  int m_panther_dummy_;
  int m_ID;
  float m_Helix[5];
  int m_cdc_trk;
  int m_svd_cluster[24];
  int m_Status;
};
#endif
#ifndef PANTHER_RECTRK_TOF_
#define PANTHER_RECTRK_TOF_
struct rectrk_tof {
  int m_panther_dummy_;
  int m_ID;
  int m_rectrk;
  float m_tof;
  float m_tof_k;
  float m_tof_p;
};
#endif
#ifndef PANTHER_MDST_TRK_
#define PANTHER_MDST_TRK_
struct mdst_trk {
  int m_panther_dummy_;
  int m_ID;
  int m_mhyp[5];
  int m_quality;
  int m_quality_dedx;
  float m_dEdx;
  float m_dEdx_exp[5];
  float m_sigma_dEdx[5];
  float m_pid_e;
  float m_pid_mu;
  float m_pid_pi;
  float m_pid_K;
  float m_pid_p;
};
#endif
#ifndef PANTHER_MDST_TRK_ADD_
#define PANTHER_MDST_TRK_ADD_
struct mdst_trk_add {
  int m_panther_dummy_;
  int m_ID;
  int m_stat[10];
  float m_buff[10];
};
#endif
#ifndef PANTHER_MDST_TRK_EXTRA_
#define PANTHER_MDST_TRK_EXTRA_
struct mdst_trk_extra {
  int m_panther_dummy_;
  int m_ID;
  int m_mhyp[5];
  int m_quality;
};
#endif
#ifndef PANTHER_MDST_TRK_FIT_
#define PANTHER_MDST_TRK_FIT_
struct mdst_trk_fit {
  int m_panther_dummy_;
  int m_ID;
  int m_quality;
  int m_hit_cdc[2];
  int m_hit_svd;
  int m_nhits[5];
  float m_mass;
  float m_pivot_x;
  float m_pivot_y;
  float m_pivot_z;
  float m_helix[5];
  float m_error[15];
  float m_chisq;
  int m_ndf;
  float m_first_x;
  float m_first_y;
  float m_first_z;
  float m_last_x;
  float m_last_y;
  float m_last_z;
};
#endif


#ifdef TRASAN_DEBUG

#endif

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  TPoint2D TTrack::_points0[100];
  TPoint2D TTrack::_points1[100];

#ifdef TRASAN_DEBUG
//cnv BelleHistogram * h_nTrial;
  unsigned TTrack::_nTTracks = 0;
  unsigned TTrack::_nTTracksMax = 0;

  unsigned
  TTrack::nTTracks(void)
  {
    return _nTTracks;
  }

  unsigned
  TTrack::nTTracksMax(void)
  {
    return _nTTracksMax;
  }
#endif


  const THelixFitter
  TTrack::_fitter = THelixFitter("TTrack Default THelix Fitter");

  TTrack::TTrack(const TCircle& c)
    : TTrackBase(c.links()),
      _state(0),
      _charge(c.charge()),
      _mother(0),
      _daughter(0),
      _helix(new THelix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0))),
      _ndf(0),
      _chi2(0.),
      _name("none"),
      _type(0)
  {

    //...Set a defualt fitter...
    fitter(& TTrack::_fitter);

    //...Calculate helix parameters...
    CLHEP::HepVector a(5);
    a[1] = fmod(atan2(_charge * (c.center().y() - ORIGIN.y()),
                      _charge * (c.center().x() - ORIGIN.x()))
                + 4. * M_PI,
                2. * M_PI);
    a[2] = THelix::ConstantAlpha / c.radius();
    a[0] = (c.center().x() - ORIGIN.x()) / cos(a[1]) - c.radius();
    a[3] = 0.;
    a[4] = 0.;
    _helix->a(a);

    //...Update links...
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++)
      _links[i]->track(this);

    _fitted = false;
    _fittedWithCathode = false;

#ifdef TRASAN_DEBUG
    ++_nTTracks;
    if (_nTTracks > _nTTracksMax)
      _nTTracksMax = _nTTracks;
#endif
  }

  TTrack::TTrack(const TTrack& a)
    : TTrackBase((TTrackBase&) a),
      _state(a._state),
      _charge(a._charge),
      _segments(a._segments),
      _mother(a._mother),
      _daughter(a._daughter),
      _helix(new THelix(* a._helix)),
      _ndf(a._ndf),
      _chi2(a._chi2),
      _name("copy of " + a._name),
      _type(a._type)
  {
//cnv  , _catHits(a._catHits) {
#ifdef TRASAN_DEBUG
    ++_nTTracks;
    if (_nTTracks > _nTTracksMax)
      _nTTracksMax = _nTTracks;
#endif
  }

  TTrack::TTrack(const THelix& h)
    : TTrackBase(),
      _state(0),
      _mother(0),
      _daughter(0),
      _helix(new THelix(h)),
      _ndf(0),
      _chi2(0.),
      _name("none"),
      _type(0)
  {

    //...Set a defualt fitter...
    fitter(& TTrack::_fitter);

    if (_helix->kappa() > 0.) _charge = 1.;
    else _charge = -1.;

    _fitted = false;
    _fittedWithCathode = false;
#ifdef TRASAN_DEBUG
    ++_nTTracks;
    if (_nTTracks > _nTTracksMax)
      _nTTracksMax = _nTTracks;
#endif
  }

  TTrack::TTrack()
    : TTrackBase(),
      _state(0),
      _charge(1.),
      _mother(0),
      _daughter(0),
      _helix(new THelix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0))),
      _ndf(0),
      _chi2(0.),
      _name("empty track")
  {
#ifdef TRASAN_DEBUG
    ++_nTTracks;
    if (_nTTracks > _nTTracksMax)
      _nTTracksMax = _nTTracks;
#endif
  }

  TTrack::~TTrack()
  {
    delete _helix;
#ifdef TRASAN_DEBUG
    --_nTTracks;
#endif
  }

  void
  TTrack::dump(const std::string& msg, const std::string& pre0) const
  {
    bool def = false;
    if (msg == "") def = true;
    std::string pre = pre0;
    std::string tab;
    for (unsigned i = 0; i < pre.length(); i++)
      tab += " ";

    std::cout << pre << name() << std::endl;

    if (def ||
        msg.find("track") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre
                << p() << "(pt=" << pt() << ")" << std::endl
                << tab
                << "links=" << _links.length()
                << "(cores=" << nCores()
                << "),chrg=" << _charge
                << ",ndf=" << _ndf
                << ",chi2=" << _chi2
                << std::endl;
      pre = tab;
    }
    if (msg.find("helix") != std::string::npos ||
        msg.find("detail") != std::string::npos) {
      std::cout << pre
                << "pivot=" << _helix->pivot()
                << ",center=" << _helix->center() << std::endl
                << tab
                << "helix=(" << _helix->a()[0] << "," << _helix->a()[1]
                << "," << _helix->a()[2] << "," << _helix->a()[3] << ","
                << _helix->a()[4] << ")" << std::endl;
      pre = tab;
    }
    if (! def) TTrackBase::dump(msg, pre);
  }

  void
  TTrack::movePivot(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const std::string stage = "TTrack::movePivot";
    EnterStage(stage);
    std::cout << Tab() << "track=" << name() << std::endl;
#endif

    unsigned n = _links.length();
    if (! n) {
#ifdef TRASAN_DEBUG
      std::cout << "TTrack::movePivot !!! can't move a pivot"
                << " because of no link" << std::endl;
#endif
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return;
    }

    //...Check cores...
    const AList<TLink> & cores = TTrackBase::cores();
    const AList<TLink> * links = & cores;
    if (cores.length() == 0) links = & _links;

    //...Hit loop...
    unsigned innerMost = 0;
    unsigned innerMostLayer = (* links)[0]->wire()->layerId();
    n = links->length();
    for (unsigned i = 1; i < n; i++) {
      TLink* l = (* links)[i];
      if (l->wire()->layerId() < innerMostLayer) {
        innerMost = i;
        innerMostLayer = l->wire()->layerId();
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "innerMost=" << innerMost << std::endl;
    (* links)[innerMost]->dump("detail", Tab(+1));
    std::cout << Tab() << "before moving pivot" << std::endl;
    dump("helix", Tab(+1));
#endif

    //...Move pivot...
    HepGeom::Point3D<double> newPivot = (* links)[innerMost]->positionOnWire();

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "new pivot=" << newPivot << std::endl;
#endif

    if (quality() & TrackQuality2D)
      newPivot.setZ(0.);
#if defined(BELLE_DEBUG)
    try {
#endif
      _helix->pivot(newPivot);
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout
          << "TTrack::helix is invalid" << std::endl;
    }
#endif

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "after moving pivot" << std::endl;
    dump("helix", Tab(+1));
    LeaveStage(stage);
#endif
  }

  int
  TTrack::approach(TLink& l) const
  {
    return approach(l, false);
  }

  void
  TTrack::refine2D(AList<TLink> & list, float maxSigma)
  {
    unsigned n = _links.length();
    AList<TLink> bad;
    for (unsigned i = 0; i < n; ++i) {
      if (_links[i]->pull() > maxSigma) bad.append(_links[i]);
    }
    _links.remove(bad);
    if (bad.length()) {
      _fitted = false;
      fit2D();
    }
    list.append(bad);
  }

  int
  TTrack::HelCyl(double rhole,
                 double rCyl,
                 double zb,
                 double zf,
                 double epsl,
                 double& phi,
                 HepGeom::Point3D<double> & xp) const
  {


    int status(0);  // return value
    //---------------------------------------------------------------------
    //  value | ext |   status
    //---------------------------------------------------------------------
    //   1. |  OK |
    //  -1. |  NO |  charge = 0
    //   0. |  NO |  | tanl | < 0.1  ( neglect | lamda | < 5.7 deg. )
    //    |     |  or | dPhi | > 2 pi ( neglect curly track )
    //    |     |  or cannot reach to r=rhole at z = zb or zf.
    //   2. |  OK |   backward , ext point set on z = zb
    //   3. |  OK |   forward  , ext point set on z = zf
    //---------------------------------------------------------------------
    // * when value = 0,2,3 , ext(z) <= zb  or ext(z) >= zf

    //--- debug
    // std::cout << "   "  << std::endl;
    // std::cout << "HelCyl called ..  rhole=" << rhole << " rCyl=" << rCyl ;
    // std::cout << " zb=" << zb << " zf=" << zf << " epsl=" << epsl << std::endl;
    //--- debug end

    // Check of Charge

    if (int(_charge) == 0) {
      std::cout << "HelCyl gets a straight line !!" << std::endl;
      return -1 ;
    }

    // parameters

    HepGeom::Point3D<double> CenterCyl(0., 0., 0.);
    HepGeom::Point3D<double> CenterTrk = _helix->center();
    double  rTrk = fabs(_helix->radius());

//double rPivot = fabs( _helix->pivot().perp() );

    double phi0 = _helix->a()[1];
    double tanl = _helix->a()[4];
    //  double zdz = _helix->pivot().z() + _helix->a()[3];
    double dPhi;
    double zee;


    // Calculate intersections between cylinder and track
    // if return value = 2 track hitting barrel part

    HepGeom::Point3D<double> Cross1, Cross2;

    if (intersection(CenterTrk, _charge * rTrk , CenterCyl, rCyl,
                     epsl,
                     Cross1, Cross2)
        == 2) {

      double phiCyl = atan2(_charge * (CenterTrk.y() - Cross1.y()),
                            _charge * (CenterTrk.x() - Cross1.x()));
      phiCyl = (phiCyl > 0.) ? phiCyl : phiCyl + 2. * M_PI;

      dPhi = phiCyl - phi0;

      // dPhi region ( at cylinder )
      //    -pi <= dPhi < pi


      double PhiYobun = 1. / fabs(_helix->radius());
      double zero = 0.00001;

      if (_charge >= 0.) {
        if (dPhi > PhiYobun) dPhi -= 2. * M_PI;
        if (-2. * M_PI  - zero <= dPhi <= (-2.* M_PI + PhiYobun))
          dPhi += 2. * M_PI;
      }

      if (_charge < 0.) {
        if (dPhi < -PhiYobun) dPhi += 2. * M_PI;
        if (2. * M_PI + zero >= dPhi >= (2. * M_PI - PhiYobun))
          dPhi -= 2. * M_PI;
      }

      if (dPhi < - M_PI) dPhi += 2. * M_PI;
      if (dPhi >= M_PI) dPhi -= 2. * M_PI;

      //--debug
      // std::cout << "dPhi = " << dPhi << std::endl;
      //--debug end

      xp.setX(Cross1.x());
      xp.setY(Cross1.y());
      xp.setZ(_helix->x(dPhi).z());
      //    xp.setZ( zdz - _charge * rTrk * tanl * dPhi );

      if (xp.z() > zb && xp.z() < zf) {
        phi = dPhi;
//--- debug ---
// std::cout << "return1 ( ext success )" <<  std::endl;
// std::cout << " xp:= " << xp.x() << ", " << xp.y() << ", " << xp.z() << std::endl;
        //--- debug  ----
        return 1 ;
      }
    }


    // tracks hitting endcaps

    if (fabs(tanl) < 0.1) {
      //--- debug ---
      // std::cout << "return0 ( ext failed , |tanl| < 0.1 )" <<  std::endl;
      // std::cout << " xp:= " << xp.x() << ", " << xp.y() << ", " << xp.z() << std::endl;
      //--- debug ---
      return 0;
    }

    if (tanl > 0.) {
      zee = zf ;
      status = 3 ;
    } else {
      zee = zb ;
      status = 2 ;
    }

    dPhi = _charge * (_helix->x(0.).z() - zee) / rTrk / tanl;
    //  dPhi = _charge * ( zdz - zee )/rTrk/tanl;

    // Requre dPhi < 2*pi

    if (fabs(dPhi) > 2. * M_PI) {
      //--- debug ---
      // std::cout << " return0 ( ext failed , dPhi > 2pi )" << std::endl;
      // std::cout << " xp:= " << xp.x() << ", " << xp.y() << ", " << xp.z() << std::endl;
      //--- debug  ---
      return 0 ;
    }

    xp.setX(_helix->x(dPhi).x());
    xp.setY(_helix->x(dPhi).y());
    xp.setZ(zee);

    double test = xp.perp2()  - rhole * rhole ;
    if (test < 0.)  {
      //--- debug ---
      // std::cout << "return0 ( cannot reach to rhole at z=edge )" << std::endl;
      // std::cout << " xp:= " << xp.x() << ", " << xp.y() << ", " << xp.z() << std::endl;
      //--- debug ---
      return 0 ;
    }

    phi = dPhi ;
    //--- debug ---
    // std::cout << "return" << status << std::endl;
    // std::cout << " xp:= " << xp.x() << ", " << xp.y() << ", " << xp.z() << std::endl;
    //--- debug ---

    return status ;

  }

  void
  TTrack::findCatHit(unsigned trackid)
  {

//cnv //  unsigned i = 0;
//   //  while ( TRGCDC01CatHit * chit = _cathits.last() ) {
//   //    _cathits.remove(chit);
//   //    delete chit;
//   //  }

//   //--- debug ---
//   // std::cout << std::endl << "FindCatHit  called !! " << std::endl << std::endl;
//   //--- debug end ----

//   // set cylinder geometry
//   double rcyl[3];
//   rcyl[0] = 8.80 ;
//   rcyl[1] = 9.80 ;
//   rcyl[2] = 10.85 ;
//   double rhole = 8.00 ;
//   double zb = -26.17 ;
//   double zf =  45.87 ;
//   double epsl = 0.01 ;

//   //
//   HepGeom::Point3D<double> xp ;
//   double phi ;
//   TRGCDCCatHit * chit;

//   // loop over layers and find cathits

//   for ( unsigned layer = 0 ; layer < 3 ; layer++ ) {
//     if ( HelCyl (rhole, rcyl[layer], zb,zf,epsl, phi, xp ) == 1 ) {
//       chit = new TRGCDCCatHit( this, trackid, layer, xp.x(), xp.y(), xp.z()) ;
//       _catHits.append(chit);
//     }
//   }

//   //--- debug ---
//   //if(_cathits.length()) {
//   // std::cout << std::endl;
//   //  for ( int j = 0 ; j < _cathits.length() ; j++ ) {
//   //    _cathits[j]->dump(" ", " ") ;
//   //  }
//   //}
//   // chit->dump(" ", " ") ;
//   //--- debug end ----

  }
//--- Nagoya mods. 19981225 end ---------------------------


//  int
//  TTrack::fitx(void) {

//  #ifdef TRASAN_DEBUG_DETAIL
// std::cout << "TTrack::fit !!! This is obsolete !!!" << std::endl;
//  #endif
//      if (_fitted) return 0;

//      //...Check # of hits...
//      if (_links.length() < 5) return -1;

//      //...Setup...
//      unsigned nTrial = 0;
//      CLHEP::HepVector a(5), da(5);
//      a = _helix->a();
//      CLHEP::HepVector dxda(5);
//      CLHEP::HepVector dyda(5);
//      CLHEP::HepVector dzda(5);
//      CLHEP::HepVector dDda(5);
//      CLHEP::HepVector dchi2da(5);
//      CLHEP::HepSymMatrix d2chi2d2a(5, 0);
//      double chi2;
//      double chi2Old = 10e99;
//      const double convergence = 1.0e-5;
//      bool allAxial = true;
//     CLHEP::HepMatrix e(3, 3);
//      CLHEP::HepVector f(3);
//      int err = 0;
//      double factor = 1.0;//jtanaka0715

//      CLHEP::HepVector maxDouble(5);
//      for (unsigned i = 0; i < 5; i++) maxDouble[i] = (FLT_MAX);

//      //...Fitting loop...
//      while (nTrial < 100) {

//    //...Set up...
//    chi2 = 0.;
//    for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
//    d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

//    //...Loop with hits...
//    unsigned i = 0;
//    while (TLink * l = _links[i++]) {
//        const TRGCDCWireHit & h = * l->hit();

//        //...Check state...
//        if (h.state() & CellHitInvalidForFit) continue;

//        //...Check wire...
//        if (! nTrial)
//      if (h.wire()->stereo()) allAxial = false;

//        //...Cal. closest points...
//        approach(* l);
//        double dPhi = l->dPhi();
//        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
//        const HepGeom::Point3D<double> & onWire = l->positionOnWire();

//  #ifdef TRASAN_DEBUG_DETAIL
//  //      std::cout << "    in fit " << onTrack << ", " << onWire;
//  //      h.dump();
//  #endif

//        //...Obtain drift distance and its error...
//        unsigned leftRight = CellHitRight;
//        if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;
//        double distance = h.distance(leftRight);
//        double eDistance = h.dDistance(leftRight);
//        double eDistance2 = eDistance * eDistance;

//        //...Residual...
//        Vector3D v = onTrack - onWire;
//        double vmag = v.mag();
//        double dDistance = vmag - distance;

//        //...dxda...
//        this->dxda(* l, dPhi, dxda, dyda, dzda);

//        //...Chi2 related...
//        // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
//        HepGeom::Vector3D<double> vw = h.wire().direction();
//              dDda = (vmag > 0.)
//      ? ((v.x() * (1. - vw.x() * vw.x()) -
//          v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
//         * dxda +
//         (v.y() * (1. - vw.y() * vw.y()) -
//          v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
//         * dyda +
//         (v.z() * (1. - vw.z() * vw.z()) -
//          v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
//         * dzda) / vmag
//      :CLHEP::HepVector(5, 0);
//        if(vmag<=0.0) {
//          std::cout<< "    in fit " << onTrack << ", " << onWire;
//          h.dump();
//        }
//        dchi2da += (dDistance / eDistance2) * dDda;
//        d2chi2d2a += vT_times_v(dDda) / eDistance2;
//        double pChi2 = dDistance * dDistance / eDistance2;
//        chi2 += pChi2;

//        //...Store results...
//        l->update(onTrack, onWire, leftRight, pChi2);
//    }

//    //...Check condition...
//    double change = chi2Old - chi2;
//    if (fabs(change) < convergence) break;
//    //if (change < 0.) break;
//    //jtanaka -- from traffs -- Ozaki-san added this part to traffs.
//    if (change < 0.){
//  #ifdef TRASAN_DEBUG_DETAIL
// std::cout << "chi2Old, chi2=" << chi2Old <<" "<< chi2 << std::endl;
//  #endif
//      //change to the old value.
//            a += factor*da;
//            _helix->a(a);

//            chi2 = 0.;
//            for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
//            d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

//            //...Loop with hits...
//            unsigned i = 0;
//            while (TLink * l = _links[i++]) {
//          const TRGCDCWireHit & h = * l->hit();

//              //...Check wire...
//              if (! nTrial)
//                if (h.wire().stereo()) allAxial = false;

//              //...Cal. closest points...
//              approach(* l);
//              double dPhi = l->dPhi();
//              const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
//              const HepGeom::Point3D<double> & onWire = l->positionOnWire();

//  #ifdef TRASAN_DEBUG_DETAIL
//              // std::cout << "    in fit in case of change < 0. " << onTrack << ", " << onWire;
//              // h.dump();
//  #endif

//              //...Obtain drift distance and its error...
//              unsigned leftRight = CellHitRight;
//              if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;
//              double distance = h.distance(leftRight);
//              double eDistance = h.dDistance(leftRight);
//              double eDistance2 = eDistance * eDistance;

//              //...Residual...
//              Vector3D v = onTrack - onWire;
//        double vmag = v.mag();
//        double dDistance = vmag - distance;

//              //...dxda...
//              this->dxda(* l, dPhi, dxda, dyda, dzda);

//              //...Chi2 related...
//              //dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
//              HepGeom::Vector3D<double> vw = h.wire().direction();
//              dDda = (vmag > 0.)
//      ? ((v.x() * (1. - vw.x() * vw.x()) -
//          v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
//         * dxda +
//         (v.y() * (1. - vw.y() * vw.y()) -
//          v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
//         * dyda +
//         (v.z() * (1. - vw.z() * vw.z()) -
//          v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
//         * dzda) / vmag
//      :CLHEP::HepVector(5, 0);
//        if(vmag<=0.0) {
//          std::cout<< "    in fit " << onTrack << ", " << onWire;
//          h.dump();
//        }
//              dchi2da += (dDistance / eDistance2) * dDda;
//              d2chi2d2a += vT_times_v(dDda) / eDistance2;
//              double pChi2 = dDistance * dDistance / eDistance2;
//              chi2 += pChi2;

//              //...Store results...
//              l->update(onTrack, onWire, leftRight, pChi2);
//            }
//      //break;
//      factor *= 0.75;
//  #ifdef TRASAN_DEBUG_DETAIL
//      std::cout<< "factor = " << factor << std::endl;
//      std::cout<< "chi2 = " << chi2 << std::endl;
//  #endif
//      if(factor < 0.01)break;
//          }

//    chi2Old = chi2;

//    //...Cal. helix parameters for next loop...
//    if (allAxial) {
//        f = dchi2da.sub(1, 3);
//        e = d2chi2d2a.sub(1, 3);
//        f = solve(e, f);
//        da[0] = f[0];
//        da[1] = f[1];
//        da[2] = f[2];
//        da[3] = 0.;
//        da[4] = 0.;
//    }
//    else {
//        da = solve(d2chi2d2a, dchi2da);
//    }

//  #ifdef TRASAN_DEBUG_DETAIL
//    // std::cout << "    fit " << nTrial << " : " << da << std::endl;
//    // std::cout << "        d2chi " << d2chi2d2a << std::endl;
//    // std::cout << "        dchi2 " << dchi2da << std::endl;
//  #endif

//    a -= factor*da;
//    _helix->a(a);
//    ++nTrial;
//      }

//      //...Cal. error matrix...
//      CLHEP::HepSymMatrix Ea(5, 0);
//      unsigned dim;
//      if (allAxial) {
//    dim = 3;
//    CLHEP::HepSymMatrix Eb(3, 0), Ec(3, 0);
//    Eb = d2chi2d2a.sub(1, 3);
//    Ec = Eb.inverse(err);
//    Ea[0][0] = Ec[0][0];
//    Ea[0][1] = Ec[0][1];
//    Ea[0][2] = Ec[0][2];
//    Ea[1][1] = Ec[1][1];
//    Ea[1][2] = Ec[1][2];
//    Ea[2][2] = Ec[2][2];
//      }
//      else {
//    dim = 5;
//    Ea = d2chi2d2a.inverse(err);
//      }

//      //...Store information...
//      if (! err) {
//    _helix->a(a);
//    _helix->Ea(Ea);
//      }

//      _ndf = _links.length() - dim;
//      _chi2 = chi2;

//      _fitted = true;
//      return err;
//  }

  int
  TTrack::dxda(const TLink& link,
               double dPhi,
               CLHEP::HepVector& dxda,
               CLHEP::HepVector& dyda,
               CLHEP::HepVector& dzda) const
  {

    //...Setup...
    const Belle2::TRGCDCWire& w = * link.wire();
    CLHEP::HepVector a = _helix->a();
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

    //...Axial case...
    if (w.axial()) {
      Point3D d = _helix->center() - w.xyPosition();
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
      Point3D onTrack = _helix->x(dPhi);
      Vector3D v = w.direction();
      CLHEP::HepVector c(3);
      c = Point3D(w.backwardPosition() - (v * w.backwardPosition()) * v);

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

#define NEW_FIT2D 1
#if !(NEW_FIT2D)
  int
  TTrack::fit2D(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TTrack::fit2D(r-phi) ..." << std::endl;
#endif
    if (_fitted) return 0;

    //...Check # of hits...
    if (_links.length() < 4) return -1;
    // std::cout << "# = " << _links.length() << std::endl;
    //...Setup...
    unsigned nTrial = 0;
    CLHEP::HepVector a = _helix->a();
    CLHEP::HepVector da(5), dxda(5), dyda(5), dzda(5);
    CLHEP::HepVector dDda(5), dchi2da(5);
    CLHEP::HepSymMatrix d2chi2d2a(5, 0);
    double chi2;
    double chi2Old = 1.0e+99;
    const double convergence = 1.0e-5;
    CLHEP::HepMatrix e(3, 3);
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;

    //...Fitting loop...
    while (nTrial < 100) {
#ifdef TRASAN_DEBUG_DETAIL
      if (a[3] != 0. || a[4] != 0.)std::cout << "Error in 2D functions of TTrack : a = " << a << std::endl;
#endif
      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 5; ++j) dchi2da[j] = 0.;
      d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = _links[i++]) {
        const Belle2::TRGCDCWireHit& h = * l->hit();

        //...Cal. closest points...
        approach2D(* l);
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire = l->positionOnWire();
#ifdef TRASAN_DEBUG_DETAIL
        if (onTrack.z() != 0.)std::cout << "Error in 2D functions of TTrack : onTrack = " << onTrack << std::endl;
        if (onWire.z() != 0.)std::cout << "Error in 2D functions of TTrack : onWire = " << onWire << std::endl;
#endif
        HepGeom::Point3D<double> onTrack2(onTrack.x(), onTrack.y(), 0.);
        HepGeom::Point3D<double> onWire2(onWire.x(), onWire.y(), 0.);

        //...Obtain drift distance and its error...
        unsigned leftRight = CellHitRight;
        if (onWire2.cross(onTrack2).z() < 0.) leftRight = CellHitLeft;
        double distance = h.distance(leftRight);
        double eDistance = h.dDistance(leftRight);
        double eDistance2 = eDistance * eDistance;

        //...Residual...
        Vector3D v = onTrack2 - onWire2;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        this->dxda2D(* l, dPhi, dxda, dyda, dzda);

        //...Chi2 related...
        //HepGeom::Vector3D<double> vw(0.,0.,1.);
        dDda = (vmag > 0.)
               ? (v.x() * dxda +
                  v.y() * dyda) / vmag
               : CLHEP::HepVector(5, 0);
        //dDda = (vmag > 0.)
        //? ((v.x() * (1. - vw.x() * vw.x()) -
        //    v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
        //   * dxda +
        //   (v.y() * (1. - vw.y() * vw.y()) -
        //    v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
        //   * dyda +
        //   (v.z() * (1. - vw.z() * vw.z()) -
        //    v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
        //   * dzda) / vmag
        //:CLHEP::HepVector(5, 0);
        if (vmag <= 0.0) {
          std::cout << "    in fit2D " << onTrack << ", " << onWire;
          h.dump();
        }
        dchi2da += (dDistance / eDistance2) * dDda;
        d2chi2d2a += vT_times_v(dDda) / eDistance2;
        double pChi2 = dDistance * dDistance / eDistance2;
        chi2 += pChi2;

        //...Store results...
        l->update(onTrack2, onWire2, leftRight, pChi2);
      }

      //...Check condition...
      double change = chi2Old - chi2;
      if (fabs(change) < convergence) break;
      if (change < 0.) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "chi2Old, chi2=" << chi2Old << " " << chi2 << std::endl;
#endif
        //change to the old value.
        a += factor * da;
        _helix->a(a);

        chi2 = 0.;
        for (unsigned j = 0; j < 5; ++j) dchi2da[j] = 0.;
        d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

        //...Loop with hits...
        unsigned i = 0;
        while (TLink* l = _links[i++]) {
          const Belle2::TRGCDCWireHit& h = * l->hit();

          //...Cal. closest points...
          approach2D(* l);
          double dPhi = l->dPhi();
          const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
          const HepGeom::Point3D<double> & onWire = l->positionOnWire();
          Point3D onTrack2(onTrack.x(), onTrack.y(), 0.);
          Point3D onWire2(onWire.x(), onWire.y(), 0.);

          //...Obtain drift distance and its error...
          unsigned leftRight = CellHitRight;
          if (onWire2.cross(onTrack2).z() < 0.) leftRight = CellHitLeft;
          double distance = h.distance(leftRight);
          double eDistance = h.dDistance(leftRight);
          double eDistance2 = eDistance * eDistance;

          //...Residual...
          Vector3D v = onTrack2 - onWire2;
          double vmag = v.mag();
          double dDistance = vmag - distance;

          //...dxda...
          this->dxda2D(* l, dPhi, dxda, dyda, dzda);

          //...Chi2 related...
          dDda = (vmag > 0.)
                 ? (v.x() * dxda +
                    v.y() * dyda) / vmag
                 : CLHEP::HepVector(5, 0);
          if (vmag <= 0.0) {
            std::cout << "    in fit2D " << onTrack << ", " << onWire;
            h.dump();
          }
          dchi2da += (dDistance / eDistance2) * dDda;
          d2chi2d2a += vT_times_v(dDda) / eDistance2;
          double pChi2 = dDistance * dDistance / eDistance2;
          chi2 += pChi2;

          //...Store results...
          l->update(onTrack2, onWire2, leftRight, pChi2);
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
      f = dchi2da.sub(1, 3);
      e = d2chi2d2a.sub(1, 3);
      f = solve(e, f);
      da[0] = f[0];
      da[1] = f[1];
      da[2] = f[2];
      da[3] = 0.;
      da[4] = 0.;

      a -= factor * da;
      _helix->a(a);
      // std::cout << nTrial << ": chi2 = " << chi2
      // << ", helix = (" << a[0] << ", " << a[1]
      // << ", " << a[2] << ", " << a[3]
      // << ", " << a[4] << ")" << std::endl;
      ++nTrial;
    }

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(5, 0);
    unsigned dim = 3;
    CLHEP::HepSymMatrix Eb = d2chi2d2a.sub(1, 3);
    CLHEP::HepSymMatrix Ec = Eb.inverse(err);
    Ea[0][0] = Ec[0][0];
    Ea[0][1] = Ec[0][1];
    Ea[0][2] = Ec[0][2];
    Ea[1][1] = Ec[1][1];
    Ea[1][2] = Ec[1][2];
    Ea[2][2] = Ec[2][2];

    //...Store information...
    if (! err) {
      _helix->a(a);
      _helix->Ea(Ea);
    }

    _ndf = _links.length() - dim;
    _chi2 = chi2;

    _fitted = true;
    return err;
  }
#endif

  int TTrack::fitWithCathode(float window, int SysCorr)
  {

// #ifdef TRASAN_DEBUG_DETAIL
//     std::cout<< "    TTrack::fitCathode ..." << std::endl;
// #endif

//     if (_fittedWithCathode) return 0;

//     //...Check # of hits...
//     if (nCores() < 5) return -1;

//     //...for cathode ndf...
//     int NusedCathode(0);

//     //...Setup...
//     unsigned nTrial = 0;
//     CLHEP::HepVector a(5), da(5);
//     a = _helix->a();
//     CLHEP::HepVector dxda(5);
//     CLHEP::HepVector dyda(5);
//     CLHEP::HepVector dzda(5);
//     CLHEP::HepVector dDda(5);
//     CLHEP::HepVector dDzda(5);  // for cathode z
//     CLHEP::HepVector dchi2da(5);
//     CLHEP::HepSymMatrix d2chi2d2a(5, 0);
//     double chi2;
//     double chi2Old = 10e99;
//     const double convergence = 1.0e-5;
//     bool allAxial = true;
//     int LayerStat(0); // layer status  axial:0 stereo:1 cathode:2
//    CLHEP::HepMatrix e(3, 3);
//     CLHEP::HepVector f(3);
//     int err = 0;
//     double factor = 1.0;

// //  const AList<TRGCDCCatHit> & chits = _catHits;

//     CLHEP::HepVector maxDouble(5);
//     for (unsigned i = 0; i < 5; i++) maxDouble[i] = (FLT_MAX);

//     //...Fitting loop...
//     while (nTrial < 100) {

//       //...Set up...
//       chi2 = 0.;
//       NusedCathode = 0;
//       for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
//       d2chi2d2a = CLHEP::HepSymMatrix(5, 0);

//       //...Loop with hits...
//       unsigned i = 0;
//       while (TLink * l = cores()[i++]) {

//    const TRGCDCWireHit & h = * l->hit();

//  // Check layer status ( cathode added )
//  LayerStat = 0;
//  if ( h.wire().stereo() ) LayerStat = 1;
//  unsigned nlayer = h.wire().layerId();
//  if (nlayer == 0 || nlayer == 1 || nlayer == 2 ) LayerStat = 2;

//  //...Check wire...
//  if (! nTrial)
//    if (h.wire().stereo() || LayerStat == 2 ) allAxial = false;

//  //...Cal. closest points...
//  approach(* l);
//  double dPhi = l->dPhi();
//  const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
//  const HepGeom::Point3D<double> & onWire = l->positionOnWire();

// #ifdef TRASAN_DEBUG_DETAIL
//  // std::cout << "    in fitCathode " << onTrack << ", " << onWire;
//  // h.dump();
// #endif

//  //...Obtain drift distance and its error...
//  unsigned leftRight = CellHitRight;
//  if (onWire.cross(onTrack).z() < 0.) leftRight = CellHitLeft;
//  double distance = h.drift(leftRight);
//  double eDistance = h.dDrift(leftRight);
//  double eDistance2 = eDistance * eDistance;

//  //...Residual...
//  Vector3D v = onTrack - onWire;
//  double vmag = v.mag();
//  double dDistance = vmag - distance;

//  //...dxda...
//  this->dxda(* l, dPhi, dxda, dyda, dzda);

//  // ... Chi2 related ...
//  double pChi2(0.);

//  if( LayerStat == 0 || LayerStat == 1 ){
//      // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
//             HepGeom::Vector3D<double> vw = h.wire().direction();
//             dDda = (vmag > 0.)
//    ? ((v.x() * (1. - vw.x() * vw.x()) -
//        v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
//       * dxda +
//       (v.y() * (1. - vw.y() * vw.y()) -
//        v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
//       * dyda +
//       (v.z() * (1. - vw.z() * vw.z()) -
//        v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
//       * dzda) / vmag
//    : CLHEP::HepVector(5, 0);
//      if(vmag<=0.0) {
//        std::cout << "    in fit " << onTrack << ", " << onWire;
//        h.dump();
//      }
//    dchi2da += (dDistance / eDistance2) * dDda;
//    d2chi2d2a += vT_times_v(dDda) / eDistance2;
//    double pChi2 = dDistance * dDistance / eDistance2;
//    chi2 += pChi2;

//  } else {


//    dDda = ( v.x() * dxda + v.y() * dyda )/v.perp();
//    dchi2da += (dDistance/eDistance2) * dDda;
//    d2chi2d2a += vT_times_v(dDda)/eDistance2;

//    pChi2 = 0;
//    pChi2 += (dDistance/eDistance) * (dDistance/eDistance) ;

//    if ( l->usecathode() >= 3 ) {

//      TRGCDCClust * mclust = l->getmclust();

//        double dDistanceZ(this->helix().x(dPhi).z());

//        if( SysCorr ){
//    if( !nTrial ) {
//      mclust->zcalc( atan( this->helix().tanl()) );
//        mclust->esterz( atan( this->helix().tanl()) );
//    }

//    dDistanceZ -= mclust->zclustWithSysCorr();
//       } else {
//          dDistanceZ -= mclust->zclust();
//       }

//              double eDistanceZ(mclust->erz());
//              if( !eDistanceZ ) eDistanceZ = 99999.;

//        double eDistance2Z = eDistanceZ * eDistanceZ;
//        double pChi2z = 0;
//        pChi2z = (dDistanceZ/eDistanceZ);
//        pChi2z *= pChi2z;

// #ifdef TRASAN_DEBUG_DETAIL
//        std::cout<< "dDistanceZ = " << dDistanceZ << std::endl;
// #endif

//       //.... requirement for use of cluster

//       if( nTrial == 0 &&
//    fabs(dDistanceZ)< window &&
//    mclust->chits().length() == 1
//    ) l->setusecathode(4);

//       if( l->usecathode() == 4 ){
//    NusedCathode++;
//    dDzda = dzda ;
//    dchi2da += (dDistanceZ/eDistance2Z) * dDzda;
//    d2chi2d2a += vT_times_v(dDzda)/eDistance2Z;
//    pChi2 +=  pChi2z ;

//             }
//    }

//  } // end Chi2 related

//  chi2 += pChi2;


//  //...Store results...
//  l->update(onTrack, onWire, leftRight, pChi2);


//       } // Tlink loop end

//       //...Check condition...
//       double change = chi2Old - chi2;
//       //if(TRASAN_DEBUG_DETAIL>0)  std::cout<< "chi2 change = " <<change << std::endl;

//       if (fabs(change) < convergence) break;
//       if (change < 0.) {

// #ifdef TRASAN_DEBUG_DETAIL
//           std::cout<< "chi2Old, chi2=" << chi2Old <<" "<< chi2 << std::endl;
// #endif

//    NusedCathode = 0;
//           //change to the old value.
//           a += factor*da;
//    _helix->a(a);

//    chi2 = 0.;
//           for (unsigned j = 0; j < 5; j++) dchi2da[j] = 0.;
//           d2chi2d2a = CLHEP::HepSymMatrix(5, 0);


//           //...Loop with hits...
//           unsigned i = 0;
//           while (TLink * l = _links[i++]) {
//        const TRGCDCWireHit & h = * l->hit();

//      // Check layer status ( cathode added )
//         LayerStat = 0;
//         if ( h.wire().stereo() ) LayerStat = 1;
//         unsigned nlayer = h.wire().layerId();
//         if (nlayer == 0 || nlayer == 1 || nlayer == 2 ) LayerStat = 2;

//         //...Cal. closest points...
//         approach(* l);
//         double dPhi = l->dPhi();
//         const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
//         const HepGeom::Point3D<double> & onWire = l->positionOnWire();

// #ifdef TRASAN_DEBUG_DETAIL
//         // std::cout << "    in fitCathode " << onTrack << ", " << onWire;
//         // h.dump();
// #endif

//         //...Obtain drift distance and its error...
//         unsigned leftRight = CellHitRight;
//         if (onWire.cross(onTrack).z() < 0.)     leftRight = CellHitLeft;
//         double distance = h.drift(leftRight);
//         double eDistance = h.dDrift(leftRight);
//         double eDistance2 = eDistance * eDistance;

//         //...Residual...
//         Vector3D v = onTrack - onWire;
//  double vmag = v.mag();
//  double dDistance = vmag - distance;

//         //...dxda...
//         this->dxda(* l, dPhi, dxda, dyda, dzda);

//         // ... Chi2 related ...
//         double pChi2(0.);

//         if( LayerStat == 0 || LayerStat == 1 ){
//      // dDda = (v.x() * dxda + v.y() * dyda + v.z() * dzda) / vmag;
//             HepGeom::Vector3D<double> vw = h.wire().direction();
//             dDda = (vmag > 0.)
//    ? ((v.x() * (1. - vw.x() * vw.x()) -
//        v.y() * vw.x() * vw.y() - v.z() * vw.x() * vw.z())
//       * dxda +
//       (v.y() * (1. - vw.y() * vw.y()) -
//        v.z() * vw.y() * vw.z() - v.x() * vw.y() * vw.x())
//       * dyda +
//       (v.z() * (1. - vw.z() * vw.z()) -
//        v.x() * vw.z() * vw.x() - v.y() * vw.z() * vw.y())
//       * dzda) / vmag
//    :CLHEP::HepVector(5, 0);
//      if(vmag<=0.0) {
//        std::cout << "    in fit " << onTrack << ", " << onWire;
//        h.dump();
//      }
//           dchi2da += (dDistance / eDistance2) * dDda;
//           d2chi2d2a += vT_times_v(dDda) / eDistance2;
//           double pChi2 = dDistance * dDistance / eDistance2;
//           chi2 += pChi2;

//         } else {

//           dDda = ( v.x() * dxda + v.y() * dyda )/v.perp();
//           dchi2da += (dDistance/eDistance2) * dDda;
//           d2chi2d2a += vT_times_v(dDda)/eDistance2;

//           pChi2 = 0;
//           pChi2 += (dDistance/eDistance) * (dDistance/eDistance) ;

//    if( l->usecathode() == 4 ){

//      TRGCDCClust * mclust = l->getmclust();

//      if( mclust ){
//                 NusedCathode++;

//         double dDistanceZ(this->helix().x(dPhi).z());
//         if( SysCorr ) dDistanceZ -= mclust->zclustWithSysCorr();
//                else          dDistanceZ -= mclust->zclust();

//        double eDistanceZ(99999.);
//        if( mclust->erz() != 0. ) eDistanceZ = mclust->erz();

//        double eDistance2Z = eDistanceZ * eDistanceZ;
//        double pChi2z = 0;
//        pChi2z = (dDistanceZ/eDistanceZ);
//        pChi2z *= pChi2z;

//                 dDzda = dzda ;
//                 dchi2da += (dDistanceZ/eDistance2Z) * dDzda;
//                 d2chi2d2a += vT_times_v(dDzda)/eDistance2Z;
//                 pChi2 +=  pChi2z ;
//      }

//     }

//         } // end Chi2 related

//         chi2 += pChi2;


//         //...Store results...
//         l->update(onTrack, onWire, leftRight, pChi2);

//           }

//            //break;

//           factor *= 0.75;
// #ifdef TRASAN_DEBUG_DETAIL
//           std::cout<< "factor = " << factor << std::endl;
//           std::cout<< "chi2 = " << chi2 << std::endl;
// #endif
//           if(factor < 0.01)break;

//       }

//       chi2Old = chi2;

//       //...Cal. helix parameters for next loop...
//       if (allAxial) {
//  f = dchi2da.sub(1, 3);
//  e = d2chi2d2a.sub(1, 3);
//  f = solve(e, f);
//  da[0] = f[0];
//  da[1] = f[1];
//  da[2] = f[2];
//  da[3] = 0.;
//  da[4] = 0.;
//       }
//       else {
//  da = solve(d2chi2d2a, dchi2da);
//       }

// #ifdef TRASAN_DEBUG_DETAIL
//       // std::cout << "    fit " << nTrial << " : " << da << std::endl;
//       // std::cout << "        d2chi " << d2chi2d2a << std::endl;
//       // std::cout << "        dchi2 " << dchi2da << std::endl;
// #endif

//       a -= da;
//       _helix->a(a);
//       ++nTrial;
//     }

//     //...Cal. error matrix...
//     CLHEP::HepSymMatrix Ea(5, 0);
//     unsigned dim;
//     if (allAxial) {
//       dim = 3;
//       CLHEP::HepSymMatrix Eb(3, 0), Ec(3, 0);
//       Eb = d2chi2d2a.sub(1, 3);
//       Ec = Eb.inverse(err);
//       Ea[0][0] = Ec[0][0];
//       Ea[0][1] = Ec[0][1];
//       Ea[0][2] = Ec[0][2];
//       Ea[1][1] = Ec[1][1];
//       Ea[1][2] = Ec[1][2];
//       Ea[2][2] = Ec[2][2];
//     }
//     else {
//       dim = 5;
//       Ea = d2chi2d2a.inverse(err);
//     }

//     //...Store information...
//     if (! err) {
//       _helix->a(a);
//       _helix->Ea(Ea);
//     }

//     _ndf = nCores() + NusedCathode - dim;
//     _chi2 = chi2;

//     _fittedWithCathode = true;

//     return err;
    return -1;
  }

#if OLD_STEREO
  int
  TTrack::stereoHitForCurl(TLink& link, AList<Point3D> & arcZList) const
  {
    /*
      ATTENTION!!!!!
      Elements of arcZList are made by "new". We must delete them out of this function!!!!!

      This function is used in "stereoHitForCurl(TLink &link1, TLink &link2)" and
      "stereoHitForCurl(TLink &link1, TLink &link2, TLink &link3)" only.
      (I(jtanaka) checked it. --> no memory leak!!!)

      */

    // std::cout << "\n\nWire ID = " << link.hit()->wire().id() << std::endl;
    const Belle2::TRGCDCWireHit& h = *link.hit();
    Vector3D X     = 0.5 * (h.wire().forwardPosition() +
                            h.wire().backwardPosition());

    HepGeom::Point3D<double> center = _helix->center();
    HepGeom::Point3D<double> tmp(-999., -999., 0.);
    Vector3D x     = Vector3D(X.x(), X.y(), 0.);
    Vector3D w     = x - center;
    Vector3D V     = h.wire().direction();
    Vector3D v     = Vector3D(V.x(), V.y(), 0.);
    double   vmag2 = v.mag2();
    double   vmag  = sqrt(vmag2);
    //...temporary
    link.position(tmp);
    arcZList.removeAll();

    //...stereo?
    if (vmag == 0.) {
      return -1;
    }

    double r  = _helix->curv();
    double R[2];
    double drift = h.drift(CellHitLeft);
    R[0] = r + drift;
    R[1] = r - drift;
    double wv = w.dot(v);
    double d2[2];
    d2[0] = vmag2 * R[0] * R[0] + (wv * wv - vmag2 * w.mag2()); //...= v^2*(r^2 - w^2*sin()^2)...outer
    d2[1] = vmag2 * R[1] * R[1] + (wv * wv - vmag2 * w.mag2()); //...= v^2*(r^2 - w^2*sin()^2)...inner

    //...No crossing in R/Phi plane...
    if (d2[0] < 0. && d2[1] < 0.) {
      return -1;
    }

    bool ok_inner, ok_outer;
    ok_inner = true;
    ok_outer = true;
    double d[2];
    d[0] = -1.;
    d[1] = -1.;
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

    //...boolean
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
      if (_charge * (center.x() * p[0][0].y() - center.y() * p[0][0].x())  < 0.)
        ok_xy[0][0] = false;
      if (_charge * (center.x() * p[1][0].y() - center.y() * p[1][0].x())  < 0.)
        ok_xy[1][0] = false;
    }
    if (ok_inner) {
      if (_charge * (center.x() * p[0][1].y() - center.y() * p[0][1].x())  < 0.)
        ok_xy[0][1] = false;
      if (_charge * (center.x() * p[1][1].y() - center.y() * p[1][1].x())  < 0.)
        ok_xy[1][1] = false;
    }
    if (!ok_inner && ok_outer && (!ok_xy[0][0]) && (!ok_xy[1][0])) {
      return -1;
    }
    if (ok_inner && !ok_outer && (!ok_xy[0][1]) && (!ok_xy[1][1])) {
      return -1;
    }

#if 0
    std::cout << "Drift Dist. = " << h.distance(CellHitLeft) << std::endl;
    std::cout << "outer ok? = " << ok_outer << std::endl;
    std::cout << "Z cand = " << z[0][0] << ", " << z[1][0] << std::endl;
    std::cout << "inner ok? = " << ok_inner << std::endl;
    std::cout << "Z cand = " << z[0][1] << ", " << z[1][1] << std::endl;
#endif

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
      return -1;
    }

    double cosdPhi, dPhi;

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

      Point3D* arcZ = new Point3D;
      arcZ->setX(r * dPhi);
      arcZ->setY(z[0][0]);
      arcZList.append(arcZ);
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

      Point3D* arcZ = new Point3D;
      arcZ->setX(r * dPhi);
      arcZ->setY(z[1][0]);
      arcZList.append(arcZ);
    }
    if (ok_xy[0][1]) {
      //...cal. arc length...
      cosdPhi = - center.dot((p[0][1] - center).unit()) / center.mag();
      if (cosdPhi > 1.0) cosdPhi = 1.0;
      if (cosdPhi < -1.0) cosdPhi = -1.0;

      if (fabs(cosdPhi) < 1.0) {
        dPhi = acos(cosdPhi);
      } else if (cosdPhi >= 1.0) {
        dPhi = 0.;
      } else {
        dPhi = M_PI;
      }

      Point3D* arcZ = new Point3D;
      arcZ->setX(r * dPhi);
      arcZ->setY(z[0][1]);
      arcZList.append(arcZ);
    }
    if (ok_xy[1][1]) {
      //...cal. arc length...
      cosdPhi = - center.dot((p[1][1] - center).unit()) / center.mag();
      if (cosdPhi > 1.0) cosdPhi = 1.0;
      if (cosdPhi < -1.0) cosdPhi = -1.0;
      if (fabs(cosdPhi) < 1.0) {
        dPhi = acos(cosdPhi);
      } else if (cosdPhi >= 1.0) {
        dPhi = 0.;
      } else {
        dPhi = M_PI;
      }

      Point3D* arcZ = new Point3D;
      arcZ->setX(r * dPhi);
      arcZ->setY(z[1][1]);
      arcZList.append(arcZ);
    }

    if (arcZList.length() == 1 ||
        arcZList.length() == 2) {
      return 0;
    }
    return -1;
  }


  int
  TTrack::stereoHitForCurl(TLink& link1, TLink& link2) const
  {
    int flag1 = 0;
    int flag2 = 0;
    double minZ = 9999999.;

    AList<Point3D> arcZList1;
    AList<Point3D> arcZList2;
    if (stereoHitForCurl(link1, arcZList1) == 0) {
      if (stereoHitForCurl(link2, arcZList2) == 0) {
        //...finds
        int n1 = arcZList1.length();
        int n2 = arcZList2.length();
        for (int i = 0; i < n1; i++) {
          for (int j = 0; j < n2; j++) {
            if (fabs(arcZList1[i]->y() - arcZList2[j]->y()) < minZ) {
              minZ = fabs(arcZList1[i]->y() - arcZList2[j]->y());
              flag1 = i;
              flag2 = j;
            }
          }
        }
      }
    }

    if (minZ == 9999999.) {
      //...deletes
      deleteListForCurl(arcZList1, arcZList2);
      return -1;
    }

    //...sets the best values
    HepGeom::Point3D<double> tmp1 = *arcZList1[flag1];
    HepGeom::Point3D<double> tmp2 = *arcZList2[flag2];
    link1.position(tmp1);
    link2.position(tmp2);
    //...deletes
    deleteListForCurl(arcZList1, arcZList2);
    return 0;
  }

#if defined(__GNUG__)
  int
  TArcOrder(const HepGeom::Point3D<double> **a, const HepGeom::Point3D<double> **b)
  {
    if ((*a)->x() < (*b)->x()) {
      return 1;
    } else if ((*a)->x() == (*b)->x()) {
      return 0;
    } else {
      return -1;
    }
  }
#else
  extern "C" int
  TArcOrder(const void* av, const void* bv)
  {
    const HepGeom::Point3D<double> **a((const HepGeom::Point3D<double> **)av);
    const HepGeom::Point3D<double> **b((const HepGeom::Point3D<double> **)bv);
    if ((*a)->x() < (*b)->x()) {
      return 1;
    } else if ((*a)->x() == (*b)->x()) {
      return 0;
    } else {
      return -1;
    }
  }
#endif

  int
  TTrack::stereoHitForCurl(TLink& link1, TLink& link2, TLink& link3) const
  {
    //...definition of the return values
    //   -1 = error
    //    0 = normal = can use 3 links
    //   12 = can use 1 and 2 only
    //   23 = can use 2 and 3 only

    int flag1 = 0;
    int flag2 = 0;
    int flag3 = 0;
    double minZ1   = 9999999.;
    double minZ2   = 9999999.;
    double minZ01  = 9999999.;
    double minZ12  = 9999999.;

    AList<Point3D> arcZList1;
    AList<Point3D> arcZList2;
    AList<Point3D> arcZList3;
    int ok1 = stereoHitForCurl(link1, arcZList1);
    int ok2 = stereoHitForCurl(link2, arcZList2);
    int ok3 = stereoHitForCurl(link3, arcZList3);

    if ((ok1 != 0 && ok2 != 0 && ok3 != 0) ||
        (ok1 == 0 && ok2 != 0 && ok3 != 0) ||
        (ok1 != 0 && ok2 == 0 && ok3 != 0) ||
        (ok1 != 0 && ok2 != 0 && ok3 == 0) ||
        (ok1 == 0 && ok2 != 0 && ok3 == 0)) {
      //...deletes
      deleteListForCurl(arcZList1, arcZList2, arcZList3);
      return -1;
    }

    if (ok1 == 0 && ok2 == 0 && ok3 == 0) {
      //...finds
//  double checkArc;
      int n1 = arcZList1.length();
      int n2 = arcZList2.length();
      int n3 = arcZList3.length();
      for (int i = 0; i < n1; i++) {
        for (int j = 0; j < n2; j++) {
          for (int k = 0; k < n3; k++) {
            AList<Point3D> arcZ;
            arcZ.append(*arcZList1[i]);
            arcZ.append(*arcZList2[j]);
            arcZ.append(*arcZList3[k]);
            arcZ.sort(TArcOrder);
            double z01 = fabs(arcZ[0]->y() - arcZ[1]->y());
            double z12 = fabs(arcZ[1]->y() - arcZ[2]->y());
            if (z01 <= minZ1 && z12 <= minZ2) {
              minZ1 = z01;
              minZ2 = z12;
              flag1 = i;
              flag2 = j;
              flag3 = k;
            }
          }
        }
      }
      if (minZ1 == 9999999. ||
          minZ2 == 9999999.) {
        deleteListForCurl(arcZList1, arcZList2, arcZList3);
        return -1;
      }
      //...sets the best values
      HepGeom::Point3D<double> tmp1 = *arcZList1[flag1];
      HepGeom::Point3D<double> tmp2 = *arcZList2[flag2];
      HepGeom::Point3D<double> tmp3 = *arcZList3[flag3];
      link1.position(tmp1);
      link2.position(tmp2);
      link3.position(tmp3);
      deleteListForCurl(arcZList1, arcZList2, arcZList3);
      return 0;
    } else if (ok1 == 0 && ok2 == 0 && ok3 != 0) {
      int n1 = arcZList1.length();
      int n2 = arcZList2.length();
      for (int i = 0; i < n1; i++) {
        for (int j = 0; j < n2; j++) {
          if (fabs(arcZList1[i]->y() - arcZList2[j]->y()) < minZ01) {
            minZ01 = fabs(arcZList1[i]->y() - arcZList2[j]->y());
            flag1 = i;
            flag2 = j;
          }
        }
      }
      if (minZ01 == 9999999.) {
        deleteListForCurl(arcZList1, arcZList2, arcZList3);
        return -1;
      }
      //...sets the best values
      HepGeom::Point3D<double> tmp1 = *arcZList1[flag1];
      HepGeom::Point3D<double> tmp2 = *arcZList2[flag2];
      link1.position(tmp1);
      link2.position(tmp2);
      deleteListForCurl(arcZList1, arcZList2, arcZList3);
      return 12;
    } else if (ok1 != 0 && ok2 == 0 && ok3 == 0) {
      int n2 = arcZList2.length();
      int n3 = arcZList3.length();
      for (int i = 0; i < n2; i++) {
        for (int j = 0; j < n3; j++) {
          if (fabs(arcZList2[i]->y() - arcZList3[j]->y()) < minZ12) {
            minZ12 = fabs(arcZList2[i]->y() - arcZList3[j]->y());
            flag2 = i;
            flag3 = j;
          }
        }
      }
      if (minZ12 == 9999999.) {
        deleteListForCurl(arcZList1, arcZList2, arcZList3);
        return -1;
      }
      //...sets the best values
      HepGeom::Point3D<double> tmp2 = *arcZList2[flag2];
      HepGeom::Point3D<double> tmp3 = *arcZList3[flag3];
      link1.position(tmp2);
      link2.position(tmp3);
      deleteListForCurl(arcZList1, arcZList2, arcZList3);
      return 23;
    } else {
      deleteListForCurl(arcZList1, arcZList2, arcZList3);
      return -1;
    }
  }

  void
  TTrack::deleteListForCurl(AList<Point3D> &l1,
                            AList<Point3D> &l2) const
  {
    HepAListDeleteAll(l1);
    HepAListDeleteAll(l2);
  }

  void
  TTrack::deleteListForCurl(AList<Point3D> &l1,
                            AList<Point3D> &l2,
                            AList<Point3D> &l3) const
  {
    HepAListDeleteAll(l1);
    HepAListDeleteAll(l2);
    HepAListDeleteAll(l3);
  }
#endif //OLD_STEREO

  int
  TTrack::stereoHitForCurl(AList<TLink> & list) const
  {
    if (list.length() == 0)return -1;

    HepGeom::Point3D<double> center = _helix->center();
    HepGeom::Point3D<double> tmp(-999., -999., 0.);
    double r  = fabs(_helix->curv());
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
#if 1
      Vector3D tp[2][2];
#endif
      if (ok_outer) {
        p[0][0] = x + l[0][0] * v;
        p[1][0] = x + l[1][0] * v;
#if 1
        tp[0][0] = p[0][0];
        tp[1][0] = p[1][0];
#endif
        Vector3D tmp_pc = p[0][0] - center;
        Vector3D pc0 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[0][0] -= drift / pc0.mag() * pc0;
        tmp_pc = p[1][0] - center;
        Vector3D pc1 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[1][0] -= drift / pc1.mag() * pc1;
      }
#define JJTEST 0
      if (ok_inner) {
        p[0][1] = x + l[0][1] * v;
        p[1][1] = x + l[1][1] * v;
#if JJTEST
        tp[0][1] = p[0][1];
        tp[1][1] = p[1][1];
#endif
        Vector3D tmp_pc = p[0][1] - center;
        Vector3D pc0 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[0][1] += drift / pc0.mag() * pc0;
        tmp_pc = p[1][1] - center;
        Vector3D pc1 = Vector3D(tmp_pc.x(), tmp_pc.y(), 0.);
        p[1][1] += drift / pc1.mag() * pc1;
      }

      //...boolean
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
        if (_charge * (center.x() * p[0][0].y() - center.y() * p[0][0].x())  < 0.)
          ok_xy[0][0] = false;
        if (_charge * (center.x() * p[1][0].y() - center.y() * p[1][0].x())  < 0.)
          ok_xy[1][0] = false;
      }
      if (ok_inner) {
        if (_charge * (center.x() * p[0][1].y() - center.y() * p[0][1].x())  < 0.)
          ok_xy[0][1] = false;
        if (_charge * (center.x() * p[1][1].y() - center.y() * p[1][1].x())  < 0.)
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
      unsigned index;
      index = 0;
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
        list[i]->arcZ(Point3D(r * dPhi, z[0][0], 0.), index);
        // std::cout << r*dPhi << ", " << z[0][0] << std::endl;
        ++index;
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
        list[i]->arcZ(Point3D(r * dPhi, z[1][0], 0.), index);
        // std::cout << r*dPhi << ", " << z[1][0] << std::endl;
        ++index;
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
        list[i]->arcZ(Point3D(r * dPhi, z[0][1], 0.), index);
        // std::cout << r*dPhi << ", " << z[0][1] << std::endl;
        ++index;
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
        list[i]->arcZ(Point3D(r * dPhi, z[1][1], 0.), index);
        // std::cout << r*dPhi << ", " << z[1][1] << std::endl;
        ++index;
      }

#if JJTEST
      double gmaxX = -9999. , gminX = 9999.;
      double gmaxY = -9999. , gminY = 9999.;
      FILE* gnuplot, *data;
      double step = 100.;
      double dStep = 2.*M_PI / step;
      if ((data = fopen("dat1", "w")) != NULL) {
        if (ok_xy[0][0]) {
          for (int ii = 0; ii < step; ++ii) {
            double X = tp[0][0].x() + drift * cos(dStep * static_cast<double>(ii));
            double Y = tp[0][0].y() + drift * sin(dStep * static_cast<double>(ii));
            std::fprintf(data, "%lf, %lf\n", X, Y);
            if (gmaxX < X)gmaxX = X;
            if (gminX > X)gminX = X;
            if (gmaxY < Y)gmaxY = Y;
            if (gminY > Y)gminY = Y;
          }
        }
        fclose(data);
      }
      if ((data = fopen("dat2", "w")) != NULL) {
        if (ok_xy[1][0]) {
          for (int ii = 0; ii < step; ++ii) {
            double X = tp[1][0].x() + drift * cos(dStep * static_cast<double>(ii));
            double Y = tp[1][0].y() + drift * sin(dStep * static_cast<double>(ii));
            std::fprintf(data, "%lf, %lf\n", X, Y);
            if (gmaxX < X)gmaxX = X;
            if (gminX > X)gminX = X;
            if (gmaxY < Y)gmaxY = Y;
            if (gminY > Y)gminY = Y;
          }
        }
        fclose(data);
      }
      if ((data = fopen("dat3", "w")) != NULL) {
        if (ok_xy[0][1]) {
          for (int ii = 0; ii < step; ++ii) {
            double X = tp[0][1].x() + drift * cos(dStep * static_cast<double>(ii));
            double Y = tp[0][1].y() + drift * sin(dStep * static_cast<double>(ii));
            std::fprintf(data, "%lf, %lf\n", X, Y);
            if (gmaxX < X)gmaxX = X;
            if (gminX > X)gminX = X;
            if (gmaxY < Y)gmaxY = Y;
            if (gminY > Y)gminY = Y;
          }
        }
        fclose(data);
      }
      if ((data = fopen("dat4", "w")) != NULL) {
        if (ok_xy[1][1]) {
          for (int ii = 0; ii < step; ++ii) {
            double X = tp[1][1].x() + drift * cos(dStep * static_cast<double>(ii));
            double Y = tp[1][1].y() + drift * sin(dStep * static_cast<double>(ii));
            std::fprintf(data, "%lf, %lf\n", X, Y);
            if (gmaxX < X)gmaxX = X;
            if (gminX > X)gminX = X;
            if (gmaxY < Y)gmaxY = Y;
            if (gminY > Y)gminY = Y;
          }
        }
        fclose(data);
      }
      Vector3D tX = h.wire().forwardPosition() - h.wire().backwardPosition();
      Vector3D tDist(tX.x(), tX.y(), 0.);
      double tD = tDist.mag();
      double vvvM = 1. / v.mag();
      Vector3D tDire = vvvM * v;
      step = 2.;
      dStep = tD / step;
      if ((data = fopen("dat5", "w")) != NULL) {
        for (int ii = 0; ii < step + 1; ++ii) {
          double X = h.wire().backwardPosition().x() + dStep * static_cast<double>(ii) * tDire.x();
          double Y = h.wire().backwardPosition().y() + dStep * static_cast<double>(ii) * tDire.y();
          std::fprintf(data, "%lf, %lf\n", X, Y);
          if (gmaxX < X)gmaxX = X;
          if (gminX > X)gminX = X;
          if (gmaxY < Y)gmaxY = Y;
          if (gminY > Y)gminY = Y;
        }
        fclose(data);
      }
      if ((data = fopen("dat6", "w")) != NULL) {
        double X = h.wire().backwardPosition().x();
        double Y = h.wire().backwardPosition().y();
        std::fprintf(data, "%lf, %lf\n", X, Y);
        if (gmaxX < X)gmaxX = X;
        if (gminX > X)gminX = X;
        if (gmaxY < Y)gmaxY = Y;
        if (gminY > Y)gminY = Y;
        fclose(data);
      }
      if ((data = fopen("dat7", "w")) != NULL) {
        double X = h.wire().forwardPosition().x();
        double Y = h.wire().forwardPosition().y();
        std::fprintf(data, "%lf, %lf\n", X, Y);
        if (gmaxX < X)gmaxX = X;
        if (gminX > X)gminX = X;
        if (gmaxY < Y)gmaxY = Y;
        if (gminY > Y)gminY = Y;
        fclose(data);
      }
      step = 300.;
      dStep = 2.*M_PI / step;
      if ((data = fopen("dat8", "w")) != NULL) {
        for (int ii = 0; ii < step; ++ii) {
          double X = center.x() + r * cos(dStep * static_cast<double>(ii));
          double Y = center.y() + r * sin(dStep * static_cast<double>(ii));
          std::fprintf(data, "%lf, %lf\n", X, Y);
        }
        fclose(data);
      }
      if ((data = fopen("dat9", "w")) != NULL) {
        if (ok_xy[0][0]) {
          double X = p[0][0].x();
          double Y = p[0][0].y();
          std::fprintf(data, "%lf, %lf\n", X, Y);
        }
        fclose(data);
      }
      if ((data = fopen("dat10", "w")) != NULL) {
        if (ok_xy[1][0]) {
          double X = p[1][0].x();
          double Y = p[1][0].y();
          std::fprintf(data, "%lf, %lf\n", X, Y);
        }
        fclose(data);
      }
      if ((data = fopen("dat11", "w")) != NULL) {
        if (ok_xy[0][1]) {
          double X = p[0][1].x();
          double Y = p[0][1].y();
          std::fprintf(data, "%lf, %lf\n", X, Y);
        }
        fclose(data);
      }
      if ((data = fopen("dat12", "w")) != NULL) {
        if (ok_xy[1][1]) {
          double X = p[1][1].x();
          double Y = p[1][1].y();
          std::fprintf(data, "%lf, %lf\n", X, Y);
        }
        fclose(data);
      }
      std::cout << "Drift Distance = " << drift << ", xy1cm -> z" << V.z() / v.mag() << "cm, xyDist(cm) = " << tD << std::endl;
      if (tX.z() < 0.)std::cout << "ERROR : F < B" << std::endl;
      if ((gnuplot = popen("gnuplot", "w")) != NULL) {
        std::fprintf(gnuplot, "set size 0.721,1.0 \n");
        std::fprintf(gnuplot, "set xrange [%f:%f] \n", gminX, gmaxX);
        std::fprintf(gnuplot, "set yrange [%f:%f] \n", gminY, gmaxY);
        std::fprintf(gnuplot, "plot \"dat1\" with line, \"dat2\" with line, \"dat3\" with line, \"dat4\" with line, \"dat5\" with line, \"dat6\", \"dat7\", \"dat8\" with line, \"dat9\", \"dat10\", \"dat11\", \"dat12\" \n");
        fflush(gnuplot);
        char tmp[8];
        gets(tmp);
        pclose(gnuplot);
      }
#endif
    }
    return 0;
  }

#if !(NEW_FIT2D)
  int
  TTrack::approach2D(TLink& l) const
  {

    //...Setup...
    const Belle2::TRGCDCWire& w = * l.wire();
    CLHEP::HepVector a = _helix->a();
    double kappa = a[2];
    double phi0  = a[1];
    HepGeom::Point3D<double> xc = _helix->center();
    HepGeom::Point3D<double> xw = w.xyPosition();
    HepGeom::Point3D<double> xt = _helix->x();
    Vector3D v0, v1;
    v0 = xt - xc;
    v1 = xw - xc;
    //if (_charge > 0.) {
    //v0 *= -1;
    //v1 *= -1;
    //}
    double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
    double vDot = v0.x() * v1.x() + v0.y() * v1.y();
    double dPhi = atan2(vCrs, vDot);

    //...Cal. phi to rotate...
    //double phiWire = atan2(_charge * (xc.y() - xw.y()),
    //                     _charge * (xc.x() - xw.x()));
    //phiWire = (phiWire > 0.) ? phiWire : phiWire + 2. * M_PI;
    //double dPhi = phiWire - phi0;

    //...Why this is needed?...
    //if ((_charge >= 0.) && (dPhi > 0.)) dPhi -= 2. * M_PI;
    //else if ((_charge < 0.) && (dPhi < 0.)) dPhi += 2. * M_PI;

    // std::cout << _helix->x(dPhi) << " , " << _helix->x(dPhi+0.2) << " , " << _helix->x(dPhi-0.1) << std::endl;

    l.positionOnTrack(_helix->x(dPhi));
    HepGeom::Point3D<double> x = xw;
    x.setZ(0.);
    l.positionOnWire(x);
    l.dPhi(dPhi);
    return 0;
  }

  int
  TTrack::dxda2D(const TLink& link,
                 double dPhi,
                 CLHEP::HepVector& dxda,
                 CLHEP::HepVector& dyda,
                 CLHEP::HepVector& dzda) const
  {

    //...Setup...
    const Belle2::TRGCDCWire& w = * link.wire();
    CLHEP::HepVector a = _helix->a();
    double dRho  = a[0];
    double phi0  = a[1];
    double kappa = a[2];
    double rho   = THelix::ConstantAlpha / kappa;
    //double tanLambda = a[4];

    double sinPhi0 = sin(phi0);
    double cosPhi0 = cos(phi0);
    double sinPhi0dPhi = sin(phi0 + dPhi);
    double cosPhi0dPhi = cos(phi0 + dPhi);
    CLHEP::HepVector dphida(5);

    HepGeom::Point3D<double> d = _helix->center() - w.xyPosition();
    double dmag2 = d.mag2();

    dphida[0] = (sinPhi0 * d.x() - cosPhi0 * d.y()) / dmag2;
    dphida[1] = (dRho + rho)    * (cosPhi0 * d.x() + sinPhi0 * d.y())
                / dmag2 - 1.;
    dphida[2] = (- rho / kappa) * (sinPhi0 * d.x() - cosPhi0 * d.y())
                / dmag2;
    dphida[3] = 0.;
    dphida[4] = 0.;

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

    dzda[0] = 0.;
    dzda[1] = 0.;
    dzda[2] = 0.;
    dzda[3] = 1.;
    dzda[4] = - rho * dPhi;
    //dzda[0] = - rho * tanLambda * dphida[0];
    //dzda[1] = - rho * tanLambda * dphida[1];
    //dzda[2] = rho / kappa * tanLambda * dPhi - rho * tanLambda * dphida[2];
    //dzda[3] = 1. - rho * tanLambda * dphida[3];
    //dzda[4] = - rho * dPhi - rho * tanLambda * dphida[4];

    return 0;
  }
#endif
#if 0
  int
  TTrack::svdHitForCurl(AList<TSvdHit>& svdList) const
  {
    HepGeom::Point3D<double> center = _helix->center();
    double r  = fabs(_helix->curv());

    for (unsigned i = 0, size = svdList.length(); i < size; ++i) {
      HepGeom::Point3D<double> p(svdList[i]->x(), svdList[i]->y(), 0.);
      double cosdPhi = - center.dot((p - center).unit()) / center.mag();
      double dPhi;
      if (fabs(cosdPhi) < 1.0) {
        dPhi = acos(cosdPhi);
      } else if (cosdPhi >= 1.0) {
        dPhi = 0.;
      } else {
        dPhi = M_PI;
      }
      HepGeom::Point3D<double> tmp(r * dPhi, svdList[i]->z(), 0.);
      svdList[i]->arcZ(tmp);
    }
    return 0;
  }
#endif

#if defined(__GNUG__)
  int
  TTrack::sortByPt(const TTrack** a, const TTrack** b)
  {
    if ((* a)->pt() < (* b)->pt()) return 1;
    else if
    ((* a)->pt() == (* b)->pt()) return 0;
    else return -1;
  }
#else
  extern "C" int
  SortByPt(const void* av, const void* bv)
  {
    const TTrack** a((const TTrack**)av);
    const TTrack** b((const TTrack**)bv);
    if ((* a)->pt() < (* b)->pt()) return 1;
    else if
    ((* a)->pt() == (* b)->pt()) return 0;
    else return -1;
  }
#endif

#if NEW_FIT2D
  int
  TTrack::fit2D(unsigned ipFlag, double ipDistance, double ipError)
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TTrack::fit2D(r-phi) ..." << std::endl;
#endif
    //if(_fitted)return 0;

    //...Check # of hits...

    // std::cout << "# = " << _links.length() << std::endl;
    //...Setup...
    unsigned nTrial(0);
    CLHEP::HepVector a(_helix->a());
    double chi2;
    double chi2Old = 1.0e+99;
    CLHEP::HepVector dchi2da(3);
    CLHEP::HepSymMatrix d2chi2d2a(3, 0);
    CLHEP::HepVector da(5), dxda(3), dyda(3);
    CLHEP::HepVector dDda(3);
    const double convergence = 1.0e-5;
    CLHEP::HepVector f(3);
    int err = 0;
    double factor = 1.0;
    unsigned usedWireNumber = 0;

    //...Fitting loop...
    while (nTrial < 100) {
      //...Set up...
      chi2 = 0.;
      for (unsigned j = 0; j < 3; ++j) dchi2da[j] = 0.;
      d2chi2d2a = CLHEP::HepSymMatrix(3, 0);
      usedWireNumber = 0;

      //...Loop with hits...
      unsigned i = 0;
      while (TLink* l = _links[i++]) {
        if (l->fit2D() == 0)continue;
        const Belle2::TRGCDCWireHit& h = *l->hit();

        //...Cal. closest points...
        if (approach2D(*l) != 0)continue;
        double dPhi = l->dPhi();
        const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
        const HepGeom::Point3D<double> & onWire  = l->positionOnWire();
        HepGeom::Point3D<double> onTrack2(onTrack.x(), onTrack.y(), 0.);
        HepGeom::Point3D<double> onWire2(onWire.x(), onWire.y(), 0.);

        //...Obtain drift distance and its error...
        unsigned leftRight = CellHitRight;
        if (onWire2.cross(onTrack2).z() < 0.) leftRight = CellHitLeft;
        double distance   = h.drift(leftRight);
        double eDistance  = h.dDrift(leftRight);
        double eDistance2 = eDistance * eDistance;
        if (eDistance == 0.) {
          std::cout << "Error(?) : Drift Distance Error = 0 in fit2D of Trasan." << std::endl;
          continue;
        }

        //...Residual...
        Vector3D v = onTrack2 - onWire2;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        //...dxda...
        if (this->dxda2D(*l, dPhi, dxda, dyda) != 0)continue;

        //...Chi2 related...
        //HepGeom::Vector3D<double> vw(0.,0.,1.);
        dDda = (vmag > 0.)
               ? (v.x() * dxda + v.y() * dyda) / vmag
               : CLHEP::HepVector(3, 0);
        if (vmag <= 0.0) {
          std::cout << "    in fit2D " << onTrack << ", " << onWire;
          h.dump();
          continue;
        }
        dchi2da     += (dDistance / eDistance2) * dDda;
        d2chi2d2a   += vT_times_v(dDda) / eDistance2;
        double pChi2 = dDistance * dDistance / eDistance2;
        chi2        += pChi2;

        //...Store results...
        l->update(onTrack2, onWire2, leftRight, pChi2);
        ++usedWireNumber;
      }
      if (ipFlag != 0) {
//    double kappa = _helix->a()[2];
//    double phi0  = _helix->a()[1];
        HepGeom::Point3D<double> xc(_helix->center());
        HepGeom::Point3D<double> onWire(0., 0., 0.);
        HepGeom::Point3D<double> xt(_helix->x());
        Vector3D v0(xt - xc);
        Vector3D v1(onWire - xc);
        double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
        double vDot = v0.x() * v1.x() + v0.y() * v1.y();
        double dPhi = atan2(vCrs, vDot);
        HepGeom::Point3D<double> onTrack(_helix->x(dPhi).x(), _helix->x(dPhi).y(), 0.);
        double distance   = ipDistance;
        double eDistance  = ipError;
        double eDistance2 = eDistance * eDistance;

        Vector3D v = onTrack - onWire;
        double vmag = v.mag();
        double dDistance = vmag - distance;

        if (this->dxda2D(dPhi, dxda, dyda) != 0)goto ipOff;

        dDda = (vmag > 0.)
               ? (v.x() * dxda + v.y() * dyda) / vmag
               : CLHEP::HepVector(3, 0);
        if (vmag <= 0.0) {
          goto ipOff;
        }
        dchi2da     += (dDistance / eDistance2) * dDda;
        d2chi2d2a   += vT_times_v(dDda) / eDistance2;
        double pChi2 = dDistance * dDistance / eDistance2;
        chi2        += pChi2;

        ++usedWireNumber;
      }
ipOff:
      if (usedWireNumber < 4) {
        err = -2;
        break;
      }

      //...Check condition...
      double change = chi2Old - chi2;
      if (fabs(change) < convergence)break;
      if (change < 0.) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "chi2Old, chi2=" << chi2Old << " " << chi2 << std::endl;
#endif
        //change to the old value.
        a += factor * da;
        _helix->a(a);

        chi2 = 0.;
        for (unsigned j = 0; j < 3; ++j) dchi2da[j] = 0.;
        d2chi2d2a = CLHEP::HepSymMatrix(3, 0);
        usedWireNumber = 0;

        //...Loop with hits...
        unsigned i = 0;
        while (TLink* l = _links[i++]) {
          if (l->fit2D() == 0)continue;
          const Belle2::TRGCDCWireHit& h = * l->hit();

          //...Cal. closest points...
          if (approach2D(*l) != 0)continue;
          double dPhi = l->dPhi();
          const HepGeom::Point3D<double> & onTrack = l->positionOnTrack();
          const HepGeom::Point3D<double> & onWire  = l->positionOnWire();
          Point3D onTrack2(onTrack.x(), onTrack.y(), 0.);
          Point3D onWire2(onWire.x(), onWire.y(), 0.);

          //...Obtain drift distance and its error...
          unsigned leftRight = CellHitRight;
          if (onWire2.cross(onTrack2).z() < 0.) leftRight = CellHitLeft;
          double distance   = h.drift(leftRight);
          double eDistance  = h.dDrift(leftRight);
          double eDistance2 = eDistance * eDistance;

          //...Residual...
          Vector3D v = onTrack2 - onWire2;
          double vmag = v.mag();
          double dDistance = vmag - distance;

          //...dxda...
          if (this->dxda2D(*l, dPhi, dxda, dyda) != 0)continue;

          //...Chi2 related...
          dDda = (vmag > 0.)
                 ? (v.x() * dxda + v.y() * dyda) / vmag
                 : CLHEP::HepVector(3, 0);
          if (vmag <= 0.0) {
            std::cout << "    in fit2D " << onTrack << ", " << onWire;
            h.dump();
            continue;
          }
          dchi2da     += (dDistance / eDistance2) * dDda;
          d2chi2d2a   += vT_times_v(dDda) / eDistance2;
          double pChi2 = dDistance * dDistance / eDistance2;
          chi2        += pChi2;

          //...Store results...
          l->update(onTrack2, onWire2, leftRight, pChi2);
          ++usedWireNumber;
        }
        if (ipFlag != 0) {
//  double kappa = _helix->a()[2];
//  double phi0  = _helix->a()[1];
          Point3D xc(_helix->center());
          Point3D onWire(0., 0., 0.);
          Point3D xt(_helix->x());
          Vector3D v0(xt - xc);
          Vector3D v1(onWire - xc);
          double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
          double vDot = v0.x() * v1.x() + v0.y() * v1.y();
          double dPhi = atan2(vCrs, vDot);
          Point3D onTrack(_helix->x(dPhi).x(), _helix->x(dPhi).y(), 0.);
          double distance   = ipDistance;
          double eDistance  = ipError;
          double eDistance2 = eDistance * eDistance;

          Vector3D v = onTrack - onWire;
          double vmag = v.mag();
          double dDistance = vmag - distance;

          if (this->dxda2D(dPhi, dxda, dyda) != 0)goto ipOff2;

          dDda = (vmag > 0.)
                 ? (v.x() * dxda + v.y() * dyda) / vmag
                 : CLHEP::HepVector(3, 0);
          if (vmag <= 0.0) {
            goto ipOff2;
          }
          dchi2da     += (dDistance / eDistance2) * dDda;
          d2chi2d2a   += vT_times_v(dDda) / eDistance2;
          double pChi2 = dDistance * dDistance / eDistance2;
          chi2        += pChi2;

          ++usedWireNumber;
        }
ipOff2:
        if (usedWireNumber < 4) {
          err = -2;
          break;
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
      f = solve(d2chi2d2a, dchi2da);
      da[0] = f[0];
      da[1] = f[1];
      da[2] = f[2];
      da[3] = 0.;
      da[4] = 0.;

      a -= factor * da;
      _helix->a(a);
      ++nTrial;
    }
    if (err) {
      return err;
    }

    //...Cal. error matrix...
    CLHEP::HepSymMatrix Ea(5, 0);
    unsigned dim = 3;
    CLHEP::HepSymMatrix Eb = d2chi2d2a;
    CLHEP::HepSymMatrix Ec = Eb.inverse(err);
    Ea[0][0] = Ec[0][0];
    Ea[0][1] = Ec[0][1];
    Ea[0][2] = Ec[0][2];
    Ea[1][1] = Ec[1][1];
    Ea[1][2] = Ec[1][2];
    Ea[2][2] = Ec[2][2];

    //...Store information...
    if (!err) {
      _helix->a(a);
      _helix->Ea(Ea);
    } else {
      err = -2;
    }

    _ndf  = usedWireNumber - dim;
    _chi2 = chi2;

    //_fitted = true;

#define JJJTEST 0
#if JJJTEST
    double gmaxX = -9999. , gminX = 9999.;
    double gmaxY = -9999. , gminY = 9999.;
    FILE* gnuplot, *data;
    double step = 200.;
    double dStep = 2.*M_PI / step;
    for (int i = 0, size = _links.length(); i < size; ++i) {
      TLink* l = _links[i];
      double drift = l->hit()->distance(0);
      char name[100] = "dat";
      char counter[10] = "";
      std::sprintf(counter, "%02d", i);
      strcat(name, counter);
      if ((data = fopen(name, "w")) != NULL) {
        for (int ii = 0; ii < step; ++ii) {
          double X = l->wire().xyPosition().x() + drift * cos(dStep * static_cast<double>(ii));
          double Y = l->wire()->xyPosition().y() + drift * sin(dStep * static_cast<double>(ii));
          std::fprintf(data, "%lf, %lf\n", X, Y);
          if (gmaxX < X)gmaxX = X;
          if (gminX > X)gminX = X;
          if (gmaxY < Y)gmaxY = Y;
          if (gminY > Y)gminY = Y;
        }
        fclose(data);
      }
    }
    step = 300.;
    dStep = 2.*M_PI / step;
    if ((data = fopen("datc", "w")) != NULL) {
      for (int ii = 0; ii < step; ++ii) {
        double X = _helix->center().x() + _helix->radius() * cos(dStep * static_cast<double>(ii));
        double Y = _helix->center().y() + _helix->radius() * sin(dStep * static_cast<double>(ii));
        std::fprintf(data, "%lf, %lf\n", X, Y);
      }
      fclose(data);
    }
    if ((gnuplot = popen("gnuplot", "w")) != NULL) {
      std::fprintf(gnuplot, "set size 0.721,1.0 \n");
      std::fprintf(gnuplot, "set xrange [%f:%f] \n", gminX, gmaxX);
      std::fprintf(gnuplot, "set yrange [%f:%f] \n", gminY, gmaxY);
      if (_links.length() == 4) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line \n");
      } else if (_links.length() == 5) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line \n");
      } else if (_links.length() == 6) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line, \"dat05\" with line \n");
      } else if (_links.length() == 7) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line, \"dat05\" with line, \"dat06\" with line \n");
      } else if (_links.length() == 8) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line, \"dat05\" with line, \"dat06\" with line, \"dat07\" with line \n");
      } else if (_links.length() == 9) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line, \"dat05\" with line, \"dat06\" with line, \"dat07\" with line, \"dat08\" with line \n");
      } else if (_links.length() == 10) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line, \"dat05\" with line, \"dat06\" with line, \"dat07\" with line, \"dat08\" with line, \"dat09\" with line \n");
      } else if (_links.length() >= 11) {
        std::fprintf(gnuplot, "plot \"datc\" with line, \"dat00\" with line, \"dat01\" with line, \"dat02\" with line, \"dat03\" with line, \"dat04\" with line, \"dat05\" with line, \"dat06\" with line, \"dat07\" with line, \"dat08\" with line, \"dat09\" with line, \"dat10\" with line \n");
      }
      fflush(gnuplot);
      char tmp[8];
      gets(tmp);
      pclose(gnuplot);
    }
#endif //JJJTEST

    return err;
  }

  int
  TTrack::approach2D(TLink& l) const
  {

    const Belle2::TRGCDCWire& w = *l.wire();
//double kappa = _helix->a()[2];
//double phi0  = _helix->a()[1];
    HepGeom::Point3D<double> xc(_helix->center());
    HepGeom::Point3D<double> xw(w.xyPosition());
    HepGeom::Point3D<double> xt(_helix->x());
    Vector3D v0(xt - xc);
    Vector3D v1(xw - xc);

    double vCrs = v0.x() * v1.y() - v0.y() * v1.x();
    double vDot = v0.x() * v1.x() + v0.y() * v1.y();
    double dPhi = atan2(vCrs, vDot);

    xt = _helix->x(dPhi);
    xt.setZ(0.);
    l.positionOnTrack(xt);
    xw.setZ(0.);
    l.positionOnWire(xw);
    l.dPhi(dPhi);
    return 0;
  }

  int
  TTrack::dxda2D(const TLink& link,
                 double dPhi,
                 CLHEP::HepVector& dxda,
                 CLHEP::HepVector& dyda) const
  {

    //...Setup...
    double kappa = (_helix->a())[2];
    if (kappa == 0.) {
      std::cout << "Error(?) : kappa == 0 in dxda2D of Trasan." << std::endl;
      return 1;
    }
    const Belle2::TRGCDCWire& w = *link.wire();
    double dRho  = (_helix->a())[0];
    double phi0  = (_helix->a())[1];
    double rho   = THelix::ConstantAlpha / kappa;

    double sinPhi0 = sin(phi0);
    double cosPhi0 = cos(phi0);
    double sinPhi0dPhi = sin(phi0 + dPhi);
    double cosPhi0dPhi = cos(phi0 + dPhi);
    CLHEP::HepVector dphida(3);

    HepGeom::Point3D<double> d = _helix->center() - w.xyPosition();
    d.setZ(0.);
    double dmag2 = d.mag2();

    if (dmag2 == 0.) {
      std::cout << "Error(?) : Distance(center-xyPosition) == 0 in dxda2D of Trasan." << std::endl;
      return 1;
    }

    dphida[0] = (sinPhi0 * d.x() - cosPhi0 * d.y()) / dmag2;
    dphida[1] = (dRho + rho) * (cosPhi0 * d.x() + sinPhi0 * d.y()) / dmag2 - 1.;
    dphida[2] = (-rho / kappa) * (sinPhi0 * d.x() - cosPhi0 * d.y()) / dmag2;

    dxda[0] = cosPhi0 + rho * sinPhi0dPhi * dphida[0];
    dxda[1] = -(dRho + rho) * sinPhi0 + rho * sinPhi0dPhi * (1. + dphida[1]);
    dxda[2] = -rho / kappa * (cosPhi0 - cosPhi0dPhi) + rho * sinPhi0dPhi * dphida[2];

    dyda[0] = sinPhi0 - rho * cosPhi0dPhi * dphida[0];
    dyda[1] = (dRho + rho) * cosPhi0 - rho * cosPhi0dPhi * (1. + dphida[1]);
    dyda[2] = -rho / kappa * (sinPhi0 - sinPhi0dPhi) - rho * cosPhi0dPhi * dphida[2];

    return 0;
  }

  int
  TTrack::dxda2D(double dPhi,
                 CLHEP::HepVector& dxda,
                 CLHEP::HepVector& dyda) const
  {

    //...Setup...
    double kappa = (_helix->a())[2];
    if (kappa == 0.) {
      std::cout << "Error(?) : kappa == 0 in dxda2D of Trasan." << std::endl;
      return 1;
    }
    double dRho  = (_helix->a())[0];
    double phi0  = (_helix->a())[1];
    double rho   = THelix::ConstantAlpha / kappa;

    double sinPhi0 = sin(phi0);
    double cosPhi0 = cos(phi0);
    double sinPhi0dPhi = sin(phi0 + dPhi);
    double cosPhi0dPhi = cos(phi0 + dPhi);
    CLHEP::HepVector dphida(3);

    HepGeom::Point3D<double> d = _helix->center(); // d = center - (0,0,0)
    d.setZ(0.);
    double dmag2 = d.mag2();

    if (dmag2 == 0.) {
      std::cout << "Error(?) : Distance(center-xyPosition) == 0 in dxda2D of Trasan." << std::endl;
      return 1;
    }

    dphida[0] = (sinPhi0 * d.x() - cosPhi0 * d.y()) / dmag2;
    dphida[1] = (dRho + rho) * (cosPhi0 * d.x() + sinPhi0 * d.y()) / dmag2 - 1.;
    dphida[2] = (-rho / kappa) * (sinPhi0 * d.x() - cosPhi0 * d.y()) / dmag2;

    dxda[0] = cosPhi0 + rho * sinPhi0dPhi * dphida[0];
    dxda[1] = -(dRho + rho) * sinPhi0 + rho * sinPhi0dPhi * (1. + dphida[1]);
    dxda[2] = -rho / kappa * (cosPhi0 - cosPhi0dPhi) + rho * sinPhi0dPhi * dphida[2];

    dyda[0] = sinPhi0 - rho * cosPhi0dPhi * dphida[0];
    dyda[1] = (dRho + rho) * cosPhi0 - rho * cosPhi0dPhi * (1. + dphida[1]);
    dyda[2] = -rho / kappa * (sinPhi0 - sinPhi0dPhi) - rho * cosPhi0dPhi * dphida[2];

    return 0;
  }
#endif

  unsigned
  TTrack::defineType(void) const
  {
    bool highPt = true;
    if (pt() < 0.25) highPt = false;
    bool fromIP = true;
    if (fabs(impact()) > 8.3) fromIP = false;

    if (fromIP && highPt) return _type = TrackTypeNormal;
    else if (fromIP && (! highPt)) return _type = TrackTypeCurl;

    if ((fabs(radius()) * 2. + fabs(impact())) < 87.)
      return _type = TrackTypeCircle;
    return _type = TrackTypeCosmic;
  }

  std::string
  TrackType(unsigned type)
  {
    switch (type) {
      case TrackTypeUndefined:
        return std::string("undefined");
      case TrackTypeNormal:
        return std::string("normal");
      case TrackTypeCurl:
        return std::string("curl  ");
      case TrackTypeCircle:
        return std::string("circle");
      case TrackTypeCosmic:
        return std::string("cosmic");
    }
    return std::string("unknown  ");
  }

#ifdef OPTNK
#define t_dot(a,b) (a[0]*b[0]+a[1]*b[1]+a[2]*b[2])
#define t_dot2(a,b) (a[0]*b[0]+a[1]*b[1])
#define t_print(a,b) std::cout<< b << " = " << a[0] << " " << a[1] << " " << a[2] << std::endl;
#endif


  int
  TTrack::approach(TLink& link, bool doSagCorrection) const
  {

    //...Cal. dPhi to rotate...
    const Belle2::TRGCDCWire& w = * link.wire();
    double wp[3]; w.xyPosition(wp);
    double wb[3]; w.backwardPosition(wb);
    double v[3];
    v[0] = w.direction().x();
    v[1] = w.direction().y();
    v[2] = w.direction().z();

    //...Sag correction...
    if (doSagCorrection) {
      std::cout << "TTrack::approach !!! sag correction is not implemented"
                << std::endl;
//  Vector3D dir = w.direction();
//  Point3D xw(wp[0], wp[1], wp[2]);
//  Point3D wireBackwardPosition(wb[0], wb[1], wb[2]);
//  w.wirePosition(link.positionOnTrack().z(),
//           xw,
//           wireBackwardPosition,
//           dir);
//  v[0] = dir.x();
//  v[1] = dir.y();
//  v[2] = dir.z();
//  wp[0] = xw.x();
//  wp[1] = xw.y();
//  wp[2] = xw.z();
//  wb[0] = wireBackwardPosition.x();
//  wb[1] = wireBackwardPosition.y();
//  wb[2] = wireBackwardPosition.z();
    }

    //...Cal. dPhi to rotate...
    double dPhi;
#if defined(BELLE_DEBUG)
    try {
#endif
      const HepGeom::Point3D<double> & xc = _helix->center();
      double xt[3]; _helix->x(0., xt);
      double x0 = - xc.x();
      double y0 = - xc.y();
      double x1 = wp[0] + x0;
      double y1 = wp[1] + y0;
      x0 += xt[0];
      y0 += xt[1];
      dPhi = atan2(x0 * y1 - y0 * x1, x0 * x1 + y0 * y1);
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "TTrack::helix is invalid" << std::endl;
      //      dump("Badhelix","Trasan::TTrack::approach");
      return -1;
    }
#endif

    //...Setup...
    double kappa = _helix->kappa();
    double phi0 = _helix->phi0();

    //...Axial case...
    if (w.axial()) {
      link.positionOnTrack(_helix->x(dPhi));
      Point3D x(wp[0], wp[1], wp[2]);
      x.setZ(link.positionOnTrack().z());
      link.positionOnWire(x);
      link.dPhi(dPhi);
      return 0;
    }

#ifdef TRASAN_DEBUG
    double firstdfdphi = 0.;
    static bool first = true;
    if (first) {
//cnv   extern BelleTupleManager * BASF_Histogram;
//  BelleTupleManager * m = BASF_Histogram;
//  h_nTrial = m->histogram("TTrack::approach nTrial", 100, 0., 100.);
    }
#endif

    //...Stereo case...
    double rho = THelix::ConstantAlpha / kappa;
    double tanLambda = _helix->tanl();
    static CLHEP::HepVector x(3);
    double t_x[3];
    double t_dXdPhi[3];
    const double convergence = 1.0e-5;
    double l;
    unsigned nTrial = 0;
    while (nTrial < 100) {

      x = link.positionOnTrack(_helix->x(dPhi));
      t_x[0] = x[0];
      t_x[1] = x[1];
      t_x[2] = x[2];

      l = v[0] * t_x[0] + v[1] * t_x[1] + v[2] * t_x[2]
          - v[0] * wb[0] - v[1] * wb[1] - v[2] * wb[2];

      double rcosPhi = rho * cos(phi0 + dPhi);
      double rsinPhi = rho * sin(phi0 + dPhi);
      t_dXdPhi[0] =   rsinPhi;
      t_dXdPhi[1] = - rcosPhi;
      t_dXdPhi[2] = - rho * tanLambda;

      //...f = d(Distance) / d phi...
      double t_d2Xd2Phi[2];
      t_d2Xd2Phi[0] = rcosPhi;
      t_d2Xd2Phi[1] = rsinPhi;

      //...iw new...
      double n[3];
      n[0] = t_x[0] - wb[0];
      n[1] = t_x[1] - wb[1];
      n[2] = t_x[2] - wb[2];

      double a[3];
      a[0] = n[0] - l * v[0];
      a[1] = n[1] - l * v[1];
      a[2] = n[2] - l * v[2];
      double dfdphi = a[0] * t_dXdPhi[0]
                      + a[1] * t_dXdPhi[1]
                      + a[2] * t_dXdPhi[2];

#ifdef TRASAN_DEBUG
      if (nTrial == 0) {
//      break;
        firstdfdphi = dfdphi;
      }

      //...Check bad case...
      if (nTrial > 3) {
        std::cout << Tab() << "TTrack::approach:" << w.name() << " "
                  << "dfdphi(0)=" << firstdfdphi
                  << ",(" << nTrial << ")=" << dfdphi << std::endl;
      }
#endif

      //...Is it converged?...
      if (fabs(dfdphi) < convergence)
        break;

      double dv = v[0] * t_dXdPhi[0]
                  + v[1] * t_dXdPhi[1]
                  + v[2] * t_dXdPhi[2];
      double t0 = t_dXdPhi[0] * t_dXdPhi[0]
                  + t_dXdPhi[1] * t_dXdPhi[1]
                  + t_dXdPhi[2] * t_dXdPhi[2];
      double d2fd2phi = t0 - dv * dv
                        + a[0] * t_d2Xd2Phi[0]
                        + a[1] * t_d2Xd2Phi[1];
//      + a[2] * t_d2Xd2Phi[2];

      dPhi -= dfdphi / d2fd2phi;

//    std::cout<< "nTrial=" << nTrial << std::endl;
//      std::cout<< "iw f,df,dphi=" << dfdphi << "," << d2fd2phi << "," << dPhi << std::endl;

      ++nTrial;
    }

    //...Cal. positions...
    link.positionOnWire(Point3D(wb[0] + l * v[0],
                                wb[1] + l * v[1],
                                wb[2] + l * v[2]));
    link.dPhi(dPhi);

#ifdef TRASAN_DEBUG
//cnv    h_nTrial->accumulate((float) nTrial + .5);
#endif

    return nTrial;
  }

  void
  TTrack::relationClusterWithWire()
  {
//----------------------------------------------------------
// Note: Selection of associating cluster to TLink is done.
//       if appropriate cluster was found ,
//       relation to cluster is set to the TLink.
//----------------------------------------------------------

//cnv   int j = 0;
//   while( TLink *l = _links[j] ) {

//   // initialization
//    int flag(-1);
//    l->setusecathode(0);
//    double dPhi = l->dPhi();
//    l->setZphiBeforeCathode( _helix->x(dPhi).z() );

//    int k1  = 0;
//    while( TRGCDCCatHit *c = _catHits[k1] ){

//    // Matching of layer
//     if( c->layerID() != l->hit()->wire().layerId() ) {
//        k1++; continue;
//        }

//    // Matching of wire hit
//     AList<TRGCDCWireHit>& cwire = c->candwire();
//     AList<TRGCDCClust>&  cclust = c->candclust();

//     if( cwire.length() == 0 || cclust.length() == 0 ){
//  k1++; continue;
//        }

//     int k2 = 0;
//     while( TRGCDCWireHit *cw = cwire[k2] ){
//       if( cw == l->hit() ){  flag = 1; break; }
//  k2++;
//      }

//     if( flag == -1 ){
//        k1++; continue;
//     }

//    // Selection of cluster
//     flag = -1;
//     float cand_sector = CathodeSectorId( cwire[k2]->wire().id());

//     // --- debug ---
//     //     std::cout<< "cand_sector = " << cand_sector << std::endl;
//     //--- debug ---

//     cand_sector = cand_sector - (int(cand_sector/8))*8;

//     // --- debug ---
//     // std::cout << "cand_sector(local) = " << cand_sector << std::endl;
//     // --- debug ---

//     int count_at_boundary(0);
//     float tmaxpad_at_boundary(0.);
//     TRGCDCClust * cc_at_boundary = NULL;

//     k2 = 0;
//     while( TRGCDCClust *cc = cclust[k2] ){

//       unsigned cathit_sector = cc->maxpad()->sectorId();

//     if( cand_sector == float(cathit_sector) ) {
//       l->setusecathode(1);  l->setmclust(cc); break ;
//     } else if( abs( cand_sector - float(cathit_sector)) == 0.5 ) {

//  count_at_boundary += 1;
//  l->setusecathode(1);
//  l->setmclust(cc);

//  if( count_at_boundary == 1 ) {
//        cc_at_boundary = cc;
//        tmaxpad_at_boundary = cc->tmaxpad();
//  }

//  //.. if 2 candidates exist at sector boundary,
//  //   choose the best matching of T between cluster and wire
//         if( count_at_boundary == 2 ) {
//    if( fabs( cc->tmaxpad() - l->hit()->reccdc()->m_tdc )
//             > fabs( tmaxpad_at_boundary - l->hit()->reccdc()->m_tdc ) )
//        l->setmclust(cc_at_boundary);
//    break;
//          }

//     }

//   k2++;
//  }
//     k1++;
//     } // TRGCDCCatHit loop

//    j++;
//    } // TLink loop

  }

// addition by matsu ( 1999/07/05 )
  void TTrack::relationClusterWithLayer(int SysCorr)
  {

//     //... selection of cathode cluster
//     for(unsigned it0=0;it0<3;it0++){

//       AList<TLink> catLink = TLink::sameLayer( cores(), it0 );

//       unsigned n(catLink.length());
//       if( !n ) continue;

//       int BestID(-1);

//       double tmpz(DBL_MAX);
//       for(unsigned it1=0;it1<n;it1++){
//         TLink & l = * catLink[it1];

//        if( l.getmclust() && l.usecathode() != 0 ){

//      l.setusecathode(2);

//           double Zdiff(l.positionOnTrack().z());
//           if(SysCorr ) Zdiff -=  l.getmclust()->zclustWithSysCorr();
//           else         Zdiff -=  l.getmclust()->zclust();
//           if( fabs(Zdiff) < tmpz ){
//           tmpz = fabs(Zdiff);  BestID = it1;
//           }
//        }
//       }

//    if( BestID == -1 ) continue;

//      for(unsigned it1=0;it1<n;it1++){
//      TLink & l = * catLink[it1];
//      if( l.getmclust() && l.usecathode() != 0 ){
//         if((int) it1 == BestID ) {
//    l.setusecathode(3); break;
//  }
//        }
//      }
//    }
  }

  int
  TTrack::szPosition(TLink& link) const
  {
    const Belle2::TRGCDCWireHit& h = * link.hit();
    Vector3D X = 0.5 * (h.wire().forwardPosition()
                        + h.wire().backwardPosition());
    //    double theta = atan2(X.y(), X.x());
    //    Vector3D lr(h.distance(CellHitLeft) * sin(theta),
    //    - h.distance(CellHitLeft) * cos(theta),
    //    0.);

    Vector3D xx = Vector3D(X.x(), X.y(), 0.);
    HepGeom::Point3D<double> center = _helix->center();
    Vector3D yy = center - xx;
    Vector3D ww = Vector3D(yy.x(), yy.y(), 0.);
    double wwmag2 = ww.mag2();
    double wwmag = sqrt(wwmag2);
    Vector3D lr(h.drift(CellHitLeft) / wwmag * ww.x(),
                h.drift(CellHitLeft) / wwmag * ww.y(),
                0.);

    //...Check left or right...
    if (link.leftRight() == CellHitRight) lr = - lr;
    else if (link.leftRight() == 2) lr = ORIGIN;
    X += lr;

    //...Prepare vectors...
    //    HepGeom::Point3D<double> center = _helix->center();
    HepGeom::Point3D<double> tmp(-9999., -9999., 0.);
    Vector3D x = Vector3D(X.x(), X.y(), 0.);
    Vector3D w = x - center;
    Vector3D V = h.wire().direction();
    Vector3D v = Vector3D(V.x(), V.y(), 0.);
    double vmag2 = v.mag2();
//  double vmag = sqrt(vmag2);

    double r = _helix->curv();
    double wv = w.dot(v);
    double d2 = wv * wv - vmag2 * (w.mag2() - r * r);

    //...No crossing in R/Phi plane... This is too tight...

    if (d2 < 0.) {
      link.position(tmp);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TTrack ... stereo: 0. > d2 = " << d2 << " "
                << link.leftRight() << std::endl;
#endif
      return -1;
    }
    double d = sqrt(d2);

    //...Cal. length to crossing points...
    double l[2];
    l[0] = (- wv + d) / vmag2;
    l[1] = (- wv - d) / vmag2;

    //...Cal. z of crossing points...
    bool ok[2];
    ok[0] = true;
    ok[1] = true;
    double z[2];
    z[0] = X.z() + l[0] * V.z();
    z[1] = X.z() + l[1] * V.z();

#ifdef TRASAN_DEBUG_DETAIL
    // std::cout << "    l0, l1 = " << l[0] << ", " << l[1] << std::endl;
    // std::cout << "    z0, z1 = " << z[0] << ", " << z[1] << std::endl;
#endif

    //...Check z position...
    if (link.leftRight() == 2) {
      if (z[0] < h.wire().backwardPosition().z() - 20.
          || z[0] > h.wire().forwardPosition().z() + 20.) ok[0] = false;
      if (z[1] < h.wire().backwardPosition().z() - 20.
          || z[1] > h.wire().forwardPosition().z() + 20.) ok[1] = false;
    } else {
      if (z[0] < h.wire().backwardPosition().z()
          || z[0] > h.wire().forwardPosition().z()) ok[0] = false;
      if (z[1] < h.wire().backwardPosition().z()
          || z[1] > h.wire().forwardPosition().z()) ok[1] = false;
    }
    if ((! ok[0]) && (! ok[1])) {
      link.position(tmp);
      return -2;
    }

    //...Cal. xy position of crossing points...
    Vector3D p[2];
    p[0] = x + l[0] * v;
    p[1] = x + l[1] * v;
    if (_charge * (center.x() * p[0].y() - center.y() * p[0].x())  < 0.)
      ok[0] = false;
    if (_charge * (center.x() * p[1].y() - center.y() * p[1].x())  < 0.)
      ok[1] = false;
    if ((! ok[0]) && (! ok[1])) {
      //        double tmp1 = _charge * (center.x() * p[0].y() - center.y() * p[0].x());
      //        double tmp2 = _charge * (center.x() * p[1].y() - center.y() * p[1].x()) ;
      //        if (link.leftRight() == 2) std::cout<<tmp1<<" "<<tmp2<<std::endl;
      link.position(tmp);
      return -3;
    }

    //...Which one is the best?... Study needed...
    unsigned best = 0;
    if (ok[1]) best = 1;

    //...Cal. arc length...
    double cosdPhi = - center.dot((p[best] - center).unit()) / center.mag();
    double dPhi;
    if (fabs(cosdPhi) <= 1.0) {
      dPhi = acos(cosdPhi);
    } else if (cosdPhi > 1.0) {
      dPhi = 0.0;
    } else {
      dPhi = M_PI;
    }

    //...Finish...
    tmp.setX(r * dPhi);
    tmp.setY(z[best]);
    link.position(tmp);

    return 0;
  }

  int
  TTrack::szPosition(const TSegment& segment, TLink& a) const
  {

    //...Pick up a wire which represents segment position...
    AList<TLink> links = segment.cores();
    unsigned n = links.length();
    if (! n) return -1;
    TLink* minL = links[0];
    float minDist = links[0]->drift();
    for (unsigned i = 1; i < n; i++) {
      if (links[i]->hit()->drift() < minDist) {
        minDist = links[i]->drift();
        minL = links[i];
      }
    }

    //...sz calculation...
    a.position(minL->position());
    a.leftRight(2);
    a.hit(minL->hit());
    int err = szPosition(a);
    if (err) return -2;
    return 0;
  }

  int
  TTrack::szPosition(const HepGeom::Point3D<double> & p, HepGeom::Point3D<double> & sz) const
  {

    //...Cal. arc length...
    HepGeom::Point3D<double> center = _helix->center();
    HepGeom::Point3D<double> xy = p;
    xy.setZ(0.);
    double cosdPhi = - center.dot((xy - center).unit()) / center.mag();
    double dPhi;
    if (fabs(cosdPhi) <= 1.0) {
      dPhi = acos(cosdPhi);
    } else if (cosdPhi > 1.0) {
      dPhi = 0.0;
    } else {
      dPhi = M_PI;
    }

    //...Finish...
    sz.setX(_helix->curv() * dPhi);
    sz.setY(p.z());
    sz.setZ(0.);

    return 0;
  }

  void
  TTrack::assign(unsigned hitMask)
  {
    hitMask |= CellHitUsed;

    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      TLink* l = _links[i];
      const Belle2::TRGCDCWireHit* h = l->hit();

#ifdef TRASAN_DEBUG
      if (h->track()) {
        std::cout << "TTrack::assign !!! hit(" << h->wire().name();
        std::cout << ") already assigned" << std::endl;
      }
#endif

      //...This function will be moved to TTrackManager...
      h->track((TTrack*) this);
      h->state(h->state() | hitMask);
    }
  }

  THelix
  TTrack::track2THelix(const rectrk_localz& t)
  {
    CLHEP::HepVector a(5);
    CLHEP::Hep3Vector p(t.m_pivot[0], t.m_pivot[1], t.m_pivot[2]);
    CLHEP::HepSymMatrix er(5, 0);
    a(1) = t.m_helix[0];
    a(2) = t.m_helix[1];
    a(3) = t.m_helix[2];
    a(4) = t.m_helix[3];
    a(5) = t.m_helix[4];
    er(1, 1) = t.m_error[0];
    er(2, 1) = t.m_error[1];
    er(2, 2) = t.m_error[2];
    er(3, 1) = t.m_error[3];
    er(3, 2) = t.m_error[4];
    er(3, 3) = t.m_error[5];
    er(4, 1) = t.m_error[6];
    er(4, 2) = t.m_error[7];
    er(4, 3) = t.m_error[8];
    er(4, 4) = t.m_error[9];
    er(5, 1) = t.m_error[10];
    er(5, 2) = t.m_error[11];
    er(5, 3) = t.m_error[12];
    er(5, 4) = t.m_error[13];
    er(5, 5) = t.m_error[14];
    return THelix(p, a, er);
  }

  THelix
  TTrack::track2THelix(const reccdc_trk& t)
  {
    CLHEP::HepVector a(5);
    CLHEP::Hep3Vector p(t.m_pivot[0], t.m_pivot[1], t.m_pivot[2]);
    CLHEP::HepSymMatrix er(5, 0);
    a(1) = t.m_helix[0];
    a(2) = t.m_helix[1];
    a(3) = t.m_helix[2];
    a(4) = t.m_helix[3];
    a(5) = t.m_helix[4];
    er(1, 1) = t.m_error[0];
    er(2, 1) = t.m_error[1];
    er(2, 2) = t.m_error[2];
    er(3, 1) = t.m_error[3];
    er(3, 2) = t.m_error[4];
    er(3, 3) = t.m_error[5];
    er(4, 1) = t.m_error[6];
    er(4, 2) = t.m_error[7];
    er(4, 3) = t.m_error[8];
    er(4, 4) = t.m_error[9];
    er(5, 1) = t.m_error[10];
    er(5, 2) = t.m_error[11];
    er(5, 3) = t.m_error[12];
    er(5, 4) = t.m_error[13];
    er(5, 5) = t.m_error[14];
    return THelix(p, a, er);
  }

  THelix
  TTrack::track2THelix(const mdst_trk_fit& t)
  {
    CLHEP::HepVector a(5);
    CLHEP::Hep3Vector p(t.m_pivot_x, t.m_pivot_y, t.m_pivot_z);
    CLHEP::HepSymMatrix er(5, 0);
    a(1) = t.m_helix[0];
    a(2) = t.m_helix[1];
    a(3) = t.m_helix[2];
    a(4) = t.m_helix[3];
    a(5) = t.m_helix[4];
    er(1, 1) = t.m_error[0];
    er(2, 1) = t.m_error[1];
    er(2, 2) = t.m_error[2];
    er(3, 1) = t.m_error[3];
    er(3, 2) = t.m_error[4];
    er(3, 3) = t.m_error[5];
    er(4, 1) = t.m_error[6];
    er(4, 2) = t.m_error[7];
    er(4, 3) = t.m_error[8];
    er(4, 4) = t.m_error[9];
    er(5, 1) = t.m_error[10];
    er(5, 2) = t.m_error[11];
    er(5, 3) = t.m_error[12];
    er(5, 4) = t.m_error[13];
    er(5, 5) = t.m_error[14];
    return THelix(p, a, er);
  }

  std::string
  TTrack::trackKinematics(const THelix& h)
  {
    static const HepGeom::Point3D<double> IP(0., 0., 0.);
    THelix hIp = h;
    hIp.pivot(IP);

    float chrg = hIp.a()[2] / fabs(hIp.a()[2]);
    std::string s;
    if (chrg > 0.) s = "+";
    else           s = "-";

    float x[4];
    x[0] = fabs(hIp.a()[0]);
    x[1] = hIp.a()[3];
    x[2] = 1. / fabs(hIp.a()[2]);
    x[3] = (1. / fabs(hIp.a()[2])) * hIp.a()[4];

    if ((x[0] < 2.) && (fabs(x[1]) < 4.)) s += "i ";
    else                                  s += "  ";

    for (unsigned i = 0; i < 4; i++) {
      if (i) s += " ";

      if (x[i] < 0.) s += "-";
      else           s += " ";

      int y = int(fabs(x[i]));
      s += itostring(y) + ".";
      float z = fabs(x[i]);
      for (unsigned j = 0; j < 3; j++) {
        z *= 10.;
        y = (int(z) % 10);
        s += itostring(y);
      }
    }

    return s;
  }

  std::string
  TTrack::trackStatus(const TTrack& t)
  {
    return trackStatus(t.finder(),
                       t.type(),
                       t.quality(),
                       t.fitting(),
                       0,
                       0);
  }

  std::string
  TTrack::trackStatus(const reccdc_trk& c)
  {
//     const reccdc_trk_add a = reccdc_trk_add::reccdc_trk_add();
//     return trackStatus(a);
    return("not implemented yet");
  }

  std::string
  TTrack::trackStatus(const reccdc_trk_add& a)
  {
    return trackStatus(a.m_decision,
                       a.m_kind,
                       a.m_quality,
                       a.m_stat,
                       a.m_mother,
                       a.m_daughter);
  }

  std::string
  TTrack::trackStatus(unsigned md,
                      unsigned mk,
                      unsigned mq,
                      unsigned ms,
                      unsigned mm,
                      unsigned ma)
  {

    std::string f;
    if (md & TrackOldConformalFinder) f += "o";
    if (md & TrackFastFinder) f += "f";
    if (md & TrackSlowFinder) f += "s";
    if (md & TrackCurlFinder) f += "c";
    if (md & TrackTrackManager) f += "t";
    if (f == "") f = "?";

    std::string k;
    if (mk & TrackTypeNormal) k += "Norm";
    if (mk & TrackTypeCurl) k += "Curl";
    if (mk & TrackTypeCircle) k += "Circ";
    if (mk & TrackTypeIncomingCosmic) k += "Inco";
    if (mk & TrackTypeOutgoingCosmic) k += "Outc";
    if (mk & TrackTypeKink) k += "Kink";
    if (mk & TrackTypeSVDOnly) k += "Svd";
    if (k == "") k = "?";

    std::string b;
    if (mq & TrackQualityOutsideCurler) b += "Curlback";
    if (mq & TrackQualityAfterKink) b += "Afterkink";
    if (mq & TrackQualityCosmic) b += "Cosmic";
    if (mq & TrackQuality2D) b += "2D";
    if (b == "") b = "ok";

    std::string s;
    if (ms & TrackFitGlobal) s += "HFit";
    if (ms & TrackFitCosmic) s += "CFit";
    if (ms & TrackFitCdcKalman) s += "CKal";
    if (ms & TrackFitSvdCdcKalman) s += "SKal";
    if (s == "") s = "?";

    int m = mm;
    if (m) --m;

    int d = ma;
    if (d) --d;

    std::string p = " ";
    return f + p + k + p + b + p + s + p + itostring(m) + p + itostring(d);
  }

  std::string
  TTrack::trackInformation(const TTrack& t)
  {
    const AList<TLink> cores = t.cores();
    unsigned n = cores.length();
    unsigned nS = TLink::nStereoHits(cores);
    unsigned nA = n - nS;
    std::string p;
    if (! positiveDefinite(t.helix())) p = " negative";
    //    if (THelixHasNan(t.helix())) p += " NaN";
    if (helixHasNan(t.helix())) p += " Han/Invalid";
    return trackInformation(nA, nS, n, t.chi2()) + p;
  }

  std::string
  TTrack::trackInformation(const reccdc_trk& r)
  {
    std::string p;
    if (positiveDefinite(track2THelix(r))) p = " posi";
    else                                  p = " nega";
    //    if (THelixHasNan(track2THelix(r))) p += " with NaN";
    if (helixHasNan(track2THelix(r))) p += " with NaN/invalid";
    return trackInformation(r.m_nhits - r.m_nster,
                            r.m_nster,
                            r.m_nhits,
                            r.m_chiSq) + p;
  }

  std::string
  TTrack::trackInformation(unsigned nA, unsigned nS, unsigned n, float chisq)
  {
    std::string s;

    s += "a" + itostring(int(nA));
    s += " s" + itostring(int(nS));
    s += " n" + itostring(int(n));
    // s += " ndf" + std::string(int(r.m_ndf));
    float x = chisq;

    if (x < 0.) s += " -";
    else        s += " ";

    int y = int(fabs(x));
    s += itostring(y) + ".";
    float z = fabs(x);
    for (unsigned j = 0; j < 1; j++) {
      z *= 10.;
      y = (int(z) % 10);
      s += itostring(y);
    }

    return s;
  }

  std::string
  TTrack::trackLayerUsage(const TTrack& t)
  {
    return TLink::layerUsage(t.links());
  }

  bool
  TTrack::positiveDefinite(const THelix& h)
  {
    const CLHEP::HepSymMatrix& e = h.Ea();
    CLHEP::HepSymMatrix e2 = e.sub(1, 2);
    CLHEP::HepSymMatrix e3 = e.sub(1, 3);
    CLHEP::HepSymMatrix e4 = e.sub(1, 4);

    bool positive = true;
    if (e[0][0] <= 0.) positive = false;
    else if (e2.determinant() <= 0.) positive = false;
    else if (e3.determinant() <= 0.) positive = false;
    else if (e4.determinant() <= 0.) positive = false;
    else if (e.determinant() <= 0.) positive = false;
    return positive;
  }

  bool
  TTrack::helixHasNan(const THelix& h)
  {
#if defined(BELLE_DEBUG)
    try {
#endif
      const CLHEP::HepVector& a = h.a();
      for (unsigned i = 0; i < 5; i++)
        if (isnan(a[i]))
          return true;
      const CLHEP::HepSymMatrix& Ea = h.Ea();
      for (unsigned i = 0; i < 5; i++)
        for (unsigned j = 0; j <= i; j++)
          if (isnan(Ea[i][j]))
            return true;
      return false;
#if defined(BELLE_DEBUG)
    } catch (std::string& e) {
      std::cout << "TTrack::THelixHasNan::helix is invalid" << std::endl;
      return true;
    }
#endif
  }

//cnv THelix
// TTrack::track2THelix(const gen_hepevt & t) {
//     float charge = 1;
//     if (t.m_idhep == 11) charge = -1;
//     else if (t.m_idhep == -11) charge = 1;
//     else if (t.m_idhep == 13) charge = -1;
//     else if (t.m_idhep == -13) charge = 1;
//     else if (t.m_idhep == 211) charge = 1;
//     else if (t.m_idhep == -211) charge = -1;
//     else if (t.m_idhep == 321) charge = 1;
//     else if (t.m_idhep == -321) charge = -1;
//     else if (t.m_idhep == 2212) charge = 1;
//     else if (t.m_idhep == -2212) charge = -1;
//     else {
//  std::cout << "track2THelix(gen_hepevt) !!! charge of id=";
//  std::cout << t.m_idhep << " is unknown" << std::endl;
//     }

//     CLHEP::Hep3Vector mom(t.m_P[0], t.m_P[1], t.m_P[2]);
//     CLHEP::Hep3Vector pos(t.m_V[0] / 10., t.m_V[1] / 10., t.m_V[2] / 10.);
//     return THelix(pos, mom, charge);
// }

  float
  TTrack::distance(const TTrack& t0, const TTrack& t1)
  {

    //...Check links...
    if (! t0.nCores()) return HUGE_VAL;
    if (! t1.nCores()) return HUGE_VAL;

    //...Get cross points to layers...
    int err = 0;
    err = TFinderBase::crossPointsByLayer(t0, _points0);
    err = TFinderBase::crossPointsByLayer(t1, _points1);

    //...Get starting points...
    const unsigned i0 = TLink::innerMost(t0.cores())->wire()->layerId();
    const unsigned i1 = TLink::innerMost(t1.cores())->wire()->layerId();
    const unsigned o0 = TLink::outerMost(t0.cores())->wire()->layerId();
    const unsigned o1 = TLink::outerMost(t1.cores())->wire()->layerId();

    //...Clear inner and outer points...
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    const unsigned n = cdc.nLayers() + 1;
    for (unsigned i = 0; i < n; i++) {
      if (i < i0)
        _points0[i] = Origin;
      if (i < i1)
        _points1[i] = Origin;
      if (i + 1 > o0)
        _points0[i + 1] = Origin;
      if (i + 1 > o1)
        _points1[i + 1] = Origin;
    }

    //...Sum up distance in each layer...
    float d = 0;
    unsigned nd = 0;
    for (unsigned i = 0; i < n; i++) {
      if (_points0[i] == Origin) continue;
      if (_points1[i] == Origin) continue;
      TPoint2D dd = _points1[i] - _points0[i];
      d += dd.mag();
      ++nd;

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "        i=" << i << ",d=" << dd.mag() << std::endl;
#endif
    }

    //...Average...
    if (nd)
      d /= float(nd);
    else
      d = HUGE_VAL;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        d(av)=" << d << ",n=" << nd << std::endl;

    if (d == 0) {
      t0.dump("detail", "t0  ");
      t1.dump("detail", "t1  ");
    }
#endif

    return d;
  }

  float
  TTrack::distanceB(const TTrack& t0, const TTrack& t1)
  {

    //...Check links...
    if (! t0.nCores()) return HUGE_VAL;
    if (! t1.nCores()) return HUGE_VAL;

    //...Get cross points to layers...
    int err = 0;
    err = TFinderBase::crossPointsByLayer(t0, _points0);
    err = TFinderBase::crossPointsByLayer(t1, _points1);

    //...Get starting points...
    if (! TLink::innerMost(t0.cores())) return HUGE_VAL;
    if (! TLink::innerMost(t1.cores())) return HUGE_VAL;
    if (! TLink::outerMost(t0.cores())) return HUGE_VAL;
    if (! TLink::outerMost(t1.cores())) return HUGE_VAL;
    const unsigned i0 = TLink::innerMost(t0.cores())->wire()->layerId();
    const unsigned i1 = TLink::innerMost(t1.cores())->wire()->layerId();
    const unsigned o0 = TLink::outerMost(t0.cores())->wire()->layerId();
    const unsigned o1 = TLink::outerMost(t1.cores())->wire()->layerId();

    //...Clear inner and outer points...
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    const unsigned n = cdc.nLayers() + 1;
    for (unsigned i = 0; i < n; i++) {
      if (i < i0)
        _points0[i] = Origin;
      if (i < i1)
        _points1[i] = Origin;
      if (i + 1 > o0)
        _points0[i + 1] = Origin;
      if (i + 1 > o1)
        _points1[i + 1] = Origin;
    }

    //...Set minr and maxr...
    float dA = 0;
    unsigned nd = 0;
    for (unsigned i = 0; i < n; i++) {
      if (_points0[i] == Origin) continue;
      if (_points1[i] == Origin) continue;
      TPoint2D dd = _points1[i] - _points0[i];
      dA += dd.mag();
      ++nd;
    }
    if (nd)
      dA /= float(nd);
    else
      dA = HUGE_VAL;

    if (dA > 3.) return dA;

    //...Sum up distance in each layer...
    TPoint2D c0 = t0.center();
    TPoint2D c1 = t1.center();
    double radius0 = std::abs(t0.radius());
    double radius1 = std::abs(t1.radius());
    float     d_to_1(0.), d_to_0(0.);
    unsigned nd_to_1(0), nd_to_0(0);

    for (unsigned i = 0; i < n; i++) {
      if (_points0[i] == Origin) continue;
      if (_points1[i] == Origin) continue;

      float dd_to_1 = std::abs((_points0[i] - c1).mag() - radius1);
      d_to_1 += dd_to_1;
      ++nd_to_1;

      float dd_to_0 = std::abs((_points1[i] - c0).mag() - radius0);
      d_to_0 += dd_to_0;
      ++nd_to_0;

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "        i=" << i << ",dd_to_1,dd_to_0=" << dd_to_1 << " " << dd_to_0 << std::endl;
#endif
    }

    //...Average...
    float dB = d_to_1 + d_to_0;
    if (nd_to_1 + nd_to_0)
      dB /= float(nd_to_1 + nd_to_0);
    else
      dB = HUGE_VAL;

    if (dA < dB) dB = dA;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        dB(av),n1,n0=" << dB << " " << nd_to_1 << " " << nd_to_0 << std::endl;
#endif

    return dB;
  }

// from by jtanaka
  double
  TTrack::chisq2confLevel(int n, double chi2)
  {
#define SRTOPI 0.7978846
#define UPL 340.0
#define ROOT2I 0.70710678

    double prob = 0.0;
    double sum, term;
    int m;
    int i, k;
    double temp_i, temp_n;
    double srty;

    if ((n <= 0) || (chi2 < 0.0)) {
      return prob;
    }
    if (n > 60) {
      temp_n = (double)n;
      srty = sqrt(chi2) - sqrt(temp_n - 0.5);
      if (srty < 12.0) {
        prob = 0.5 * erfc(srty);
        return prob;
      }
      return prob;
    }
    if (chi2 > UPL) {
      return prob;
    }
    sum = exp(-0.5 * chi2);
    term = sum;
    m = (int)floor(n / 2);

    if (2 * m == n) {
      if (m == 1) {
        prob = sum;
        return prob;
      } else {
        for (i = 2; i < m + 1; i++) {
          temp_i = (double)i;
          term = 0.5 * chi2 * term / (temp_i - 1.0);
          sum = sum + term;

        }
        prob = sum;
        return prob;
      }
    } else {
      srty = sqrt(chi2);
      prob = erfc(ROOT2I * srty);
      if (n == 1) {
        return prob;
      }
      if (n == 3) {
        prob = SRTOPI * srty * sum + prob;
        return prob;
      } else {
        k = m - 1;
        for (i = 1; i < k + 1; i++) {
          temp_i = (double)i;
          term = term * chi2 / (2.0 * temp_i + 1.0);
          sum = sum + term;
        }
        prob = SRTOPI * srty * sum + prob;
        return prob;
      }
    }
  }

  int
  TTrack::intersection(const HepGeom::Point3D<double> & c1,
                       double r1,
                       const HepGeom::Point3D<double> & c2,
                       double r2,
                       double eps,
                       HepGeom::Point3D<double> & x1,
                       HepGeom::Point3D<double> & x2)
  {

    double c0x = c2.x() - c1.x() ;
    double c0y = c2.y() - c1.y() ;
    double c0  = sqrt(c0x * c0x + c0y * c0y) ;
    double rr1 = abs(r1) ;
    double rr2 = abs(r2) ;
    double Radd = rr1 + rr2 ;
    double Rsub = abs(rr1 - rr2) ;

    // no intersections

    if (c0 > Radd + eps || c0 < 0.001 || c0 < Rsub - eps) {
      //-- debug
      // std::cout << "Int2Cir return 0 " << std::endl;
      //-- debug end
      return 0 ;
    }

    // single intersection

    else {
      if (c0 > Radd - eps) {
        x1.setX(c1.x() + rr1 * c0x / c0);
        x1.setY(c1.y() + rr1 * c0y / c0);
        x2.setX(0.0);
        x2.setY(0.0);
        //--debug
        // std::cout << "Int2Cir return 1" << std::endl;
        //--debug end
        return 1 ;
      }
    }

    // two intersections

    double chg = abs(r1) / r1 ;
    double cosPsi = (c0 * c0 + rr1 * rr1 - rr2 * rr2) / (2.*c0 * rr1) ;
    double sinPsi = - (chg / abs(chg)) * sqrt(1.0 - cosPsi * cosPsi) ;
    x1.setX(c1.x() + (rr1 / c0) * (cosPsi * c0x - sinPsi * c0y));
    x1.setY(c1.y() + (rr1 / c0) * (cosPsi * c0y + sinPsi * c0x));
    x2.setX(c1.x() + (rr1 / c0) * (cosPsi * c0x + sinPsi * c0y));
    x2.setY(c1.y() + (rr1 / c0) * (cosPsi * c0y - sinPsi * c0x));
    //-- debug
    // std::cout << "Int2Cir return 2" << std::endl;
    //-- debug end
    return 2 ;

  }

} // namespace Belle
