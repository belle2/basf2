//-----------------------------------------------------------------------------
// $Id: THoughFinder.h 10640 2008-09-12 05:11:38Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : THoughFinder.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the Hough method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.6  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.5  2005/03/11 03:58:34  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.4  2004/04/15 05:34:26  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.3  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.2  2004/02/18 04:07:46  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.1  2003/12/19 07:36:15  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
//-----------------------------------------------------------------------------

#ifndef THoughFinder_FLAG_
#define THoughFinder_FLAG_

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
#include "tracking/modules/trasan/TBuilder.h"
#include "tracking/modules/trasan/TPeakFinder.h"
#include "tracking/modules/trasan/THoughTransformationCircle.h"
#include "tracking/modules/trasan/THoughPlaneMulti.h"
#include "tracking/modules/trasan/THoughPlaneMulti2.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/TUtilities.h"

namespace Belle2 {
  class TRGCDC;
}

namespace Belle {

  class THoughPlane;
  class THoughPlaneMulti;
  class THoughTransformation;
  class THoughTransformationCircleGeneral;

  int SortByY(const TPoint2D**, const TPoint2D**);
  int SortByYReverse(const TPoint2D**, const TPoint2D**);

#ifdef TRASAN_WINDOW_GTK
  extern AList<TLink> AXIAL_ALL;
  extern AList<TLink> STEREO_ALL;
#endif

/// A class to find tracks with the Hough method.
  class THoughFinder : public TFinderBase {

  public:
    /// Constructor.
    THoughFinder(int doCurlSearch,
                 float axialLoadWidth,      // in cell width
                 float axialLoadWidthCurl,  // in cell width
                 float maxSigma,            // in sigma^2
                 float maxSigmaStereo,      // in sigma^2
                 float salvageLevel,        // in sigma^2
                 float szLinkDistance,
                 unsigned fittingFlag,
                 float stereoLoadWidth,     // in cell width
                 float salvageLoadWidth,    // in cell width
                 int meshX,                 // mesh division in X
                 int meshY,                 // mesh division in Y
                 float ptBoundary,          // mesh Pt boundary in MeV
                 float threshold,           // peak threshold (0 ~ 1)
                 int meshXLowPt,
                 int meshYLowPt,
                 float ptBoundaryLowPt,
                 float thresholdLowPt,
                 int mode);                 // 0:default, !=0:test

    /// Destructor
    virtual ~THoughFinder();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

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
    int doit0(const CAList<Belle2::TRGCDCWireHit> & axialHits,
              const CAList<Belle2::TRGCDCWireHit> & stereoHits,
              AList<TTrack> & tracks,
              AList<TTrack> & tracks2D);
    int doit1(const CAList<Belle2::TRGCDCWireHit> & axialHits,
              const CAList<Belle2::TRGCDCWireHit> & stereoHits,
              AList<TTrack> & tracks,
              AList<TTrack> & tracks2D);
    int doit2(const CAList<Belle2::TRGCDCWireHit> & axialHits,
              const CAList<Belle2::TRGCDCWireHit> & stereoHits,
              AList<TTrack> & tracks,
              AList<TTrack> & tracks2D);
    /// Prepared to reduce duplicated tracks.
    int doit3(const CAList<Belle2::TRGCDCWireHit> & axialHits,
              const CAList<Belle2::TRGCDCWireHit> & stereoHits,
              AList<TTrack> & tracks,
              AList<TTrack> & tracks2D);

    int curlSearch(AList<TTrack> & tracks,
                   AList<TTrack> & tracks2D);

  private:// Modifiers
    void selectGoodHits(const CAList<Belle2::TRGCDCWireHit> & axial,
                        const CAList<Belle2::TRGCDCWireHit> & stereo);

    /// transforms wire hits.
    void houghTransformation(const AList<TLink> & hits,
                             const THoughTransformation& trans,
                             THoughPlane& plane) const;
    void houghTransformation(const AList<TLink> & hits,
                             const THoughTransformation& trans,
                             float charge,
                             THoughPlane& plane,
//           unsigned weight = 1,
                             int weight = 1,
                             const TPoint2D& localOrigin = Origin) const;
    void houghTransformation(const AList<TLink> & hits,
                             const THoughTransformation& trans,
                             float charge,
                             THoughPlaneMulti& plane,
                             int weight = 1,
                             const TPoint2D& localOrigin = Origin) const;
    void houghTransformation2(const AList<TLink> & hits,
                              THoughPlaneMulti2& plane,
                              int weight = 1) const;

    /// builds a track.
    TTrack* build0(const TPoint2D& point,
                   THoughPlane* planes[2],
                   float charge,
                   unsigned pt,
                   unsigned threshold);
    TTrack* build1(const TPoint2D& point,
                   THoughPlane* planes[2][2],
                   float charge,
                   unsigned pt,
                   unsigned threshold);
    TTrack* buildCurl(const TPoint2D& point,
                      const THoughPlane& planes,
                      float charge,
                      unsigned threshold,
                      AList<TLink> & hits);
    TTrack* build3(const TPoint2D& point,
                   THoughPlane* planes[2],
                   float charge,
                   unsigned pt,
                   unsigned threshold);

    /// builds a 2D track.
    TTrack* build2D(const TCircle& circle, const AList<TLink> & links) const;

    /// builds a 3D track.
    TTrack* build3D(TTrack& track, const AList<TLink> & links) const;

    /// salvages unused hits.
    void salvage(TTrack& track) const;

    /// removes used links from _axial and _stereo.
    void removeUsedHits(const TTrack& t);

  private://
    bool goodTrackHoughMatch(const THoughPlane& hp,
                             const THoughPlane& hm,
                             const THoughTransformation& trans,
                             const TTrack& t,
                             unsigned threshold,
                             const TPoint2D& origin = Origin) const;
    bool goodTrack(const AList<TLink> &) const;
    bool goodTrack(const TTrack& t) const;
    bool goodTrackLowPt(const TTrack& t) const;
    AList<TLink> adjustAxialLinks(const AList<TLink> & list) const;

  private:// Parameters
    const bool _doCurlSearch;
    const float _axialLoadWidth;
    const float _axialLoadWidthCurl;
    const float _stereoLoadWidth;
    const float _salvageLoadWidth;
    const unsigned _minThreshold;
    const unsigned _minThresholdLowPt;
    const int _meshX;
    const int _meshY;
    const float _ptBoundary;
    const float _threshold;
    const int _meshXLowPt;
    const int _meshYLowPt;
    const float _ptBoundaryLowPt;
    const float _thresholdLowPt;
    const int _mode;

  private:
    const TBuilder _builder;
    AList<TLink> _axial;
    AList<TLink> _stereo;
    AList<TLink> _all;
    const TPeakFinder _peakFinder;
    const THoughTransformationCircle _circleHough;
    const float _ptBoundaryInHough;
//     THoughPlaneMulti _planeHP;
//     THoughPlaneMulti _planeHM;
    THoughPlaneMulti* _planeHP;
    THoughPlaneMulti* _planeHM;
    THoughPlane* _planes0[2];
    THoughPlaneMulti* _planes[2];

//     THoughPlaneMulti2 _planeHP2;
//     THoughPlaneMulti2 _planeHM2;
    THoughPlaneMulti2* _planeHP2;
    THoughPlaneMulti2* _planeHM2;
    THoughPlane* _planes02[2];
    THoughPlaneMulti2* _planes2[2];

    //    THoughPlaneMulti _tmp;
    THoughPlaneMulti* _tmp;

  private:
    unsigned _nsl;
    unsigned* _nHits2D;
    unsigned* _nHits3D;
    unsigned* _nHitsG;
  };

//-----------------------------------------------------------------------------

#ifdef THoughFinder_NO_INLINE
#define inline
#else
#undef inline
#define THoughFinder_INLINE_DEFINE_HERE
#endif

#ifdef THoughFinder_INLINE_DEFINE_HERE

  inline
  std::string
  THoughFinder::name(void) const
  {
    return "Hough Finder";
  }

#endif

#undef inline

} // namespace Belle

#endif
