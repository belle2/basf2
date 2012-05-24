//-----------------------------------------------------------------------------
// $Id: TFastFinder.h 9932 2006-11-12 14:26:53Z katayama $
//-----------------------------------------------------------------------------
// Filename : TFastFinder.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the conformal method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.8  2004/03/26 06:07:26  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.7  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:54  katayama
// Uss find,istring
//
// Revision 1.5  2001/12/14 02:54:49  katayama
// For gcc-3.0
//
// Revision 1.4  2001/04/11 01:10:03  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.3  1999/10/30 10:12:48  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.2  1999/07/15 08:43:22  yiwasaki
// Trasan 1.55b release : Curl finder # of hits protection, bug in TManager for MC, helix fitter # of hits protection, fast finder improved
//
// Revision 1.1  1999/06/29 00:03:05  yiwasaki
// Trasan 1.48a release : TFastFinder added
//
//
//-----------------------------------------------------------------------------

#ifndef TFastFinder_FLAG_
#define TFastFinder_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif

#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TFinderBase.h"
#include "tracking/modules/trasan/TBuilder0.h"

namespace Belle {

  class TLink;

/// A class to find tracks with the conformal method.
  class TFastFinder : public TFinderBase {

  public:
    /// Constructor.
    TFastFinder();

    /// Destructor
    virtual ~TFastFinder();

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

    /// finds tracks.
    int doit(const AList<Belle2::TRGCDCWireHit> & axialHits,
             const AList<Belle2::TRGCDCWireHit> & stereoHits,
             AList<TTrack> & tracks);

  private:
    /// selects isolated hits.(conf.finder type)
    void selectHits(const AList<Belle2::TRGCDCWireHit> & axialHits,
                    const AList<Belle2::TRGCDCWireHit> & stereoHits);

    /// selects isolated hits.(calling selectSimpleSegments)
    void selectHits2(const AList<Belle2::TRGCDCWireHit> & axialHits,
                     const AList<Belle2::TRGCDCWireHit> & stereoHits);

    /// selects simple segments.
    void selectSimpleSegments(const AList<Belle2::TRGCDCWireHit> & hits,
                              AList<Belle2::TRGCDCWireHit> & output) const;

    /// finds close hits.
    AList<TLink> findCloseHits(const AList<TLink> & links,
                               const TTrack& track) const;

  private:
    AList<Belle2::TRGCDCWireHit> _axialHits;
    AList<Belle2::TRGCDCWireHit> _stereoHits;
    AList<TLink> _axialLinks;
    AList<TLink> _stereoLinks;

    TBuilder0 _builder;
    TSelector0 _selector;
  };

//-----------------------------------------------------------------------------

#ifdef TFastFinder_NO_INLINE
#define inline
#else
#undef inline
#define TFastFinder_INLINE_DEFINE_HERE
#endif

#ifdef TFastFinder_INLINE_DEFINE_HERE

  inline
  std::string
  TFastFinder::name(void) const
  {
    return "Fast Finder";
  }

#endif

#undef inline

} // namespace Belle

#endif /* TFastFinder_FLAG_ */

