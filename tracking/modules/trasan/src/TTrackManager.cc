//-----------------------------------------------------------------------------
// $Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $
//-----------------------------------------------------------------------------
// Filename : TTrackManager.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A manager of TTrack information to make outputs as Reccdc_trk.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.111  2005/11/03 23:20:13  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.110  2005/03/11 03:57:52  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.109  2005/01/14 00:55:40  katayama
// uninitialized variable
//
// Revision 1.108  2004/04/26 06:29:11  yiwasaki
// Trasan 3.13 : TTrackManager MC analysis bug fixed
//
// Revision 1.107  2004/04/15 05:34:10  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.106  2004/04/08 22:02:47  yiwasaki
// MC quality check added
//
// Revision 1.105  2004/03/26 06:07:05  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.104  2003/12/25 12:03:36  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.103  2002/04/15 07:39:04  yiwasaki
// Update from H.Ozaki
//
// Revision 1.102  2002/03/23 07:36:43  yiwasaki
// T0 fit with pt sorted tracks : H.O
//
// Revision 1.101  2002/02/22 06:37:44  katayama
// Use __sparc
//
// Revision 1.100  2002/02/21 23:49:16  katayama
// For -ansi and other warning flags
//
// Revision 1.99  2002/01/03 11:04:56  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.98  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.97  2001/12/19 02:59:48  katayama
// Uss find,istring
//
// Revision 1.96  2001/12/05 12:35:18  katayama
// For gcc-3
//
// Revision 1.95  2001/04/11 01:09:12  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.92  2001/02/07 22:25:03  yiwasaki
// Trasan 2.28 : conf minor change in shared memory
//
// Revision 1.91  2001/02/07 21:18:27  yiwasaki
// Trasan 2.27 : pivot of 2D track is moved to (x,y,0)
//
// Revision 1.90  2001/02/01 06:15:48  yiwasaki
// Trasan 2.25 : conf bug fix for chisq=0
//
// Revision 1.89  2001/01/31 11:38:27  yiwasaki
// Trasan 2.24 : curl finder fix
//
// Revision 1.88  2001/01/30 20:51:50  yiwasaki
// Trasan 2.22 : bug fixes
//
// Revision 1.87  2001/01/30 04:54:15  yiwasaki
// Trasan 2.21 release : bug fixes
//
// Revision 1.86  2001/01/29 09:27:46  yiwasaki
// Trasan 2.20 release : 2D & cosmic tracks are output, curler decision is made by dr and dz
//
// Revision 1.85  2001/01/11 04:40:23  yiwasaki
// minor changes
//
// Revision 1.84  2000/10/13 00:28:08  yiwasaki
// Trasan 2.15 : trkmgr bug fix(YI)
//
// Revision 1.83  2000/10/12 21:48:50  yiwasaki
// Trasan 2.14 : trkmgr bug fix(YI) and updates(JT), new profiler for cdc hits
//
// Revision 1.82  2000/10/11 11:30:45  yiwasaki
// trasan 2.12 : #hits protection, chi2 sorting modified
//
// Revision 1.81  2000/10/07 01:44:51  yiwasaki
// Trasan 2.10 : updates of trkmgr from JT
//
// Revision 1.80  2000/10/05 23:54:27  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.79  2000/10/02 22:34:02  yiwasaki
// Trasan 2.08 : trkmgr update from JT
//
// Revision 1.78  2000/10/02 08:09:29  yiwasaki
// Trasan 2.07 : updates of pmf and trkmgr by JT
//
// Revision 1.77  2000/09/29 22:11:22  yiwasaki
// Trasan 2.06 : trkmgr updates from JT
//
// Revision 1.76  2000/09/27 07:45:19  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.75  2000/06/27 08:00:35  yiwasaki
// Trasan 2.03 : TTrackManager sorting bug fixed
//
// Revision 1.74  2000/06/13 05:19:03  yiwasaki
// Trasan 2.02 : RECCDC_MCTRK sorting
//
// Revision 1.73  2000/06/05 23:42:49  yiwasaki
// Trasan 2.01 : condition changed for 6dim t0 fitting, by H.Ozaki
//
// Revision 1.72  2000/05/03 11:13:58  yiwasaki
// Trasan 2.00 : version up without any change in trasan, trkmgr rectrk_tof fixed
//
// Revision 1.71  2000/04/29 02:16:22  yiwasaki
// oichan update, trasan unchaged
//
// Revision 1.70  2000/04/25 04:29:32  yiwasaki
// Trasan 2.00rc32 : fix again
//
// Revision 1.69  2000/04/25 02:53:26  yiwasaki
// Trasan 2.00rc31 : Definition of nhits, nster, and ndf in RECCDC_WIRHIT
//
// Revision 1.68  2000/04/15 13:40:42  katayama
// Add/remove const so that they compile
//
// Revision 1.67  2000/04/14 05:20:46  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.66  2000/04/13 02:53:42  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.65  2000/04/11 13:05:49  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.64  2000/04/07 14:24:39  yiwasaki
// blank output fixed
//
// Revision 1.63  2000/04/07 07:05:45  yiwasaki
// previous one is not the latest
//
// Revision 1.62  2000/04/07 05:45:18  yiwasaki
// Trasan 2.00rc28 : curl finder bad point rejection, table clear, chisq info
//
// Revision 1.61  2000/04/04 12:14:17  yiwasaki
// Trasan 2.00RC27 : bad point rejection in conf., association check for dE/dx
//
// Revision 1.60  2000/04/04 07:52:38  yiwasaki
// Trasan 2.00RC26 : additions
//
// Revision 1.59  2000/04/04 07:40:08  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.58  2000/04/03 04:26:45  yiwasaki
// trkmgr fix again
//
// Revision 1.57  2000/03/31 08:33:24  yiwasaki
// Trasan 2.00RC24 : fix again
//
// Revision 1.56  2000/03/31 07:21:14  yiwasaki
// PM finder updates from J.Tanaka, trkmgr bug fixes
//
// Revision 1.55  2000/03/26 13:59:44  yiwasaki
// Trasan 2.00RC21 : track manager bug fix
//
// Revision 1.54  2000/03/24 10:22:57  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.53  2000/03/24 02:55:42  yiwasaki
// track manager
//
// Revision 1.52  2000/03/23 13:27:55  yiwasaki
// Trasan 2.00RC18 : bug fixes
//
// Revision 1.51  2000/03/21 11:55:17  yiwasaki
// Trasan 2.00RC17 : curler treatments
//
// Revision 1.50  2000/03/21 07:01:29  yiwasaki
// tmp updates
//
// Revision 1.49  2000/03/17 11:01:39  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.48  2000/02/28 11:44:02  yiwasaki
// Trasan 2.00RC12 : t0 fit fix, salvage fix
//
// Revision 1.47  2000/02/15 13:46:45  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.46  2000/01/12 14:25:01  yiwasaki
// Trasan 1.67d : tuned trasan, bug in masking fixed, sakura modified, mitsuo added
//
// Revision 1.45  1999/11/19 09:13:11  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.44  1999/10/30 10:12:25  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.43  1999/10/20 12:04:15  yiwasaki
// Trasan 1.64b : new T0 determination methode by H.Ozaki
//
// Revision 1.42  1999/08/25 06:25:50  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.41  1999/07/27 08:01:38  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.40  1999/07/17 06:42:59  yiwasaki
// THelixFitter has tof correction, Trasan time has option for tof correction
//
// Revision 1.39  1999/07/15 08:43:20  yiwasaki
// Trasan 1.55b release : Curl finder # of hits protection, bug in TManager for MC, helix fitter # of hits protection, fast finder improved
//
// Revision 1.38  1999/07/12 13:41:15  yiwasaki
// Trasan 1.54a release : cpu improvement in TTrack::approach, TTrackManager::saveTables checks # of hits
//
// Revision 1.37  1999/07/09 01:47:22  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.36  1999/07/06 10:41:10  yiwasaki
// Trasan 1.52a release : TRGCDC bug fix, associated hits output
//
// Revision 1.35  1999/07/01 08:15:24  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.34  1999/06/26 07:05:43  yiwasaki
// Trasan 1.47a release : hit and tracking efficiency improved
//
// Revision 1.33  1999/06/24 11:27:43  itoh
// Fixes for DEC Alpha
//
// Revision 1.32  1999/06/17 09:45:01  yiwasaki
// Trasan 1.45 release : T0 determination by 2D fitting
//
// Revision 1.31  1999/06/15 06:33:41  yiwasaki
// Trasan 1.43 release : minor changes in TRGCDCClust and TBuilder
//
// Revision 1.30  1999/06/09 15:09:53  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.29  1999/06/08 01:04:51  yiwasaki
// Trasan 1.37 release : TrkMgr::mask bug fix, reported by DST prod. group
//
// Revision 1.28  1999/06/06 09:33:01  yiwasaki
// Trasan 1.36 release : Trasan T0 output
//
// Revision 1.27  1999/05/28 18:17:56  yiwasaki
// Trasan 1.35 release : cathode and sakura updates
//
// Revision 1.26  1999/05/28 07:11:11  yiwasaki
// Trasan 1.35 alpha release : cathdoe test version
//
// Revision 1.25  1999/05/26 05:03:49  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.24  1999/05/18 04:44:30  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.23  1999/03/21 15:45:43  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.22  1999/03/12 13:12:01  yiwasaki
// Trasan 1.26 : bug fix in RECCDC_TRK output
//
// Revision 1.21  1999/03/12 06:56:45  yiwasaki
// Trasan 1.25 release : curl finder updated, salvage 2 is default, minor bug fixes
//
// Revision 1.20  1999/03/11 23:27:21  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.19  1999/03/09 13:43:02  yiwasaki
// Trasan 1.22 release : TrackManager bug fix
//
// Revision 1.18  1999/03/09 06:29:25  yiwasaki
// Trasan 1.21 release : conformal finder bug fix
//
// Revision 1.17  1999/02/04 02:12:24  yiwasaki
// Trasan 1.15 release : bug fix in cluster table output from S.Suzuki
//
// Revision 1.16  1999/02/03 06:23:13  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.15  1999/01/27 11:52:35  yiwasaki
// very minor changes
//
// Revision 1.14  1999/01/25 03:16:13  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.13  1999/01/20 01:02:49  yiwasaki
// Trasan 1.12 release : movePivot problem avoided temporary, new sakura
//
// Revision 1.12  1999/01/16 10:32:42  katayama
// clean up includes
//
// Revision 1.11  1999/01/11 03:09:11  yiwasaki
// Trasan 1.11 release
//
// Revision 1.10  1999/01/11 03:03:18  yiwasaki
// Fitters added
//
// Revision 1.9  1998/12/23 02:07:53  katayama
// trk->confl() removed. Use probutil/Chisq::Prob
//
// Revision 1.8  1998/12/14 16:59:10  yiwasaki
// bad hits removed from outputs
//
// Revision 1.7  1998/11/27 08:15:38  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.6  1998/11/12 12:27:32  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.5  1998/11/10 09:09:13  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.4  1998/09/29 01:24:27  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.3  1998/09/25 02:14:40  yiwasaki
// modification for cosmic
//
// Revision 1.2  1998/09/24 22:56:42  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.1  1998/09/17 16:05:26  yiwasaki
// Trasan 1.1 alpha 1 release : TTrackManager added to manage reconstructed tracks, TTrack::P() added, TTrack::_charge no longer constant
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

#define TTRACKMANAGER_INLINE_DEFINE_HERE
#include <values.h>
#include <cstring>
#include <cstdlib>
#include <cfloat>
#include <framework/gearbox/Const.h>
#include "tracking/modules/trasan/Strings.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/Trasan.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TTrackMC.h"
#include "tracking/modules/trasan/TTrackManager.h"
//cnv #include "tracking/modules/trasan/TProfiler.h"
//cnv #include "tracking/modules/trasan/TSvdAssociator.h"
#include "GFTrackCand.h"
#include "TVector3.h"

#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifdef TRASAN_WINDOW_GTK
#include "tracking/modules/trasan/TWindowGTKConformal.h"
#include "tracking/modules/trasan/TWindowGTKHough.h"
#include "tracking/modules/trasan/TWindowGTKSZ.h"
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

using namespace std;

namespace Belle {

  static float tosingle(double d)
  {
    if (d > FLT_MAX) return FLT_MAX;
    else if (d < -(FLT_MAX)) return -(FLT_MAX);
    else return (float)d;
  }

  TTrackManager::TTrackManager()
    : _debugLevel(0),
      _maxMomentum(10.),
      _minPt(0.),
      _maxTanl(0.),
      _sigmaCurlerMergeTest(sqrt(100.)),
      _nCurlerMergeTest(4),
      _fitter("TTrackManager Fitter"),
      _cFitter("TTrackManager 2D Fitter"),
//cnv _s(0) {
      _s(new summary())
  {
    memset(_s, 0, sizeof(struct summary));
  }

  TTrackManager::~TTrackManager()
  {
  }

  string
  TTrackManager::version(void) const
  {
    return string("5.02");
  }

  void
  TTrackManager::dump(const string& msg, const string& pref) const
  {
    bool def = (msg == "") ? true : false;
    string pre = pref;
    string tab;
    for (unsigned i = 0; i < pre.size(); i++)
      tab += " ";

    if (msg.find("summary") != string::npos ||
        msg.find("detail") != string::npos) {
      struct summary s;
      //        bzero((char*)& s, sizeof(struct summary));
      memset((char*)& s, 0, sizeof(struct summary));
      for (int i = 0; i < 0; i++) {
//cnv     int size;
        struct summary r;
        s._nEvents += r._nEvents;
        s._nToBeMerged += r._nToBeMerged;
        s._nToBeMergedMoreThanTwo += r._nToBeMergedMoreThanTwo;
        for (unsigned j = 0; j < 8; j++) {
          s._nTracks[j] += r._nTracks[j];
          s._nTracksAll[j] += r._nTracksAll[j];
          s._nTracks2D[j] += r._nTracks2D[j];
          s._nTracksFinal[j] += r._nTracksFinal[j];
          s._nSuperMoms[j] += r._nSuperMoms[j];
          s._nPtCut[j] += r._nPtCut[j];
          s._nTanlCut[j] += r._nTanlCut[j];
          for (unsigned k = 0; k < 5; k++)
            s._nMCQuality[j][k] += r._nMCQuality[j][k];
        }
      }

      cout << pre << "all events : " << s._nEvents << endl;
      pre = tab;

      //...Finder loop...
      string tab = pre + "    ";
      for (unsigned i = 0; i < 8; i++) {
        if (! s._nTracksAll[i]) continue;

        cout << pre << TTrack::finderName((1 << i)) << endl;
        cout << tab << "all tracks   : " << s._nTracksAll[i]
             << endl;
        cout << tab << "good tracks  : " << s._nTracks[i]
             << endl;
        cout << tab << "2D tracks    : " << s._nTracks2D[i]
             << endl;
        cout << tab << "final tracks : " << s._nTracksFinal[i]
             << endl;
        cout << tab << "super mom.   : " << s._nSuperMoms[i]
             << endl;
        cout << tab << "Pt cut       : " << s._nPtCut[i]
             << endl;
        cout << tab << "tanl cut     : " << s._nTanlCut[i]
             << endl;
      }
      cout << pre << "Curler Information" << endl;
      cout << tab << "to be mreged : " << s._nToBeMerged
           << endl;
      cout << tab << "to be mreged2: "
           << s._nToBeMergedMoreThanTwo << endl;

      for (unsigned i = 0; i < 8; i++) {
        if (! s._nTracksAll[i]) continue;
        if (msg.find("MC") != string::npos) {
          cout << pre << TTrack::finderName((1 << i))
               << " MC information" << endl;
          cout << tab << "Good     : " << s._nMCQuality[i][0]
               << endl;
          cout << tab << "Ghost    : " << s._nMCQuality[i][1]
               << endl;
          cout << tab << "Bad      : " << s._nMCQuality[i][2]
               << endl;
          cout << tab << "Charge   : " << s._nMCQuality[i][3]
               << endl;
          cout << tab << "Garbage  : " << s._nMCQuality[i][4]
               << endl;
        }
      }
    }
    if (def || msg.find("eventSummary") != string::npos ||
        msg.find("detail") != string::npos) {
      cout << pre << "tracks reconstructed : " << _tracksAll.length()
           << endl;
      cout << tab << "good tracks          : " << _tracks.length()
           << endl;
      cout << tab << "2D tracks            : " << _tracks2D.length()
           << endl;
      cout << tab << "Track list:[name,finder,type,status,?,0,0,charge*ip,dr,dz,pt,pz,links..,chisq,errmtx]" << endl;

      for (unsigned i = 0; i < (unsigned) _tracksAll.length(); i++) {
        cout << tab << tab << TTrack::trackDump(* _tracksAll[i])
             << endl;
        if (msg.find("helix") != string::npos)
          _tracksAll[i]->dump("helix detail", tab + "    ");
        if (msg.find("hits") != string::npos ||
            msg.find("detail") != string::npos)
          TLink::dump(_tracksAll[i]->links(), "hits sort flag");
      }
      pre = tab;
    }
  }

  void
  TTrackManager::maskCurlHits(const CAList<Belle2::TRGCDCWireHit> &axial,
                              const CAList<Belle2::TRGCDCWireHit> &stereo,
                              const AList<TTrack> &tracks) const
  {
//...Coded by jtanaka...

    int i = 0;
    while (const TTrack* t = tracks[i++]) {
      int j = 0;
      while (const Belle2::TRGCDCWireHit* a = axial[j++]) {
        double x = t->helix().center().x() - a->xyPosition().x();
        double y = t->helix().center().y() - a->xyPosition().y();
        double r = sqrt(x * x + y * y);
        double R = fabs(t->helix().radius());
        double q = t->helix().center().x() * a->xyPosition().y() -
                   t->helix().center().y() * a->xyPosition().x();
        double qq = q * t->charge();
        if (R - 2. < r && r < R + 2. && qq > 0.) {
          a->state(a->state() | CellHitUsed);
        }
      }
      j = 0;
      while (const Belle2::TRGCDCWireHit* s = stereo[j++]) {
        double x = t->helix().center().x() - s->xyPosition().x();
        double y = t->helix().center().y() - s->xyPosition().y();
        double r = sqrt(x * x + y * y);
        double R = fabs(t->helix().radius());
        double q = t->helix().center().x() * s->xyPosition().y() -
                   t->helix().center().y() * s->xyPosition().x();
        double qq = q * t->charge();
        if (R - 2.5 < r && r < R + 2.5 && qq > 0.) {
          s->state(s->state() | CellHitUsed);
        }
      }
    }
  }

  void
  TTrackManager::salvage(const CAList<Belle2::TRGCDCWireHit> & hits) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    cout << name() << " ... salvaging" << endl;
    cout << "    # of given hits=" << hits.length() << endl;
#endif

    //...Check arguments...
    unsigned nTracks = _tracks.length();
    if (nTracks == 0) return;
    unsigned nHits = hits.length();
    if (nHits == 0) return;

    //...Hit loop...
    for (unsigned i = 0; i < nHits; i++) {
      const Belle2::TRGCDCWireHit& h = * hits[i];

      //...Already used?...
      if (h.state() & CellHitUsed) continue;
#ifdef TRASAN_DEBUG_DETAIL
      cout << "    checking " << h.wire().name() << endl;
#endif

      //...Select the closest track to a hit...
      TTrack* best = closest(_tracks, h);
#ifdef TRASAN_DEBUG_DETAIL
      if (! best) {
        cout << "        no track candidate returned";
        cout << "by TTrackManager::closest" << endl;
      }
#endif
      if (! best) continue;

      //...Try to append this hit...
      AList<TLink> link;
      link.append(new TLink(0, & h));
      best->appendByApproach(link, 30.);
      // best->assign(CellHitConformalFinder);
      best->finder(TrackTrackManager);
    }
  }

  TTrack*
  TTrackManager::closest(const AList<TTrack> & tracks,
                         const Belle2::TRGCDCWireHit& hit) const
  {

    TLink t;
    t.hit(& hit);
    unsigned n = tracks.length();
    double minDistance = MAXDOUBLE;
    TTrack* minTrk = NULL;

    //...Loop over all tracks...
    for (unsigned i = 0; i < n; i++) {
      TTrack& trk = * tracks[i];
      int err = trk.approach(t);
      if (err < 0) continue;
      if (minDistance > t.distance()) {
        minDistance = t.distance();
        minTrk = & trk;
      }
    }

    return minTrk;
  }

  void
  TTrackManager::saveTables(void)
  {
    if (_debugLevel) monitor();

#ifdef TRASAN_DEBUG_DETAIL
    cout << "TTrackManager::saveTables ... # 3D tracks=" << _tracks.length()
         << ", # 2D tracks=" << _tracks2D.length()
         << ", all tracks=" << _tracksAll.length() << endl;
#endif

    //...For 3D tracks...
    AList<TTrack> badTracks;
    unsigned n = _tracks.length();
    unsigned* id;
    if (NULL == (id = (unsigned*) malloc(n * sizeof(unsigned)))) {
      perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:id:malloc");
      exit(1);
    }
    //    bzero((char *) id, n * sizeof(unsigned));
    memset((char*) id, 0, n * sizeof(unsigned));
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];

      //...Track quality check...
      if (! t.nLinks()) {
        badTracks.append((TTrack&) t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << "    Bad track found(no link):" << t.name()
             << endl;
#endif
        continue;
      }
      if (! goodTrack(t, false))
        continue;

      //...Copy track parameters...
      reccdc_trk* r = 0;
      reccdc_trk_add* a = 0;
      int err = copyTrack(t, & r, & a);
      if (err) {
        badTracks.append(t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << "    Bad track found(copyTrack err=" << err << "):"
             << t.name() << endl;
#endif
        continue;
      }
      _tracksFinal.append(t);

      //...Type and quality...
      id[i] = r->m_ID;
      r->m_stat = t.state();
      a->m_kind = t.type();
      a->m_decision = t.finder();
      a->m_stat = t.fitting();
//    if (a->m_kind == TrackTypeCosmic) {
//        a->m_quality = TrackQualityCosmic;
//    }
//    if (t.daughter() && (_tracks.index(t.daughter()) >= 0))
//        a->m_daughter = _tracks.index(t.daughter()) + 1;

//cnv     reccdc_svd_trk cs;

    }

    //...Daughter treatment...
    for (unsigned i = 0; i < n; i++) {

#ifdef TRASAN_DEBUG_DETAIL
      cout << "id[" << i << "]=" << id[i] << endl;
#endif
      if (!(id[i])) continue;
      if (!(_tracks[i]->daughter())) continue;

      int dId = _tracks.index(_tracks[i]->daughter());

#ifdef TRASAN_DEBUG_DETAIL
      cout << "    dId=" << dId;
      if (dId >= 0) cout << ", id[dId]=" << id[dId];
      cout << endl;
#endif

      if (dId >= 0) {
        if (id[dId]) {
          reccdc_trk_add* a;
          a->m_daughter = id[dId];
        }
      }
    }
    free(id);

    //...Remove bad tracks...
    _tracks.remove(badTracks);
    badTracks.removeAll();

    //...For 2D tracks...
    n = _tracks2D.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks2D[i];

      //...Copy track parameters...
      reccdc_trk* r = 0;
      reccdc_trk_add* a = 0;
      int err = copyTrack(t, & r, & a);
      if (err) {
#ifdef TRASAN_DEBUG_DETAIL
        cout << "TTrackManager::saveTables !!! bad 2D tracks found"
             << " : err=" << err << endl
             << TTrack::trackDump(t) << endl;
#endif
        badTracks.append(t);
        continue;
      }
      _tracksFinal.append(t);

      //...Reset helix parameter...
//    r->m_helix[3] = 0.;
//    r->m_helix[4] = 0.;
//    r->m_nhits -= r->m_nster;
//    r->m_nster = 0;

      //...Table filling...
      r->m_stat = t.state();
      a->m_kind = t.type();
      a->m_decision = t.finder();
      // a->m_quality = t.quality();
      a->m_quality = TrackQuality2D;
      a->m_stat = t.fitting();

#ifdef TRASAN_DEBUG
      if ((r->m_ndf == 0) && (r->m_chiSq > 0.)) {
        cout << "TTrackManager::saveTables !!! chisq>0 with ndf=0."
             << endl
             << "    Here is a track dump"
             << "    " << TTrack::trackDump(t) << endl;
        t.dump("detail");
      }
      if ((r->m_ndf > 0) && (r->m_chiSq == 0.)) {
        cout << "TTrackManager::saveTables !!! chisq=0 with ndf>0."
             << endl
             << "    Here is a track dump"
             << "    " << TTrack::trackDump(t) << endl;
        t.dump("detail");
      }

      if (r->m_ndf == 0)
        cout << "TTrackManager::saveTables ... ndf = 0" << endl
             << "    " << TTrack::trackDump(t) << endl;
      if (r->m_chiSq == 0.)
        cout << "TTrackManager::saveTables ... chisq = 0" << endl
             << "    " << TTrack::trackDump(t) << endl;
#endif
    }
    _tracks2D.remove(badTracks);
  }

  void
  TTrackManager::saveMCTables(void) const
  {
    unsigned n = _tracksFinal.length();
    for (unsigned i = 0; i < n; i++) {
      const TTrack& t = * _tracksFinal[i];

      struct reccdc_trk* r;
      r = 0;

      //...Set type...

      //...Hit loop...
      const AList<TLink> & hits = t.finalHits();
      unsigned nHits = hits.length();
      for (unsigned j = 0; j < nHits;  j++) {
//cnv     TLink * l = hits[j];
//      reccdc_wirhit * h = l->hit()->reccdc();
        reccdc_wirhit* h = 0;
//      datcdc_mcwirhit * m = l->hit()->mc()->datcdc();
        datcdc_mcwirhit* m = 0;
        m->m_trk = r->m_ID;
        struct reccdc_mctrk2hep* c;
        c = 0;
        c->m_wir = h->m_ID;
        c->m_trk = r->m_ID;
        c->m_hep = 0;
      }

      const TTrackMC* const mc = t.mc();
      struct reccdc_mctrk* m;
      m = 0;
      m->m_wirFrac = mc->wireFraction();
      m->m_wirFracHep = mc->wireFractionHEP();
      m->m_charge = mc->charge();
      m->m_ptFrac = mc->ptFraction();
      m->m_pzFrac = mc->pzFraction();
      m->m_quality = mc->quality();
      if (mc->hep()) m->m_hep = 0;
      else           m->m_hep = 0;
    }
  }

  void
  TTrackManager::saveBelle2(Belle2::StoreArray<GFTrackCand> & cands)
  {
    if (_debugLevel) monitor();

#ifdef TRASAN_DEBUG_DETAIL
    cout << "TTrackManager::saveBelle2 ... # 3D tracks="
         << _tracks.length() << ", # 2D tracks=" << _tracks2D.length()
         << ", all tracks=" << _tracksAll.length() << endl;
#endif

    //...For 3D tracks...
    AList<TTrack> badTracks;
    unsigned n = _tracks.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];

      //...Track quality check...
      if (! t.nLinks()) {
        badTracks.append((TTrack&) t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << "    Bad track found(no link):" << t.name()
             << endl;
#endif
        continue;
      }
      if (! goodTrack(t, false))
        continue;

      //...Copy track parameters...
      int err = copyTrack(cands, t);
      if (err) {
        badTracks.append(t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << "    Bad track found(copyTrack err=" << err << "):"
             << t.name() << endl;
#endif
        continue;
      }
      _tracksFinal.append(t);

      //...Type and quality...
    }

    //...Daughter treatment...

    //...Remove bad tracks...

    //...For 2D tracks...
  }

  void
  TTrackManager::movePivot(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::movePivot";
    EnterStage(stage);
#endif

    unsigned n = _tracksAll.length();
//  unsigned n = _tracks.length();
    for (unsigned i = 0; i < n; i++) {
      if (_tracksAll[i]->nLinks()) _tracksAll[i]->movePivot();
//  if (_tracks[i]->nLinks()) _tracks[i]->movePivot();
    }
    nameTracks();

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TTrackManager::clear(bool termination)
  {
    HepAListDeleteAll(_tracksAll);
    _tracks.removeAll();
    _tracks2D.removeAll();
    _tracksFinal.removeAll();
    HepAListDeleteAll(_associateHits);
    static bool first = true;
    if (first && !termination) {
      first = false;
//cnv int size;
//cnv _s = 0;
    }
  }

  void
  TTrackManager::finish(void)
  {
    refit();
    movePivot();
    if (_debugLevel > 1) {
      cout << name() << " ... finishing" << endl;
//  unsigned n = _tracksAll.length();
      unsigned n = _tracks.length();
      for (unsigned i = 0; i < n; i++) {
//      TTrack & t = * _tracksAll[i];
        TTrack& t = * _tracks[i];
        cout << "    " << t.name() << endl;
        t.dump("hits mc track flag sort", "    ");
      }
    }
  }

  void
  TTrackManager::append(AList<TTrack> & list)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::append";
    EnterStage(stage);
#endif

    _tracksAll.append(list);
    _tracks.append(selectGoodTracks(list));
    list.removeAll();

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TTrackManager::append2D(AList<TTrack> & list)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::append2D";
    EnterStage(stage);
#endif
    _tracksAll.append(list);
    _tracks2D.append(selectGoodTracks(list, true));
    _tracks2D.sort(TTrack::sortByPt);
    list.removeAll();
#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TTrackManager::refit(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::refit";
    EnterStage(stage);
#endif

    unsigned n = _tracks.length();
    AList<TTrack> bads;
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];
      int err;
      err = _fitter.fit(t);
      if (err < 0) {
        bads.append(t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab() << t.name()
             << " rejected because of fitting failure" << endl;
#endif
        continue;
      }
      t.refine(30. * 10.);
      err = _fitter.fit(t);
      if (err < 0) {
        bads.append(t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab() << t.name()
             << " rejected because of fitting failure" << endl;
#endif
        continue;
      }
      t.refine(30. * 1.);
      err = _fitter.fit(t);
      if (err < 0) {
        bads.append(t);
#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab() << t.name()
             << " rejected because of fitting failure" << endl;
#endif
        continue;
      }
    }
    _tracks.remove(bads);

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TTrackManager::mask(void) const
  {
//cnv #ifdef TRASAN_DEBUG_DETAIL
//     cout << name() << " ... masking" << endl;
// #endif
//     static const unsigned nLayers = Belle2::TRGCDC::getTRGCDC()->nLayers();
//     static unsigned * nHits = new unsigned[nLayers];

//     unsigned n = _tracks.length();
//     for (unsigned i = 0; i < n; i++) {
//  TTrack & t = * _tracks[i];

//  //...Skip if no core...
//  //   This should not be happend...
//  if (! t.cores().length()) continue;

//  //...Counts # of hits per layer...
//  static unsigned * nHits = new unsigned[Belle2::TRGCDC::getTRGCDC()->nLayers()];
// // unsigned nHits[50];
//  TLink::nHits(t.cores(), nHits);

//  //...Check each layer...
//  bool needMask = false;
//  for (unsigned j = 0; j < 50; j++) {
//      if (nHits[j] > 1) {
//    AList<TLink> linksInLayer = TLink::sameLayer(t.links(), j);
//    if (TLink::width(linksInLayer) > 2) {
//        needMask = true;

// #ifdef TRASAN_DEBUG_DETAIL
//        TLink::dump(linksInLayer, "sort", "    -->");
// #endif
//        break;
//    }
//      }
//  }
//  if (! needMask) continue;

// #ifdef TRASAN_DEBUG_DETAIL
//  cout << "    trk" << i << "(id is tmp) needs mask" << endl;
//  cout << "        type = " << t.type() << endl;
// #endif

//  //...Switch by track type...
//  switch (t.type()) {
//  case TrackTypeNormal:
//      maskNormal(t);
//      maskMultiHits(t);
//      break;
//  case TrackTypeCurl:
//      maskCurl(t);
//      maskMultiHits(t);
//      break;
//  default:
//      break;
//  }

//  //...Refit...
//  // refit() ???
//  _fitter.fit(t);

// #ifdef TRASAN_DEBUG_DETAIL
//  cout << "    masking result : ";
//  t.dump("detail sort", "    ");
// #endif
//     }
  }

  void
  TTrackManager::nameTracks(void)
  {
    unsigned n = _tracks.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];
      t._name = "trk" + itostring(i) + "(" + t._name + ")";
    }
    AList<TTrack> tmp = _tracksAll;
    tmp.remove(_tracks);
    unsigned n1 = tmp.length();
    for (unsigned i = 0; i < n1; i++) {
      TTrack& t = * tmp[i];
      t._name = "trk" + itostring(i + n)  + "(" + t._name + ")";
    }
  }

  TLink&
  TTrackManager::divide(const TTrack& t, AList<TLink> * l) const
  {
    TLink& start = * TLink::outerMost(t.links());
    const HepGeom::Point3D<double> & center = t.helix().center();
    const Vector3D a = start.positionOnTrack() - center;
    for (unsigned j = 0; j < t.nLinks(); j++) {
      if (t[j] == & start) continue;
      TLink& k = * t[j];
      const Vector3D b = k.positionOnTrack() - center;
      if (a.cross(b).z() >= 0.) l[0].append(k);
      else                      l[1].append(k);
    }

#ifdef TRASAN_DEBUG_DETAIL
    cout << "    outer link = " << start.hit()->wire().name() << endl;
    cout << "        nLinks of 0 = " << l[0].length() << endl;
    cout << "        nLinks of 1 = " << l[1].length() << endl;
#endif

    if (l[0].length() == 0 || l[1].length() == 0)
      return divideByIp(t, l);

    return start;
  }

  TLink&
  TTrackManager::divideByIp(const TTrack& t, AList<TLink> * l) const
  {
    l[0].removeAll();
    l[1].removeAll();

    const HepGeom::Point3D<double> & center = t.helix().center();
    const Vector3D a = ORIGIN - center;
    for (unsigned j = 0; j < t.nLinks(); j++) {
      TLink& k = * t[j];
      const Vector3D b = k.positionOnTrack() - center;
      if (a.cross(b).z() >= 0.) l[0].append(k);
      else                      l[1].append(k);
    }

    //...This is a dummy...
    TLink& start = * TLink::outerMost(t.links());
    return start;
  }

  void
  TTrackManager::removeHitsAcrossOverIp(AList<TLink> & l) const
  {

    //...Calculate average phi...
    unsigned n = l.length();
    float phiSum = 0.;
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWire& w = l[i]->hit()->wire();
      unsigned j = w.localId();
      unsigned nWire = w.layer().nCells();

      float phi = (float) j / (float) nWire;
      phiSum += phi;
    }
    float average = phiSum / (float) n;

    AList<TLink> cross;
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWire& w = l[i]->hit()->wire();
      unsigned j = w.localId();
      unsigned nWire = w.layer().nCells();

      float phi = (float) j / (float) nWire;
      float dif = fabs(phi - average);
      if (dif > 0.5) dif = 1. - dif;

      if (dif > 0.3) cross.append(l[i]);
    }
    l.remove(cross);

#ifdef TRASAN_DEBUG_DETAIL
    cout << "    Cross over IP reduction : ";
    for (unsigned i = 0; i < (unsigned) cross.length(); i++) {
      cout << cross[i]->wire()->name() << ",";
    }
    cout << endl;
#endif
  }


  void
  TTrackManager::maskOut(TTrack& t, const AList<TLink> & links) const
  {
    unsigned n = links.length();
    if (! n) return;
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWireHit& hit = * links[i]->hit();
      hit.state(hit.state() | CellHitInvalidForFit);
    }
    t._fitted = false;

#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(links, "detail", "    TTrackManager::maskOut ... masking ");
#endif
  }

  void
  TTrackManager::maskMultiHits(TTrack& t) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    cout << "... masking multi-hits" << endl;
#endif

    if (! t.cores().length()) return;
    AList<TLink> cores = t.cores();
    unsigned n = cores.length();
    bool layerLimited = false;
    AList<TLink> bads;

    cores.sort(TLink::sortByWireId);
    for (unsigned i = 0; i < n; i++) {
      if (layerLimited) {
        bads.append(cores[i]);
        continue;
      }
      AList<TLink> linksInLayer =
        TLink::sameLayer(cores, cores[i]->wire()->layerId());
      if (linksInLayer.length() > 3) {
        bads.append(cores[i]);
        layerLimited = true;
      }
    }
    maskOut(t, bads);
  }

  void
  TTrackManager::maskNormal(TTrack& t) const
  {

    //...Divide into two tracks...
    AList<TLink> l[2];
//  TLink & start = divideByIp(t, l);

#ifdef TRASAN_DEBUG_DETAIL
    cout << "    normal : divided by IP" << endl;
    cout << "    0=";
    for (unsigned j = 0; j < (unsigned) l[0].length(); j++) {
      cout << "," << l[0][j]->wire()->name();
    }
    cout << endl;
    cout << "    1=";
    for (unsigned j = 0; j < (unsigned) l[1].length(); j++) {
      cout << "," << l[1][j]->wire()->name();
    }
    cout << endl;
#endif

    //...Which should be masked out ?...
    unsigned maskSide = 2;

    //...1. Check by # of super layers...
    if (TLink::nSuperLayers(l[0]) < TLink::nSuperLayers(l[1]))      maskSide = 0;
    else if (TLink::nSuperLayers(l[0]) > TLink::nSuperLayers(l[1])) maskSide = 1;
#ifdef TRASAN_DEBUG_DETAIL
    cout << "    NSuperLayers 0, 1 = " << TLink::nSuperLayers(l[0]) << ", ";
    cout << TLink::nSuperLayers(l[1]) << endl;
#endif
    if (maskSide != 2) {
      maskOut(t, l[maskSide]);
      return;
    }

    //...2. Check by the inner-most layer...
    unsigned i0 = TLink::innerMost(l[0])->wire()->layerId();
    unsigned i1 = TLink::innerMost(l[1])->wire()->layerId();
    if (i0 < i1) maskSide = 1;
    else if (i0 > i1) maskSide = 0;
#ifdef TRASAN_DEBUG_DETAIL
    cout << "    i0, i1 = " << i0 << ", " << i1 << endl;
#endif
    if (maskSide != 2) {
      maskOut(t, l[maskSide]);
      return;
    }

    //...3. Check by # of layers...
    if (TLink::nLayers(l[0]) < TLink::nLayers(l[1])) maskSide = 0;
    else if (TLink::nLayers(l[0]) > TLink::nLayers(l[1])) maskSide = 1;
#ifdef TRASAN_DEBUG_DETAIL
    cout << "    NLayers 0, 1 = " << TLink::nLayers(l[0]) << ", ";
    cout << TLink::nLayers(l[1]) << endl;
#endif
    if (maskSide != 2) {
      maskOut(t, l[maskSide]);
      return;
    }

    //...4. Check by pt...
    if (maskSide == 2) {
      TTrack* tt[2];
      for (unsigned j = 0; j < 2; j++) {
        tt[j] = new TTrack(t);
        tt[j]->remove(l[j]);
        _fitter.fit(* tt[j]);
      }
      if (tt[1]->pt() > tt[0]->pt()) maskSide = 1;
      else                           maskSide = 0;
#ifdef TRASAN_DEBUG_DETAIL
      cout << "    pt 0 = " << tt[1]->pt() << endl;
      cout << "    pt 1 = " << tt[0]->pt() << endl;
#endif
      delete tt[0];
      delete tt[1];
    }
    maskOut(t, l[maskSide]);
    return;
  }

  void
  TTrackManager::maskCurl(TTrack& t) const
  {

    //...Divide into two tracks...
    AList<TLink> l[2];
//  TLink & start = divideByIp(t, l);
    if (l[0].length() == 0) return;
    if (l[1].length() == 0) return;

#ifdef TRASAN_DEBUG_DETAIL
    cout << "    curl : divided by IP" << endl;
    cout << "    0:";
    TLink::dump(l[0], "flag sort");
    cout << "    1:";
    TLink::dump(l[1], "flag sort");
    cout << endl;
#endif

    //...Which should be masked out ?...
    unsigned maskSide = 2;

    //...1. Check by # of super layers...
    if (TLink::nSuperLayers(l[0]) < TLink::nSuperLayers(l[1]))      maskSide = 0;
    else if (TLink::nSuperLayers(l[0]) > TLink::nSuperLayers(l[1])) maskSide = 1;
#ifdef TRASAN_DEBUG_DETAIL
    cout << "    NSuperLayers 0, 1 = " << TLink::nSuperLayers(l[0]) << ", ";
    cout << TLink::nSuperLayers(l[1]) << endl;
#endif
    if (maskSide != 2) {
      maskOut(t, l[maskSide]);
      return;
    }

    //...Make two tracks...
    TTrack* tt[2];
    tt[0] = new TTrack(t);
    tt[1] = new TTrack(t);
    tt[0]->remove(l[1]);
    tt[1]->remove(l[0]);
    _fitter.fit(* tt[0]);
    _fitter.fit(* tt[1]);
    THelix h0 = THelix(tt[0]->helix());
    THelix h1 = THelix(tt[1]->helix());

    //...Check by z...
    h0.pivot(ORIGIN);
    h1.pivot(ORIGIN);
    if (fabs(h0.dz()) < fabs(h1.dz())) maskSide = 1;
    else                               maskSide = 0;

    delete tt[0];
    delete tt[1];
    maskOut(t, l[maskSide]);
    return;
  }

  void
  TTrackManager::determineT0(unsigned level, unsigned nMax)
  {
#ifdef TRASAN_DEBUG_DETAIL
    if (level == 0) {
      cout << "TTrackManager::determineT0 !!! called with level = 0";
      cout << endl;
    }
#endif

    static bool first = true;
    static unsigned methode = 0;
    if (first) {
      first = false;

      if (level == 1) {
        _cFitter.fit2D(true);
      } else if (level == 2) {
        // default setting
      } else if (level == 3) {
        _cFitter.sag(true);
      } else if (level == 4) {
        _cFitter.sag(true);
        _cFitter.propagation(true);
      } else if (level == 5) {
        _cFitter.sag(true);
        _cFitter.propagation(true);
        _cFitter.tof(true);
      } else if (level == 6) {
        methode = 1;
        _cFitter.sag(true);
        _cFitter.propagation(true);
        _cFitter.tof(true);
      } else if (level == 7) {
        methode = 2;
        _cFitter.sag(true);
        _cFitter.propagation(true);
        _cFitter.tof(true);
      }
    }

    unsigned n = _tracks.length();
    if (! n) return;

    if (nMax == 0) nMax = n;
    if (n > nMax) n = nMax;

    float t0 = 0.;
    if (methode == 0) t0 = T0(n);
    else if (methode == 1) t0 = T0Fit(n);
    // cout << "reccdc_timing=" << BsCouTab(RECCDC_TIMING) << endl;
//cnv    else if (methode == 2 && BsCouTab(RECCDC_TIMING) != 0) {
    else if (methode == 2) {
//cnv   struct reccdc_timing * r0 = (struct reccdc_timing *)
//cnv       BsGetEnt(RECCDC_TIMING, BsCouTab(RECCDC_TIMING), BBS_No_Index);
//cnv struct reccdc_timing * r0 = 0;
//cnv   if (r0->m_quality == 102) {
//    if (BsCouTab(BELLE_EVENT)) {
//      struct belle_event * b0 = (struct belle_event *)
//      BsGetEnt(BELLE_EVENT, 1, BBS_No_Index);
//      if(1==b0->m_ExpMC)                    t0 = T0Fit(n);
//      if(2==b0->m_ExpMC && r0->m_time !=0.) t0 = T0Fit(n);
//    }
//  }
//  else if (r0->m_quality == 100) t0 = T0Fit(n);
      // cout << "quality=" << r0->m_quality << endl;
    }

    //...For debug...
    if (_debugLevel) {
      cout << "TTrackManager::determineT0 ... methode=" << methode;
      cout << ", T0 offset=" << - t0;
      cout << ", # of tracks used=" << n << endl;
    }
  }

  float
  TTrackManager::T0(unsigned n)
  {

#define X0 -10.
#define X1 0.
#define X2 10.
#define STEP 10.

    //...Determine T0 for each track...
    float t0Sum = 0.;
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];
      float y[3];
      for (unsigned j = 0; j < 3; j++) {
        float offset = X0 + j * STEP;
        _cFitter.fit(t, offset);
        y[j] = t.chi2();
      }
      t0Sum += minimum(y[0], y[1], y[2]);
    }
    float t0 = t0Sum / (float) n;
    if (isnan(t0)) t0 = 0.;

    //...Fit with T0 correction...
    n = _tracks.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];
      _cFitter.fit(t, t0);
    }

    //...Store it...
//cnv    reccdc_timing * t = (reccdc_timing *) BsNewEnt(RECCDC_TIMING);
    reccdc_timing* t = 0;
    t->m_time = - t0;
    t->m_quality = 11;

    return - t0;
  }

  float
  TTrackManager::T0Fit(unsigned n)
  {

    float tev_err;
//  float tev_sum0= 0.;
    double tev_sum = 0.;
//  float tev_sum2= 0.;
    double w_sum   = 0.;

    //sort in order of pt
    // cout << "length=" << _tracks.length() << endl;
    const unsigned cn = _tracks.length();
//    float* sort = new float[cn];
    unsigned* sort = new unsigned[cn];
    float ptmax_pre = 1.e10;
    for (unsigned i = 0; i < cn; i++) {
      float ptmax = -999.;
      int   jmax;
      for (unsigned j = 0; j < cn; j++) {
        TTrack& tj = * _tracks[j];
        float pt = fabs(1. / tj.helix().a()[2]);
        if (pt < ptmax_pre && pt > ptmax) {
          ptmax = pt;
          jmax  = j;
        }
        sort[i] = jmax;
      }
      ptmax_pre = ptmax;
    }

    //    cout << "cn,n=" << cn <<" "<< n << endl;
    //    for (unsigned i = 0; i < n; i++) {
    int n_good(0);
    for (unsigned i = 0; i < cn; i++) {
      //srtbypt TTrack & t = * _tracks[i];
      TTrack& t = * _tracks[sort[i]];
      //      if( t.ndf() < 2 ) continue;
      if ((t.helix().a()[3] == 0.) && (t.helix().a()[4] == 0.)) continue;
      if (t.ndf() < 5)                                    continue;
      //      if( t.pt()  < 0.2)                                    continue;
      const HepGeom::Point3D<double>  pvt = t.helix().pivot();
      const CLHEP::HepVector  a   = t.helix().a();
      THelix hl(pvt, a);
      hl.ignoreErrorMatrix();
      hl.pivot(ORIGIN);
      if (abs(hl.dr()) > 10.) continue;
      if (abs(hl.dz()) > 25.) continue;
      n_good++;
      if (n_good > (int)n) break;

      //      float pt = fabs(1./t.helix().a()[2]);
      // cout << "pt=" << pt << endl;
      float tev = 0.;
      int fitstat = _cFitter.fit(t, tev, tev_err);
      //  cout << "tev,tev_err=" <<tev<< " "<<tev_err<<endl;
      if (fitstat != 0) continue;
      double w = 1. / tev_err / tev_err;
      tev_sum += w * tev;
      w_sum += w;
      // tev_sum0 += tev;
      // tev_sum2 += tev * tev;
    }

    delete [] sort;

    float tev_mean = 0;
    if (w_sum != 0) tev_mean = tev_sum / w_sum;
    // float tev_err_a = 1. / sqrt(w_sum);
    // float tev_err_b = (tev_sum2 - tev_sum0 * tev_sum0 / (n + 1)) / n;
    // tev_err_b = sqrt(tev_err_b);
    // cout << "comp,t0,mean tev,err ="<<t0<<" "<<tev_mean<<" "<<tev_err_a<<" "<<tev_err_b<<endl;
    if (isnan(tev_mean)) tev_mean = 0.;

    //...Store it...
//cnv    reccdc_timing * tt = (reccdc_timing *) BsNewEnt(RECCDC_TIMING);
    reccdc_timing* tt = 0;
    tt->m_time = tev_mean;
    tt->m_quality = 151;

    return - tev_mean;
  }

  float
  TTrackManager::minimum(float y0, float y1, float y2) const
  {
    float xMin = X1 + 0.5 * STEP * (y0 - y2) / (y0 + y2 - 2. * y1);
    return xMin;
  }

// added by matsu ( 1999/05/24 )
  void
  TTrackManager::merge(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::merge";
    EnterStage(stage);
#endif

    //...Merging...
    unsigned n = _tracks.length();
    AList<TTrack> bads;
    unsigned* flagTrk;
    if (NULL == (flagTrk = (unsigned*) malloc(n * sizeof(unsigned)))) {
      perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:flagTrk:malloc");
      exit(1);
    }
    for (unsigned i = 0; i < n; i++) flagTrk[i] = 0;

    //...Search a track to be merged...
    for (unsigned i0 = 0; i0 < n; i0++) {

      if (flagTrk[i0] != 0) continue;
      TTrack& t0 = * _tracks[i0];
      if (!(t0.pt() < 0.25)) continue;

      unsigned Noverlap(0), Nall(0);
      float OverlapRatioMax(-999.);
      unsigned MaxID(0);

      for (unsigned i1 = 0 ; i1 < n; i1++) {

        if (i0 == i1 || flagTrk[i1] != 0) continue;
        TTrack& t1 = * _tracks[i1];
        if (!(t1.pt() < 0.25)) continue;
        Nall = t1.links().length();
        if (! Nall) continue;

        Noverlap = 0;
        for (unsigned j = 0; j < Nall; j++) {
          TLink& l =  * t1.links()[j];
          const Belle2::TRGCDCWireHit& whit =  * l.hit();
          double load(2.);
          if (whit.state() & CellHitStereo) load = 3.;

          double x = t0.helix().center().x() - l.positionOnTrack().x();
          double y = t0.helix().center().y() - l.positionOnTrack().y();
          double r = sqrt(x * x + y * y);
          double R = fabs(t0.helix().radius());

          if ((R - load) < r && r < (R + load)) Noverlap++;
        }

        if (! Noverlap) continue;
        float tmpRatio = float(Noverlap) / float(Nall);

        if (tmpRatio > OverlapRatioMax) {
          OverlapRatioMax = tmpRatio;
          MaxID = i1;
        }
      }

      if (OverlapRatioMax < 0.8) continue;

      //...Mask should be done...
      unsigned MaskID[2] = {MaxID , i0};
      AList<TLink> l[2];

      for (unsigned j0 = 0; j0 < 2; j0++) {
        for (unsigned j1 = 0; j1 < _tracks[MaskID[j0]]->nLinks(); j1++) {
          TLink& k = * _tracks[MaskID[j0]]->links()[j1];
          l[j0].append(k);
        }
      }
      // _tracks[i0]->links().append( _tracks[MaxID]->links() );
      // _tracks[MaxID]->links().append( _tracks[i0]->links ());
      _tracks[i0]->append(_tracks[MaxID]->links());
      _tracks[MaxID]->append(_tracks[i0]->links());

#ifdef TRASAN_DEBUG_DETAIL
      cout << "    mask & merge " << endl;
      cout << "    0:";
      TLink::dump(l[0], "flag sort");
      cout << "    1:";
      TLink::dump(l[1], "flag sort");
      cout << endl;
#endif

      //...Which should be masked out ?...
      unsigned maskSide = 2;

#if 0
      //...0. Check by # of super layers... ( not applied now )
      unsigned super0 = NSuperLayers(l[0]);
      unsigned super1 = NSuperLayers(l[1]);

      if (super0 < super1) maskSide = 0;
      else if (super0 > super1) maskSide = 1;

#ifdef TRASAN_DEBUG_DETAIL
      cout << "    NSuperLayers 0, 1 = " << NSuperLayers(l[0]) << ", ";
      cout << NSuperLayers(l[1]) << endl;
#endif

      if (maskSide == 2) {
#endif

        //...1. Check by the inner-most layer...
        unsigned inner0 = TLink::innerMost(l[0])->wire()->layerId();
        unsigned inner1 = TLink::innerMost(l[1])->wire()->layerId();
        if (inner0 < inner1) maskSide = 1;
        else if (inner0 > inner1) maskSide = 0;

        if (maskSide == 2) {

          //...2. Check by dz

          //...Make two tracks...
          TTrack* tt[2];
          tt[0] = new TTrack(*(_tracks[MaskID[0]]));
          tt[1] = new TTrack(*(_tracks[MaskID[1]]));
          _fitter.fit(* tt[0]);
          _fitter.fit(* tt[1]);
          THelix h0 = THelix(tt[0]->helix());
          THelix h1 = THelix(tt[1]->helix());

          //...Check dz...
          h0.pivot(ORIGIN);
          h1.pivot(ORIGIN);
          if (fabs(h0.dz()) < fabs(h1.dz())) maskSide = 1;
          else                               maskSide = 0;

          delete tt[0];
          delete tt[1];
        }
#if 0
      }
#endif
      bads.append(_tracks[MaskID[maskSide]]);
      flagTrk[MaskID[maskSide]] = 1;
    }

    _tracks.remove(bads);

    //*****  Masking *****
    n = _tracks.length();

    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * _tracks[i];
      for (unsigned j = 0; j < (unsigned) t.links().length(); j++) {
        TLink& l =  * t.links()[j];
        const Belle2::TRGCDCWireHit& whit =  * l.hit();

        if (!(whit.state() & CellHitFittingValid)) continue;

        // within half circle or not?
        double q = t.helix().center().x() * l.positionOnTrack().y() -
                   t.helix().center().y() * l.positionOnTrack().x();
        double qq =  q * t.charge();

        if (qq > 0) whit.state(whit.state() & ~CellHitInvalidForFit);
        else         whit.state(whit.state() | CellHitInvalidForFit);
#ifdef TRASAN_DEBUG_DETAIL
        cout << "TTrackManager::merge ... masking" << endl;
#endif
      }
    }

    free(flagTrk);

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }
// end of addition

  int
  TTrackManager::copyTrack(TTrack& t,
                           reccdc_trk** pr,
                           reccdc_trk_add** pra) const
  {

    static const unsigned GoodHitMask = (CellHitTimeValid |
                                         CellHitChargeValid |
                                         CellHitFindingValid |
                                         CellHitFittingValid);
    int err = 0;

    //...Hit loop...
#ifdef TRASAN_DEBUG
    t.fitted(true);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    cout << "    checking hits ... " << t.name()
         << " quality = " << t.quality();
    cout << " : " << t.cores().length() << ", " << t.ndf() << " : ";
#endif
//  unsigned j = 0;
    unsigned nClst = 0;
    unsigned nStereos = 0;
    unsigned nOccupied = 0;
    AList<TLink> hits;
    AList<TLink> badHits;
    const unsigned n = t.links().length();
    for (unsigned i = 0; i < n; i++) {
      TLink* l = t.links()[i];
//cnv reccdc_wirhit * h = l->hit()->reccdc();
      reccdc_wirhit* h = 0;

#ifdef TRASAN_DEBUG_DETAIL
      cout << l->wire()->name();
      if (h->m_trk) cout << "(n/a)";
      if ((l->hit()->state() & GoodHitMask) == GoodHitMask) {
        if (l->hit()->state() & CellHitInvalidForFit) {
          if (!(h->m_stat & CellHitInvalidForFit))
            cout << "(bad)";
        }
      }
      cout << ",";
#endif

      if (h->m_trk) {
        ++nOccupied;
        if (!(h->m_stat & CellHitInvalidForFit))
          continue;
      }
      if ((l->hit()->state() & GoodHitMask) == GoodHitMask) {
        if (l->hit()->state() & CellHitInvalidForFit) {
          if (!(h->m_stat & CellHitInvalidForFit))
            badHits.append(l);
        } else {
          hits.append(l);
          if (l->wire()->stereo()) ++nStereos;
        }
      }
    }
    t.finalHits(hits);
#ifdef TRASAN_DEBUG_DETAIL
    cout << endl;
#endif

    //...Check # of hits...
    if (t.quality() & TrackQuality2D) {
      if (hits.length() < 3) err = 3;
      if (nOccupied > 2) err = 4;
    } else {
      if (hits.length() < 5) err = 1;
      if (nStereos < 2) err = 2;
    }
    if (err) return err;

    //...Create new tables...
//cnv     * pr = (reccdc_trk *) BsNewEnt(RECCDC_TRK);
//     * pra = (reccdc_trk_add *) BsNewEnt(RECCDC_TRK_ADD);
    * pr = 0;
    * pra = 0;
    reccdc_trk* r = * pr;
//  reccdc_trk_add * ra = * pra;

    //...Copy hit information...
    // const AList<TLink> & cores = t.cores();
    // const AList<TLink> & links = t.links();
    // unsigned allHits = cores.length();
    // unsigned stereoHits = NStereoHits(cores);
    // r.m_chiSq = t.chi2();
    // r.m_confl = t.confidenceLevel();
    // r.m_ndf = t.ndf();
    // r.m_nhits = allHits;
    // r.m_nster = stereoHits;
    float chisq = 0.;
    unsigned nHits = hits.length();
    for (unsigned i = 0; i < nHits; i++) {
      TLink* l = hits[i];
//cnv reccdc_wirhit * h = hits[i]->hit()->reccdc();
      reccdc_wirhit* h = 0;
      h->m_trk = r->m_ID;
      h->m_pChiSq = l->pull();
      h->m_lr = l->leftRight();
      if (l->usecathode() == 4) ++nClst;
      chisq += h->m_pChiSq;

#ifdef TRASAN_DEBUG_DETAIL
      cout << "        chisq,sum(" << l->wire()->name() << ")=:"
           << h->m_pChiSq << "," << chisq << endl;
#endif
    }
    r->m_chiSq = chisq;
    r->m_nhits = nHits;
    r->m_nster = nStereos;
    r->m_ndf = nHits - 5;
    if (t.quality() & TrackQuality2D)
      r->m_ndf = nHits - 3;

    //...Bad hits...
    const unsigned n2 = badHits.length();
    for (unsigned i = 0; i < n2; i++) {
//cnv reccdc_wirhit * h = badHits[i]->hit()->reccdc();
      reccdc_wirhit* h = 0;
      h->m_trk = r->m_ID;
      h->m_stat |= CellHitInvalidForFit;
    }

    //...Cathode...
    r->m_nclus = nClst;

    //...THelix parameter...
    const CLHEP::HepVector& a = t.helix().a();
    const CLHEP::HepSymMatrix& ea = t.helix().Ea();
    const HepGeom::Point3D<double> & x = t.helix().pivot();
    r->m_helix[0] = tosingle(a[0]);
    r->m_helix[1] = tosingle(a[1]);
    r->m_helix[2] = tosingle(a[2]);
    r->m_helix[3] = tosingle(a[3]);
    r->m_helix[4] = tosingle(a[4]);

    r->m_pivot[0] = tosingle(x.x());
    r->m_pivot[1] = tosingle(x.y());
    r->m_pivot[2] = tosingle(x.z());

    r->m_error[0] = tosingle(ea[0][0]);
    r->m_error[1] = tosingle(ea[1][0]);
    r->m_error[2] = tosingle(ea[1][1]);
    r->m_error[3] = tosingle(ea[2][0]);
    r->m_error[4] = tosingle(ea[2][1]);
    r->m_error[5] = tosingle(ea[2][2]);
    r->m_error[6] = tosingle(ea[3][0]);
    r->m_error[7] = tosingle(ea[3][1]);
    r->m_error[8] = tosingle(ea[3][2]);
    r->m_error[9] = tosingle(ea[3][3]);
    r->m_error[10] = tosingle(ea[4][0]);
    r->m_error[11] = tosingle(ea[4][1]);
    r->m_error[12] = tosingle(ea[4][2]);
    r->m_error[13] = tosingle(ea[4][3]);
    r->m_error[14] = tosingle(ea[4][4]);

    //...Get outer most hit(=termination point)...
    TLink* last = TLink::outerMost(hits);

    //...Calculate phi of the termination point...
    t.approach(* last);
    r->m_fiTerm = last->dPhi();

    return err;
  }

  int
  TTrackManager::copyTrack(Belle2::StoreArray<GFTrackCand> & trackCandidates,
                           TTrack& t) const
  {

    //...Get # for new candidate...
    const int counter = trackCandidates.getEntries();

    //...New storage...
    trackCandidates.appendNew();

    //...Helix parameters... Need to check pivot position
    //    TVector3 momentum(t.helix().momentum().x(),
    //                      t.helix().momentum().y(),
    //                      t.helix().momentum().z());
    //    TVector3 position(t.helix().x().x(),
    //                      t.helix().x().y(),
    //                      t.helix().x().z());
    //
    //    cout << t.helix().pivot() << endl;

    THelix hAtOrigin = t.helix();
    hAtOrigin.ignoreErrorMatrix();
    hAtOrigin.pivot(ORIGIN);
    TVector3 momentum(hAtOrigin.momentum().x(),
                      hAtOrigin.momentum().y(),
                      hAtOrigin.momentum().z());
    TVector3 position(hAtOrigin.x().x(),
                      hAtOrigin.x().y(),
                      hAtOrigin.x().z());
    //    cout << hAtOrigin.pivot() << endl;

    //...Erros on helix parameters...
    //   I don't know the meaning of these errors.
    //   So copied from MCTrckFinderModule.
    TMatrixDSym seedCov(6);
    seedCov(0, 0) = 1;
    seedCov(1, 1) = 1;
    seedCov(2, 2) = 4;
    seedCov(3, 3) = 0.01;
    seedCov(4, 4) = 0.01;
    seedCov(5, 5) = 0.04;

    //...Let's assume this as pion...
    int pdg = 211 * int(t.charge());

    //...Copy info...
    trackCandidates[counter]->setPosMomSeedAndPdgCode(position, momentum, pdg, seedCov);

    //...No MC info now...
    // trackCandidates[counter]->setMcTrackId(iPart);

    //...CDC hit info... I don't know this is correct or not
    const unsigned n = t.links().length();

    //sort in order of |dphi| in a brute-force way; to be replaced with a smarter way
    float df[n];
    for (unsigned i = 0; i < n; ++i) {
      df[i] = fabs(t.links()[i]->dPhi());
    }
    unsigned sort[n];
    for (unsigned i = 0; i < n; ++i) sort[i] = 9999;
    float dfmin;
    float dfmin_pre = -1.e10;
    for (unsigned i = 0; i < n; ++i) {
      dfmin = 1.e10;
      for (unsigned j = 0; j < n; ++j) {
        if (df[j] < dfmin && df[j] > dfmin_pre) {
          dfmin   = df[j];
          sort[i] = j;
        }
      }
      dfmin_pre = dfmin;
    }

    for (unsigned i = 0; i < n; i++) {
      //      const Belle2::TRGCDCWireHit& h = * t.links()[i]->hit();
      if (sort[i] == 9999) {
        std::cerr << "Trasan: sorting error in copyTrack." << std::endl;
        continue;
      }
      const Belle2::TRGCDCWireHit& h = * t.links()[sort[i]]->hit();

      const unsigned layerId = h.wire().layerId();
      const int hitID = h.iCDCHit();
      //      const double driftTime = h.drift();
//      const HepGeom::Point3D<double> & onTrack = t.links()[sort[i]]->positionOnTrack();
//      const double rho = onTrack.mag();
//      const int uniqueId = layerId * 10000 + h.wire().localId();

      trackCandidates[counter]->addHit(Belle2::Const::CDC, hitID);
    }

    return 0;
  }

  void
  TTrackManager::sortTracksByQuality(void)
  {
    unsigned n = _tracks.length();
    if (n < 2) return;

    for (unsigned i = 0; i < n - 1; i++) {
      TTrack& t0 = * _tracks[i];
      float bestRChisq = HUGE_VAL;
      if (t0.ndf() > 0) bestRChisq = t0.chi2() / t0.ndf();
      for (unsigned j = i + 1; j < n; j++) {
        TTrack& t1 = * _tracks[j];
        float rChisq = HUGE_VAL;
        if (t1.ndf() > 0) rChisq = t1.chi2() / t1.ndf();
        if (rChisq < bestRChisq) {
          bestRChisq = rChisq;
          _tracks.swap(i, j);
        }
      }
    }
  }

  void
  TTrackManager::sortTracksByPt(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    cout << "trkmgr::sortTracksByPt : # of tracks="
         << _tracks.length() << endl;
#endif

    unsigned n = _tracks.length();
    if (n < 2) return;

    for (unsigned i = 0; i < n - 1; i++) {
      TTrack& t0 = * _tracks[i];
      float bestPt = t0.pt();
      for (unsigned j = i + 1; j < n; j++) {
        TTrack& t1 = * _tracks[j];
        float pt = t1.pt();
#ifdef TRASAN_DEBUG_DETAIL
        cout << "i,j=" << i << "," << j
             << " : pt i,j=" << bestPt << "," << pt << endl;
#endif
        if (pt > bestPt) {
          bestPt = pt;
          _tracks.swap(i, j);
        }
      }
    }
  }

  void
  TTrackManager::treatCurler(rectrk& trk1,
                             reccdc_trk_add& cdc1,
                             unsigned flag) const
  {
//cnv //...Originally coded by j.tanaka...

//     //...Check inputs...
//     if (trk1.m_zero[2] == 0) return;
//     if (cdc1.m_daughter == 0) return;

//     //...The other side...
//     reccdc_trk_add & cdc2 = * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
//                cdc1.m_daughter,
//                BBS_No_Index);
//     if (cdc2.m_daughter == 0) return;
//     if (cdc2.m_rectrk == 0) return;
//     rectrk & trk2 = * (rectrk *) BsGetEnt(RECTRK, cdc2.m_rectrk, BBS_No_Index);
//     if (trk2.m_zero[2] == 0) return;

//     //...Obtain RECTRK_LOCALZ...
//     rectrk_localz & z1 = * (rectrk_localz *) BsGetEnt(RECTRK_LOCALZ,
//                  trk1.m_zero[2],
//                  BBS_No_Index);
//     rectrk_localz & z2 = * (rectrk_localz *) BsGetEnt(RECTRK_LOCALZ,
//                  trk2.m_zero[2],
//                  BBS_No_Index);

//     //...Pointer to mother and daughter...
//     reccdc_trk_add * mother = & cdc1;
//     reccdc_trk_add * daughter = & cdc2;

// //      //...By dr...
// //      if (flag == 1) {
// //   float dr1 = fabs(z1.m_helix[0]);
// //   float dr2 = fabs(z2.m_helix[0]);
// //   if (dr1 > dr2) {
// //       mother = & cdc2;
// //       daughter = & cdc1;
// //   }
// //      }

// //      //...By dz...
// //      else {
// //   float dz1 = fabs(z1.m_helix[3]);
// //   float dz2 = fabs(z2.m_helix[3]);
// //   if (dz1 > dz2) {
// //       mother = & cdc2;
// //       daughter = & cdc1;
// //   }
// //      }

//     //...By dz + dr...
//     if(flag == 3){
//  float dz1 = fabs(z1.m_helix[3]);
//  float dz2 = fabs(z2.m_helix[3]);
//  if (fabs(dz1 - dz2) < 2.) flag = 1;
//  else                      flag = 2;
//     }

//     //...By dr...
//     if(flag == 1){
//  float dr1 = fabs(z1.m_helix[0]);
//  float dr2 = fabs(z2.m_helix[0]);
//  if (dr1 > dr2) {
//      mother = & cdc2;
//      daughter = & cdc1;
//  }
//     }

//     //...By dz...
//     else if(flag == 2){
//  float dz1 = fabs(z1.m_helix[3]);
//  float dz2 = fabs(z2.m_helix[3]);
//  if (dz1 > dz2) {
//      mother = & cdc2;
//      daughter = & cdc1;
//  }
//     }

//     //...Update information...
//     mother->m_quality &= (~ TrackQualityOutsideCurler);
//     mother->m_likelihood[0] = 1.;
//     mother->m_decision |= TrackTrackManager;
//     daughter->m_quality |= TrackQualityOutsideCurler;
//     daughter->m_likelihood[0] = 0.;
//     daughter->m_mother = mother->m_ID;
//     daughter->m_daughter = 0;
//     daughter->m_decision |= TrackTrackManager;
  }

  void
  TTrackManager::sortBanksByPt(void) const
  {
//cnv #ifdef TRASAN_DEBUG_DETAIL
//     cout << "trkmgr::sortBanksByPt : # of tracks="
//   << BsCouTab(RECCDC_TRK_ADD) << endl;
// #endif

//     unsigned n = BsCouTab(RECCDC_TRK_ADD);
//     if (n < 2) return;

//     //...Sort RECCDC...
//     unsigned * id;
//     if (NULL == (id = (unsigned *) malloc(n * sizeof(unsigned)))) {
//       perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:id:malloc");
//       exit(1);
//     }
//     for (unsigned i = 0; i < n; i++) id[i] = i;
//     for (unsigned i = 0; i < n - 1; i++) {
//  reccdc_trk & cdc0 =
//      * (reccdc_trk *) BsGetEnt(RECCDC_TRK,
//              i + 1,
//              BBS_No_Index);
//  reccdc_trk_add & add0 =
//      * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
//            i + 1,
//            BBS_No_Index);
//  reccdc_mctrk & mc0 =
//      * (reccdc_mctrk *) BsGetEnt(RECCDC_MCTRK,
//          i + 1,
//          BBS_No_Index);
//  float bestPt = 1. / fabs(cdc0.m_helix[2]);
//  unsigned bestQuality = add0.m_quality;
//  for (unsigned j = i + 1; j < n; j++) {
//      reccdc_trk & cdc1 =
//    * (reccdc_trk *) BsGetEnt(RECCDC_TRK,
//            j + 1,
//            BBS_No_Index);
//      reccdc_trk_add & add1 =
//    * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
//                j + 1,
//                BBS_No_Index);
//      reccdc_mctrk & mc1 =
//    * (reccdc_mctrk *) BsGetEnt(RECCDC_MCTRK,
//              j + 1,
//              BBS_No_Index);
//      float pt = 1. / fabs(cdc1.m_helix[2]);
// #ifdef TRASAN_DEBUG_DETAIL
//      cout << "i,j=" << i << "," << j
//     << " : quality i,j=" << bestQuality << ","
//     << add1.m_quality << endl;
// #endif
//      unsigned quality = add1.m_quality;
//      if (quality > bestQuality) continue;
//      else if (quality < bestQuality) {
//    bestQuality = quality;
//    bestPt = pt;
//    swapReccdc(cdc0, add0, mc0, cdc1, add1, mc1);
//    unsigned tmp = id[i];
//    id[i] = id[j];
//    id[j] = tmp;
// #ifdef TRASAN_DEBUG_DETAIL
//    cout << "swapped" << endl;
// #endif
//    continue;
//      }
// #ifdef TRASAN_DEBUG_DETAIL
//      cout << "i,j=" << i << "," << j
//     << " : pt i,j=" << bestPt << "," << pt << endl;
// #endif
//      if (pt > bestPt) {
// #ifdef TRASAN_DEBUG_DETAIL
//    cout << "swapping ... " << & cdc0 << "," << & add0 << ","
//         << & mc0 << " <-> " << & cdc1 << "," << & add1 << ","
//         << & mc1 << endl;
// #endif
//    bestQuality = quality;
//    bestPt = pt;
//    swapReccdc(cdc0, add0, mc0, cdc1, add1, mc1);
//    unsigned tmp = id[i];
//    id[i] = id[j];
//    id[j] = tmp;
// #ifdef TRASAN_DEBUG_DETAIL
//    cout << "swapped" << endl;
// #endif
//      }
//  }
//     }
// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "trkmgr::sortBanksByPt : first phase finished" << endl;
// #endif

//     tagReccdc(id, n);
//     free(id);

// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "trkmgr::sortBanksByPt : second phase finished" << endl;
// #endif

// #if 0
//     //...Sort RECTRK...
//     n = BsCouTab(RECTRK);
//     if (NULL == (id = (unsigned *) malloc(n * sizeof(unsigned)))) {
//       perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:id:malloc");
//       exit(1);
//     }
//     for (unsigned i = 0; i < n; i++) id[i] = i;
//     if (n > 1) {
//  unsigned i = 0;
//  while (i < n - 1) {
//      rectrk & t = * (rectrk *) BsGetEnt(RECTRK, i + 1, BBS_No_Index);
//      if (t.m_prekal == (i + 1)) {
//    ++i;
//    continue;
//      }

//      rectrk & s = * (rectrk *) BsGetEnt(RECTRK,
//                 t.m_prekal,
//                 BBS_No_Index);

//      swapRectrk(t, s);
//      unsigned tmp = id[i];
//      id[i] = id[s.m_ID - 1];
//      id[s.m_ID - 1] = tmp;

//      // cout << "swap " << i + 1 << " and " << s.m_ID << endl;

// //cnv      reccdc_trk_add & a =
// //     * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
// //                 t.m_prekal,
// //                 BBS_No_Index);
// //       reccdc_trk_add & b =
// //     * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
// //                 s.m_prekal,
// //                 BBS_No_Index);
// //       a.m_rectrk = t.m_ID;
// //       b.m_rectrk = s.m_ID;
//  }
//     }
// #else
// // jtanaka 000925 -->
//     n = BsCouTab(RECTRK);
//     if (NULL == (id = (unsigned *) malloc(n * sizeof(unsigned)))) {
//       perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:id:malloc");
//       exit(1);
//     }
//     for (unsigned i = 0; i < n; i++) id[i] = i;
//     int foundId = 0;
//     while(foundId != (int) n){
//       rectrk & t = * (rectrk *) BsGetEnt(RECTRK, foundId + 1, BBS_No_Index);

//       //...Move tracks having "prekal = 0" to the last 2003/04/12 H.Kakuno...
//       // int minPrekal = t.m_prekal;
//       int minPrekal = t.m_prekal ? t.m_prekal : n+1;
//       int exchangeId = foundId;
//       for(int i=foundId+1;i<(int) n;++i){
//  rectrk & s = * (rectrk *) BsGetEnt(RECTRK, i + 1, BBS_No_Index);
//  int s_prekal = s.m_prekal ? s.m_prekal : n+1;
//  if(s_prekal < minPrekal){
//  // if(s.m_prekal < minPrekal){
//    minPrekal = s.m_prekal;
//    exchangeId = i;
//  }
//       }
//       if(exchangeId != foundId){
//  rectrk & s = * (rectrk *) BsGetEnt(RECTRK,
//             exchangeId + 1,
//             BBS_No_Index);

//  swapRectrk(t, s);
//  unsigned tmp = id[t.m_ID - 1];
//  id[t.m_ID - 1] = id[s.m_ID - 1];
//  id[s.m_ID - 1] = tmp;

// //cnv  reccdc_trk_add & a =
// //     * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
// //           t.m_prekal,
// //           BBS_No_Index);
// //   reccdc_trk_add & b =
// //     * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
// //           s.m_prekal,
// //           BBS_No_Index);
// //   a.m_rectrk = t.m_ID;
// //   b.m_rectrk = s.m_ID;
//       }
//       ++foundId;
//     }
// // <-- jtanaka 000925
// #endif

//     tagRectrk(id, n);
//     free(id);
  }

  void
  TTrackManager::swapReccdc(reccdc_trk& cdc0,
                            reccdc_trk_add& add0,
                            reccdc_mctrk& mc0,
                            reccdc_trk& cdc1,
                            reccdc_trk_add& add1,
                            reccdc_mctrk& mc1) const
  {
#define RECCDC_ACTUAL_SIZE 124
#define RECCDCADD_ACTUAL_SIZE 40
#define RECCDCMC_ACTUAL_SIZE 28

    static bool first = true;
    static void* swapRegion;
    if (first) {
      first = false;
      if (NULL == (swapRegion = malloc(RECCDC_ACTUAL_SIZE))) {
        perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:PedNoiseValid:malloc");
        exit(1);
      }
    }

    void* s0 = & cdc0.m_helix[0];
    void* s1 = & cdc1.m_helix[0];
    memcpy(swapRegion, s0, RECCDC_ACTUAL_SIZE);
    memcpy(s0, s1, RECCDC_ACTUAL_SIZE);
    memcpy(s1, swapRegion, RECCDC_ACTUAL_SIZE);

    s0 = & add0.m_quality;
    s1 = & add1.m_quality;
    memcpy(swapRegion, s0, RECCDCADD_ACTUAL_SIZE);
    memcpy(s0, s1, RECCDCADD_ACTUAL_SIZE);
    memcpy(s1, swapRegion, RECCDCADD_ACTUAL_SIZE);

    if ((& mc0) && (& mc1)) {
      s0 = & mc0.m_hep;
      s1 = & mc1.m_hep;
      memcpy(swapRegion, s0, RECCDCMC_ACTUAL_SIZE);
      memcpy(s0, s1, RECCDCMC_ACTUAL_SIZE);
      memcpy(s1, swapRegion, RECCDCMC_ACTUAL_SIZE);
    }
  }

  void
  TTrackManager::swapRectrk(rectrk& rec0,
                            rectrk& rec1) const
  {
#define RECTRK_ACTUAL_SIZE 84

    static bool first = true;
    static void* swapRegion;
    if (first) {
      first = false;
      if (NULL == (swapRegion = malloc(RECTRK_ACTUAL_SIZE))) {
        perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:swapRegion:malloc");
        exit(1);
      }
    }

    void* s0 = & rec0.m_glob[0];
    void* s1 = & rec1.m_glob[0];
    memcpy(swapRegion, s0, RECTRK_ACTUAL_SIZE);
    memcpy(s0, s1, RECTRK_ACTUAL_SIZE);
    memcpy(s1, swapRegion, RECTRK_ACTUAL_SIZE);
  }

  void
  TTrackManager::tagReccdc(unsigned* id0, unsigned nTrk) const
  {
//cnv     unsigned * id;
//     if (NULL == (id = (unsigned *) malloc(nTrk * sizeof(unsigned)))) {
//       perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:id:malloc");
//       exit(1);
//     }
//     for (unsigned i = 0; i < nTrk; i++)
//  id[id0[i]] = i;

// #ifdef TRASAN_DEBUG_DETAIL
//     for (unsigned i = 0; i < nTrk; i++)
//    cout << "id0 " << i << " ... " << id0[i] << endl;
//     for (unsigned i = 0; i < nTrk; i++)
//    cout << "id  " << i << " ... " << id[i] << endl;
// #endif
// //cnv    unsigned n = BsCouTab(RECCDC_TRK_ADD);
//     unsigned n = 0;

//     for (unsigned i = 0; i < n; i++) {
// //   reccdc_trk_add & w = * (reccdc_trk_add *) BsGetEnt(RECCDC_TRK_ADD,
// //                  i + 1,
// //                  BBS_No_Index);
//  reccdc_trk_add & w = 0;
//  if (w.m_mother) w.m_mother = id[w.m_mother - 1] + 1;
//  if (w.m_daughter) w.m_daughter = id[w.m_daughter - 1] + 1;
//     }

// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "TTrackManager::tagReccdc ... RECCDC_TRK_ADD done" << endl;
// #endif

//     n = BsCouTab(RECCDC_WIRHIT);
//     for (unsigned i = 0; i < n; i++) {
//  reccdc_wirhit & w = * (reccdc_wirhit *) BsGetEnt(RECCDC_WIRHIT,
//               i + 1,
//               BBS_No_Index);
//  if (w.m_trk == 0) continue;
//  w.m_trk = id[w.m_trk - 1] + 1;
//     }

// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "TTrackManager::tagReccdc ... RECCDC_WIRHIT done" << endl;
// #endif

//     n = BsCouTab(DATRGCDC_MCWIRHIT);
//     for (unsigned i = 0; i < n; i++) {
//  datcdc_mcwirhit & m =
//      * (datcdc_mcwirhit *) BsGetEnt(DATRGCDC_MCWIRHIT,i + 1,BBS_No_Index);
//  if (m.m_trk == 0) continue;
//  m.m_trk = id[m.m_trk - 1] + 1;
//     }

// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "TTrackManager::tagReccdc ... DATRGCDC_MCWIRHIT done" << endl;
// #endif

//     n = BsCouTab(RECTRK);
//     for (unsigned i = 0; i < n; i++) {
//  rectrk & r = * (rectrk *) BsGetEnt(RECTRK, i + 1, BBS_No_Index);
//  if (r.m_prekal == 0) continue;
//  r.m_prekal = id[r.m_prekal - 1] + 1;
//     }

// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "TTrackManager::tagReccdc ... RECTRK done" << endl;
// #endif

//     // jtanaka
//     n = BsCouTab(RECCDC_SVD_TRK);
//     for (unsigned i = 0; i < n; i++) {
//  reccdc_svd_trk & r = * (reccdc_svd_trk *) BsGetEnt(RECCDC_SVD_TRK, i + 1, BBS_No_Index);
//  if (r.m_cdc_trk == 0) continue;
//  r.m_cdc_trk = id[r.m_cdc_trk - 1] + 1;
//     }

// #ifdef TRASAN_DEBUG_DETAIL
//     cout << "TTrackManager::tagReccdc ... RECCDC_SVD_TRK done" << endl;
// #endif

//     free(id);
  }

  void
  TTrackManager::setCurlerFlags(void)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::setCurlerFlags";
    EnterStage(stage);
#endif

    unsigned n = _tracks.length();
    if (n < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return;
    }

    for (unsigned i = 0; i < n - 1; i++) {
      TTrack& t0 = * _tracks[i];
      if (t0.type() != TrackTypeCurl) continue;
      float c0 = t0.charge();

      for (unsigned j = i + 1; j < n; j++) {
        TTrack& t1 = * _tracks[j];
        float c1 = t1.charge();
        if (c0 * c1 > 0.) continue;
        if (t1.type() != TrackTypeCurl) continue;

        bool toBeMerged = false;
        unsigned n0 = t0.testByApproach(t1.cores(), _sigmaCurlerMergeTest);
        if (n0 > _nCurlerMergeTest) toBeMerged = true;
        if (! toBeMerged) {
          unsigned n1 = t1.testByApproach(t0.cores(),
                                          _sigmaCurlerMergeTest);
          if (n1 > _nCurlerMergeTest) toBeMerged = true;
        }

        if (toBeMerged) {
//cnv     ++_s->_nToBeMerged;
//    if ((t0.daughter()) || (t1.daughter()))
//        ++_s->_nToBeMergedMoreThanTwo;
//    t0.daughter(& t1);
//    t1.daughter(& t0);
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TTrackManager::salvageAssociateHits(const CAList<Belle2::TRGCDCWireHit> & hits,
                                      float maxSigma2)
  {
#ifdef TRASAN_DEBUG_DETAIL
    const string stage = "TRKMGR::salvage";
    EnterStage(stage);
    cout << Tab() << "#given hits=" << hits.length() << endl;
#endif

    //...Check arguments...
    unsigned nTracks = _tracks.length();
    if (nTracks == 0) {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return;
    }
    unsigned nHits = hits.length();
    if (nHits == 0) {
#ifdef TRASAN_DEBUG_DETAIL
      LeaveStage(stage);
#endif
      return;
    }

    static const TPoint2D o(0., 0.);

    //...Hit loop...
    for (unsigned i = 0; i < nHits; i++) {
      const Belle2::TRGCDCWireHit& h = * hits[i];

      //...Already used ?...
      if (h.state() & CellHitUsed) continue;
#ifdef TRASAN_DEBUG_DETAIL
      cout << Tab() << "checking " << h.wire().name() << endl;;
#endif

      //...Track loop...
      AList<TLink> toBeDeleted;
      TLink* best = NULL;
      TTrack* bestTrack = NULL;
      for (unsigned j = 0; j < nTracks; j++) {
        TTrack& t = * _tracks[j];

#ifdef TRASAN_DEBUG_DETAIL
        t.dump("", Tab(+1) + t.name());
#endif

        //...Pre-selection...
        TPoint2D c = t.center();
        TPoint2D co = - c;
        TPoint2D x = h.wire().xyPosition();

#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab(+2) << "c= " << co.cross(x - c) * t.charge()
             << ",d=" << fabs((x - c).mag() - fabs(t.radius()))
             << endl;
#endif

        if (co.cross(x - c) * t.charge() > 0.)
          continue;
        if (fabs((x - c).mag() - fabs(t.radius())) > 5.)
          continue;

        //...Try to append this hit...
        TLink& link = * new TLink(0, & h);
        int err = t.approach(link);
        if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
          cout << ":" << t.name() << " approach failure";
#endif
          toBeDeleted.append(link);
          continue;
        }

        //...Calculate sigma...
        float distance = link.distance();
        float diff = fabs(distance - link.hit()->drift());
        float sigma = diff / link.hit()->dDrift();
        link.pull(sigma * sigma);

#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab(+2) << "pull=" << link.pull() << endl;
#endif
        if (link.pull() > maxSigma2) {
          toBeDeleted.append(link);
          continue;
        }

        if (best) {
          if (best->pull() > link.pull()) {
            toBeDeleted.append(best);
            best = & link;
            bestTrack = & t;
          } else {
            toBeDeleted.append(link);
          }
        } else {
          best = & link;
          bestTrack = & t;
        }
      }

      if (best) {
        bestTrack->append(* best);
        best->hit()->state(best->hit()->state() | CellHitInvalidForFit);
        _associateHits.append(best);
#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab(+1) << best->hit()->wire().name()
             << "->" << bestTrack->name() << endl;
#endif
      }
      HepAListDeleteAll(toBeDeleted);
    }

#ifdef TRASAN_DEBUG_DETAIL
    LeaveStage(stage);
#endif
  }

  void
  TTrackManager::maskBadHits(const AList<TTrack> & tracks, float maxSigma2)
  {
#ifdef TRASAN_DEBUG_DETAIL
    cout << "... trkmgr::maskBadHits" << endl;
#endif

    unsigned n = tracks.length();
    for (unsigned i = 0; i < n; i++) {
      TTrack& t = * tracks[i];
      bool toBeUpdated = false;
      const AList<TLink> links = t.links();
      unsigned nHits = links.length();
      for (unsigned j = 0; j < nHits; j++) {
        if (links[j]->pull() > maxSigma2) {
          links[j]->hit()->state(links[j]->hit()->state() |
                                 CellHitInvalidForFit);
          toBeUpdated = true;
#ifdef TRASAN_DEBUG_DETAIL
          cout << "    " << t.name() << " : ";
          cout << links[j]->wire()->name() << "(pull=";
          cout << links[j]->pull() << ") is masked" << endl;
#endif
        }
      }
      if (toBeUpdated) t.update();
    }
  }

  void
  TTrackManager::clearTables(void) const
  {
//cnv     BsDelEnt(RECCDC_TRK, BBS_ID_ALL);
//     BsDelEnt(RECCDC_TRK_ADD, BBS_ID_ALL);
//     BsDelEnt(RECCDC_MCTRK, BBS_ID_ALL);
//     BsDelEnt(RECCDC_MCTRK2HEP, BBS_ID_ALL);

//     //...Clear track association...
//     unsigned n = BsCouTab(RECCDC_WIRHIT);
//     for (unsigned i = 0; i < n; i++) {
//  reccdc_wirhit & h = * (reccdc_wirhit *)
//      BsGetEnt(RECCDC_WIRHIT, i + 1, BBS_No_Index);
//  h.m_trk = 0;
//     }
//     n = BsCouTab(DATRGCDC_MCWIRHIT);
//     for (unsigned i = 0; i < n; i++) {
//  datcdc_mcwirhit & h = * (datcdc_mcwirhit *)
//      BsGetEnt(DATRGCDC_MCWIRHIT, i + 1, BBS_No_Index);
//  h.m_trk = 0;
//     }
  }

  AList<TTrack>
  TTrackManager::selectGoodTracks(const AList<TTrack> & list,
                                  bool track2D) const
  {
    AList<TTrack> goodTracks;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      const TTrack& t = * list[i];
      if (! goodTrack(t, track2D)) continue;

      //...Remove super momentum...
      if (_maxMomentum > 0.) {
        if (t.ptot() > _maxMomentum) {
          ++_s->_nSuperMoms[0];
          const unsigned finder = t.finder();
          for (unsigned j = 1; j < 8; j++)
            if (finder & (1 << j))
              ++_s->_nSuperMoms[j];
          continue;
        }
      }

      //...Pt cut...
      if (_minPt > 0.) {
        if (t.pt() < _minPt) {
          ++_s->_nPtCut[0];
          const unsigned finder = t.finder();
          for (unsigned j = 1; j < 8; j++)
            if (finder & (1 << j))
              ++_s->_nPtCut[j];
          continue;
        }
      }

      //...tanl cut...
      if (_maxTanl > 0.) {
        if (t.helix().tanl() > _maxTanl) {
          ++_s->_nTanlCut[0];
          const unsigned finder = t.finder();
          for (unsigned j = 1; j < 8; j++)
            if (finder & (1 << j))
              ++_s->_nTanlCut[j];
          continue;
        }
      }

      goodTracks.append((TTrack&) t);
    }

#ifdef TRASAN_DEBUG_DETAIL
    if (list.length() != goodTracks.length()) {
      cout << Tab() << "TTrackManager::selectGoodTracks"
           << "... bad tracks found" << endl
           << Tab(+1) << "#given tracks="
           << list.length() << endl
           << Tab(+1) << "#bad tracks="
           << list.length() - goodTracks.length()
           << ":2D flag = " << track2D << endl;
      AList<TTrack> tmp;
      tmp.append(list);
      tmp.remove(goodTracks);
      cout << Tab() << "Bad track dump" << endl;
      for (unsigned i = 0; i < (unsigned) tmp.length(); i++)
        cout << Tab(+1) << TTrack::trackDump(* tmp[i]) << endl;
    }
#endif

    return goodTracks;
  }

  bool
  TTrackManager::checkNumberOfHits(const TTrack& t, bool track2D)
  {
    const AList<TLink> & cores = t.cores();

    if (track2D) {
      unsigned axialHits = TLink::nAxialHits(cores);
      if (axialHits < 3) return false;
    } else {
      unsigned allHits = cores.length();
      if (allHits < 5) return false;
      unsigned stereoHits = TLink::nStereoHits(cores);
      if (stereoHits < 2) return false;
      unsigned axialHits = allHits - stereoHits;
      if (axialHits < 3) return false;
    }
    return true;
  }

  void
  TTrackManager::determineIP(void)
  {
    static const HepGeom::Vector3D<double> InitialVertex(0., 0., 0.);

//cnv     //...Track selection...
//     unsigned n = BsCouTab(RECTRK);
//     AList<rectrk_localz> zList;
//     for (unsigned i = 0; i < n; i++) {
//  const rectrk & t = * (rectrk *) BsGetEnt(RECTRK, i + 1, BBS_No_Index);
//  if (t.m_prekal == 0) continue;
//  const reccdc_trk_add & c = * (reccdc_trk_add *)
//      BsGetEnt(RECCDC_TRK_ADD, t.m_prekal, BBS_No_Index);

//  //...Only good tracks...
//  if (c.m_quality) continue;

//  //...Require SVD hits...
//  const rectrk_global & g = * (rectrk_global *) BsGetEnt(RECTRK_GLOBAL,
//                     t.m_glob[2],
//                     BBS_No_Index);
//  if (! & g) continue;
//  if (g.m_nhits[3] < 2) continue;
//  if (g.m_nhits[4] < 2) continue;

//  //...OK...
//  const rectrk_localz & z = * (rectrk_localz *) BsGetEnt(RECTRK_LOCALZ,
//                     t.m_zero[2],
//                     BBS_No_Index);
//  if (! & z) continue;
//  zList.append((rectrk_localz &) z);
//     }
//     unsigned nZ = zList.length();
//     if (nZ < 2) return;

    //...Fitting...
//      kvertexfitter kvf;
//      kvf.initialVertex(initialVertex);
//      for (unsigned i = 0; i < nZ; i++) {
//    kvf.addTrack();
//      }
  }

  void
  TTrackManager::tagRectrk(unsigned* id0, unsigned nTrk) const
  {
//cnv    unsigned * id;
//     if (NULL == ( id = (unsigned *) malloc(nTrk * sizeof(unsigned)))) {
//       perror("$Id: TTrackManager.cc 10700 2008-11-06 08:47:27Z hitoshi $:id:malloc");
//       exit(1);
//     }
//     for (unsigned i = 0; i < nTrk; i++)
//  id[id0[i]] = i;

// //      for (unsigned i = 0; i < nTrk; i++)
// //     cout << "id0 " << i << " ... " << id0[i] << endl;
// //      for (unsigned i = 0; i < nTrk; i++)
// //     cout << "id  " << i << " ... " << id[i] << endl;
// //      BsShwDat(RECTRK_TOF);

//     unsigned n = BsCouTab(RECTRK_TOF);
//     for (unsigned i = 0; i < n; i++) {
//  rectrk_tof & t = * (rectrk_tof *) BsGetEnt(RECTRK_TOF,
//               i + 1,
//               BBS_No_Index);
//  if (t.m_rectrk) t.m_rectrk = id[t.m_rectrk - 1] + 1;
//     }

// //      BsShwDat(RECTRK_TOF);

//     // jtanaka
//     n = BsCouTab(RECSVD_HIT);
//     for (unsigned i = 0; i < n; i++) {
//         recsvd_hit & t = * (recsvd_hit *) BsGetEnt(RECSVD_HIT,
//                                                    i + 1,
//                                                    BBS_No_Index);
//         if (t.m_trk) t.m_trk = id[t.m_trk - 1] + 1;
//     }

//     free(id);
  }

// jtanaka 000925 -->
#define TRASAN_REPLACE_TABLE 1
#if !(TRASAN_REPLACE_TABLE)
  void
  copyRecCDC_trk_Table(const Reccdc_trk& org,
                       Reccdc_trk& copied)
  {
    copied.helix(0, org.helix(0));
    copied.helix(1, org.helix(1));
    copied.helix(2, org.helix(2));
    copied.helix(3, org.helix(3));
    copied.helix(4, org.helix(4));
    copied.pivot(0, org.pivot(0));
    copied.pivot(1, org.pivot(1));
    copied.pivot(2, org.pivot(2));
    copied.error(0, org.error(0));
    copied.error(1, org.error(1));
    copied.error(2, org.error(2));
    copied.error(3, org.error(3));
    copied.error(4, org.error(4));
    copied.error(5, org.error(5));
    copied.error(6, org.error(6));
    copied.error(7, org.error(7));
    copied.error(8, org.error(8));
    copied.error(9, org.error(9));
    copied.error(10, org.error(10));
    copied.error(11, org.error(11));
    copied.error(12, org.error(12));
    copied.error(13, org.error(13));
    copied.error(14, org.error(14));
    copied.chiSq(org.chiSq());
    copied.ndf(org.ndf());
    copied.fiTerm(org.fiTerm());
    copied.nhits(org.nhits());
    copied.nster(org.nster());
    copied.nclus(org.nclus());
    copied.stat(org.stat());
    copied.mass(org.mass());
  }

  void
  copyRecCDC_trk_add_Table(const Reccdc_trk_add& org,
                           Reccdc_trk_add& copied)
  {
    copied.quality(org.quality());
    copied.kind(org.kind());
    copied.mother(org.mother());
    copied.daughter(org.daughter());
    copied.decision(org.decision());
    copied.likelihood(0, org.likelihood(0));
    copied.likelihood(1, org.likelihood(1));
    copied.likelihood(2, org.likelihood(2));
    copied.stat(org.stat());
    copied.rectrk(org.rectrk());
  }

  void
  copyRecCDC_MCtrk_Table(const Reccdc_mctrk& org,
                         Reccdc_mctrk& copied)
  {
    /*cout << org.wirFrac() << endl;
    cout << org.wirFracHep() << endl;
    cout << org.charge() << endl;
    cout << org.ptFrac() << endl;
    cout << org.pzFrac() << endl;
    cout << org.quality() << endl;
    cout << copied.wirFrac() << endl;
    cout << copied.wirFracHep() << endl;
    cout << copied.charge() << endl;
    cout << copied.ptFrac() << endl;
    cout << copied.pzFrac() << endl;
    cout << copied.quality() << endl;*/

    copied.hep(org.hep());
    copied.wirFrac(org.wirFrac());
    copied.wirFracHep(org.wirFracHep());
    copied.charge(org.charge());
    copied.ptFrac(org.ptFrac());
    copied.pzFrac(org.pzFrac());
    copied.quality(org.quality());
  }

  void
  copyRecCDC_MCtrk2hep_Table(const Reccdc_mctrk2hep& org,
                             Reccdc_mctrk2hep& copied)
  {
    copied.wir(org.wir());
    copied.clust(org.clust());
    copied.trk(org.trk());
    copied.hep(org.hep());
  }
#endif

  void
  TTrackManager::addSvd(const int mcFlag) const
  {
//cnv  TSvdAssociator svdA(-20000.,20000.);
//   svdA.fillClusters();

//   //BsShwDat(RECTRK);
//   //BsShwDat(RECCDC_TRK);
//   //BsShwDat(RECCDC_MCTRK);

//   Reccdc_trk_Manager& trkMgr =
//     Reccdc_trk_Manager::get_manager();
//   Reccdc_trk_add_Manager& trkMgr2 =
//     Reccdc_trk_add_Manager::get_manager();
//   Reccdc_svd_trk_Manager& svdMgr =
//     Reccdc_svd_trk_Manager::get_manager();

// #if !(TRASAN_REPLACE_TABLE)
//   Reccdc_wirhit_Manager& wirMgr =
//     Reccdc_wirhit_Manager::get_manager();
//   Reccdc_mctrk_Manager& mcMgr =
//     Reccdc_mctrk_Manager::get_manager();
//   Reccdc_mctrk2hep_Manager& mcMgr2 =
//     Reccdc_mctrk2hep_Manager::get_manager();
//   Datcdc_mcwirhit_Manager& mcMgr3 =
//     Datcdc_mcwirhit_Manager::get_manager();
// #endif

//   int nSize = trkMgr.count();
//   for(int i=0;i<nSize;++i){
//     // cout << "trk " << i << ": " << trkMgr[i].helix(0) << endl;
// #if 1
//     // Reconstruction Info --> SVD Recon.
//     if(trkMgr2[i].decision() != TrackPMCurlFinder &&
//        (trkMgr2[i].quality() & TrackQuality2D) != TrackQuality2D &&
//        trkMgr[i].helix(2) != 0. && fabs(1./trkMgr[i].helix(2))<0.2){
//       CLHEP::HepVector a(5);
//       a[0] = trkMgr[i].helix(0);
//       a[1] = trkMgr[i].helix(1);
//       a[2] = trkMgr[i].helix(2);
//       a[3] = trkMgr[i].helix(3);
//       a[4] = trkMgr[i].helix(4);
//       HepGeom::Point3D<double>  p(trkMgr[i].pivot(0),
//       trkMgr[i].pivot(1),
//       trkMgr[i].pivot(2));
//       THelix th(p,a);
//       th.pivot(HepGeom::Point3D<double> (0.,0.,0.)); // pivot = (0,0,0)
//       AList<TSvdHit> cand;
//       double tz,tt;
//       if(svdA.recTrk(th,tz,tt,0.5,50.0,cand,0.5)){
//  // cout << "SVD in " << i << endl;
// #if TRASAN_REPLACE_TABLE
//  Reccdc_svd_trk & newSvd  = svdMgr.add();
// #else
//  Reccdc_trk     & newTrk  = trkMgr.add();
//  Reccdc_trk_add & newTrk2 = trkMgr2.add();
//  Reccdc_svd_trk & newSvd  = svdMgr.add();
//  // copy all information
//  copyRecCDC_trk_Table(trkMgr[i],newTrk);
//  copyRecCDC_trk_add_Table(trkMgr2[i],newTrk2);
// #endif
//  if(mcFlag){
// #if TRASAN_REPLACE_TABLE
//    ;
// #else
//    Reccdc_mctrk & newMcTrk = mcMgr.add();
//    copyRecCDC_MCtrk_Table(mcMgr[i],mcMgr[mcMgr.count()-1]);
//    int nMCt2h = mcMgr2.count();
//    for(int j=0;j<nMCt2h;++j){
//      if(mcMgr2[j].trk() &&
//         mcMgr2[j].trk().get_ID() == trkMgr[i].get_ID()){
//        Reccdc_mctrk2hep & newMcTrk2Hep = mcMgr2.add();
//        copyRecCDC_MCtrk2hep_Table(mcMgr2[j],newMcTrk2Hep);
//        newMcTrk2Hep.trk(newTrk);
//      }
//    }
//    int nMCwire = mcMgr3.count();
//    for(int j=0;j<nMCwire;++j){
//      if(mcMgr3[j].trk().get_ID() == trkMgr[i].get_ID()){
//        mcMgr3[j].trk(newTrk);
//      }
//    }
// #endif
//  }
//  CLHEP::HepVector ta = th.a(); // pivot = (0,0,0)
//  ta[3] = tz;
//  ta[4] = tt;
//  th.a(ta);
//  th.pivot(p); // pivot, (0,0,0) --> p
// #if TRASAN_REPLACE_TABLE
//  trkMgr[i].helix(3, th.a()[3]);
//  trkMgr[i].helix(4, th.a()[4]);
// #else
//  newTrk.helix(3, th.a()[3]);
//  newTrk.helix(4, th.a()[4]);
// #endif

//  newSvd.THelix(0, ta[0]);
//  newSvd.THelix(1, ta[1]);
//  newSvd.THelix(2, ta[2]);
//  newSvd.THelix(3, ta[3]);
//  newSvd.THelix(4, ta[4]);
// #if TRASAN_REPLACE_TABLE
//  newSvd.cdc_trk(trkMgr2[i]);
// #else
//  newSvd.cdc_trk(newTrk2);
// #endif
//  newSvd.Status(0); // 0 is normal.
//  int indexSvd  = 0;
//  for(int j=0;j<cand.length();++j){
//    if(indexSvd >= 16)break;
//    if((cand[j])->rphi() && (cand[j])->z()){
//      newSvd.svd_cluster(indexSvd, *(cand[j]->rphi()));
//      ++indexSvd;
//      newSvd.svd_cluster(indexSvd, *(cand[j]->z()));
//      ++indexSvd;
//    }else{
//      cout << "[TTrackManager of Trasan] Why ? no associated SVDhit ?" << endl;
//    }
//  }
// #if TRASAN_REPLACE_TABLE
//  trkMgr2[i].quality(0); // set to 0 --> GOOD!
//  trkMgr2[i].decision((trkMgr2[i].decision() | TrackSVDAssociator));
// #else
//  newTrk2.quality(1); // temporary
//  newTrk2.decision((newTrk2.decision() | TrackSVDAssociator));
// #endif
// #if !(TRASAN_REPLACE_TABLE)
//  // CDC Wire information
//  for(int j=0;j<wirMgr.count();++j){
//    if(wirMgr[j].trk() &&
//       wirMgr[j].trk().get_ID() == trkMgr[i].get_ID()){
//      wirMgr[j].trk(newTrk);
//    }
//  }
// #endif
//       }else if(fabs(th.a()[3]) > 30.){
//  if(svdA.recTrk(th,tz,tt,0.5,-1.0,cand,0.5)){
//    // cout << "SVD in " << i << endl;
// #if TRASAN_REPLACE_TABLE
//    Reccdc_svd_trk & newSvd  = svdMgr.add();
// #else
//    Reccdc_trk     & newTrk  = trkMgr.add();
//    Reccdc_trk_add & newTrk2 = trkMgr2.add();
//    Reccdc_svd_trk & newSvd  = svdMgr.add();
//    // copy all information
//    copyRecCDC_trk_Table(trkMgr[i],newTrk);
//    copyRecCDC_trk_add_Table(trkMgr2[i],newTrk2);
// #endif
//    if(mcFlag){
// #if TRASAN_REPLACE_TABLE
//      ;
// #else
//      Reccdc_mctrk & newMcTrk = mcMgr.add();
//      copyRecCDC_MCtrk_Table(mcMgr[i],mcMgr[mcMgr.count()-1]);
//      int nMCt2h = mcMgr2.count();
//      for(int j=0;j<nMCt2h;++j){
//        if(mcMgr2[j].trk() &&
//     mcMgr2[j].trk().get_ID() == trkMgr[i].get_ID()){
//    Reccdc_mctrk2hep & newMcTrk2Hep = mcMgr2.add();
//    copyRecCDC_MCtrk2hep_Table(mcMgr2[j],newMcTrk2Hep);
//    newMcTrk2Hep.trk(newTrk);
//        }
//      }
//      int nMCwire = mcMgr3.count();
//      for(int j=0;j<nMCwire;++j){
//        if(mcMgr3[j].trk().get_ID() == trkMgr[i].get_ID()){
//    mcMgr3[j].trk(newTrk);
//        }
//      }
// #endif
//    }
//    CLHEP::HepVector ta = th.a(); // pivot = (0,0,0)
//    ta[3] = tz;
//    ta[4] = tt;
//    th.a(ta);
//    th.pivot(p); // pivot, (0,0,0) --> p
// #if TRASAN_REPLACE_TABLE
//    trkMgr[i].helix(3, th.a()[3]);
//    trkMgr[i].helix(4, th.a()[4]);
// #else
//    newTrk.helix(3, th.a()[3]);
//    newTrk.helix(4, th.a()[4]);
// #endif

//    newSvd.THelix(0, ta[0]);
//    newSvd.THelix(1, ta[1]);
//    newSvd.THelix(2, ta[2]);
//    newSvd.THelix(3, ta[3]);
//    newSvd.THelix(4, ta[4]);
// #if TRASAN_REPLACE_TABLE
//    newSvd.cdc_trk(trkMgr2[i]);
// #else
//    newSvd.cdc_trk(newTrk2);
// #endif
//    newSvd.Status(0); // 0 is normal.
//    int indexSvd  = 0;
//    for(int j=0;j<cand.length();++j){
//      if(indexSvd >= 16)break;
//      if((cand[j])->rphi() && (cand[j])->z()){
//        newSvd.svd_cluster(indexSvd, *(cand[j]->rphi()));
//        ++indexSvd;
//        newSvd.svd_cluster(indexSvd, *(cand[j]->z()));
//        ++indexSvd;
//      }else{
//        cout << "[TTrackManager of Trasan] Why ? no associated SVDhit ?" << endl;
//      }
//    }
// #if TRASAN_REPLACE_TABLE
//    trkMgr2[i].quality(0); // set to 0 --> GOOD!
//    trkMgr2[i].decision((trkMgr2[i].decision() | TrackSVDAssociator));
// #else
//    newTrk2.quality(1); // temporary
//    newTrk2.decision((newTrk2.decision() | TrackSVDAssociator));
// #endif
// #if !(TRASAN_REPLACE_TABLE)
//    // CDC Wire information
//    for(int j=0;j<wirMgr.count();++j){
//      if(wirMgr[j].trk() &&
//         wirMgr[j].trk().get_ID() == trkMgr[i].get_ID()){
//        wirMgr[j].trk(newTrk);
//      }
//    }
// #endif
//  }
//       }
//     }
//   }
// #endif
  }
// <-- jtanaka 000925

  bool
  TTrackManager::goodTrack(const TTrack& t, bool track2D)
  {

    //...Check number of hits...
    if (! checkNumberOfHits(t, track2D)) return false;

    //...Check helix parameter...
    if (TTrack::helixHasNan(t.helix())) return false;

    return true;
  }

  void
  TTrackManager::monitor(void) const
  {
//cnv     const unsigned nTracks = _tracks.length();
//     for (unsigned i = 0; i < nTracks; i++) {
//  const TTrack & t = * _tracks[i];
//  const unsigned finder = t.finder();

//  if ((finder & TrackFastFinder) || (finder & TrackSlowFinder))
//      _profiler[0]->monitor(t);
//  if (finder & TrackCurlFinder)
//      _profiler[1]->monitor(t);
//  if (finder & TrackPMCurlFinder)
//      _profiler[2]->monitor(t);
//  if (finder & TrackHoughFinder)
//      _profiler[3]->monitor(t);
//     }
  }

  void
  TTrackManager::defineHistograms(void)
  {
//cnv     _profiler[0] = new TProfiler();
//     _profiler[0]->name("conformal");
//     _profiler[0]->baseId(100);
//     if (_debugLevel > 10)
//  _profiler[0]->tuple(true);
//     _profiler[0]->initialize();

//     _profiler[1] = new TProfiler();
//     _profiler[1]->name("curl");
//     _profiler[1]->baseId(200);
//     if (_debugLevel > 10)
//  _profiler[1]->tuple(true);
//     _profiler[1]->initialize();

//     _profiler[2] = new TProfiler();
//     _profiler[2]->name("PM");
//     _profiler[2]->baseId(300);
//     if (_debugLevel > 10)
//  _profiler[2]->tuple(true);
//     _profiler[2]->initialize();

//     _profiler[3] = new TProfiler();
//     _profiler[3]->name("hough");
//     _profiler[3]->baseId(400);
//     if (_debugLevel > 10)
//  _profiler[3]->tuple(true);
//     _profiler[3]->initialize();
  }

  void
  TTrackManager::statistics(bool doMCAnalysis)
  {
    ++_s->_nEvents;

    //...All finders...
    _s->_nTracks[0] += _tracks.length();
    _s->_nTracksAll[0] += _tracksAll.length();
    _s->_nTracks2D[0] += _tracks2D.length();
    _s->_nTracksFinal[0] += _tracksFinal.length();

    //..._tracks...
    unsigned n = _tracks.length();
    for (unsigned i = 0; i < n; i++) {
      const unsigned finder = _tracks[i]->finder();
      for (unsigned j = 1; j < 8; j++)
        if (finder & (1 << j))
          ++_s->_nTracks[j];
    }

    //..._tracksAll...
    n = _tracksAll.length();
    for (unsigned i = 0; i < n; i++) {
      const unsigned finder = _tracksAll[i]->finder();
      for (unsigned j = 1; j < 8; j++)
        if (finder & (1 << j))
          ++_s->_nTracksAll[j];
    }

    //..._tracks2D...
    n = _tracks2D.length();
    for (unsigned i = 0; i < n; i++) {
      const unsigned finder = _tracks2D[i]->finder();
      for (unsigned j = 1; j < 8; j++)
        if (finder & (1 << j))
          ++_s->_nTracks2D[j];
    }

    //..._tracksFinal...
    n = _tracksFinal.length();
    for (unsigned i = 0; i < n; i++) {
      const unsigned finder = _tracksFinal[i]->finder();
      for (unsigned j = 1; j < 8; j++)
        if (finder & (1 << j))
          ++_s->_nTracksFinal[j];
    }

    if (doMCAnalysis) {
      n = _tracks.length();
      for (unsigned i = 0; i < n; i++) {
        const unsigned finder = _tracks[i]->finder();
        if (_tracks[i]->mc()) {
          const unsigned quality = _tracks[i]->mc()->quality();
          for (unsigned j = 0; j < 8; j++) {
            if (finder & (1 << j) || (j == 0)) {
              if (quality & (TTrackGood))
                ++_s->_nMCQuality[j][0];
              else if (quality & (TTrackGhost))
                ++_s->_nMCQuality[j][1];
              else if (quality & (TTrackBad))
                ++_s->_nMCQuality[j][2];
              else if (quality & (TTrackCharge))
                ++_s->_nMCQuality[j][3];
              else if (quality & (TTrackGarbage))
                ++_s->_nMCQuality[j][4];
            }
          }
        }
      }
    }
  }

  void
  TTrackManager::mergeTracks(int level, float threshold)
  {
#ifdef TRASAN_DEBUG
    const string stage = "merging";
    EnterStage(stage);
#endif

    AList<TTrack> toBeRemoved;
    const unsigned n = _tracksAll.length();
    if (n < 2) return;
    for (unsigned i = 0; i < n - 1; i++) {
      TTrack& t0 = * _tracksAll[i];
      if (toBeRemoved.hasMember(t0))
        continue;
      if (! t0.nCores())
        continue;
      for (unsigned j = i + 1; j < n; j++) {
        TTrack& t1 = * _tracksAll[j];
        if (toBeRemoved.hasMember(t1))
          continue;
        if (! t1.nCores())
          continue;

        //...Check charge...
        if (t0.charge() != t1.charge())
          continue;

        //...Calculate a kind of distance...
//      float d = TTrack::distance(t0, t1);
        float d = TTrack::distanceB(t0, t1);

#ifdef TRASAN_DEBUG_DETAIL
        cout << Tab() << "distance=" << d << ":" << t0.name() << " <-> "
             << t1.name() << endl;
#endif

        if (d > threshold) {
#ifdef TRASAN_DEBUG_DETAIL
          cout << Tab() << "They are not identical : no merge"
               << endl;
#endif
#ifdef TRASAN_WINDOW_GTK_HOUGH
          TWindowGTKConformal& w = Trasan::getTrasan()->w();
          w.endOfEvent();
          w.clear();
          w.stage("Track Manager : merging tracks");
          w.information("gray:all hits, green:candidates to be built");
          AList<TTrack> tl0;
          tl0.append((TTrack&) t0);
          tl0.append((TTrack&) t1);
          w.append(tl0, Gdk::Color("green"));
//    w.run();
#endif
          continue;
        }

        //...Merge tracks...
        TTrack* x0 = 0;
        TTrack* x1 = 0;
        const unsigned ns0 = TLink::nStereoHits(t0.cores());
        const unsigned ns1 = TLink::nStereoHits(t1.cores());
        const unsigned n0 = t0.nCores();
        const unsigned n1 = t1.nCores();
        if (ns0 && (ns1 == 0)) {
          x0 = & t0;
          x1 = & t1;
        } else if (ns1 && (ns0 == 0)) {
          x0 = & t1;
          x1 = & t0;
        } else if (n0 > n1) {
          x0 = & t0;
          x1 = & t1;
        } else if (n0 < n1) {
          x0 = & t1;
          x1 = & t0;
        } else if (n0 == n1) {
//    const float p0 = t0.chi2() / float(t0.ndf());
//    const float p1 = t1.chi2() / float(t1.ndf());
          const float p0 = t0.chi2();
          const float p1 = t1.chi2();
          if (p0 < p1) {
            x0 = & t0;
            x1 = & t1;
          } else {
            x0 = & t1;
            x1 = & t0;
          }
        } else {
          continue;
        }

#ifdef TRASAN_DEBUG
        x0->dump("track breif", Tab() + "x0  ");
        x1->dump("track breif", Tab() + "x1  ");
#endif
#ifdef TRASAN_WINDOW_GTK_HOUGH
        TTrack tmp0(* x0);
        TTrack tmp1(* x1);
#endif

        AList<TLink> links = x1->links();
        x0->append(links);
        x1->remove(links);
        x0->assign(CellHitTrackManager);
        x0->finder(TrackTrackManager);
        toBeRemoved.append(x1);
        //...Refine...
        //      cout <<"REFIT "<< level <<endl;
        if (level == 2) {
          x0->fit();
        } else if (level == 3) {
          AList<TLink> bad;
          x0->fit();
          x0->refine(bad, 30. * 100.);
          x0->fit();
        } else if (level == 4) {
          AList<TLink> bad;
          x0->fit();
          x0->refine(bad, 30. * 100.);
          x0->fit();
          x0->refine(bad, 30. * 10.);
          x0->fit();
        } else if (level == 5) {
          AList<TLink> bad;
          x0->fit();
          x0->refine(bad, 30. * 100.);
          x0->fit();
          x0->refine(bad, 30. * 10.);
          x0->fit();
          x0->refine(bad, 30);
          x0->fit();
        }

#ifdef TRASAN_DEBUG
        x0->dump("detail sort", Tab() + "x0  ");
#endif
#ifdef TRASAN_WINDOW_GTK_HOUGH
        TWindowGTKConformal& w = Trasan::getTrasan()->w();
        w.endOfEvent();
        w.clear();
        w.stage("Track Manager : merging tracks");
        w.information("gray:all hits, green:candidates to be built");
        AList<TTrack> tl0;
        tl0.append((TTrack&) tmp0);
        AList<TTrack> tl1;
        tl1.append(tmp1);
        w.append(tl0, Gdk::Color("green"));
        w.append(tl1, Gdk::Color("red"));
        w.run();
#endif

      }
    }

    for (unsigned i = 0; i < (unsigned) toBeRemoved.length(); i++) {
      _tracksAll.remove(* toBeRemoved[i]);
      _tracks.remove(* toBeRemoved[i]);
      _tracks2D.remove(* toBeRemoved[i]);
      _tracksFinal.remove(* toBeRemoved[i]);
      delete toBeRemoved[i];
    }

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

} // namespace Belle
