//-----------------------------------------------------------------------------
// $Id: TSegmentLinker.h 10002 2007-02-26 06:56:17Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSegmentLinker.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to link TSegments.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2001/12/23 09:58:57  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:56  katayama
// Uss find,istring
//
// Revision 1.5  2001/12/14 02:54:51  katayama
// For gcc-3.0
//
// Revision 1.4  2001/04/11 01:10:04  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.3  2000/01/28 06:30:32  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.2  1999/10/30 10:12:51  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1998/08/03 12:39:10  yiwasaki
// TSegmentLinker, TrackQuality.h and sakura.h added, sakura.cc modified
//
//-----------------------------------------------------------------------------

#ifndef TSegmentLinker_FLAG_
#define TSegmentLinker_FLAG_

#ifdef TRASAN_DEBUG_DETAIL
#ifndef TRASAN_DEBUG
#define TRASAN_DEBUG
#endif
#endif


#include <string>

#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/AList.h"

namespace Belle {

  class Range;
  class TSegment0;

/// A class to link TSegment0s.
  class TSegmentLinker {

  public:
    /// Constructor.
    TSegmentLinker(unsigned nLayer,
                   TSegment0* baseSegmnet,
                   AList<TSegment0> * segmentLists);

    /// Destructor
    virtual ~TSegmentLinker();

  public:// Selectors
    /// dumps debug information.
    void dump(const std::string& message = std::string(""),
              const std::string& prefix = std::string("")) const;

    /// returns a list of best segment link.
    AList<TSegment0> bestLink(void);

  private://
    /// selects possible segments.
    void roughSelection(void);

  private:
    unsigned _nLayer;
    TSegment0* _base;
    AList<TSegment0> * _list[11];
  };

//-----------------------------------------------------------------------------

#ifdef TRASAN_NO_INLINE
#define inline
#else
#undef inline
#define TSegmentLinker_INLINE_DEFINE_HERE
#endif

#ifdef TSegmentLinker_INLINE_DEFINE_HERE


#endif

#undef inline

} // namespace Belle

#endif /* TSegmentLinker_FLAG_ */

