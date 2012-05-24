//-----------------------------------------------------------------------------
// $Id: TConformalFinder0.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TConformalFinder0.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the conformal method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.10  2003/12/25 12:04:43  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.9  2002/01/03 11:04:58  katayama
// HepGeom::Point3D<double> and other header files are cleaned
//
// Revision 1.8  2001/12/23 09:58:55  katayama
// removed Strings.h
//
// Revision 1.7  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.6  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.5  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.4  2000/03/17 11:01:42  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.3  2000/01/28 06:30:31  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.2  1999/10/30 10:12:47  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1999/10/21 15:45:20  yiwasaki
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

#ifndef TConformalFinder0_FLAG_
#define TConformalFinder0_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#include "tracking/modules/trasan/TFinderBase.h"
#include "tracking/modules/trasan/TBuilder0.h"

namespace Belle2 {
  class TRGCDCWireHit;
}

namespace Belle {


  class TLink;
  class THistogram;
  class TCircle;
  class TSegment0;

/// A class to find tracks with the conformal method.
  class TConformalFinder0 : public TFinderBase {

  public:
    /// Constructor.
    TConformalFinder0(float maxSigma,
                      float fraction,
                      float stereoZ3,
                      float stereoZ4,
                      float stereoChisq3,
                      float stereoChisq4,
                      float stereoMaxSigma,
                      unsigned fittingCorrections,
                      float salvageLevel,
                      bool cosmic);

    /// Destructor
    virtual ~TConformalFinder0();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a list of conformal links.
    const AList<TLink> & axialConformalLinks(void) const;

    /// returns a list of conformal links.
    const AList<TLink> & stereoConformalLinks(void) const;

  public:// Modifiers
    /// clear internal information.
    void clear(void);

    /// initializes internal caches.
    void init(void);

    /// finds tracks.
    int doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
             const CAList<Belle2::TRGCDCWireHit> & stereoHits,
             AList<TTrack> & tracks,
             AList<TTrack> & tracks3D);

    /// transforms hits into a conformal plane. 'center' is a center of the transformation. Z position of 'center' must be zero. Transformed positions are newly created, and are stored in 'links'.
    static void conformalTransformation(const HepGeom::Point3D<double>  & center,
                                        const CAList<Belle2::TRGCDCWireHit> & hits,
                                        AList<TLink> & links);

    /// transforms hits into a conformal plane. 'center' is a center of the transformation. Z position of 'center' must be zero. Transformed positions are newly created, and are stored in 'links'.
    static void conformalTransformationRphi(const HepGeom::Point3D<double>  & center,
                                            const CAList<Belle2::TRGCDCWireHit> & hits,
                                            AList<TLink> & links);

    /// finds segments.
    AList< AList<TSegment0> > findSegments(const AList<TLink> & in) const;
    AList< AList<TSegment0> > findSegments2(const AList<TLink> & in) const;

    /// finds segments. (obsolete functions)
    AList<TSegment0> findClusters(const THistogram&) const;
    AList<TSegment0> findClusters2(const THistogram&) const;


    /// sets swtich for stereo reconstruction.
    bool doStereo(bool);

    /// sets switch for salvaging.
    bool doSalvage(bool);

  private:
    /// returns a list of links close to a track.
    AList<TLink> findCloseHits(const AList<TLink> & in,
                               const TTrack& track) const;

    /// returns a pointer to the best cluster to be linked.
    TSegment0* findBestLink(const TSegment0&,
                            const AList<TSegment0> & in) const;

    /// appends the best cluster in 'list' to 'track'.
    TSegment0* appendCluster(TTrack& track, AList<TSegment0> & in) const;

    /// returns a list of clusters to be a track.
    AList<TSegment0> findClusterLink(TSegment0&,
                                     const AList<TSegment0> * const) const;

    /// makes a track.
    TTrack* makeTrack(const AList<TSegment0> &) const;

    /// appends TLinks in a list.
    void appendClusters2(TTrack& track, AList<TSegment0> &) const;

    /// returns a list of clusters close to a cluster.
    AList<TSegment0> findCloseClusters(const TTrack&,
                                       const AList<TSegment0> &,
                                       double maxDistance) const;

    /// main loop with salvaging.
    void standardFinding(AList<TLink> & seeds,
                         AList<TLink> & unusedLinks,
                         double fraction);

    /// main loop with salvaging for second trial.
    void specialFinding(AList<TLink> & seeds,
                        AList<TLink> & unusedLinks,
                        double fraction);

  private:
    /// Track builder.
    TBuilder0* _builder;

    /// Parameters.
    TSelector0 _circleSelector;
    TSelector0 _trackSelector;
    float _fraction;

    /// List of hit positions in the conformal plane.
    AList<TLink> _axialConfLinks;
    AList<TLink> _stereoConfLinks;
    AList<TLink> _unusedAxialConfLinks;
    AList<TLink> _unusedStereoConfLinks;
    AList<TLink> _goodAxialConfLinks;

    /// Internal lists.
    AList<TCircle> _circles;
    AList<TTrack> _tracks;

    /// Switches
    bool _doStereo;
    bool _doSalvage;
  };

//-----------------------------------------------------------------------------

#ifdef TConformalFinder0_NO_INLINE
#define inline
#else
#undef inline
#define TConformalFinder0_INLINE_DEFINE_HERE
#endif

#ifdef TConformalFinder0_INLINE_DEFINE_HERE

  inline
  const AList<TLink> &
  TConformalFinder0::axialConformalLinks(void) const
  {
    return _axialConfLinks;
  }

  inline
  const AList<TLink> &
  TConformalFinder0::stereoConformalLinks(void) const
  {
    return _stereoConfLinks;
  }

  inline
  bool
  TConformalFinder0::doStereo(bool a)
  {
    return _doStereo = a;
  }

  inline
  bool
  TConformalFinder0::doSalvage(bool a)
  {
    return _doSalvage = a;
  }

  inline
  std::string
  TConformalFinder0::name(void) const
  {
    return "Conformal Finder 0";
  }

  inline
  void
  TConformalFinder0::init(void)
  {
    return;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TConformalFinder0_FLAG_ */
