//-----------------------------------------------------------------------------
// $Id: Trasan.h 10700 2008-11-06 08:47:27Z hitoshi $
//-----------------------------------------------------------------------------
// Filename : Trasan.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A tracking module.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.77  2005/11/03 23:20:36  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.76  2005/03/11 03:58:35  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.75  2004/02/18 04:07:47  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.74  2003/12/19 07:36:15  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.73  2003/10/05 22:39:40  yiwasaki
// Trasan 3.05 : PMCurlFinder modification again : PMCurlFinder always off in exp>=30, default on in exp<30
//
// Revision 1.72  2003/09/10 01:18:42  yiwasaki
// Trasan for small cell CDC
//
// Revision 1.71  2002/02/13 21:59:59  yiwasaki
// Trasan 3.03 : T0 reset test mode added
//
// Revision 1.70  2001/12/23 09:58:58  katayama
// removed Strings.h
//
// Revision 1.69  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.68  2001/12/14 02:54:52  katayama
// For gcc-3.0
//
// Revision 1.67  2001/06/18 00:16:40  yiwasaki
// Trasan 3.01 : perfect segment finder option, segv in term on linux fixed
//
// Revision 1.66  2001/04/25 02:36:08  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.65  2001/04/12 07:11:02  yiwasaki
// Trasan 3.00 RC4 : new stereo code for curl
//
// Revision 1.64  2001/04/11 01:10:05  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.63  2001/02/05 05:48:58  yiwasaki
// Trasan 2.26 : bug fix for data processing
//
// Revision 1.62  2001/01/31 11:38:30  yiwasaki
// Trasan 2.24 : curl finder fix
//
// Revision 1.61  2001/01/30 22:40:15  yiwasaki
// Trasan 2.23 : curl fix
//
// Revision 1.60  2001/01/30 20:51:52  yiwasaki
// Trasan 2.22 : bug fixes
//
// Revision 1.59  2001/01/11 03:40:52  yiwasaki
// minor changes
//
// Revision 1.58  2000/11/17 06:41:38  yiwasaki
// Trasan 2.18 : option for new CDC geometry
//
// Revision 1.57  2000/10/23 08:22:30  yiwasaki
// Trasan 2.17 : curl can output 2D tracks. Default parameters changed
//
// Revision 1.56  2000/10/05 23:54:32  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.55  2000/09/27 07:45:23  yiwasaki
// Trasan 2.05 : updates of curl and pm finders by JT
//
// Revision 1.54  2000/08/31 23:51:51  yiwasaki
// Trasan 2.04 : pefect finder added
//
// Revision 1.53  2000/04/04 12:14:19  yiwasaki
// Trasan 2.00RC27 : bad point rejection in conf., association check for dE/dx
//
// Revision 1.52  2000/03/28 22:57:16  yiwasaki
// Trasan 2.00RC22 : PM finder from J.Tanaka
//
// Revision 1.51  2000/03/24 10:23:04  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.50  2000/03/23 13:27:57  yiwasaki
// Trasan 2.00RC18 : bug fixes
//
// Revision 1.49  2000/03/21 07:01:34  yiwasaki
// tmp updates
//
// Revision 1.48  2000/03/18 13:40:06  katayama
// begin run function etc are temporarily required
//
// Revision 1.47  2000/03/17 11:01:43  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.46  2000/03/17 07:00:53  katayama
// Module function modified
//
// Revision 1.45  2000/02/28 01:59:03  yiwasaki
// Trasan 2.00RC11 : curl updates only
//
// Revision 1.44  2000/02/23 08:45:09  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.43  2000/02/15 13:46:53  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.42  2000/02/10 13:11:43  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.41  1999/10/30 10:12:56  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.40  1999/10/21 15:45:21  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.39  1999/10/15 04:28:09  yiwasaki
// TWindow is hidden, curl finder parameter
//
// Revision 1.38  1999/09/10 09:20:01  yiwasaki
// Trasan 1.61b release : new parameters added, TTrackMC bug fixed
//
// Revision 1.37  1999/08/25 06:25:55  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.36  1999/08/04 01:01:44  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.35  1999/07/09 01:47:25  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.34  1999/06/26 07:05:45  yiwasaki
// Trasan 1.47a release : hit and tracking efficiency improved
//
// Revision 1.33  1999/06/17 09:45:03  yiwasaki
// Trasan 1.45 release : T0 determination by 2D fitting
//
// Revision 1.32  1999/06/09 15:09:58  yiwasaki
// Trasan 1.38 release : changes for lp
//
// Revision 1.31  1999/05/28 07:11:23  yiwasaki
// Trasan 1.35 alpha release : cathdoe test version
//
// Revision 1.30  1999/05/26 05:03:53  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.29  1999/05/18 04:44:34  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.28  1999/03/21 15:45:52  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.27  1999/02/09 06:24:03  yiwasaki
// Trasan 1.17 release : cathode codes updated by T.Matsumoto, FPE error fixed by J.Tanaka
//
// Revision 1.26  1998/12/04 15:11:08  yiwasaki
// TRGCDC creation timing changed, zero-division protection for TTrackMC
//
// Revision 1.25  1998/11/27 08:15:53  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.24  1998/11/12 12:27:44  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.23  1998/11/11 11:04:17  yiwasaki
// protection again
//
// Revision 1.22  1998/09/28 16:52:17  yiwasaki
// TBuilderCosmic added
//
// Revision 1.21  1998/09/28 14:54:23  yiwasaki
// MC tables, TUpdater, oichan added
//
// Revision 1.20  1998/09/25 02:14:47  yiwasaki
// modification for cosmic
//
// Revision 1.19  1998/09/24 22:56:52  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.18  1998/09/17 16:05:30  yiwasaki
// Trasan 1.1 alpha 1 release : TTrackManager added to manage reconstructed tracks, TTrack::P() added, TTrack::_charge no longer constant
//
// Revision 1.17  1998/08/31 05:16:10  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.16  1998/08/03 15:01:19  yiwasaki
// Trasan 1.07 release : cluster finder from S.Suzuki-san added
//
// Revision 1.15  1998/07/29 04:35:30  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.12  1998/07/06 15:48:59  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.11  1998/06/21 18:38:28  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.10  1998/06/11 08:15:49  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.9  1998/06/08 14:39:22  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.8  1998/05/08 09:47:08  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.7  1998/04/23 17:25:09  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.6  1998/04/16 16:51:11  yiwasaki
// minor changes
//
// Revision 1.5  1998/04/14 01:05:54  yiwasaki
// TRGCDCWireHitMC added
//
// Revision 1.4  1998/04/10 09:38:21  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.3  1998/04/10 00:51:16  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef Trasan_FLAG_
#define Trasan_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#define HEP_SHORT_NAMES

#include <string>
//#include "tracking/modules/trasan/TrasanModule.h"
#include "tracking/modules/trasan/AList.h"
#include "framework/core/Module.h"
#include "tracking/modules/trasan/TUpdater.h"
#include "tracking/modules/trasan/TTrackManager.h"
#ifdef TRASAN_WINDOW_GTK
#undef ERROR
#include "tracking/modules/trasan/TWindowGTKConformal.h"
#include "tracking/modules/trasan/TWindowGTKHough.h"
#include "tracking/modules/trasan/TWindowGTKSZ.h"
#endif

namespace Belle2 {
  class TrasanModule;
  class TRGCDC;
  class TRGCDCWireHit;
}

namespace Belle {

  class TRGCDCCat;
  class TRGCDCClustFinder;
  class TFinderBase;
  class TConformalFinder0;
  class TCurlFinder;
  class TTrack;
  class TTrackMC;
  class TPMCurlFinder;

  extern float TrasanTHelixFitterChisqMax;
  extern int   TrasanTHelixFitterNtrialMax;
  extern const HepGeom::Point3D<double> ORIGIN;

/// A tracking module.
  class Trasan : public Belle2::Module, TUpdater {

  public:
    /// returns Trasan.
    static Trasan* getTrasan(void);

    virtual Belle2::ModulePtr newModule() {
      Belle2::ModulePtr nm(new Trasan()); return nm;
    };

//cnv  private:
  public:
    /// Constructor
    Trasan();

    /// Destructor
    ~Trasan();

  public:// BASF interfaces
    /// initializes Trasan.
    void initialize();

    /// terminates Trasan.
    void terminate();

    /// processes an event.
    void event();

    /// temporarily required
    void beginRun();
    void endRun();
    void disp_stat(const char*);

  public:// Trasan interfaces
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a pointer to the rphi finder.
    const TFinderBase* confFinder(void) const;

    /// returns a pointer to the curl finder.
    const TFinderBase* curlFinder(void) const;

    /// returns a pointer to the cluster finder.
    const TRGCDCClustFinder* clustFinder(void) const;

    /// returns a pointer to TTrackManager.
    const TTrackManager& trackManager(void) const;

    /// returns a pointer to the pm curl finder.
    const TFinderBase* pmCurlFinder(void) const;

  public: // to access information
    /// clears all TRGCDC information.
    void clear(bool termination = false);

    /// clears TRGCDC information.
    void fastClear(void);

    /// returns a list of reconstructed tracks.
    const AList<TTrack> & tracks(void) const;

  private:
    /// returns true if an event is MC.
    bool mcEvent(void) const;

    /// creates MC info. of reconstructed tracks.
    void mcInformation(void);

    /// returns unused hits.
    void selectUnusedHits(const CAList<Belle2::TRGCDCWireHit> & hits,
                          CAList<Belle2::TRGCDCWireHit> & unusedHits) const;

    /// Cathode
    void cathode(float);

    /// standard main loop.
    void main0(const CAList<Belle2::TRGCDCWireHit> & axialHits,
               const CAList<Belle2::TRGCDCWireHit> & stereoHits,
               const CAList<Belle2::TRGCDCWireHit> & allHits,
               AList<TTrack> & tracks,
               AList<TTrack> & tracks2D);

    /// Hough + Conf + Curl (Conf doesn't use used hits)
    void main1(const CAList<Belle2::TRGCDCWireHit> & axialHits,
               const CAList<Belle2::TRGCDCWireHit> & stereoHits,
               const CAList<Belle2::TRGCDCWireHit> & allHits,
               AList<TTrack> & tracks,
               AList<TTrack> & tracks2D);

  public:// public members for basf interface

    //...CDC...
    float b_cdcVersion;           // 0:automatic, 1:normal cell, 2:small cell
    std::string _cdcVersion;
    float b_fudgeFactor;

    //...Trasan...
    int b_debugLevel;
    int b_useAllHits;
    int b_doT0Reset;
    int b_nT0ResetMax;
    int b_doMCAnalysis;
    int b_mode;
    float b_helixFitterChisqMax;
    int   b_helixFitterNtrialMax;

    //...Perfect Finder...
    int b_doPerfectFinder;
    int b_perfectFitting;

    //...Conformal Finder...
    int b_conformalFinder;
    int b_doConformalFinder;
    int b_doConformalFastFinder;
    int b_doConformalSlowFinder;
    int b_conformalPerfectSegmentFinding;
    int b_conformalUseSmallCells;
    int b_conformalFittingFlag;
    float b_conformalMaxSigma;
    int b_conformalMinNLinksForSegment;
    int b_conformalMinNCores;
    int b_conformalMinNSegments;
    float b_salvageLevel;
    int b_conformalSalvageLoadWidth;
    int b_conformalStereoMode;
    int b_conformalStereoLoadWidth;
    float b_conformalStereoMaxSigma;
    float b_conformalStereoSzSegmentDistance;
    float b_conformalStereoSzLinkDistance;

    //...Old Conformal Finder...
    int b_doConformalFinderStereo;
    int b_doConformalFinderCosmic;
    float b_conformalFraction;
    float b_conformalStereoZ3;
    float b_conformalStereoZ4;
    float b_conformalStereoChisq3;
    float b_conformalStereoChisq4;
    int b_conformalFittingCorrections;

    //...Track Manager...
    double b_momentumCut;
    double b_ptCut;
    double b_tanlCut;
    int b_fittingFlag;
    int b_doSalvage;
    int b_mergeTracks;
    float b_mergeTrackDistance;
    int b_mergeCurls;
    int b_doT0Determination;
    int b_nTracksForT0;
    int b_sortMode;
    int b_doAssociation;
    float b_associateSigma;
    int b_test;

    //...Curl Finder...
    int    b_doCurlFinder;
    int    min_segment;
    int    min_salvage;
    double bad_distance_for_salvage;
    double good_distance_for_salvage;
    int    min_sequence;
    int    min_fullwire;
    double range_for_axial_search;
    double range_for_stereo_search;
    int    superlayer_for_stereo_search;
    double range_for_axial_last2d_search;
    double range_for_stereo_last2d_search;
    double trace2d_distance;
    double trace2d_first_distance;
    double trace3d_distance;
    int    determine_one_track;
    double selector_max_impact;
    double selector_max_sigma;
    double selector_strange_pz;
    double selector_replace_dz;
    int    stereo_2dfind;
    int    merge_exe;
    double merge_ratio;
    double merge_z_diff;
    double mask_distance;
    double ratio_used_wire;
    double range_for_stereo1;
    double range_for_stereo2;
    double range_for_stereo3;
    double range_for_stereo4;
    double range_for_stereo5;
    double z_cut;
    double z_diff_for_last_attend;
    int    svd_reconstruction;
    double min_svd_electrons;
    int    on_correction;
    int    output_2dtracks;
    int    curl_version;

    //...PM(Pattrn Matching) Curl Finder...
    int b_pmCurlFinder;
    int b_doPMCurlFinder;
    int _doPMCurlFinder;
    double min_svd_electrons_in_pmc;

    //...SVD Associator for Low Pt Tracks...
    int b_doSvdAssociator;

    //...Clust finder...
    int b_doClustFinder;
    float b_cathodeWindow;
    int b_cathodeSystematics;
    int b_cathodeCosmic;

    //...Hough Finder...
    int b_doHoughFinder;
    int b_doHoughFinderCurlSearch;
    float b_houghAxialLoadWidth;
    float b_houghAxialLoadWidthCurl;
    float b_houghMaxSigma;
    float b_houghStereoMaxSigma;
    float b_houghSalvageLevel;
    int b_houghMeshX;
    int b_houghMeshY;
    float b_houghPtBoundary;
    float b_houghThreshold;
    int b_houghMeshXLowPt;
    int b_houghMeshYLowPt;
    float b_houghPtBoundaryLowPt;
    float b_houghThresholdLowPt;
    int b_houghMode;

    //...Special flag for MC study of effects of inner layers...
    int b_simulateSmallCell;

    //...THelix error...
    double b_amin0;
    double b_amin1;
    double b_amin2;
    double b_amin3;
    double b_amin4;
    double b_amax0;
    double b_amax1;
    double b_amax2;
    double b_amax3;
    double b_amax4;

  private:
    /// shows the banner.
    void banner(void) const;

  private:
    static Trasan* _trasan;
    Belle2::TRGCDC* _cdc;
    TRGCDCCat* _cdccat;
    TFinderBase* _perfectFinder;
    TFinderBase* _confFinder;
    TCurlFinder* _curlFinder;
//cnv    TRGCDCClustFinder * _clustFinder;
    TTrackManager _trackManager;

//cnv    TPMCurlFinder * _pmCurlFinder;
    TFinderBase* _houghFinder;

    unsigned _nEvents;
    AList<TTrackMC> _mcTracks;

    /// GFTrackCandidates name.
    std::string _gfTrackCandsName;

#ifdef TRASAN_WINDOW_GTK
    //...Trasan window...
  public:
    TWindowGTKConformal& w(void);
    TWindowGTKHough& hp(void);
    TWindowGTKHough& hm(void);
    TWindowGTKHough& hc(void);
    TWindowGTKHough& hl(void);
    TWindowGTKSZ& sz(void);

  private:
    TWindowGTKConformal* _w;
    TWindowGTKHough* _hp;
    TWindowGTKHough* _hm;
    TWindowGTKHough* _hc;
    TWindowGTKHough* _hl;
    TWindowGTKSZ* _sz;
#endif

    friend class Belle2::TrasanModule;
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define Trasan_INLINE_DEFINE_HERE
#endif

#ifdef Trasan_INLINE_DEFINE_HERE

  inline
  const TFinderBase*
  Trasan::confFinder(void) const
  {
    return (TFinderBase*) _confFinder;
  }

  inline
  const TFinderBase*
  Trasan::curlFinder(void) const
  {
    return (TFinderBase*) _curlFinder;
  }

  inline
  const AList<TTrack> &
  Trasan::tracks(void) const
  {
    return _trackManager.tracksFinal();
  }

  /* inline */
  /* const TRGCDCClustFinder * */
  /* Trasan::clustFinder(void) const { */
  /*     return _clustFinder; */
  /* } */

  inline
  std::string
  Trasan::name(void) const
  {
    return std::string("Trasan");
  }

  inline
  const TTrackManager&
  Trasan::trackManager(void) const
  {
    return _trackManager;
  }

  /* inline */
  /* const TFinderBase * */
  /* Trasan::pmCurlFinder(void) const { */
  /*     return (TFinderBase *) _pmCurlFinder; */
  /* } */

#ifdef TRASAN_WINDOW_GTK

  inline
  TWindowGTKConformal&
  Trasan::w(void)
  {
    return * _w;
  }

  inline
  TWindowGTKHough&
  Trasan::hp(void)
  {
    return * _hp;
  }

  inline
  TWindowGTKHough&
  Trasan::hm(void)
  {
    return * _hm;
  }

  inline
  TWindowGTKHough&
  Trasan::hc(void)
  {
    return * _hc;
  }

  inline
  TWindowGTKHough&
  Trasan::hl(void)
  {
    return * _hl;
  }

  inline
  TWindowGTKSZ&
  Trasan::sz(void)
  {
    return * _sz;
  }

#endif

#endif

#undef inline

} // namespace Belle

#endif /* Trasan_FLAG_ */
