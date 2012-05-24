//-----------------------------------------------------------------------------
// $Id: TFinderBase.h 10660 2008-09-25 04:27:48Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TFinderBase.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A virtual class for a track finder in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.16  2005/11/03 23:20:35  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.15  2005/04/18 23:42:04  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.14  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.13  2003/12/19 07:36:15  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.12  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.11  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.10  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.9  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.8  2000/03/17 11:01:42  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.7  1999/10/21 15:45:20  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.6  1998/07/29 04:35:19  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.4  1998/06/11 08:15:46  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.3  1998/04/23 17:25:03  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.2  1998/04/10 09:38:19  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:51:14  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#ifndef TFinderBase_FLAG_
#define TFinderBase_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>
#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"

namespace Belle2 {
  class TRGCDCWireHit;
}

namespace Belle {

  class TCircle;
  class TTrack;
  class TPoint2D;
  class TLink;

// extern const float WIDTHXXX[11];
// extern const float RXXX[12];
// extern const float R2XXX[12];

/// A virtual class for a track finder in tracking.
  class TFinderBase {

  public:
    /// Constructor
    TFinderBase();

    /// Destructor
    virtual ~TFinderBase();

  public:// General interfaces
    /// returns name.
    virtual std::string name(void) const = 0;

    /// returns version.
    virtual std::string version(void) const = 0;

    /// dumps debug information.
    virtual void dump(const std::string& message = std::string(""),
                      const std::string& prefix = std::string("")) const;

    /// returns debug level.
    virtual int debugLevel(void) const;

  public:// Modifiers
    /// clear internal information.
    virtual void clear(void) = 0;

    /// initializes internal caches.
    virtual void init(void) = 0;

    /// sets debug level.
    virtual int debugLevel(int);

    /// sets flag to reconstruct 3D.
    virtual bool doStereo(bool);

    /// sets flag to salvage hits.
    virtual bool doSalvage(bool);

    /// finds tracks. 'hits' are used to reconstruct. 'tracks' can be used for both inputs and outputs. Return value = (0, +, -) means (success, warning, fatal error).
    virtual int doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                     const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                     AList<TTrack> & tracks3D,
                     AList<TTrack> & tracks2D) = 0;

  public:// Finder utilities
//  int crossPoints(const TCircle &, TPoint2D points[12]) const;
//  int crossPointsDetail(const TCircle &, TPoint2D points[50]) const;
    static int crossPointsBySuperLayer(const TCircle&, TPoint2D* points);
    static int crossPointsByLayer(const TCircle&, TPoint2D* points);

    static AList<TLink> pickUpLinks(const TCircle&,
                                    const AList<TLink> & links,
                                    float loadWidth,
                                    unsigned axialStereo);
    static AList<TLink> pickUpLinksDetail(const TCircle&,
                                          const AList<TLink> & links,
                                          float loadWidth,
                                          unsigned axialStereo);
    static AList<TLink> pickUpNeighborLinks(const AList<TLink> & seeds,
                                            const AList<TLink> & links);

    //...For Hough finder...
    static AList<TLink> pickUpLinks2(const TCircle&,
                                     const AList<TLink> & links,
                                     float loadWidth,
                                     unsigned axialStereo);
//     AList<TLink> pickUpLinks2(const TTrack &,
//            const AList<TLink> & links,
//            float loadWidth,
//            unsigned axialStereo) const;

  private:
    int _debugLevel;
    static TPoint2D _points0[100];
    static TPoint2D _points1[100];
    static TPoint2D _points2[100];
  };

//-----------------------------------------------------------------------------

#ifdef TFinderBase_NO_INLINE
#define inline
#else
#undef inline
#define TFinderBase_INLINE_DEFINE_HERE
#endif

#ifdef TFinderBase_INLINE_DEFINE_HERE

  inline
  int
  TFinderBase::debugLevel(void) const
  {
    return _debugLevel;
  }

  inline
  int
  TFinderBase::debugLevel(int a)
  {
    return _debugLevel = a;
  }

  inline
  bool
  TFinderBase::doStereo(bool a)
  {
    return a;
  }

  inline
  bool
  TFinderBase::doSalvage(bool a)
  {
    return a;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TFinderBase_FLAG_ */
