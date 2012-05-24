//-----------------------------------------------------------------------------
// $Id: TBuilderConformal.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TBuilderConformal.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a track in Conformal finder.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.2  2005/03/11 03:58:34  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.1  2004/03/26 06:17:38  yiwasaki
// Trasan 3.10 : new files
//
//-----------------------------------------------------------------------------

#ifndef TBuilderConformal_FLAG_
#define TBuilderConformal_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include "tracking/modules/trasan/TBuilder.h"

namespace Belle {

/// A class to build a track.
  class TBuilderConformal : public TBuilder {

  public:
    /// Constructor with salvage level.
    TBuilderConformal(const std::string& name,
                      float maxSigma,
                      float maxSigmaStereo,
                      float salvageLevel,
                      float szSegmentDistance,
                      float szLinkDistance,
                      unsigned fittingFlag);

    /// Destructor
    virtual ~TBuilderConformal();

  public:// Track building
    /// builds a r/phi track from segments.
    TTrack* buildRphi(const AList<TSegment> &) const;
    TTrack* buildRphi(const AList<TLink> &) const;

    /// builds a 3D track from segments.
    TTrack* buildStereoNew(const TTrack& t,
                           AList<TSegment> & goodSegments,
                           AList<TSegment> & badSegments) const;

    /// salvages hits.
    void salvage(TTrack& t, AList<TSegment> & segments) const;

  public: // Obsolete functions
    TTrack* buildStereo(const TTrack& t, AList<TSegment> &) const;
    TTrack* buildStereo(TTrack& t,
                        TLine& l,
                        const AList<TLink> & links) const;

  private:
    /// initializes internal variables for stereo reconstruction. This should be called with new stereo codes.
    bool initializeForStereo(const TTrack&,
                             const AList<TSegment> &,
                             const AList<TSegment> &) const;
    AList<TLine> searchInitialLines(unsigned nSuperLayers) const;
    AList<TLine> searchLines5(void) const;
    AList<TLine> searchLines4(void) const;
    AList<TLine> searchLines3(void) const;
    AList<TLine> searchLines2(void) const;
    AList<TLine> searchLines1(void) const;
    TLine searchLine(const TLine& initialLine) const;

    /// checks stereo quality of a track.
    unsigned stereoQuality(const AList<TLink> & links) const;

    AList<TSegment> selectStereoSegment(const TLine& line,
                                        const AList<TSegment> & list,
                                        const AList<TLink> & szList) const;
    void removeFarSegment(const TLine&,
                          AList<TSegment> &,
                          AList<TLink> &) const;
    TLine* initialLine(const TTrack&, AList<TSegment> &) const;
    TLine* initialLineOld(const TTrack&, AList<TSegment> &) const;
    TLine* initialLine1(const TTrack&,
                        const AList<TSegment> &,
                        const AList<TLink> &) const;

  private:
    const float _szSegmentDistance;
    mutable unsigned _nSuperLayers;
    mutable AList<TLink> _allLinks;
    mutable AList<TLink> _links[5];
    mutable AList<TLink> _forLine;
    mutable unsigned _nHits[6];
  };

//-----------------------------------------------------------------------------

#ifdef TBuilderConformal_NO_INLINE
#define inline
#else
#undef inline
#define TBuilderConformal_INLINE_DEFINE_HERE
#endif

#ifdef TBuilderConformal_INLINE_DEFINE_HERE



#endif

#undef inline

} // namespace Belle

#endif /* TBuilderConformal_FLAG_ */
