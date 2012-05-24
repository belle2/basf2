//-----------------------------------------------------------------------------
// $Id: TSegmentLinker.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TSegmentLinker.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to link TSegments.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.10  2003/12/25 12:03:35  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.9  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.8  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.7  2001/04/11 01:09:11  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.6  2000/04/11 13:05:47  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.5  2000/01/28 06:30:25  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.4  1999/10/30 10:12:22  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.3  1998/08/31 05:15:48  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.2  1998/08/17 01:00:33  yiwasaki
// TSegmentLinker::roughSelection added, TRASAN_DEBUG option works
//
// Revision 1.1  1998/08/03 12:39:09  yiwasaki
// TSegmentLinker, TrackQuality.h and sakura.h added, sakura.cc modified
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TSegment0.h"
#include "tracking/modules/trasan/TSegmentLinker.h"
#include "tracking/modules/trasan/TLink.h"

namespace Belle {

  TSegmentLinker::TSegmentLinker(unsigned nLayer,
                                 TSegment0* baseSegment,
                                 AList<TSegment0> * segmentLists)
    : _nLayer(nLayer),
      _base(baseSegment)
  {
    for (unsigned i = 0; i < _nLayer; i++)
      _list[i] = new AList<TSegment0>(segmentLists[i]);

    roughSelection();
  }

  TSegmentLinker::~TSegmentLinker()
  {
  }

  void
  TSegmentLinker::dump(const std::string&, const std::string&) const
  {
    std::cout << "TSegmentLinker::dump ... " << std::endl;
  }

  AList<TSegment0>
  TSegmentLinker::bestLink(void)
  {

// #ifdef TRASAN_DEBUG_DETAIL
// std::cout << name() << " ... finding cluster linkage" << std::endl;
//     if (base.links().length() == 0)
//  std::cout << name() << " !!! base doesn't have any TLink." << std::endl;
// std::cout << "... base cluster" << std::endl;
//     base.dump("cluster hits mc", "  ->");
// #endif

//     //...Preparation of return value...
    AList<TSegment0> seeds;
//     seeds.append(base);

//     //...Which super layer?...
//     unsigned outerMost = (base.links())[0]->wire()->superLayerId() / 2;

//     //...Inner super layer loop...
//     int next = outerMost;
//     TSegment0 * last = & base;
//     while (next) {
//  --next;
//  const AList<TSegment0> & candidates = list[next];
//  if (candidates.length() == 0) continue;

// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << "... clusters in super layer " << next << std::endl;
// #endif

//  //...Find best match...
//  TSegment0 * best = findBestLink(* last, candidates);
//  if (best != NULL) {
//      seeds.append(best);
//      last = best;
// #ifdef TRASAN_DEBUG_DETAIL
//      std::cout << "  ->Best is ";
//      std::cout << best->position() << " ";
//      best->dump("hits mc");
// #endif
//  }
//     }

    return seeds;
  }

  void
  TSegmentLinker::roughSelection(void)
  {
  }

} // namespace Belle

