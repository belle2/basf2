//-----------------------------------------------------------------------------
// $Id: TPeakFinder.cc 10528 2008-06-20 00:08:20Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TPeakFinder.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find peaks in THoughPlan.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.7  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.6  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.5  2005/04/11 23:00:45  yiwasaki
// <sys/ddi.h> -> <cmath>
//
// Revision 1.4  2005/03/11 03:57:51  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.3  2004/04/15 05:34:09  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.2  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.1  2004/02/18 04:07:27  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
//-----------------------------------------------------------------------------

#include <strings.h>
#ifdef TRASAN_DEBUG_DETAIL
#include <cmath>
#endif


#include "tracking/modules/trasan/TPeakFinder.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/THoughPlane.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindowHough.h"
extern TWindow Window;
extern TWindowHough HWindow;
#endif


namespace Belle {

  TPeakFinder::TPeakFinder(void)
  {
  }

  TPeakFinder::~TPeakFinder()
  {
  }

  std::string
  TPeakFinder::version(void) const
  {
    return "0.01";
  }

  void
  TPeakFinder::dump(const std::string& message,
                    const std::string& prefix) const
  {
    std::cout << prefix;
    std::cout << message << std::endl;

    return;
  }

  AList<TPoint2D>
  TPeakFinder::peaks(const THoughPlane& hp,
                     const unsigned threshold,
                     const unsigned nTargetPeaks,
                     const unsigned peakSeparation) const
  {

#ifdef TRASAN_DEBUG
    const std::string stage = "TPeakFdr::peaks";
    EnterStage(stage);
#endif


    AList<TPoint2D> list;

    //...Search maximum...
    unsigned nStorages = nTargetPeaks * 100;
    static unsigned* nMax = (unsigned*) malloc(nStorages * sizeof(unsigned));
    static unsigned* x = (unsigned*) malloc(nStorages * sizeof(unsigned));
    static unsigned* y = (unsigned*) malloc(nStorages * sizeof(unsigned));
    bzero(nMax, nStorages * sizeof(unsigned));
    bzero(x, nStorages * sizeof(unsigned));
    bzero(y, nStorages * sizeof(unsigned));
    unsigned nStored = 0;
    bool finished = false;
    for (unsigned j = 0; j < hp.nY(); j++) {
      for (unsigned i = 0; i < hp.nX(); i++) {

        //...Threshold check...
        const unsigned n = hp.entry(i, j);
        if (n < threshold) continue;

        nMax[nStored] = n;
        x[nStored] = i;
        y[nStored] = j;
        ++nStored;
        if (nStored == nStorages) {
          finished = true;
          break;
        }
      }
      if (finished) break;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#target peaks="
              << nTargetPeaks << ",#storages=" << nStorages
              << ",threshold=" << threshold << std::endl;
    std::cout << Tab() << "#cells above threshold=" << nStored
              << std::endl;
#endif

    if (! nStored) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return list;
    }

    //...Sort...
    for (unsigned i = 0; i < nStored - 1; i++) {
      for (unsigned j = i + 1; j < nStored; j++) {
        if (nMax[i] < nMax[j]) {
          const unsigned tmpN = nMax[i];
          const unsigned tmpX = x[i];
          const unsigned tmpY = y[i];
          nMax[i] = nMax[j];
          x[i] = x[j];
          y[i] = y[j];
          nMax[j] = tmpN;
          x[j] = tmpX;
          y[j] = tmpY;
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "seq.id,counts,(position)" << std::endl;
//    for (unsigned i = 0; i < nStored; i++) {
    for (int i = 0; i < std::min(10, (int) nStored); i++) {
      std::cout << Tab() << i << "," << nMax[i] << ",("
                << x[i] << "," << y[i] << ")" << std::endl;
    }
#endif

    //...Select peaks...
    static unsigned* peakId =
      (unsigned*) malloc(nTargetPeaks * sizeof(unsigned));
    bzero(peakId, nTargetPeaks * sizeof(unsigned));
    unsigned nPeakId = 0;
    for (unsigned i = 0; i < nStored; i++) {

      //...Separation check...
      bool skip = false;
      for (unsigned j = 0; j < nPeakId; j++) {
        int diff0 = x[i] - x[peakId[j]];
        int diff1 = y[i] - y[peakId[j]];
        int diff = diff0 * diff0 + diff1 * diff1;
        if (diff < int(peakSeparation * peakSeparation)) {
          skip = true;
          break;
        }
      }
      if (skip) continue;

      //...OK...
      peakId[nPeakId++] = i;
      list.append(new TPoint2D(hp.xMin() + (float(x[i]) + 0.5) *hp.xSize(),
                               hp.yMin() + (float(y[i]) + 0.5) *hp.ySize()));

      //...# of peaks...
      if ((unsigned) list.length() == nTargetPeaks)
        break;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#Peaks=" << list.length()
              << " (peak separation=" << peakSeparation << ")" << std::endl;
    for (unsigned i = 0; i < (unsigned) list.length(); i++) {
      std::cout << Tab() << i << ",n=" << nMax[peakId[i]]
                << ",(" << x[peakId[i]] << "," << y[peakId[i]] << ")"
                << (* list[i]) << std::endl;
    }
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return list;
  }

  AList<TPoint2D>
  TPeakFinder::peaks5(THoughPlane& hp,
                      const unsigned threshold,
                      bool centerIsPeak) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TPeakFdr::peaks5";
    EnterStage(stage);
#endif

    AList<TPoint2D> list;

    //...Search cells above threshold...
    unsigned nCells = hp.nX() * hp.nY();
    static unsigned* candidates =
      (unsigned*) malloc(nCells * sizeof(unsigned));
    unsigned nActive = 0;
    for (unsigned j = 0; j < hp.nY(); j++) {
      for (unsigned i = 0; i < hp.nX(); i++) {

        //...Threshold check...
        const unsigned n = hp.entry(i, j);
        if (n < threshold) continue;
        candidates[nActive] = hp.serialID(i, j);
        ++nActive;
      }
    }

    //...Make connected regions (is this the best way???)...
    const unsigned used = nCells;
    for (unsigned i = 0; i < nActive; i++) {
      if (candidates[i] == used) continue;
      const unsigned id0 = candidates[i];
      candidates[i] = used;

      //...Make a new region...
      CList<unsigned> * region = new CList<unsigned>;
      region->append(id0);

      //...Search neighbors...
      for (unsigned j = 0; j < nActive; j++) {
        if (candidates[j] == used) continue;
        const unsigned id1 = candidates[j];

        unsigned x1 = 0;
        unsigned y1 = 0;
        hp.id(id1, x1, y1);

#ifdef TRASAN_DEBUG_DETAIL
//        std::cout << Tab() << "    region:x=" << x1 << ",y=" << y1
//            << std::endl;
#endif
        for (unsigned k = 0; k < unsigned(region->length()); k++) {
          unsigned id2 = * (* region)[k];
          unsigned x2 = 0;
          unsigned y2 = 0;
          hp.id(id2, x2, y2);
          int difx = abs(int(x1) - int(x2));
          int dify = abs(int(y1) - int(y2));
          if (difx > (int) hp.nX() / 2) difx = hp.nX() - difx;
          if (dify > (int) hp.nY() / 2) dify = hp.nY() - dify;
#ifdef TRASAN_DEBUG_DETAIL
//    std::cout << Tab() << "        :x=" << x2 << ",y=" << y2
//           << ":difx=" << difx << ",dify=" << dify;
//    if ((difx < 2) && (dify < 2))
//        std::cout << " ... connected" << std::endl;
//    else
//        std::cout << std::endl;
#endif
          if ((difx < 2) && (dify < 2)) {
            region->append(id1);
            candidates[j] = used;
            break;
          }

        }
      }
      hp.setRegion(region);
    }

    //...Determine peaks...
    const AList<CList<unsigned> > & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.length(); i++) {

      //...Calculate size and center of a region...
      const CList<unsigned> & r = * regions[i];
      unsigned minX = hp.nX();
      unsigned maxX = 0;
      unsigned minY = hp.nY();
      unsigned maxY = 0;
      for (unsigned j = 0; j < (unsigned) r.length(); j++) {
        const unsigned s = * r[j];
        unsigned x = 0;
        unsigned y = 0;
        hp.id(s, x, y);
        if (x < minX) minX = x;
        if (x > maxX) maxX = x;
        if (y < minY) minY = y;
        if (y > maxY) maxY = y;

#ifdef TRASAN_DEBUG_DETAIL
//        std::cout << Tab() << "region " << i << ":x=" << x << ",y=" << y
//            << std::endl;
#endif
      }
      const unsigned cX = minX + (maxX - minX) / 2;
      const unsigned cY = minY + (maxY - minY) / 2;

      //...Determine a center of a region...
      unsigned ncX = hp.nX() * hp.nY();
      unsigned ncY = ncX;
      if (! centerIsPeak) {

        //...Search for a cell which is the closest to the center...
        float minDiff2 = float(hp.nX() * hp.nX() + hp.nY() * hp.nY());
        for (unsigned j = 0; j < (unsigned) r.length(); j++) {
          const unsigned s = * r[j];
          unsigned x = 0;
          unsigned y = 0;
          hp.id(s, x, y);

          const float diff2 = (float(x) - float(cX)) *
                              (float(x) - float(cX))
                              + (float(y) - float(cY)) *
                              (float(y) - float(cY));

          if (diff2 < minDiff2) {
            minDiff2 = diff2;
            ncX = x;
            ncY = y;
#ifdef TRASAN_DEBUG_DETAIL
//        std::cout << Tab() << "region " << i << " center:x="
//         << ncX << ",y=" << ncY << "(" << j << ")"
//         << std::endl;
#endif
          }
        }
      } else {

        //...Search for a peak...
        float max = 0;
        for (unsigned j = 0; j < (unsigned) r.length(); j++) {
          const unsigned s = * r[j];
          const float entry = hp.entry(s);
          if (max < entry) {
            max = entry;
            unsigned x = 0;
            unsigned y = 0;
            hp.id(s, x, y);
            ncX = x;
            ncY = y;
          }
        }
      }

      //...Store the center position....
      list.append(new TPoint2D(hp.position(ncX, ncY)));
#ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << "region " << i << " final center:x="
//         << hp.position(ncX, ncY).x() << ",y="
//         << hp.position(ncX, ncY).y() << std::endl;
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "Peak finding:threshold=" << threshold << ",nActive="
              << nActive << ",regions=" << hp.regions().length()
              << "," << hp.name() << std::endl;
    for (unsigned i = 0; i < (unsigned) hp.regions().length(); i++) {
      const CList<unsigned> & region = * (hp.regions())[i];
      for (unsigned j = 0; j < (unsigned) region.length(); j++)
        std::cout << Tab() << "    " << * region[j] << "="
                  << hp.entry(* region[j]) << std::endl;
    }
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return list;
  }

  AList<TPoint2D>
  TPeakFinder::peaks6(THoughPlane& hp,
                      const unsigned threshold,
                      bool centerIsPeak) const
  {
// #ifdef TRASAN_DEBUG
//     const std::string stage = "TPeakFdr::peaks6";
//     EnterStage(stage);
// #endif

    AList<TPoint2D> list;
    return list;

//     //...Search for locak peaks...
//     const unsigned nCells = hp.nX() * hp.nY();
//     static unsigned * candidates =
//  (unsigned *) malloc(nCells * sizeof(unsigned));
//     unsigned nLocalPeaks = 0;
//     for (unsigned j = 0; j < hp.nY(); j++) {
//  for (unsigned i = 0; i < hp.nX(); i++) {
//      const unsigned id = hp.serialID(i, j);
//      const unsigned n = hp.entry(i, j);

//      //...Neighbor check...
//      bool localPeak = true;
//      for (unsigned k = 0; k < 8; k++) {
//    const unsigned nid = hp.neighbor(id, k);
//    if (nid == id) continue;
//    const unsigned nn = hp.entry(nid);
//    if (nn > n) {
//        localPeak = false;
//        break;
//    }
//      }

//      if (! localPeak) continue;
//      candidates[nLocalPeaks++] = id;
//  }
//     }

//     //...Make connected regions (is this the best way???)...
//     const unsigned used = nCells;
//     for (unsigned i = 0; i < nActive; i++) {
//  if (candidates[i] == used) continue;
//  const unsigned id0 = candidates[i];
//  candidates[i] = used;

//  //...Make a new region...
//  CList<unsigned> * region = new CList<unsigned>;
//  region->append(id0);

//  //...Search neighbors...
//  for (unsigned j = 0; j < nActive; j++) {
//      if (candidates[j] == used) continue;
//      const unsigned id1 = candidates[j];

//      unsigned x1 = 0;
//      unsigned y1 = 0;
//      hp.id(id1, x1, y1);

// #ifdef TRASAN_DEBUG_DETAIL
// //       std::cout << Tab() << "    region:x=" << x1 << ",y=" << y1
// //           << std::endl;
// #endif
//      for (unsigned k = 0; k < unsigned(region->length()); k++) {
//    unsigned id2 = * (* region)[k];
//    unsigned x2 = 0;
//    unsigned y2 = 0;
//    hp.id(id2, x2, y2);
//    int difx = abs(int(x1) - int(x2));
//    int dify = abs(int(y1) - int(y2));
//    if (difx > (int) hp.nX() / 2) difx = hp.nX() - difx;
//    if (dify > (int) hp.nY() / 2) dify = hp.nY() - dify;
// #ifdef TRASAN_DEBUG_DETAIL
// //     std::cout << Tab() << "        :x=" << x2 << ",y=" << y2
// //            << ":difx=" << difx << ",dify=" << dify;
// //     if ((difx < 2) && (dify < 2))
// //         std::cout << " ... connected" << std::endl;
// //     else
// //         std::cout << std::endl;
// #endif
//    if ((difx < 2) && (dify < 2)) {
//        region->append(id1);
//        candidates[j] = used;
//        break;
//    }

//      }
//  }
//  hp.setRegion(region);
//     }

//     //...Determine peaks...
//     const AList<CList<unsigned> > & regions = hp.regions();
//     for (unsigned i = 0; i < (unsigned) regions.length(); i++) {

//  //...Calculate size and center of a region...
//  const CList<unsigned> & r = * regions[i];
//  unsigned minX = hp.nX();
//  unsigned maxX = 0;
//  unsigned minY = hp.nY();
//  unsigned maxY = 0;
//  for (unsigned j = 0; j < (unsigned) r.length(); j++) {
//      const unsigned s = * r[j];
//      unsigned x = 0;
//      unsigned y = 0;
//      hp.id(s, x, y);
//      if (x < minX) minX = x;
//      if (x > maxX) maxX = x;
//      if (y < minY) minY = y;
//      if (y > maxY) maxY = y;

// #ifdef TRASAN_DEBUG_DETAIL
// //       std::cout << Tab() << "region " << i << ":x=" << x << ",y=" << y
// //           << std::endl;
// #endif
//  }
//  const unsigned cX = minX + (maxX - minX) / 2;
//  const unsigned cY = minY + (maxY - minY) / 2;

//  //...Determine a center of a region...
//  unsigned ncX = hp.nX() * hp.nY();
//  unsigned ncY = ncX;
//  if (! centerIsPeak) {

//      //...Search for a cell which is the closest to the center...
//      float minDiff2 = float(hp.nX() * hp.nX() + hp.nY() * hp.nY());
//      for (unsigned j = 0; j < (unsigned) r.length(); j++) {
//    const unsigned s = * r[j];
//    unsigned x = 0;
//    unsigned y = 0;
//    hp.id(s, x, y);

//    const float diff2 = (float(x) - float(cX)) *
//        (float(x) - float(cX))
//        + (float(y) - float(cY)) *
//        (float(y) - float(cY));

//    if (diff2 < minDiff2) {
//        minDiff2 = diff2;
//        ncX = x;
//        ncY = y;
// #ifdef TRASAN_DEBUG_DETAIL
// //         std::cout << Tab() << "region " << i << " center:x="
// //          << ncX << ",y=" << ncY << "(" << j << ")"
// //          << std::endl;
// #endif
//    }
//      }
//  }
//  else {

//      //...Search for a peak...
//      float max = 0;
//      for (unsigned j = 0; j < (unsigned) r.length(); j++) {
//    const unsigned s = * r[j];
//    const float entry = hp.entry(s);
//    if (max < entry) {
//        max = entry;
//        unsigned x = 0;
//        unsigned y = 0;
//        hp.id(s, x, y);
//        ncX = x;
//        ncY = y;
//    }
//      }
//  }

//  //...Store the center position....
//  list.append(new TPoint2D(hp.position(ncX, ncY)));
// #ifdef TRASAN_DEBUG_DETAIL
// //   std::cout << Tab() << "region " << i << " final center:x="
// //          << hp.position(ncX, ncY).x() << ",y="
// //          << hp.position(ncX, ncY).y() << std::endl;
// #endif
//     }

// #ifdef TRASAN_DEBUG_DETAIL
//     std::cout << Tab() << "Peak finding:threshold=" << threshold << ",nActive="
//       << nActive << ",regions=" << hp.regions().length()
//       << "," << hp.name() << std::endl;
//     for (unsigned i = 0; i < (unsigned) hp.regions().length(); i++) {
//    const CList<unsigned> & region = * (hp.regions())[i];
//    for (unsigned j = 0; j < (unsigned) region.length(); j++)
//        std::cout << Tab() << "    " << * region[j] << "="
//         << hp.entry(* region[j]) << std::endl;
//     }
// #endif
// #ifdef TRASAN_DEBUG
//     LeaveStage(stage);
// #endif

//     return list;
  }

} // namespace Belle

