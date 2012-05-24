//-----------------------------------------------------------------------------
// $Id: TPerfectFinder.h 10305 2007-12-05 05:19:24Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TPerfectFinder.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks using MC info.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.5  2001/12/23 09:58:56  katayama
// removed Strings.h
//
// Revision 1.4  2001/12/19 02:59:55  katayama
// Uss find,istring
//
// Revision 1.3  2001/12/14 02:54:50  katayama
// For gcc-3.0
//
// Revision 1.2  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.1  2000/08/31 23:51:50  yiwasaki
// Trasan 2.04 : pefect finder added
//
//-----------------------------------------------------------------------------

#ifndef TPERFECTFINDER_FLAG_
#define TPERFECTFINDER_FLAG_
#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif
#define HEP_SHORT_NAMES

#include "tracking/modules/trasan/TFinderBase.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TBuilder.h"
#include "tracking/modules/trasan/THelixFitter.h"

namespace Belle {

  template <class T> class AList;

/// A class to find tracks using MC info.
  class TPerfectFinder : public TFinderBase {

  public:
    /// Constructor.
    TPerfectFinder(int perfectFitting,
                   float maxSigma,
                   float maxSigmaStereo,
                   unsigned fittingFlag);

    /// Destructor
    virtual ~TPerfectFinder();

  public:// Selectors
    /// returns name.
    std::string name(void) const;

    /// returns version.
    std::string version(void) const;

    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// gets perfect momentum from MC info.
    bool perfectFitting(bool);

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

  private:
    float charge(int pType) const;

  private:
    bool _perfectFitting;
    AList<TLink> _links;
    float _maxSigma;
    float _maxSigmaStereo;
    unsigned _fittingFlag;
    TBuilder _builder;
    THelixFitter _fitter;
  };

//-----------------------------------------------------------------------------

#ifdef TPERFECTFINDER_NO_INLINE
#define inline
#else
#undef inline
#define TPERFECTFINDER_INLINE_DEFINE_HERE
#endif

#ifdef TPERFECTFINDER_INLINE_DEFINE_HERE

  inline
  std::string
  TPerfectFinder::name(void) const
  {
    return "Perfect Finder";
  }

  inline
  bool
  TPerfectFinder::perfectFitting(bool a)
  {
    return _perfectFitting = a;
  }

  inline
  void
  TPerfectFinder::init(void)
  {
    return;
  }

#endif

#undef inline

} // namespace Belle

#endif /* TPERFECTFINDER_FLAG_ */
