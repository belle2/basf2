//-----------------------------------------------------------------------------
// $Id: TFinderBase.cc 10673 2008-10-03 02:51:18Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TFinderBase.cc
// Section  : Tracking CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A virtual class for a track finder in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.12  2005/11/03 23:20:11  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.10  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.9  2005/03/11 03:57:50  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.8  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.7  2001/12/23 09:58:47  katayama
// removed Strings.h
//
// Revision 1.6  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.5  2000/04/11 13:05:44  katayama
// Added std:: to cout, cerr, std::endl etc.
//
// Revision 1.4  1998/06/11 08:14:07  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.3  1998/04/23 17:21:40  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.2  1998/04/10 09:36:27  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:50:15  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------



#include <iostream>
#include "tracking/modules/trasan/TFinderBase.h"
#include "trg/cdc/TRGCDC.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TUtilities.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  TPoint2D TFinderBase::_points0[100];
  TPoint2D TFinderBase::_points1[100];
  TPoint2D TFinderBase::_points2[100];

// const float WIDTHXXX[11] = {PI2 / 64,
//       PI2 / 80,
//       PI2 / 96,
//       PI2 / 128,
//       PI2 / 144,
//       PI2 / 160,
//       PI2 / 192,
//       PI2 / 208,
//       PI2 / 240,
//       PI2 / 256,
//       PI2 / 288};

// const float RXXX[] = {8.3,
//       16.9,
//       21.7,
//       31.3,
//       36.1,
//       44.1,
//       50.5,
//       58.5,
//       64.9,
//       72.9,
//       79.3,
//       87.4};

// const float R2XXX[] = {8.3 *  8.3,
//        16.9 * 16.9,
//        21.7 * 21.7,
//        31.3 * 31.3,
//        36.1 * 36.1,
//        44.1 * 44.1,
//        50.5 * 50.5,
//        58.5 * 58.5,
//        64.9 * 64.9,
//        72.9 * 72.9,
//        79.3 * 79.3,
//        87.4 * 87.4};

  TFinderBase::TFinderBase()
    : _debugLevel(0)
  {
  }

  TFinderBase::~TFinderBase()
  {
  }

  void
  TFinderBase::dump(const std::string& msg, const std::string& pre) const
  {
    std::cout << pre;
    if (msg.find("name")    != std::string::npos
        || msg.find("version") != std::string::npos
        || msg.find("detail")    != std::string::npos
        || msg == "") {
      std::cout << name() << "(" << version() << ")";
    }
    if (msg.find("detail") != std::string::npos || msg.find("state") != std::string::npos) {
      std::cout << "Debug Level=" << _debugLevel;
    }
    std::cout << std::endl;
  }

  int
  TFinderBase::crossPointsBySuperLayer(const TCircle& circle,
                                       TPoint2D* points)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TFndrBs::crossPointsBySuperLayer";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "cirle center=" << circle.center()
              << ",r=" << fabs(circle.radius())
              << ",d=" << circle.center().mag() << std::endl;
    std::cout << Tab() << "cross points are :" << std::endl;
#endif

    //...Parameters...
    const TPoint2D c = circle.center();
    const TPoint2D co = - c;
    const double r = fabs(circle.radius());
    const double r2 = r * r;
    const double d2 = c.mag2();
    const double d = sqrt(d2);
    const double sl = - c.x() / c.y();

    //...Calculate points...
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    unsigned nOk = 0;
    //    for (unsigned i = 0; i < 12; i++) {
    for (unsigned i = 0; i <= cdc.nSuperLayers(); i++) {
      const double minR = r < cdc.superLayerR(i) ? r : cdc.superLayerR(i);
      const double maxR = r < cdc.superLayerR(i) ? cdc.superLayerR(i) : r;

      if ((r + cdc.superLayerR(i) < d) || (minR + d < maxR)) {

//      std::cout << "minR,maxR=" << minR << "," << maxR << std::endl;

        points[i] = Origin;
        continue;
      }
      ++nOk;
      double a = cdc.superLayerR2(i) + d2 - r2;
      double s = sqrt(4. * cdc.superLayerR2(i) * d2 - a * a);
      double q = 0.5 * a / c.y();
      points[i].x(0.5 * (c.x() * a + c.y() * s) / d2);
      points[i].y(q + sl * points[i].x());
      if (co.cross(points[i] - c) * circle.charge() > 0.) {
        points[i].x(0.5 * (c.x() * a - c.y() * s) / d2);
        points[i].y(q + sl * points[i].x());
      }
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << i << " : " << points[i] << std::endl;
//    std::cout << "    chg=" << circle.charge();
//    std::cout << ", c=" << c << ", co=" << co;
//    std::cout << ", " << co.cross(points[i] - c) * circle.charge() << std::endl;
//    std::cout << "        " << 0.5 * (c.x() * a + c.y() * s) / d2;
//    std::cout << ", " << q + sl * (0.5 * (c.x() * a + c.y() * s) / d2);
//    std::cout << "        " << 0.5 * (c.x() * a - c.y() * s) / d2;
//    std::cout << ", " << q + sl * (0.5 * (c.x() * a - c.y() * s) / d2);
//    std::cout << std::endl;
#endif

    }

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    if (nOk) return 0;
    else return -1;
  }

  AList<TLink>
  TFinderBase::pickUpLinks(const TCircle& circle,
                           const AList<TLink> & links,
                           float loadWidth,
                           unsigned axialStereoSwitch)
  {

    AList<TLink> outList;
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
//  TPoint2D points[cdc.nSuperLayers()+1];
//  _points0 = new TPoint2D[cdc.nSuperLayers() + 1];
    int err = crossPointsBySuperLayer(circle, _points0);
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TFinderBase::pickUpLinks !!! circle cross-point to "
                << "super-layers not found" << std::endl;
#endif
      return outList;
    }

    //    const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
    unsigned nBad = links.length();
    for (unsigned i = 0; i < nBad; i++) {
      unsigned sl = links[i]->wire()->superLayerId();
//  unsigned as = sl % 2;
//  if (as == 0) {
//      if (! (axialStereoSwitch & 1)) continue;
//  }
//  else {
//      if (! (axialStereoSwitch & 2)) continue;
//  }
      bool axial = links[i]->wire()->axial();
      if (axial) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        if (!(axialStereoSwitch & 2)) continue;
      }

      if (_points0[sl] == Origin) continue;

      float a = cdc.cellWidth(sl) * loadWidth;
      float phi0 = _points0[sl].phi();
      float phi1 = _points0[sl + 1].phi();
      if (_points0[sl + 1] == Origin) phi1 = circle.center().phi();
      float phi = links[i]->position().phi();
      if (phi < 0.) phi += PI2;
      if (phi1 < phi0) {
        phi1 = phi0;
        phi0 = _points0[sl + 1].phi();
      }
      float dPhi = phi1 - phi0;
      if (dPhi < M_PI) {
        phi0 -= a;
        phi1 += a;
        if (phi > phi0 && phi < phi1) outList.append(links[i]);
      } else {
        phi0 += a;
        phi1 -= a;
        if (phi < phi0 || phi > phi1) outList.append(links[i]);
      }
#ifdef TRASAN_DEBUG_DETAIL
//    std::cout << links[i]->wire()->name()
//         << ":phi,phi0,phi1,dPhi,a=" << phi << "," << phi0 << "," << phi1
//       << "," << dPhi << "," << a << std::endl;
#endif
    }

    return outList;
  }

  int
  TFinderBase::crossPointsByLayer(const TCircle& circle,
                                  TPoint2D* points)
  {

    //...Check CDC version...
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    bool scdc = false;
    if (cdc.versionCDC() == "small cell") scdc = true;

    //...r...
    unsigned i0 = 0;
    if (scdc) i0 = 1;
    static float* RR(NULL);
    static float* RR2(NULL);
    static bool first = true;
    if (first) {
      RR = new float [cdc.nLayers()];
      RR2 = new float [cdc.nLayers()];
      for (unsigned i = i0; i < cdc.nLayers(); i++) {
//      const Belle2::TRGCDCWire & w = * (* cdc.layer(i))[0];
        const Belle2::TRGCDCWire& w = * cdc.wire(i, 0);
        RR[i] = w.xyPosition().perp();
        RR2[i] = RR[i] * RR[i];
      }
      first = false;
    }

    //...Parameters...
    TPoint2D c = circle.center();
    TPoint2D co = - c;
    double r = fabs(circle.radius());
    double r2 = r * r;
    double d2 = c.mag2();
    double d = sqrt(d2);
    double sl = - c.x() / c.y();

    //...Calculate points...
    unsigned nOk = 0;
    for (unsigned i = i0; i < cdc.nLayers(); i++) {
      double minR = r < RR[i] ? r : RR[i];
      double maxR = r < RR[i] ? RR[i] : r;

      if ((r + RR[i] < d) || (minR + d < maxR)) {
        points[i] = Origin;
        continue;
      }
      ++nOk;
      double a = RR2[i] + d2 - r2;
      double s = sqrt(4. * RR2[i] * d2 - a * a);
      double q = 0.5 * a / c.y();
      points[i].x(0.5 * (c.x() * a + c.y() * s) / d2);
      points[i].y(q + sl * points[i].x());
      if (co.cross(points[i] - c) * circle.charge() > 0.) {
        points[i].x(0.5 * (c.x() * a - c.y() * s) / d2);
        points[i].y(q + sl * points[i].x());
      }

#ifdef TRASAN_DEBUG_DETAIL
//  std::cout << "       " << i << " : " << points[i] << std::endl;
//    std::cout << "    chg=" << circle.charge();
//    std::cout << ", c=" << c << ", co=" << co;
//    std::cout << ", " << co.cross(points[i] - c) * circle.charge() << std::endl;
//    std::cout << "        " << 0.5 * (c.x() * a + c.y() * s) / d2;
//    std::cout << ", " << q + sl * (0.5 * (c.x() * a + c.y() * s) / d2);
//    std::cout << "        " << 0.5 * (c.x() * a - c.y() * s) / d2;
//    std::cout << ", " << q + sl * (0.5 * (c.x() * a - c.y() * s) / d2);
//    std::cout << std::endl;
#endif

    }
    if (nOk) return 0;
    else return -1;

  }

// AList<TLink>
// TFinderBase::pickUpLinks2(const TTrack & track,
//        const AList<TLink> & links,
//        float loadWidth,
//        unsigned axialStereoSwitch) {
//     TCircle circle(track);
//     return pickUpLinks2((TCircle &) circle, links, loadWidth, axialStereoSwitch);
// }

  AList<TLink>
  TFinderBase::pickUpLinks2(const TCircle& circle,
                            const AList<TLink> & links,
                            float loadWidth,
                            unsigned axialStereoSwitch)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TFndrBs::pickUpLinks2";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "loadWidth=" << loadWidth << "cells" << std::endl;
#endif

    AList<TLink> outList;
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
//  TPoint2D points[cdc.nSuperLayers()+1];
//  _points1 = new TPoint2D[cdc.nSuperLayers() + 1];
    const int err = crossPointsBySuperLayer(circle, _points1);
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "circle cross-point to "
                << "any super-layers not found:loadwidth=" << loadWidth
                << std::endl;
#endif
      return outList;
    }

    //    const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
    unsigned nBad = links.length();
    for (unsigned i = 0; i < nBad; i++) {
      const unsigned sl = links[i]->wire()->superLayerId();
//  const unsigned as = sl % 2;
//  if (as == 0) {
//      if (! (axialStereoSwitch & 1)) continue;
//  }
//  else {
//      if (! (axialStereoSwitch & 2)) continue;
//  }
      bool axial = links[i]->wire()->axial();
      if (axial) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        if (!(axialStereoSwitch & 2)) continue;
      }

      if (_points1[sl] == Origin) continue;

//  const float a = WIDTH[sl] * loadWidth;
//  float phi = links[i]->position().phi();
//  if (phi < 0.) phi += PI2;
//  float phi0 = _points1[sl].phi();
//  float phi1 = _points1[sl + 1].phi();
//  if (_points1[sl + 1] == Origin)
//      phi1 = circle.center().phi();
//  if (phi0 < 0.) phi0 += PI2;
//  if (phi1 < 0.) phi1 += PI2;
//  if (phi1 < phi0) {
//      float tmp = phi1;
//      phi1 = phi0;
//      phi0 = tmp;
//  }
//  float dPhi = fabs(phi1 - phi0);
//  if (dPhi < M_PI) {
//      phi0 -= a;
//      phi1 += a;
//      if (phi > phi0 && phi < phi1) outList.append(links[i]);
//  }
//  else {
//      phi0 += a;
//      phi1 -= a;
//      if (phi < phi0 || phi > phi1) outList.append(links[i]);
//  }

      const float a = cdc.cellWidth(sl) * loadWidth;
      float phi = links[i]->position().phi();
      float phi0 = _points1[sl].phi();
      float phi1 = _points1[sl + 1].phi();
      if (_points1[sl + 1] == Origin)
        phi1 = circle.center().phi();

      const bool inRange = InRangeRadian(phi0, phi1, phi);

      if (inRange)
        outList.append(links[i]);
      else if (DistanceRadian(phi0, phi) < a)
        outList.append(links[i]);
      else if (DistanceRadian(phi1, phi) < a)
        outList.append(links[i]);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << links[i]->wire()->name()
                << ":phi,phi0,phi1,a=" << phi << "," << phi0 << ","
                << phi1 << "," << a;
      if (inRange)
        std::cout << ":ir=ok";
      if (DistanceRadian(phi0, phi) < a)
        std::cout << ":dr0=ok";
      if (DistanceRadian(phi1, phi) < a)
        std::cout << ":dr1=ok";
      std::cout << std::endl;
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(outList, "breif", Tab() + "outList:");
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return outList;
  }

  AList<TLink>
  TFinderBase::pickUpLinksDetail(const TCircle& circle,
                                 const AList<TLink> & links,
                                 float loadWidth,
                                 unsigned axialStereoSwitch)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TFndrBs::pickUpLinksDetail";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "loadWidth=" << loadWidth << "cells" << std::endl;
#endif

    AList<TLink> outList;
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
//  TPoint2D points[cdc.nLayers()+1];
//  _points2 = new TPoint2D[cdc.nLayers() + 1];
    int err = crossPointsByLayer(circle, _points2);
    if (err) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "circle cross-point to "
                << "any layers not found:loadwidth=" << loadWidth
                << std::endl;
#endif
      return outList;
    }

    unsigned nBad = links.length();
    for (unsigned i = 0; i < nBad; i++) {
      const unsigned sl = links[i]->wire()->superLayerId();
      const unsigned lid = links[i]->wire()->layerId();
//  unsigned as = sl % 2;
//  if (as == 0) {
//      if (! (axialStereoSwitch & 1)) continue;
//  }
//  else {
//      if (! (axialStereoSwitch & 2)) continue;
//  }
      bool axial = links[i]->wire()->axial();
      if (axial) {
        if (!(axialStereoSwitch & 1)) continue;
      } else {
        if (!(axialStereoSwitch & 2)) continue;
      }

      if (_points2[lid] == Origin) continue;

      const float a = cdc.cellWidth(sl) * loadWidth;
      float phi = links[i]->position().phi();
      float phi0 = _points2[lid].phi();
      if (DistanceRadian(phi0, phi) < a)
        outList.append(links[i]);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << links[i]->wire()->name()
                << ":phi,phi0,dist,a=" << phi << "," << phi0 << "," << DistanceRadian(phi0, phi) << "," << a;
      if (DistanceRadian(phi0, phi) < a)
        std::cout << ":dr0=ok";
      std::cout << std::endl;
#endif
    }

#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(outList, "breif", Tab() + "outList:");
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
    return outList;
  }

  AList<TLink>
  TFinderBase::pickUpNeighborLinks(const AList<TLink> & seeds,
                                   const AList<TLink> & links)
  {

#ifdef TRASAN_DEBUG
    const std::string stage = "TFndrBs::pickUpNeighborLinks";
    EnterStage(stage);
#endif

    AList<TLink> outList;

    //...Loop...
    const unsigned n = seeds.length();
    for (unsigned i = 0; i < n; i++) {
      const TLink& t = * seeds[i];
      const TLink* const t2 = t.neighbor(2);
      const TLink* const t3 = t.neighbor(3);
      if (! links.hasMember((TLink*) t2)) continue;
      if (! links.hasMember((TLink*) t3)) continue;

      if (t2) {
        const TLink* const t22 = t2->neighbor(2);
        if (! t22) {
          if (! seeds.hasMember((TLink*) t2))
            outList.append((TLink*) t2);
        }
      }
      if (t3) {
        const TLink* const t33 = t3->neighbor(3);
        if (! t33) {
          if (! seeds.hasMember((TLink*) t3))
            outList.append((TLink*) t3);
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(outList, "breif", Tab() + "outList:");
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
    return outList;
  }

} // namespace Belle

