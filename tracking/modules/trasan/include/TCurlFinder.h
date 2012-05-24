//-----------------------------------------------------------------------------
// $Id: TCurlFinder.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TCurlFinder.h
// Section  : Tracking
// Owner    : J. Tanaka
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.37  2004/02/18 04:07:46  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.36  2003/12/25 12:04:43  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.35  2003/12/19 07:36:14  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
//-----------------------------------------------------------------------------

#ifndef TCURLFINDER_FLAG_
#define TCURLFINDER_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES

#include "CLHEP/Matrix/DiagMatrix.h"
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
#endif

#include "tracking/modules/trasan/TFinderBase.h"
#include "tracking/modules/trasan/TBuilderCurl.h"
#include "tracking/modules/trasan/TSelector.h"
#include "tracking/modules/trasan/TCurlFinderParameters.h"

namespace Belle2 {
  class TRGCDC;
  class TRGCDCWire;
  class TRGCDCWireHit;
}

namespace Belle {

  class TLink;
  class TTrack;
  class TCircle;
  class TSegmentCurl;

  class TCurlFinder : public TFinderBase {

  public:
    TCurlFinder(void);
    ~TCurlFinder(void);
    std::string name(void) const;
    std::string version(void) const;

    TCurlFinder(const unsigned min_segment,
                const unsigned min_salvage,
                const double bad_distance_for_salvage,
                const double good_distance_for_salvage,
                const unsigned min_sequence,
                const unsigned min_fullwire,
                const double range_for_axial_search,
                const double range_for_stereo_search,
                const unsigned superlayer_for_stereo_search,
                const double range_for_axial_last2d_search,
                const double range_for_stereo_last2d_search,
                const double trace2d_distance,
                const double trace2d_first_distance,
                const double trace3d_distance,
                const unsigned determine_one_track,
                const double selector_max_impact,
                const double selector_max_sigma,
                const double selector_strange_pz,
                const double selector_replace_dz,
                const unsigned stereo_2dfind,
                const unsigned merge_exe,
                const double merge_ratio,
                const double merge_z_diff,
                const double mask_distance,
                const double ratio_used_wire,
                const double range_for_stereo1,
                const double range_for_stereo2,
                const double range_for_stereo3,
                const double range_for_stereo4,
                const double range_for_stereo5,
                const double z_cut,
                const double z_diff_for_last_attend,
                const unsigned svd_reconstruction,
                const double min_svd_electrons,
                const unsigned on_correction,
                const unsigned output_2dtracks,
                const unsigned curl_version,
                int turnOffInnermost3Layers);

    /// main function
    int doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
             const CAList<Belle2::TRGCDCWireHit> & stereoHits,
             AList<TTrack> & tracks,
             AList<TTrack> & tracks2D);

    /// cleans all members of this class
    void clear(void);

    /// initializes internal caches.
    void init(void);

  private:
    static void set_smallcell(bool s) {
      ms_smallcell = s;
    }
    static void set_superb(bool s) {
      ms_superb = s;
    }
    friend class Belle2::TRGCDC;


  private:
    /// Utility Section
    double   distance(const double, const double) const;
    unsigned offset(const unsigned) const;
    unsigned layerId(const double&) const;
    unsigned maxLocalLayerId(const unsigned) const;
    int      nextSuperAxialLayerId(const unsigned, const int) const;
    int      nextSuperStereoLayerId(const unsigned, const int) const;
    void makeList(AList<TLink>&, const AList<TSegmentCurl>&, const AList<TLink>&);
    void makeList(AList<TLink>&, const AList<TLink>&, const AList<TLink>&);
    unsigned nAxialHits(const double&) const;

    /// Sub Main Section #1
    void makeWireHitsListsSegments(const CAList<Belle2::TRGCDCWireHit>&, const CAList<Belle2::TRGCDCWireHit>&);

    /// Sub Main Section #2
    int checkSortSegments(void);

    /// Sub Main Section #3
    void makeCurlTracks(AList<TTrack> &tracks,
                        AList<TTrack> &tracks2D);

    /// Utility of #1
    void linkNeighboringWires(AList<TLink>*, const unsigned, bool stereo = false);
    void linkNeighboringWiresSmallCell(AList<TLink>*, const unsigned, bool stereo = false);
    void setNeighboringWires(TLink*, const TLink*);
    void createSuperLayer(void);
    void createSegments(AList<TLink>&);
    void searchSegment(TLink*, AList<TLink>&, AList<TLink>&, TSegmentCurl*);
    TLink* findLink(const TLink*, const AList<TLink>&);

    /// Utility of #2
    void checkExceptionalSegmentsType01(void);
    void checkExceptionalSegmentsType02(void);
    void checkExceptionalSegmentsType03(void);

    /// Utility of #3
    /// 3D Track
    TTrack* make3DTrack(const TCircle*);
    TTrack* make3DTrack(const TCircle*, AList<TSegmentCurl>&);
    void findCloseHits(AList<TLink>&, TTrack&, AList<TLink>&);
    void salvage3DTrack(TTrack*, bool = true);
    TTrack* merge3DTrack(TTrack*, AList<TTrack>&);
    bool check3DTrack(TTrack*);
    int  trace3DTrack(TTrack*);
    void mask3DTrack(TTrack*, AList<TLink>&);
    void assignTracks(void);
    void checkRelation(AList<TTrack>&);
    void check2DTracks(void);

    /// 2D Track
    TCircle* make2DTrack(const AList<TLink>&, const AList<TSegmentCurl>&, const unsigned);
    void searchAxialCand(AList<TLink>&, const AList<TLink>&, const TCircle*,
                         const int, const unsigned, const double);
    void searchStereoCand(AList<TLink>&, const AList<TLink>&, const TCircle*,
                          const int, const unsigned, const double);
    unsigned searchHits(const TLink*, const TCircle*, const double) const;
    unsigned searchHits(AList<TLink>&, const AList<TLink>&, const TCircle*, const double) const;
    unsigned checkAppendHits(const AList<TLink>&, AList<TLink>&) const;
    double distance(const TTrack&, const TLink&) const;
    int  trace2DTrack(TCircle*);
    bool check2DCircle(TCircle*);
    TCircle* dividing2DTrack(TCircle*);
    bool fitWDD(TCircle&, double&, int&) const;
    void removeStereo(TCircle&) const;

    /// MC
#ifdef DEBUG_CURL_MC
    int makeWithMC(const AList<Belle2::TRGCDCWireHit>&, const AList<Belle2::TRGCDCWireHit>&, AList<TTrack>&);
#endif

    /// Plot
#if DEBUG_CURL_GNUPLOT+DEBUG_CURL_SEGMENT
    void makeCdcFrame(void);
#endif
#ifdef DEBUG_CURL_GNUPLOT
    void plotSegment(const AList<TLink>&, const int flag = 1);
    void plotCircle(const TCircle&, const int flag = 1);
    void plotTrack(const TTrack&, const int flag = 1);
#endif
#ifdef DEBUG_CURL_SEGMENT
    void writeSegment(const AList<TLink>&, const int type = 0);
#endif

    /// Dump
#if TRASAN_DEBUG_DETAIL
    void dumpType1(TTrack*);
    void dumpType2(TTrack*);
#endif

    AList<TLink>   m_unusedAxialHitsOriginal;
    AList<TLink>   m_unusedAxialHits;
    AList<TLink>   m_allAxialHitsOriginal;

    AList<TLink>   m_unusedStereoHitsOriginal;
    AList<TLink>   m_unusedStereoHits;
    AList<TLink>   m_allStereoHitsOriginal;

    AList<TLink>   m_removedHits;

    AList<TCircle> m_circles;
    AList<TCircle> m_allCircles;
    AList<TTrack>  m_tracks;
    AList<TTrack>  m_allTracks;
    AList<TTrack>  m_svdTracks;
    AList<TTrack>  m_2dTracks;

    CAList<Belle2::TRGCDCWireHit>   m_hitsOnInnerSuperLayer;

    AList<TLink> *m_unusedAxialHitsOnEachLayer;
    AList<TLink> *m_unusedStereoHitsOnEachLayer;
    AList<TLink> *m_unusedAxialHitsOnEachSuperLayer;
    AList<TLink> *m_unusedStereoHitsOnEachSuperLayer;

    AList<TSegmentCurl> m_segmentList;

    TCurlFinderParameter m_param;

    TBuilderCurl m_builder;
    TSelector m_trackSelector;

    THelixFitter m_fitter;

    //for debug
#if DEBUG_CURL_SEGMENT
    void debugCheckSegments0(void);
    void debugCheckSegments1(void);
    void debugCheckSegments2(void);
    void debugCheckSegments(const double localId, const double layerId,
                            const double localId2, const double layerId2);
#endif

    bool m_debugCdcFrame;
    int  m_debugPlotFlag;
    int  m_debugFileNumber;

    const int _turnOffInnermost3Layers;
    //
    // for quick dicision
    //
    static bool ms_smallcell;
    static bool ms_superb;

#ifdef TRASAN_WINDOW
  private:
    mutable TWindow _cWindow;
    void displayStatus(const std::string& m) const;
#endif
  };

  inline
  void
  TCurlFinder::init(void)
  {
    return;
  }

} // namespace Belle

#endif /* TCURLFINDER_FLAG_ */
