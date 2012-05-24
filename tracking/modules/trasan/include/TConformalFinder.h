//-----------------------------------------------------------------------------
// $Id: TConformalFinder.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TConformalFinder.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the conformal method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.59  2005/03/11 03:58:34  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.58  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.57  2003/12/19 07:36:14  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.56  2002/02/13 21:59:59  yiwasaki
// Trasan 3.03 : T0 reset test mode added
//
// Revision 1.55  2001/12/23 09:58:55  katayama
// removed Strings.h
//
// Revision 1.54  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.53  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.52  2001/06/18 00:16:40  yiwasaki
// Trasan 3.01 : perfect segment finder option, segv in term on linux fixed
//
// Revision 1.51  2001/04/25 02:36:08  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.50  2001/04/11 01:10:02  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.49  2001/02/07 22:25:06  yiwasaki
// Trasan 2.28 : conf minor change in shared memory
//
// Revision 1.48  2001/02/05 05:48:58  yiwasaki
// Trasan 2.26 : bug fix for data processing
//
// Revision 1.47  2000/04/14 05:20:48  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.46  2000/04/04 12:14:18  yiwasaki
// Trasan 2.00RC27 : bad point rejection in conf., association check for dE/dx
//
// Revision 1.45  2000/04/04 07:40:10  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.44  2000/03/24 10:23:02  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.43  2000/03/17 11:01:41  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.42  2000/02/23 08:45:08  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.41  2000/02/17 13:24:22  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.40  2000/02/15 13:46:49  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.39  2000/02/10 13:11:42  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.38  2000/01/30 08:17:15  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.37  2000/01/28 06:30:31  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.36  2000/01/23 08:23:10  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.35  2000/01/19 01:33:34  yiwasaki
// Trasan 1.67f : new conf modified
//
// Revision 1.34  1999/11/19 09:13:15  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.33  1999/10/30 10:12:45  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.32  1999/10/21 15:45:19  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.31  1999/09/21 02:01:38  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.29  1999/08/25 06:25:53  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.28  1999/08/04 01:01:43  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.27  1999/06/26 07:05:45  yiwasaki
// Trasan 1.47a release : hit and tracking efficiency improved
//
// Revision 1.26  1999/03/21 15:45:50  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.25  1999/02/03 06:23:18  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.24  1998/09/28 16:52:14  yiwasaki
// TBuilderCosmic added
//
// Revision 1.23  1998/08/31 05:16:02  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.22  1998/07/29 04:35:16  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.19  1998/07/02 09:04:45  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.18  1998/06/24 06:55:05  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.17  1998/06/21 18:38:26  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.16  1998/06/17 20:23:05  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.15  1998/06/17 20:12:42  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.14  1998/06/15 03:34:21  yiwasaki
// Trasan 1 beta 3.1, compiler error fixed
//
// Revision 1.13  1998/06/14 11:09:57  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.12  1998/06/11 12:23:52  yiwasaki
// TConformalLink removed
//
// Revision 1.11  1998/06/11 08:15:44  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.10  1998/06/03 17:16:54  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TCluster::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.9  1998/05/26 05:09:16  yiwasaki
// cvs repair
//
// Revision 1.8  1998/05/22 08:21:50  yiwasaki
// Trasan 1 alpha 4 release, TSegment added, TConformalLink no longer used
//
// Revision 1.7  1998/05/18 08:08:56  yiwasaki
// preparation for alpha 3
//
// Revision 1.6  1998/05/08 09:47:06  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.5  1998/04/23 17:25:00  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.4  1998/04/16 16:51:08  yiwasaki
// minor changes
//
// Revision 1.3  1998/04/14 01:05:53  yiwasaki
// TRGCDCWireHitMC added
//
// Revision 1.2  1998/04/10 09:38:19  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:51:14  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef TConformalFinder_FLAG_
#define TConformalFinder_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES

#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/TFinderBase.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TBuilderConformal.h"
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
#endif
#ifdef TRASAN_WINDOW_GTK
#include "tracking/modules/trasan/TWindowGTK.h"
#endif

namespace Belle2 {
  class TRGCDC;
}

namespace Belle {

  class TPoint2D;

/// A class to find tracks with the conformal method.
  class TConformalFinder : public TFinderBase {

  public:
    /// Constructor.
    TConformalFinder(unsigned fastFinder,
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
                     unsigned fittingFlag);

    /// Destructor
    virtual ~TConformalFinder();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns T0 reset is done.
    bool T0ResetDone(void) const;

  public:// Modifiers
    /// clear internal information.
    void clear(void);

    /// initializes internal caches.
    void init(void);

    /// finds tracks.
    int doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
             const CAList<Belle2::TRGCDCWireHit> & stereoHits,
             AList<TTrack> & tracks,
             AList<TTrack> & tracks2D);

    /// sets a flag to do T0 reset in the fast 2D finding.
    bool doT0Reset(bool);

  private:
    /// selects good hits.
    void selectGoodHits(void);

    /// finds segments from _goodHits.
    void findSegments(void);
    void findSegmentsPerfect(void);

    /// links segments.
    void linkSegments(unsigned level);

    /// fast findings. level:0/1:selection of segments to be used
    void fastFinding2D(unsigned level);
    void fastFinding3D(unsigned level);

    /// slow findings. level:0/1:selection of segments to be used
    void slowFinding2D(unsigned level);
    TTrack* expand(AList<TSegment> &) const;
    TTrack* expand(TTrack&) const;
    void targetSuperLayer(unsigned ptn, unsigned& in, unsigned& out) const;
    AList<TSegment> targetSegments(const TTrack&, unsigned sl) const;
    AList<TLink> targetLinks(const TTrack&, unsigned sl) const;
    bool trackQuality(const TTrack&) const;

    /// finds setereo segments.
    AList<TSegment> stereoSegments(const TTrack& t) const;
    AList<TSegment> stereoSegmentsFromBadHits(const TTrack& t) const;

    /// salvages segments.
    void salvage(TTrack& track,
                 unsigned axialStereoSwitch,
                 const AList<TSegment> & bads) const;

    /// removes bad segments.
    AList<TSegment> removeBadSegments(TTrack&) const;
    AList<TSegment> refineSegments(const TTrack&) const;
    void refineLinks(TTrack&, unsigned minNHits) const;

    /// resolves multi-assignment of segments/hits.
    void resolveSegments(AList<TTrack> & tracks) const;
    void resolveHits(AList<TTrack> & tracks) const;

    /// remove used segments.
    void removeUsedSegments(const AList<TTrack> & tracks);

    /// deletes a track.
    void deleteTrack(TTrack&) const;

    /// re-calculate TLinks.
    void updateTLinks(AList<TTrack> & tracks);

    /// Utility functions
    int crossPointsInConformal(const AList<TSegment> &,
                               HepGeom::Point3D<double> points[12]) const;
    AList<TSegment> pickUpSegments(const TPoint2D p[12],
                                   float loadWidth,
                                   unsigned axialStereo) const;
    AList<TSegment> pickUpSegmentsInConformal(float phi[12],
                                              float loadWidth,
                                              unsigned axialStereo) const;
    AList<TLink> pickUpLinks(const TPoint2D p[12],
                             float loadWidth,
                             unsigned axialStereo) const;
    AList<TLink> pickUpLinksInConformal(float phi[12],
                                        float loadWidth,
                                        unsigned axialStereo) const;
    AList<TLink> trackSide(const TTrack&, const AList<TLink> &) const;
    AList<TSegment> trackSide(const TTrack&, const AList<TSegment> &) const;
    bool quality2D(TTrack& t, unsigned level) const;
    TSegment* link(const TSegment& seed,
                   const HepGeom::Point3D<double> & p,
                   const Vector3D& v,
                   const AList<TSegment> & candidates,
                   AList<TSegment> & alternatives,
                   unsigned level) const;
    Vector3D direction(const TSegment&) const;

  public:// Utility functions
    static const Belle2::TRGCDCWire* conformal2Wire(const HepGeom::Point3D<double> & conformalPoint);

  private:// Parameters given by arguments
    const bool _fastFinder;
    const bool _slowFinder;
    const unsigned _perfectSegmentFinding;
    const bool _useSmallCells;
    const float _maxSigma2;
    const unsigned _minNLinksForSegment;
    const unsigned _minNCoreLinks;
    const unsigned _minNSegments;
    const unsigned _salvageLoadWidth;
    const unsigned _stereoMode;
    const unsigned _stereoLoadWidth;

  private:// other parameters
    bool _doT0Reset;
    bool _T0ResetDone;
    const unsigned _segmentSeparation;
    const unsigned _minNLinksForSegmentInRefine;
    const unsigned _maxNLinksForSegment;
    const unsigned _maxWidthForSegment;
    const float _minUsedFractionSlow2D;
    const unsigned _appendLoad;
    float _linkMaxDistance[3];
    float _linkMinDirAngle[3];
    const TBuilderConformal _builder;

    AList<TLink> _allHits[3];     // 0:axial, 1:stereo, 2:both
    AList<TLink> _hits[3];        // 0:axial, 1:stereo, 2:both
    AList<TLink> _unused[3];      // 0:axial, 1:stereo, 2:both

    AList<TSegment> _allSegments[2][6]; // 0:axial, 1:stereo
    AList<TSegment> _allUnused[2][6];   // 0:axial, 1:stereo
    AList<TSegment> _stereoBadSegments;

    AList<TTrack> _2DTracks;
    AList<TTrack> _3DTracks;

    struct summary {
      unsigned _nEvents;
      unsigned _nTracksFast3D;
      unsigned _nTracksSlow3D;
      unsigned _nTracksFast2D;
      unsigned _nTracksSlow2D;
      unsigned _nTracksFast2DBadQuality;
      unsigned _nTracksSlow2DBadQuality;
      unsigned _dummy;
    };
    struct summary* _s;

#ifdef TRASAN_WINDOW
  private:
    mutable TWindow _rphiWindow;

  public:
    void displayStatus(const std::string& message) const;
    void displayAppendSegments(const AList<TSegment> a[2][6],
                               leda_color = leda_black) const;
    void displayTracks(const AList<TTrack> &,
                       const AList<TSegment> seg[2][6],
                       const std::string& text) const;
#endif
#ifdef TRASAN_WINDOW_GTK
  private:
//    mutable TWindowGTK _cw;

  public:
    void displayStatus(const std::string& message) const;
//     void displayAppendSegments(const AList<TSegment> a[2][6],
//             leda_color =leda_black) const;
//     void displayTracks(const AList<TTrack> &,
//           const AList<TSegment> seg[2][6],
//           const std::string & text) const;
#endif
  };

//-----------------------------------------------------------------------------

#ifdef TConformalFinder_NO_INLINE
#define inline
#else
#undef inline
#define TConformalFinder_INLINE_DEFINE_HERE
#endif

#ifdef TConformalFinder_INLINE_DEFINE_HERE

  inline
  std::string
  TConformalFinder::name(void) const
  {
    return "Conformal Finder";
  }

  inline
  bool
  TConformalFinder::doT0Reset(bool a)
  {
    return _doT0Reset = a;
  }

  inline
  bool
  TConformalFinder::T0ResetDone(void) const
  {
    return _T0ResetDone;
  }

  inline
  void
  TConformalFinder::init(void)
  {
    return;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TConformalFinder_FLAG_ */
