//-----------------------------------------------------------------------------
// $Id: THoughFinder.cc 11152 2010-04-28 01:24:38Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : THoughFinder.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the Hough method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.12  2006/02/21 23:18:05  katayama
// max,cout
//
// Revision 1.11  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.9  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.8  2005/04/11 23:00:45  yiwasaki
// <sys/ddi.h> -> <cmath>
//
// Revision 1.7  2005/04/02 23:11:51  yiwasaki
// To fix compile errors with gcc3
//
// Revision 1.6  2005/03/11 03:57:51  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.5  2004/04/15 05:34:09  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.4  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.3  2004/02/18 04:07:26  yiwasaki
// Trasan 3.09 : Option to simualte the small cell CDC with the normal cell CDC added, new classes for Hough finder added
//
// Revision 1.2  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.1  2003/12/19 07:36:03  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
//-----------------------------------------------------------------------------





#include <cmath>
#include <cstdlib>
#include "tracking/modules/trasan/Strings.h"
#include "trg/cdc/TRGCDC.h"
#include "tracking/modules/trasan/THoughFinder.h"
#include "tracking/modules/trasan/THoughPlane.h"
#include "tracking/modules/trasan/THoughPlaneMulti.h"
#include "tracking/modules/trasan/THoughTransformationCircle.h"
#include "tracking/modules/trasan/THoughTransformationCircleGeneral.h"
#include "tracking/modules/trasan/TPeakFinder.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TTrack.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TDebugUtilities.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#endif
#ifdef TRASAN_WINDOW_GTK_HOUGH
#include "tracking/modules/trasan/Trasan.h"
#include "tracking/modules/trasan/TWindowGTKHough.h"
#endif

namespace Belle {

#ifdef TRASAN_WINDOW_GTK_HOUGH
  AList<TLink> AXIAL_ALL;
  AList<TLink> STEREO_ALL;
#endif
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
#include "tracking/modules/trasan/TWindowHough.h"
  TWindow Window("Hough Window");
  TWindowHough HWindow("circle Hough");
  TWindowHough HPWindow("circle Hough pluse");
  TWindowHough HMWindow("circle Hough minus");
  TWindowHough LWindow("line Hough plan");
  TWindowHough RWindow0("R 0", 600, 600);
  TWindowHough RWindow1("R 1", 600, 600);
  TWindowHough RWindow2("R 2", 600, 600);
  TWindowHough RWindow3("R 3", 600, 600);
  TWindowHough RWindow4("R 4", 600, 600);
  TWindowHough RWindow5("R 5", 600, 600);
  TWindowHough RWindow6("R 6", 600, 600);
  TWindowHough RWindow7("R 7", 600, 600);
  TWindowHough RWindow8("R 8", 600, 600);
  TWindowHough RWindow9("R 9", 600, 600);
  TWindowHough RWindow10("R 10", 600, 600);
  TWindowHough RWindow11("R 11", 600, 600);
  TWindowHough RWindow12("R 12", 600, 600);
  TWindowHough RWindow13("R 13", 600, 600);
  TWindowHough RWindow14("R 14", 600, 600);
  TWindowHough RWindow15("R 15", 600, 600);
  TWindowHough RWindow16("R 16", 600, 600);
  TWindowHough RWindow17("R 17", 600, 600);
  TWindowHough RWindow18("R 18", 600, 600);
  TWindowHough RWindow19("R 19", 600, 600);
#endif

  THoughFinder::THoughFinder(int doCurlSearch,
                             float axialLoadWidth,
                             float axialLoadWidthCurl,
                             float maxSigma,
                             float maxSigmaStereo,
                             float salvageLevel,
                             float szLinkDistance,
                             unsigned fittingFlag,
                             float stereoLoadWidth,
                             float salvageLoadWidth,
                             int meshX,
                             int meshY,
                             float ptBoundary,
                             float threshold,
                             int meshXLowPt,
                             int meshYLowPt,
                             float ptBoundaryLowPt,
                             float thresholdLowPt,
                             int mode)
    : _doCurlSearch(doCurlSearch),
      _axialLoadWidth(axialLoadWidth),
      _axialLoadWidthCurl(axialLoadWidthCurl),
      _stereoLoadWidth(stereoLoadWidth),
      _salvageLoadWidth(salvageLoadWidth),
//    _minThreshold(10),
      _minThreshold(7),
//    _minThreshold(5),
//    _minThreshold(6),
      _minThresholdLowPt(4),
      _meshX(meshX),
      _meshY(meshY),
      _ptBoundary(ptBoundary),
      _threshold(threshold),
      _meshXLowPt(meshXLowPt),
      _meshYLowPt(meshYLowPt),
      _ptBoundaryLowPt(ptBoundaryLowPt),
      _thresholdLowPt(thresholdLowPt),
      _mode(mode),
      _builder("hough builder",
               maxSigma,
               maxSigmaStereo,
               salvageLevel,
               szLinkDistance,
               fittingFlag),
      _peakFinder(),
      _circleHough("Circle Hough"),
      _ptBoundaryInHough(_circleHough.y(_ptBoundary, 0., 0.)),
//       _planeHP("circle Hough:high pt and pluse charge",
//         _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
//         3, 50),
//       _planeHM("circle Hough:high pt and minus charge",
//         _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
//         3, 50),
//       _tmp("circle Hough:high pt and minus charge",
//         _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
//         3, 50),
//       _planeHP2("circle Hough:high pt and pluse charge",
//    _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
//    3, 50),
//       _planeHM2("circle Hough:high pt and minus charge",
//    _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
//    3, 50),
      _planeHP(NULL),
      _planeHM(NULL),
      _planeHP2(NULL),
      _planeHM2(NULL),
      _tmp(NULL),
      _nHits2D(0),
      _nHits3D(0),
      _nHitsG(0)
  {
//     _planes0[0] = & _planeHP;
//     _planes0[1] = & _planeHM;
//     _planes[0] = & _planeHP;
//     _planes[1] = & _planeHM;

//     _planes02[0] = & _planeHP2;
//     _planes02[1] = & _planeHM2;
//     _planes2[0] = & _planeHP2;
//     _planes2[1] = & _planeHM2;

    _planes0[0] = NULL;
    _planes0[1] = NULL;
    _planes[0] = NULL;
    _planes[1] = NULL;

    _planes02[0] = NULL;
    _planes02[1] = NULL;
    _planes2[0] = NULL;
    _planes2[1] = NULL;
  }

  THoughFinder::~THoughFinder()
  {
    if (_nHits2D)
      delete[] _nHits2D;
    if (_nHits3D)
      delete[] _nHits3D;
    if (_nHitsG)
      delete[] _nHitsG;

    if (_planeHP != NULL) delete _planeHP;
    if (_planeHM != NULL) delete _planeHM;
    if (_planeHP2 != NULL) delete _planeHP2;
    if (_planeHM2 != NULL) delete _planeHM2;
    if (_tmp != NULL) delete _tmp;
  }

  std::string
  THoughFinder::version(void) const
  {
    return "4.08";
  }

  void
  THoughFinder::dump(const std::string&, const std::string&) const
  {
  }

  void
  THoughFinder::clear(void)
  {
    HepAListDeleteAll(_all);
    _axial.removeAll();
    _stereo.removeAll();
  }

  void
  THoughFinder::selectGoodHits(const CAList<Belle2::TRGCDCWireHit> & axial,
                               const CAList<Belle2::TRGCDCWireHit> & stereo)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "selectGoodHits";
    EnterStage(stage);
#endif

    const unsigned na = axial.length();
    for (unsigned i = 0; i < na; i++) {
      const Belle2::TRGCDCWireHit& h = * axial[i];
      const HepGeom::Point3D<double> & p = h.xyPosition();
      _axial.append(new TLink(0, & h, p));
    }

    const unsigned ns = stereo.length();
    for (unsigned i = 0; i < ns; i++) {
      const Belle2::TRGCDCWireHit& h = * stereo[i];
      const HepGeom::Point3D<double> & p = h.xyPosition();
      _stereo.append(new TLink(0, & h, p));
    }

    //...Set neighbors (assuming ordered list)...
    TLink* l0 = 0;
    for (unsigned i = 0; i < na; i++) {
      if (i < na - 1) {
        TLink& t = * _axial[i];
        TLink& u = * _axial[i + 1];
        const Belle2::TRGCDCWire& wt = * t.wire();
        const Belle2::TRGCDCWire& wu = * u.wire();
        if (wt.layerId() != wu.layerId())
          continue;
        const unsigned idt = wt.localId();
        const unsigned idu = wu.localId();
        if (idu == idt + 1) {
          t.neighbor(3, & u);
          u.neighbor(2, & t);
        }
        if (idt == 0) {
          l0 = & t;
        } else if (idu == wu.layer().nCells() - 1) {
          if (l0) {
            if (l0->wire()->layerId() == wu.layerId()) {
              u.neighbor(3, l0);
              l0->neighbor(2, & u);
            }
          }
        }
      }
    }

    l0 = 0;
    for (unsigned i = 0; i < ns; i++) {
      if (i < ns - 1) {
        TLink& t = * _stereo[i];
        TLink& u = * _stereo[i + 1];
        const Belle2::TRGCDCWire& wt = * t.wire();
        const Belle2::TRGCDCWire& wu = * u.wire();
        if (wt.layerId() != wu.layerId())
          continue;
        const unsigned idt = wt.localId();
        const unsigned idu = wu.localId();
        if (idu == idt + 1) {
          t.neighbor(3, & u);
          u.neighbor(2, & t);
        }
        if (idt == 0) {
          l0 = & t;
        } else if (idu == wu.layer().nCells() - 1) {
          if (l0) {
            if (l0->wire()->layerId() == wu.layerId()) {
              u.neighbor(3, l0);
              l0->neighbor(2, & u);
            }
          }
        }
      }
    }

    _all.append(_axial);
    _all.append(_stereo);
#ifdef TRASAN_WINDOW_GTK_HOUGH
    AXIAL_ALL.removeAll();
    AXIAL_ALL.append(_axial);
    STEREO_ALL.removeAll();
    STEREO_ALL.append(_stereo);
#endif

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "given hits:axial=" << na
              << ",stereo=" << ns << std::endl
              << Tab() << "good hits:axial="
              << _axial.length() << ",stereo=" << _stereo.length() << std::endl
              << Tab() << "fitting valid:axial="
              << TLink::cores(_axial).length() << ",stereo="
              << TLink::cores(_stereo).length() << std::endl;
    TLink::dump(_axial, "mc sort flag", Tab());
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_pink);
    Window.append(_stereo, leda_black);
    Window.text("THoughFinder::selectGoodHits:After selection of good hits");
    Window.wait();
#endif
  }

  void
  THoughFinder::houghTransformation(const AList<TLink> & hits,
                                    const THoughTransformation& trans,
                                    THoughPlane& plane) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "houghTransformation";
    EnterStage(stage);
#endif
    const unsigned n = hits.length();
    for (unsigned i = 0; i < n; i++) {
      const TLink& l = * hits[i];
      const float x = l.xyPosition().x();
      const float y = l.xyPosition().y();
      plane.vote(x, y, trans);
    }
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  void
  THoughFinder::houghTransformation(const AList<TLink> & hits,
                                    const THoughTransformation& trans,
                                    float charge,
                                    THoughPlane& plane,
//          unsigned weight,
                                    int weight,
                                    const TPoint2D& o) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "houghTransformation";
    EnterStage(stage);
#endif
    const unsigned n = hits.length();
    for (unsigned i = 0; i < n; i++) {
      const TLink& l = * hits[i];
      const float x = l.xyPosition().x() - o.x();
      const float y = l.xyPosition().y() - o.y();
      plane.vote(x, y, int(charge), trans, weight);
    }
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  void
  THoughFinder::houghTransformation(const AList<TLink> & hits,
                                    const THoughTransformation& trans,
                                    float charge,
                                    THoughPlaneMulti& plane,
//          unsigned weight,
                                    int weight,
                                    const TPoint2D& o) const
  {
    const unsigned n = hits.length();
    for (unsigned i = 0; i < n; i++) {
      const TLink& l = * hits[i];
      const unsigned layerId = l.wire()->layerId();
      const float x = l.xyPosition().x() - o.x();
      const float y = l.xyPosition().y() - o.y();
      plane.vote(x, y, charge, trans, weight, layerId);
    }
    plane.merge();
  }

  void
  THoughFinder::salvage(TTrack& t) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "salvage";
    EnterStage(stage);
#endif
    AList<TLink> all;
    all.append(_axial);
    all.append(_stereo);
    AList<TLink> links = pickUpLinks2(t, all, _salvageLoadWidth, 3);

    //...Salvage...
    _builder.salvage(t, links);
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

TTrack *
THoughFinder::build2D(const TCircle &,
		      const AList<TLink> & aLinks) const {
//THoughFinder::build2D(const TCircle& circle,
//		      const AList<TLink> & aLinks) const {
#ifdef TRASAN_DEBUG
    const std::string stage = "build2D";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    TLink::dump(aLinks, "", Tab() + "Seed links=");
#endif

    TTrack* t = _builder.buildRphi(aLinks);

#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(aLinks, leda_pink);
    if (t) {
	Window.append(circle);
	Window.append(* t, leda_green);
	Window.text(Stage() + ":2D track is made");
    }
    else {
	Window.append(circle, leda_red);
	Window.text(Stage() + ":failed to make 2D trk");
    }
    Window.wait();
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return t;
}

  TTrack*
  THoughFinder::build3D(TTrack& t, const AList<TLink> & sLinks) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "build3D";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "3D Building for " << t.name()
              << std::endl;
    TLink::dump(sLinks, "sort", Tab() + "Seed stereo link ");
#endif
#ifdef TRASAN_WINDOW_GTK_HOUGH
    TWindowGTKConformal& w = Trasan::getTrasan()->w();
    w.clear();
//  w.skip(false);
    w.stage("Hough Finder : before 3D building");
    w.information("gray:all hits, green:candidates to be built");
    w.append(AXIAL_ALL, Gdk::Color("gray"));
    w.append(STEREO_ALL, Gdk::Color("gray"));
    w.append(sLinks, Gdk::Color("green"));
    AList<TTrack> tmp;
    tmp.append(t);
    w.append(tmp, Gdk::Color("green"));
    w.run();
    AList<TLink> tmp2;
    tmp2.append(sLinks);
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_stereo, leda_grey2);
    Window.append(sLinks, leda_pink);
    Window.append(t, leda_green);
    Window.text(Stage() + "candidate hits for 3D building");
    Window.stereo(true);
    Window.wait();
#endif

    TTrack* s = _builder.buildStereo(t, sLinks);

#ifdef TRASAN_WINDOW_GTK_HOUGH
    w.clear();
//  w.skip(false);
    w.stage("Hough Finder : after 3D building");
    w.information("gray:all hits, green:used candidates, blue:3D track, red:unused candidates");
    w.append(AXIAL_ALL, Gdk::Color("gray"));
    w.append(STEREO_ALL, Gdk::Color("gray"));
    w.append(sLinks, Gdk::Color("green"));
    if (s)
      tmp2.remove(s->links());
    w.append(tmp2, Gdk::Color("red"));
    if (s)
      tmp.append(s);
    if (s)
      w.append(tmp, Gdk::Color("blue"));
    else
      w.append(tmp, Gdk::Color("red"));
    w.run();
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(sLinks, leda_pink);
//  Window.append(circle);
    if (s) {
      Window.append(* s, leda_green);
      Window.text(Stage() + ":3D track is made");
    } else {
      Window.skip(false);
      Window.append(t, leda_red);
      Window.text(Stage() + ":3D track is failed");
    }
    Window.wait();
    Window.stereo(false);
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return s;
  }

  void
  THoughFinder::removeUsedHits(const TTrack& t)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "removeUsedHits";
    EnterStage(stage);
#endif

    const unsigned n0 = t.nLinks();
    const AList<TLink> & links = t.links();

    _axial.remove(links);

    AList<TLink> removeList;
    const unsigned n1 = _stereo.length();
    for (unsigned i = 0; i < n0; i++) {
      for (unsigned j = 0; j < n1; j++) {
        if (links[i]->hit() == _stereo[j]->hit())
          removeList.append(_stereo[j]);
      }
    }
    _stereo.remove(removeList);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  int
  SortByY(const TPoint2D** a, const TPoint2D** b)
  {
    if ((* a)->y() < (* b)->y())
      return 1;
    else if ((* a)->y() == (* b)->y())
      return 0;
    else
      return -1;
  }

  int
  SortByYReverse(const TPoint2D** a, const TPoint2D** b)
  {
    if ((* a)->y() < (* b)->y())
      return 0;
    else if ((* a)->y() == (* b)->y())
      return 1;
    else
      return -1;
  }

TTrack *
THoughFinder::build0(const TPoint2D & point,
//                   THoughPlane * planes[2][2],
		     THoughPlane **,
		     float charge,
		     unsigned,
		     unsigned) {
#ifdef TRASAN_DEBUG
    const std::string stage = "build0";
    EnterStage(stage);
#endif

    //...Make circle...
    TPoint2D circleCenter = _circleHough.circleCenter(point);
    const TCircle circle(circleCenter.y(), circleCenter.x(), charge);

    //...Get axial hits...
    float axialLoadWidth = _axialLoadWidth;
    if (circle.pt() < 0.2) axialLoadWidth = _axialLoadWidthCurl;
//  AList<TLink> aLinks0 = pickUpLinks2(circle, _axial, axialLoadWidth, 1);
    AList<TLink> aLinks0 = pickUpLinksDetail(circle,
                                             _axial, axialLoadWidth, 1);

    //...Adjust hits...
    AList<TLink> aLinks = adjustAxialLinks(aLinks0);

    //...Require at least two super layers...
    const unsigned nSuperLayers = TLink::nSuperLayers(aLinks);
    if (nSuperLayers < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#superlayers<2" << std::endl;
      std::cout << Tab() << "    " << TLink::layerUsage(aLinks) << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial);
      Window.append(aLinks0, leda_pink);
      Window.append(aLinks, leda_red);
      Window.append(circle, leda_red);
      Window.text(Stage() + ":#superlayers<2");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track building (2D)...
    TTrack* t = build2D(circle, aLinks);
    if (! t) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:no track built"
                << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track check : charge...
    if (t->charge() != charge) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:charge flip" << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":charge flip");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

//     //...Track check : hough peak matching...
//     if (! goodTrackHoughMatch(* planes[pt][0],
//            * planes[pt][1],
//            _circleHough,
//            * t,
//            threshold)) {
// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << ">>> track rejected:no peak in hough"
//      << std::endl;
// #endif
// #ifdef TRASAN_WINDOW
//  Window.clear();
//  Window.append(_axial, leda_grey2);
//  Window.append(circle);
//  Window.append(* t, leda_red);
//  Window.text(Stage() + ":no peak in hough");
//  Window.wait();
// #endif
// #ifdef TRASAN_DEBUG
//  LeaveStage(stage);
// #endif
//  delete t;
//  return 0;
//     }

    //...Final check...
    if (! goodTrack(* t)) {
//     bool goodTrackHough = goodTrack(* t);
//     bool goodTrackManager = TTrackManager::goodTrack(* t);
//     if ((! goodTrackHough) || (! goodTrackManager)) {
#ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << ">>> track rejected:goodTrackHough=" << goodTrackHough << ",goodTrackManager=" << goodTrackManager << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":^goodTrack()");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(circle);
    Window.append(* t, leda_green);
    Window.text(Stage() + ":track check OK");
    Window.wait();
#endif

    //...2D hit pattern...
//  static unsigned * nHits2D = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
//  unsigned nHits2D[11];
    TLink::nHitsSuperLayer(t->links(), _nHits2D);

    //...Get stereo hits...
    AList<TLink> sLinks = pickUpLinks2(circle,
                                       _stereo, _stereoLoadWidth, 2);

    //...Track building (3D)...
    TTrack* s = build3D(* t, sLinks);
    if (! s) {
      removeUsedHits(* t);
      t->assign(CellHitHoughFinder);
      t->quality(TrackQuality2D);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "Hough 2D Track made:" << t->name() << std::endl;
      t->dump("brief");
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return t;
    }
    delete t;

    //...Quality check...
    // static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
    // static unsigned * nHits3D = new unsigned[nsl];
    // TLink::nHitsSuperLayer(s->links(), nHits3D);
    TLink::nHitsSuperLayer(s->links(), _nHits3D);
    for (unsigned i = 0; i < _nsl; i++) {
      if (i % 2) continue;
      if (_nHits2D[i] > 2) {
        if (! _nHits3D[i]) {
#ifdef TRASAN_DEBUG
          std::cout << Tab() << ">>> track rejected:bad quality"
                    << std::endl;
          std::cout << Tab() << "    2D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits2D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
          std::cout << Tab() << "    3D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits3D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
#endif
#ifdef TRASAN_WINDOW
          Window.clear();
          Window.append(_axial, leda_grey2);
          Window.append(_stereo, leda_grey2);
          Window.append(circle);
          Window.append(* s, leda_red);
          Window.text(Stage() + ":2D track(3D failed)");
          Window.wait();
#endif
          delete s;
#ifdef TRASAN_DEBUG
          LeaveStage(stage);
#endif
          return 0;
        }
      }
    }

    //...Remove used hits...
    removeUsedHits(* s);

    //...Salvaging...
    salvage(* s);
    removeUsedHits(* s);

    //...Store a track...
    s->assign(CellHitHoughFinder);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "Hough Track made:" << s->name() << std::endl;
    s->dump("detail", Tab());
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(_stereo, leda_grey2);
    Window.append(circle);
    Window.append(* s, leda_green);
    Window.text(Stage() + ":Final 3D track");
    Window.wait();
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return s;
  }

TTrack *
THoughFinder::build1(const TPoint2D & point,
//		     THoughPlane * planes[2][2],
		     float charge,
		     unsigned,
		     unsigned) {
#ifdef TRASAN_DEBUG
    const std::string stage = "build1";
    EnterStage(stage);
#endif

    //...Make circle...
    TPoint2D circleCenter = _circleHough.circleCenter(point);
    const TCircle circle(circleCenter.y(), circleCenter.x(), charge);

    //...Get axial hits...
    float axialLoadWidth = _axialLoadWidth;
    if (circle.pt() < 0.2) axialLoadWidth = _axialLoadWidthCurl;
//  AList<TLink> aLinks0 = pickUpLinks2(circle, _axial, axialLoadWidth, 1);
    AList<TLink> aLinks0 = pickUpLinksDetail(circle,
                                             _axial, axialLoadWidth, 1);

    //...Adjust hits...
    AList<TLink> aLinks = adjustAxialLinks(aLinks0);

//...Special code to check builder...
// std::cout << "!!! this part is a special code for debugging" << std::endl;
//     AList<TLink> tmp;
//     for (unsigned i = 0; i < aLinks0.length(); i++)
//  if (aLinks[i]->hit()->mc()->hep()->id() != 22)
//      tmp.append(aLinks[i]);
//     aLinks.remove(tmp);
//...Special code end...

    //...Require at least two super layers...
    const unsigned nSuperLayers = TLink::nSuperLayers(aLinks);
    if (nSuperLayers < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#superlayers<2" << std::endl;
      std::cout << Tab() << "    " << TLink::layerUsage(aLinks) << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial);
      Window.append(aLinks0, leda_pink);
      Window.append(aLinks, leda_red);
      Window.append(circle, leda_red);
      Window.text(Stage() + ":#superlayers<2");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track building (2D)...
    TTrack* t = build2D(circle, aLinks);
    if (! t) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:no track built"
                << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track check : charge...
    if (t->charge() != charge) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:charge flip" << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":charge flip");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

//     //...Track check : hough peak matching...
//     if (! goodTrackHoughMatch(* planes[pt][0],
//            * planes[pt][1],
//            _circleHough,
//            * t,
//            threshold)) {
// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << ">>> track rejected:no peak in hough"
//      << std::endl;
// #endif
// #ifdef TRASAN_WINDOW
//  Window.clear();
//  Window.append(_axial, leda_grey2);
//  Window.append(circle);
//  Window.append(* t, leda_red);
//  Window.text(Stage() + ":no peak in hough");
//  Window.wait();
// #endif
// #ifdef TRASAN_DEBUG
//  LeaveStage(stage);
// #endif
//  delete t;
//  return 0;
//     }

    //...Final check...
    if (! goodTrack(* t)) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:^goodTrack()" << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":^goodTrack()");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(circle);
    Window.append(* t, leda_green);
    Window.text(Stage() + ":track check OK");
    Window.wait();
#endif

    //...2D hit pattern...
//  static unsigned * nHits2D = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
//  unsigned nHits2D[11];
    TLink::nHitsSuperLayer(t->links(), _nHits2D);

    //...Get stereo hits...
    AList<TLink> sLinks = pickUpLinks2(circle,
                                       _stereo, _stereoLoadWidth, 2);

    //...Track building (3D)...
    TTrack* s = build3D(* t, sLinks);
    if (! s) {
      removeUsedHits(* t);
      t->assign(CellHitHoughFinder);
      t->quality(TrackQuality2D);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "Hough 2D Track made:" << t->name() << std::endl;
      t->dump("brief");
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return t;
    }
    delete t;

    //...Quality check...
    // static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
    // static unsigned * nHits3D = new unsigned[nsl];
    // TLink::nHitsSuperLayer(s->links(), nHits3D);
    TLink::nHitsSuperLayer(s->links(), _nHits3D);
    for (unsigned i = 0; i < _nsl; i++) {
      if (i % 2) continue;
      if (_nHits2D[i] > 2) {
        if (! _nHits3D[i]) {
#ifdef TRASAN_DEBUG
          std::cout << Tab() << ">>> track rejected:bad quality"
                    << std::endl;
          std::cout << Tab() << "    2D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits2D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
          std::cout << Tab() << "    3D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits3D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
#endif
#ifdef TRASAN_WINDOW
          Window.clear();
          Window.append(_axial, leda_grey2);
          Window.append(_stereo, leda_grey2);
          Window.append(circle);
          Window.append(* s, leda_red);
          Window.text(Stage() + ":2D track(3D failed)");
          Window.wait();
#endif
          delete s;
#ifdef TRASAN_DEBUG
          LeaveStage(stage);
#endif
          return 0;
        }
      }
    }

    //...Remove used hits...
    removeUsedHits(* s);

    //...Salvaging...
    salvage(* s);
    removeUsedHits(* s);

    //...Store a track...
    s->assign(CellHitHoughFinder);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "Hough Track made:" << s->name() << std::endl;
    s->dump("brief", Tab());
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(_stereo, leda_grey2);
    Window.append(circle);
    Window.append(* s, leda_green);
    Window.text(Stage() + ":Final 3D track");
    Window.wait();
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return s;
  }

TTrack *
THoughFinder::build3(const TPoint2D & point,
//		     THoughPlane *,
		     float charge,
		     unsigned,
		     unsigned) {
#ifdef TRASAN_DEBUG
    const std::string stage = "build3";
    EnterStage(stage);
#endif

    //...Make circle...
    TPoint2D circleCenter = _circleHough.circleCenter(point);
    const TCircle circle(circleCenter.y(), circleCenter.x(), charge);

    //...Get axial hits...
    float axialLoadWidth = _axialLoadWidth;
    if (circle.pt() < 0.2) axialLoadWidth = _axialLoadWidthCurl;
    AList<TLink> aLinks0 = pickUpLinksDetail(circle,
                                             _axial,
                                             axialLoadWidth,
                                             1);
    AList<TLink> aLinks1 = pickUpNeighborLinks(aLinks0, _axial);
#ifdef TRASAN_WINDOW_GTK_HOUGH
    AList<TCircle> cList;
    cList.append((TCircle&) circle);
    TWindowGTKConformal& w = Trasan::getTrasan()->w();
    w.clear();
    w.stage("Hough Finder : Picking up links");
    w.information("gray:all hits, green:by Detail, blue:by Neighbor");
    w.append(cList, Gdk::Color("green"));
    w.append(_axial, Gdk::Color("gray"));
    w.append(aLinks0, Gdk::Color("green"));
    w.append(aLinks1, Gdk::Color("blue"));
    w.run();
#endif

    //...Adjust hits...
    aLinks0.append(aLinks1);
    AList<TLink> aLinks = adjustAxialLinks(aLinks0);

    //...Require at least two super layers...
    const unsigned nSuperLayers = TLink::nSuperLayers(aLinks);
    if (nSuperLayers < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#superlayers<2" << std::endl;
      std::cout << Tab() << "    " << TLink::layerUsage(aLinks) << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial);
      Window.append(aLinks0, leda_pink);
      Window.append(aLinks, leda_red);
      Window.append(circle, leda_red);
      Window.text(Stage() + ":#superlayers<2");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track building (2D)...
    TTrack* t = build2D(circle, aLinks);
    if (! t) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:no track built"
                << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track check : charge...
    if (t->charge() != charge) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:charge flip" << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":charge flip");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

//     //...Track check : hough peak matching...
//     if (! goodTrackHoughMatch(* planes[pt][0],
//            * planes[pt][1],
//            _circleHough,
//            * t,
//            threshold)) {
// #ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << ">>> track rejected:no peak in hough"
//      << std::endl;
// #endif
// #ifdef TRASAN_WINDOW
//  Window.clear();
//  Window.append(_axial, leda_grey2);
//  Window.append(circle);
//  Window.append(* t, leda_red);
//  Window.text(Stage() + ":no peak in hough");
//  Window.wait();
// #endif
// #ifdef TRASAN_DEBUG
//  LeaveStage(stage);
// #endif
//  delete t;
//  return 0;
//     }

    //...Final check...
    if (! goodTrack(* t)) {
//     bool goodTrackHough = goodTrack(* t);
//     bool goodTrackManager = TTrackManager::goodTrack(* t);
//     if ((! goodTrackHough) || (! goodTrackManager)) {
#ifdef TRASAN_DEBUG_DETAIL
//  std::cout << Tab() << ">>> track rejected:goodTrackHough=" << goodTrackHough << ",goodTrackManager=" << goodTrackManager << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":^goodTrack()");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(circle);
    Window.append(* t, leda_green);
    Window.text(Stage() + ":track check OK");
    Window.wait();
#endif

    //...2D hit pattern...
//  static unsigned * nHits2D = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
//  unsigned nHits2D[11];
    TLink::nHitsSuperLayer(t->links(), _nHits2D);

    //...Get stereo hits...
    AList<TLink> sLinks = pickUpLinks2(circle,
                                       _stereo, _stereoLoadWidth, 2);

    //...Track building (3D)...
    TTrack* s = build3D(* t, sLinks);
    if (! s) {
      removeUsedHits(* t);
      t->assign(CellHitHoughFinder);
      t->quality(TrackQuality2D);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "Hough 2D Track made:" << t->name() << std::endl;
      t->dump("brief");
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return t;
    }
    delete t;

    //...Quality check...
    // static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
    // static unsigned * nHits3D = new unsigned[nsl];
    // TLink::nHitsSuperLayer(s->links(), nHits3D);
    TLink::nHitsSuperLayer(s->links(), _nHits3D);
    for (unsigned i = 0; i < _nsl; i++) {
      if (i % 2) continue;
      if (_nHits2D[i] > 2) {
        if (! _nHits3D[i]) {
#ifdef TRASAN_DEBUG
          std::cout << Tab() << ">>> track rejected:bad quality"
                    << std::endl;
          std::cout << Tab() << "    2D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits2D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
          std::cout << Tab() << "    3D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits3D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
#endif
#ifdef TRASAN_WINDOW
          Window.clear();
          Window.append(_axial, leda_grey2);
          Window.append(_stereo, leda_grey2);
          Window.append(circle);
          Window.append(* s, leda_red);
          Window.text(Stage() + ":2D track(3D failed)");
          Window.wait();
#endif
          delete s;
#ifdef TRASAN_DEBUG
          LeaveStage(stage);
#endif
          return 0;
        }
      }
    }

    //...Remove used hits...
    removeUsedHits(* s);

    //...Salvaging...
    salvage(* s);
    removeUsedHits(* s);

    //...Store a track...
    s->assign(CellHitHoughFinder);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "Hough Track made:" << s->name() << std::endl;
    s->dump("detail", Tab());
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(_stereo, leda_grey2);
    Window.append(circle);
    Window.append(* s, leda_green);
    Window.text(Stage() + ":Final 3D track");
    Window.wait();
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return s;
  }

TTrack *
THoughFinder::buildCurl(const TPoint2D & point,
			const THoughPlane &,
			float charge,
			unsigned,
			AList<TLink> & hits) {
#ifdef TRASAN_DEBUG
    const std::string stage = "buildCurl";
    EnterStage(stage);
#endif

    //...Make circle...
    TPoint2D circleCenter = _circleHough.circleCenter(point);
    const TCircle circle(circleCenter.y(), circleCenter.x(), charge);

    //...Get axial hits...
    AList<TLink> aLinks0 = pickUpLinksDetail(circle,
                                             _axial, _axialLoadWidthCurl, 1);

    //...Adjust hits...
    AList<TLink> aLinks = adjustAxialLinks(aLinks0);

    //...Require at least two super layers...
    const unsigned nSuperLayers = TLink::nSuperLayers(aLinks);
    if (nSuperLayers < 2) {
      hits.append(aLinks);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#superlayers="
                << nSuperLayers << "<2" << std::endl;
      std::cout << Tab() << "    links=";
      TLink::dump(hits, "breif");
      std::cout << Tab() << "    orign=";
      TLink::dump(aLinks0, "breif");
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(aLinks0, leda_pink);
      Window.append(aLinks0, leda_red);
      Window.append(circle, leda_red);
      Window.text(Stage() + ":#superlayers<2");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track building (2D)...
    TTrack* t = build2D(circle, aLinks);
    if (! t) {
      hits.append(aLinks);
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Track check...
    if (! goodTrack(* t)) {
      hits.append(t->links());
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:^goodTrack()" << std::endl;
#endif
#ifdef TRASAN_WINDOW
      Window.clear();
      Window.append(_axial, leda_grey2);
      Window.append(circle);
      Window.append(* t, leda_red);
      Window.text(Stage() + ":^goodTrack()");
      Window.wait();
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      delete t;
      return 0;
    }

    //...2D hit pattern...
//  static unsigned * nHits2D = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
//  unsigned nHits2D[11];
//  TLink::nHitsSuperLayer(t->links(), nHits2D);
    TLink::nHitsSuperLayer(t->links(), _nHits2D);

    //...Get stereo hits...
    AList<TLink> sLinks = pickUpLinks2(circle, _stereo, _stereoLoadWidth, 2);

    //...Track building (3D)...
    TTrack* s = build3D(* t, sLinks);
    if (! s) {
      if (t->cores().length() < 5) {
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << ">>> track rejected:2D Track quality poor"
                  << ":" << t->name() << std::endl;
        t->dump("brief", Tab());
#endif
#ifdef TRASAN_DEBUG
        LeaveStage(stage);
#endif
//      hits.append(t->links());
        delete t;
        return 0;
      }
      removeUsedHits(* t);
      t->assign(CellHitHoughFinder);
      t->quality(TrackQuality2D);
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "Hough 2D Track made:" << t->name() << std::endl;
      t->dump("brief");
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return t;
    }
    delete t;

    //...Quality check...
//  static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
//  static unsigned * nHits3D = new unsigned[nsl];
//  unsigned nHits3D[11];
    TLink::nHitsSuperLayer(s->links(), _nHits3D);
    for (unsigned i = 0; i < _nsl; i++) {
      if (i % 2) continue;
      if (_nHits2D[i] > 2) {
        if (! _nHits3D[i]) {
#ifdef TRASAN_DEBUG
          std::cout << Tab() << ">>> track rejected:bad quality"
                    << std::endl;
          std::cout << Tab() << "    2D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits2D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
          std::cout << Tab() << "    3D:";
          for (unsigned i = 0; i < _nsl; i++) {
            std::cout << _nHits3D[i];
            if (i != 10) std::cout << ",";
          }
          std::cout << std::endl;
#endif
#ifdef TRASAN_WINDOW
          Window.clear();
          Window.append(_axial, leda_pink);
          Window.append(_stereo, leda_pink);
          Window.append(circle);
          Window.append(* s, leda_red);
          Window.text("THoughFinder::build:3D bad quality");
          Window.wait();
#endif
#ifdef TRASAN_DEBUG
          LeaveStage(stage);
#endif
          delete s;
          return 0;
        }
      }
    }

    //...Remove used hits...
    removeUsedHits(* s);

    //...Salvaging...
    salvage(* s);
    removeUsedHits(* s);

    //...Store a track...
    s->assign(CellHitHoughFinder);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "Hough Track made:" << s->name() << std::endl;
    s->dump("brief", Tab());
#endif
#ifdef TRASAN_WINDOW
    Window.clear();
    Window.append(_axial, leda_grey2);
    Window.append(_stereo, leda_grey2);
    Window.append(circle);
    Window.append(* s, leda_green);
    Window.text("THoughFinder::build : Final 3D track");
    Window.wait();
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return s;
  }

  bool
  THoughFinder::goodTrack(const TTrack& t) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "goodTrack(trk)";
    EnterStage(stage);
#endif

    if (t.pt() < 0.1) {
      const bool tmp = goodTrackLowPt(t);
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return tmp;
    }

    const bool tmp = goodTrack(t.links());
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
    return tmp;
  }

  bool
  THoughFinder::goodTrack(const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "goodTrack(lnks)";
    EnterStage(stage);
#endif

    //...Require at least two super layers...
    const unsigned nSuperLayers = TLink::nSuperLayers(list);
    if (nSuperLayers < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#superlayers<2" << std::endl;
      std::cout << Tab() << "    " << TLink::layerUsage(list) << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return false;
    }

    //...Require at least two hits per axial super layer...
//  static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
//  static unsigned * nHits = new unsigned[nsl];
    TLink::nHitsSuperLayer(list, _nHitsG);
    for (unsigned i = 0; i < _nsl; i++) {
      if (i % 2) continue;
      if (_nHitsG[i] < 2)
        _nHitsG[i] = 0;
    }

    //...Require at least two adjucent axial super layers...
    unsigned nMax = 0;
    unsigned n = 0;
    for (unsigned i = 0; i < _nsl; i++) {
      if (i % 2) continue;
      if (_nHitsG[i]) ++n;
      else n = 0;
      if (n > nMax) nMax = n;
    }
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
    if (nMax < 2) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#adjucent superlayer"
                << "(nHit>1)<2" << std::endl;
      std::cout << Tab() << "    " << TLink::layerUsage(list) << std::endl;
#endif
      return false;
    }
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << ":#adjucent superlayer is enough" << std::endl;
    std::cout << Tab() << TLink::layerUsage(list) << std::endl;
#endif
    return true;
  }

  bool
  THoughFinder::goodTrackLowPt(const TTrack& t) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "goodTrackLowPt";
    EnterStage(stage);
#endif

    //...Require at least two hits per axial super layer...
    static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
    static unsigned* nHits = new unsigned[nsl];
//  unsigned nHits[11];
    TLink::nHitsSuperLayer(t.links(), nHits);
    for (unsigned i = 0; i < nsl; i++) {
      if (i % 2) continue;
      if (nHits[i] < 2)
//  if (nHits[i] < 5)
        nHits[i] = 0;
    }

    //...Require at least two adjucent axial super layers...
    unsigned nMax = 0;
    unsigned n = 0;
    for (unsigned i = 0; i < nsl; i++) {
      if (i % 2) continue;
      if (nHits[i]) ++n;
      else n = 0;
      if (n > nMax) nMax = n;
    }
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    if (nMax < 1) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:#adjucent superlayer"
                << "(nHit>1)<1" << std::endl;
#endif
      return false;
    }
    return true;
  }

  AList<TLink>
  THoughFinder::adjustAxialLinks(const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "adjustAxialLinks";
    EnterStage(stage);
#endif

//     static const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
//     static AList<TLink> * hits = new AList<TLink>[nsl];
    const unsigned nsl = Belle2::TRGCDC::getTRGCDC()->nSuperLayers();
    AList<TLink> * hits = new AList<TLink>[nsl];
//     for (unsigned i = 0; i < nsl; i++)
//  hits[i].removeAll();
//  AList<TLink> hits[11];
    TLink::nHitsSuperLayer(list, hits);

    //...Require at least two hits per super layer...
//     for (unsigned i = 0; i < nsl; i++)
//  std::cout << "    i=" << i << "," << hits[i].length() << std::endl;
    for (unsigned i = 0; i < nsl; i++)
      if (hits[i].length() < 2)
        hits[i].removeAll();

    //...Make groups using axial super layers...
    unsigned nGroups = 0;
    unsigned last = 0;
    AList<TLink> group[4];
    for (unsigned i = 0; i < nsl; i++) {
      if (i % 2) continue;

      if ((last == 0) && hits[i].length())
        ++nGroups;
      last = hits[i].length();
      if (! hits[i].length()) continue;

      group[nGroups - 1].append(hits[i]);
    }

    delete [] hits;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#groups=" << nGroups << std::endl;
    for (unsigned i = 0; i < nGroups; i++) {
      std::cout << Tab();
      TLink::dump(group[i], "sort", Tab());
    }
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    AList<TLink> nothing;
    if (nGroups == 1)
      return group[0];
    else {
      unsigned nMax = 0;
      unsigned best = 0;
      for (unsigned i = 0; i < nGroups; i++) {
        if (nMax < (unsigned) group[i].length()) {
          nMax = group[i].length();
          best = i;
        }
      }
      return group[best];
    }

    return nothing;
  }

  int
  THoughFinder::doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                     const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                     AList<TTrack> & tracks,
                     AList<TTrack> & tracks2D)
  {
    if (_mode == 0)
      return doit0(axialHits, stereoHits, tracks, tracks2D);
    else if (_mode == 1)
      return doit1(axialHits, stereoHits, tracks, tracks2D);
    else if (_mode == 2)
      return doit2(axialHits, stereoHits, tracks, tracks2D);
    else
      return doit3(axialHits, stereoHits, tracks, tracks2D);
  }

  int
  THoughFinder::doit1(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                      const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                      AList<TTrack> & tracks,
                      AList<TTrack> & tracks2D)
  {

    //...For debug...
    if (debugLevel() > 1) {
      std::cout << name() << " ... processing"
                << " axial=" << axialHits.length()
                << ",stereo=" << stereoHits.length()
                << ",tracks=" << tracks.length()
                << std::endl;
    }

#ifdef TRASAN_DEBUG
    const std::string stage = "THoughFinder::doit1";
    EnterStage(stage);
#endif
#ifdef TRASAN_WINDOW
    Window.skip(false);
    Window.skipAllWindow(false);
#endif

    //...Select good hits and make TLinks...
    selectGoodHits(axialHits, stereoHits);

    //...Hough plane for high Pt...
    const float ptBoundary = _circleHough.y(_ptBoundary, 0., 0.);
    THoughPlaneMulti* planes[2][2] = {{0, 0}, {0, 0}};
    THoughPlane* planes0[2][2] = {{0, 0}, {0, 0}};
    THoughPlaneMulti cPlaneHP("circle Hough:high pt and pluse charge",
                              _meshX, 0, 2 * M_PI, _meshY, ptBoundary, 3, 50);
    THoughPlaneMulti cPlaneHM("circle Hough:high pt and minus charge",
                              _meshX, 0, 2 * M_PI, _meshY, ptBoundary, 3, 50);
    planes[0][0] = & cPlaneHP;
    planes[0][1] = & cPlaneHM;
    planes0[0][0] = & cPlaneHP;
    planes0[0][1] = & cPlaneHM;

    //...Select outside hits only...
//  AList<TLink> superLayerHits[11];
    static AList<TLink> * superLayerHits =
      new AList<TLink>[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
    for (unsigned i = 0; i < Belle2::TRGCDC::getTRGCDC()->nSuperLayers(); i++)
      superLayerHits[i].removeAll();
    TLink::nHitsSuperLayer(_axial, superLayerHits);
    AList<TLink> outerHits;
    outerHits.append(superLayerHits[0]);
    outerHits.append(superLayerHits[2]);
    outerHits.append(superLayerHits[4]);
    outerHits.append(superLayerHits[6]);
    outerHits.append(superLayerHits[8]);
    outerHits.append(superLayerHits[10]);

    //...Circle Hough transformation...
    houghTransformation(outerHits, _circleHough, +1., cPlaneHP);
    houghTransformation(outerHits, _circleHough, -1., cPlaneHM);

    //...Main loop...
    unsigned nAxials = _axial.length();
    bool finished = (nAxials < _minThreshold);
    while (! finished) {

      for (unsigned pt = 0; pt < 1; pt++) {

        //...Determine threshold...
        const unsigned maxEntry = unsigned(
                                    std::max(planes[pt][0]->maxEntry(),
                                             planes[pt][1]->maxEntry()));
        unsigned threshold = unsigned(float(maxEntry) * _threshold);

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "threshold=" << threshold
                  << ",peakThreshold=" << _minThreshold << std::endl;
        if (threshold < _minThreshold)
          std::cout << Tab() << "main loop finished (threshold is less than"
                    << " the min. threshold" << std::endl;
#endif
        if (threshold < _minThreshold) continue;

        //...Peak finding...
//      AList<TPoint2D> listP = _peakFinder.peaks4(* planes[pt][0],
//                   threshold);
//      AList<TPoint2D> listM = _peakFinder.peaks4(* planes[pt][1],
//                   threshold);
        planes[pt][0]->clearRegion();
        planes[pt][1]->clearRegion();
        AList<TPoint2D> listP = _peakFinder.peaks5(* planes[pt][0],
                                                   threshold);
        AList<TPoint2D> listM = _peakFinder.peaks5(* planes[pt][1],
                                                   threshold);
        AList<TPoint2D> list;
        list.append(listP);
        list.append(listM);

        //...Sort peaks by R...
        list.sort(SortByY);

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "#peaks=" << list.length() << std::endl;
#endif
        //...Track building...
        const unsigned n = list.length();
        for (unsigned i = 0; i < n; i++) {

#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "peak loop " << i << std::endl;
#endif
#ifdef TRASAN_WINDOW
          Window.skip(false);
#endif

          //...Track charge...
          float charge = 0;
          if (listP.hasMember(list[i]))
            charge = +1;
          else
            charge = -1;

//        static unsigned nDump = 0;
//        const std::string mess = "#dump" + itostring(nDump);
//        cPlaneHP.dump(mess);
//        cPlaneHM.dump(mess);
//        ++nDump;

          //...Check threshold again...
          if (charge > 0) {
            const int maxEntry = (* planes[pt][0]).maxEntryInRegion(
                                   (* planes[pt][0]).serialID(* list[i]));
            if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
              std::cout << Tab() << "peak loop " << i << ":pt="
                        << pt << ",charge=" << charge << " skipped"
                        << std::endl;
              std::cout << Tab() << "    maxEntryInRegion="
                        << maxEntry << ",threshold=" << threshold
                        << std::endl;
#endif
              continue;
            }
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "peak loop " << i << ":pt="
                      << pt << ",charge=" << charge << " accepted"
                      << std::endl;
            std::cout << Tab() << "    maxEntryInRegion="
                      << maxEntry << ",threshold=" << threshold
                      << std::endl;
#endif
//        if ((* planes[pt][0]).entry(
//           (* planes[pt][0]).serialID(* list[i]))
//       < threshold) {
// #ifdef TRASAN_DEBUG_DETAIL
//      std::cout << Tab() << "peak loop " << i << ":pt="
//          << pt << ",charge=" << charge << " skipped"
//          << std::endl;
//      std::cout << Tab() << "    entry="
//          << (* planes[pt][0]).entry(
//              (* planes[pt][0]).serialID(* list[i]))
//          << ",threshold=" << threshold << std::endl;
// #endif
//      continue;
//        }
          } else {
            const int maxEntry = (* planes[pt][1]).maxEntryInRegion(
                                   (* planes[pt][1]).serialID(* list[i]));
            if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
              std::cout << Tab() << "peak loop " << i << ":pt="
                        << pt << ",charge=" << charge << " skipped"
                        << std::endl;
              std::cout << Tab() << "    maxEntryInRegion="
                        << maxEntry << ",threshold=" << threshold
                        << std::endl;
#endif
              continue;
            }
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "peak loop " << i << ":pt="
                      << pt << ",charge=" << charge << " accepted"
                      << std::endl;
            std::cout << Tab() << "    maxEntryInRegion="
                      << maxEntry << ",threshold=" << threshold
                      << std::endl;
#endif
          }

#ifdef TRASAN_WINDOW
          AList<TPoint2D> tmp;
          tmp.append(list[i]);
          string th = "THoughFinder::doit:threshold="
                      + itostring(threshold);
          string thp = th + ",peak="
                       + itostring((* planes[pt][0]).entry(
                                     (* planes[pt][0]).serialID(* list[i])));
          string thm = th + ",peak="
                       + itostring((* planes[pt][1]).entry(
                                     (* planes[pt][1]).serialID(* list[i])));
          HPWindow.text(thp.c_str());
          HMWindow.text(thm.c_str());
          HPWindow.draw(* planes[pt][0], listP, 0.06, leda_blue);
          HMWindow.draw(* planes[pt][1], listM, 0.06, leda_blue);
          HPWindow.drawOver(* planes[pt][0], tmp, 0.06, leda_green);
          HMWindow.drawOver(* planes[pt][1], tmp, 0.06, leda_green);
#endif

#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "pt=" << pt << ",charge=" << charge
                    << std::endl;
#endif

          //...Build a track...
          unsigned threshold2 = threshold / 2;
          TTrack* s = build1(* list[i], charge, pt, threshold2);
//        TTrack* s = build1(* list[i], planes0, charge, pt, threshold2);
//        if (! s) s = localSearch2(* list[i], charge);
//        TTrack * s = localSearch2(* list[i], charge);
          if (! s) {
            continue;
          }
          if (s->quality() & TrackQuality2D) {
            tracks2D.append(s);
            s->name("HoughTrack2D_" + itostring(tracks2D.length()));
          } else {
            tracks.append(s);
            s->name("HoughTrack_" + itostring(tracks.length()));
          }
          _axial.remove(s->links());

          //...Remove from the Hough plane (outer only)...
          AList<TLink> axial = TLink::axialHits(s->links());
          AList<TLink> list;
          for (unsigned j = 0; j < (unsigned) axial.length(); j++)
            if (outerHits.hasMember(axial[j]))
              list.append(axial[j]);

          static unsigned removed = 0;
          removed += list.length();

#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "#outerHits=" << outerHits.length() << ",remain="
                    << outerHits.length() - removed << ",removed="
                    << list.length() << std::endl;
#endif
          if (list.length()) {
            houghTransformation(list, _circleHough, +1.,
                                * planes[0][0], -1);
            houghTransformation(list, _circleHough, -1.,
                                * planes[0][1], -1);
          }

#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "track made:" << s->name()
                    << std::endl;
#endif
        }

#ifdef TRASAN_WINDOW
        std::cout << Tab() << "end of peak loop:pt=" << pt
                  << std::endl;
//      HPWindow.draw(* planes[pt][0], listP, 0.06, leda_blue);
//      HMWindow.draw(* planes[pt][1], listM, 0.06, leda_blue);
//      HMWindow.wait();
#endif

        if (list.length()) HepAListDeleteAll(list);
      }

      //...Finished ?...
//  std::cout << "THoughFinder ... nAxials=" << nAxials
//      << ",_axial.length=" << _axial.length() << std::endl;

      if (nAxials == (unsigned) _axial.length())
        finished = true;
      else
        nAxials = _axial.length();
    }

    //...Curl search...
    if (_doCurlSearch)
      curlSearch(tracks, tracks2D);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "THoughFinder finished" << std::endl;
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return 0;
  }

int
THoughFinder::doit0(const CAList<Belle2::TRGCDCWireHit> & axialHits,
		    const CAList<Belle2::TRGCDCWireHit> & stereoHits,
		    AList<TTrack> & tracks,
		    AList<TTrack> & tracks2D) {

    //...For debug...
    if (debugLevel() > 1) {
	std::cout << name() << " ... processing"
		  << " axial=" << axialHits.length()
		  << ",stereo=" << stereoHits.length()
		  << ",tracks=" << tracks.length()
		  << std::endl;
    }

#ifdef TRASAN_DEBUG
    const std::string stage = "THoughFinder::doit0";
    EnterStage(stage);
#endif

    //...Select good hits and make TLinks...
    selectGoodHits(axialHits, stereoHits);

#ifdef TRASAN_WINDOW_GTK_HOUGH
    TWindowGTKConformal& w = Trasan::getTrasan()->w();
    w.clear();
    w.stage("Hough Finder : good hit selection");
    w.information("gray:all hits, green:selected");
    w.append(axialHits, Gdk::Color("gray"));
    w.append(stereoHits, Gdk::Color("gray"));
    w.append(_all, Gdk::Color("green"));
//  w.run();
#endif

    //...Main loop...
    unsigned nAxials = _axial.length();
    bool finished = (nAxials < _minThreshold);
    while (! finished) {

	for (unsigned pt = 0; pt < 1; pt++) {

	    _planeHP2->clear();
	    _planeHM2->clear();
	    houghTransformation2(_axial, *_planeHP2);
	    houghTransformation2(_axial, *_planeHM2);

	    //...Determine threshold...
	    const unsigned maxEntry = unsigned(
		std::max(_planes2[0]->maxEntry(),
			 _planes2[1]->maxEntry()));
	    unsigned threshold = unsigned(float(maxEntry) * _threshold);

#ifdef TRASAN_DEBUG_DETAIL
//      _planes2[0]->dump("merged", Tab());
//      _planes2[1]->dump("merged", Tab());
	    std::cout << Tab() << "threshold=" << threshold
		      << ",peakThreshold=" << _minThreshold << std::endl;
#endif
	    if (threshold < _minThreshold) continue;

	    //...Peak finding...
	    _planes2[0]->clearRegion();
	    _planes2[1]->clearRegion();
	    AList<TPoint2D> listP = _peakFinder.peaks5(* _planes2[0],
						       threshold);
	    AList<TPoint2D> listM = _peakFinder.peaks5(* _planes2[1],
						       threshold);
	    AList<TPoint2D> list;
	    list.append(listP);
	    list.append(listM);

	    //...Sort peaks by R...
	    list.sort(SortByY);

#ifdef TRASAN_DEBUG_DETAIL
	    std::cout << Tab() << "#peaks=" << list.length() << ":plus peaks="
		      << listP.length() << ",minus peaks=" << listM.length()
		      << std::endl;
#endif

	    //...Track building...
	    unsigned lastAxialN = _axial.length();
	    const unsigned n = list.length();
	    for (unsigned i = 0; i < n; i++) {

#ifdef TRASAN_DEBUG_DETAIL
		std::cout << Tab() << "peak loop " << i << "/" << n
			  << std::endl;
		_planes2[0]->dump("region", Tab());
//    _planes2[0]->dump("merged", Tab());
		_planes2[1]->dump("region", Tab());
//    _planes2[1]->dump("merged", Tab());
#endif

//...tset...
		if (i) {
		    unsigned axialN = _axial.length();
		    if (axialN != lastAxialN) {
			_planeHP2->clearCells();
			_planeHM2->clearCells();
			houghTransformation2(_axial, *_planeHP2);
			houghTransformation2(_axial, *_planeHM2);
			lastAxialN = axialN;
		    }
		}
//...test end...

		//...Track charge...
		float charge = 0;
		if (listP.hasMember(list[i]))
		    charge = +1;
		else
		    charge = -1;

//        static unsigned nDump = 0;
//        const std::string mess = "#dump" + itostring(nDump);
//        _planeHP2.dump(mess);
//        _planeHM2.dump(mess);
//        ++nDump;

		//...Check threshold again...
		if (charge > 0) {
		    const int maxEntry = (* _planes2[0]).maxEntryInRegion(
			(* _planes2[0]).serialID(* list[i]));
		    if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
			std::cout << Tab() << "peak loop " << i << ":pt="
				  << pt << ",charge=" << charge << " skipped"
				  << std::endl;
			std::cout << Tab() << "    maxEntryInRegion="
				  << maxEntry << ",threshold=" << threshold
				  << std::endl;
#endif
			continue;
		    }
#ifdef TRASAN_DEBUG_DETAIL
		    std::cout << Tab() << "peak loop " << i << ":pt="
			      << pt << ",charge=" << charge << " accepted"
			      << std::endl;
		    std::cout << Tab() << "    maxEntryInRegion="
			      << maxEntry << ",threshold=" << threshold
			      << std::endl;
#endif
		} else {
		    const int maxEntry = (* _planes2[1]).maxEntryInRegion(
			(* _planes2[1]).serialID(* list[i]));
		    if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
			std::cout << Tab() << "peak loop " << i << ":pt="
				  << pt << ",charge=" << charge << " skipped"
				  << std::endl;
			std::cout << Tab() << "    maxEntryInRegion="
				  << maxEntry << ",threshold=" << threshold
				  << std::endl;
#endif
			continue;
		    }
#ifdef TRASAN_DEBUG_DETAIL
		    std::cout << Tab() << "peak loop " << i << ":pt="
			      << pt << ",charge=" << charge << " accepted"
			      << std::endl;
		    std::cout << Tab() << "    maxEntryInRegion="
			      << maxEntry << ",threshold=" << threshold
			      << std::endl;
#endif
		}

#ifdef TRASAN_WINDOW_GTK_HOUGH
		TWindowGTKHough& hp = Trasan::getTrasan()->hp();
		TWindowGTKHough& hm = Trasan::getTrasan()->hm();
		hp.stage("Trasan Hough: Tracking not started yet");
		hm.stage("Trasan Hough: Tracking not started yet");
		hp.clear();
		hm.clear();
		hp.append(_planes2[0]);
		hm.append(_planes2[1]);
//    hm.append(& _tmp);
		hp.show();
		hm.show();
//debug		hp.run();
#endif

#ifdef TRASAN_DEBUG_DETAIL
		std::cout << Tab() << "pt=" << pt << ",charge=" << charge
			  << std::endl;
#endif

		//...Build a track...
		unsigned threshold2 = threshold / 2;
		TTrack* s = build0(* list[i],
				   _planes02,
				   charge,
				   pt,
				   threshold2);
		if (! s) {
#ifdef TRASAN_DEBUG_DETAIL
		    std::cout << Tab() << "3D failure (bad quality)"
			      << std::endl;
#endif
		    continue;
		}
		s->finder(TrackHoughFinder);
		if (s->quality() & TrackQuality2D) {
		    tracks2D.append(s);
		    s->name("HoughTrack2D_" + itostring(tracks2D.length()));
		}
		else {
		    tracks.append(s);
		    s->name("HoughTrack_" + itostring(tracks.length()));
		}

		//...Remove from the Hough plane (outer only)...
		_axial.remove(s->links());

//...test...
//      AList<TLink> axial = TLink::axialHits(s->links());
//      houghTransformation2(axial, _planeHP2, -1);
//      houghTransformation2(axial, _planeHM2, -1);
//...test end...

//    std::cout << "#outerHits      removing " << axial.length() << std::endl;

#ifdef TRASAN_DEBUG_DETAIL
		std::cout << Tab() << "track made:" << s->name()
			  << std::endl;
#endif
	    }

	    if (list.length()) HepAListDeleteAll(list);
	}

	//...Finished ?...
#ifdef TRASAN_DEBUG_DETAIL
	std::cout << Tab() << "loop end check:nAxials=" << nAxials
		  << ",_axial.length=" << _axial.length() << std::endl;
#endif

	if (nAxials == (unsigned) _axial.length())
	    finished = true;
	else
	    nAxials = _axial.length();
    }

    //...Curl search...
    if (_doCurlSearch)
	curlSearch(tracks, tracks2D);

    //...For debug...
    if (debugLevel() > 1) {
	std::cout << name() << " ... processed"
		  << " axial=" << axialHits.length()
		  << ",stereo=" << stereoHits.length()
		  << ",tracks=" << tracks.length()
		  << std::endl;
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "THoughFinder finished" << std::endl;
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return 0;
}

  int
  THoughFinder::curlSearch(AList<TTrack> & tracks,
                           AList<TTrack> & tracks2D)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "curlSearch";
    EnterStage(stage);
#endif

    //...For debug...
    if (debugLevel() > 1) {
      std::cout << name() << "::curlSearch ... processing"
                << " axial=" << _axial.length()
                << ",stereo=" << _stereo.length()
                << ",tracks=" << tracks.length()
                << std::endl;
    }

#ifdef TRASAN_WINDOW
//    Window.skip(false);
//    Window.skipAllWindow(false);
    Window.clear();
    Window.append(_axial, leda_pink);
    Window.text(Stage());
    Window.wait();
#endif

    //...Hough plane for high Pt...
    const float ptBoundary = _circleHough.y(_ptBoundaryLowPt, 0., 0.);

    //...Hough plane for low Pt...
    THoughPlane pln("circle Hough:low pt",
                    _meshXLowPt, 0, 2 * M_PI, _meshYLowPt, 0.7, ptBoundary);

    //...Circle Hough transformation...
    houghTransformation(_axial, _circleHough, +1., pln);
    houghTransformation(_axial, _circleHough, -1., pln);

    //...Main loop...
    unsigned nLoops = 0;
    unsigned nAxials = _axial.length();
    bool finished = (nAxials < _minThresholdLowPt);
    bool planeModified = false;
    while (! finished) {

      for (unsigned pt = 0; pt < 2; pt++) {

        //...Determine threshold...
        const unsigned maxEntry = unsigned(
                                    std::max(pln.maxEntry(), pln.maxEntry()));
        unsigned threshold = unsigned(float(maxEntry) * _thresholdLowPt);

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "threshold=" << threshold
                  << ",peakThreshold=" << _minThresholdLowPt << std::endl;
        if (threshold < _minThresholdLowPt)
          std::cout << Tab() << "main loop finished (threshold is less than"
                    << " the min. threshold" << std::endl;
#endif
        if (threshold < _minThresholdLowPt) break;

        //...Peak finding...
        AList<TPoint2D> list = _peakFinder.peaks5(pln, threshold);

        //...Sort peaks by R...
        list.sort(SortByY);

        //...Track building...
        const unsigned n = list.length();
        for (unsigned i = 0; i < n; i++) {

          ++nLoops;

          //...Check threshold again...
          if (pln.entry(pln.serialID(* list[i])) < threshold)
            continue;
#ifdef TRASAN_WINDOW
          AList<TPoint2D> tmp;
          tmp.append(list[i]);
          string th = "THoughFinder::curlSearch:threshold="
                      + itostring(threshold);
          string thp = th + ",peak="
                       + itostring(pln.entry(
                                     pln.serialID(* list[i])));
          HPWindow.text(thp.c_str());
          HPWindow.draw(pln, list, 0.06, leda_blue);
          HPWindow.drawOver(pln, tmp, 0.06, leda_green);
#endif
#ifdef TRASAN_WINDOW_GTK_HOUGH
          TWindowGTKHough& hc = Trasan::getTrasan()->hc();
          hc.stage("Trasan Hough:curlSearch");
          hc.clear();
          hc.append(& pln);
          hc.show();
//debug          hc.run();
#endif

          //...Track charge loop...
          for (unsigned j = 0; j < 2; j++) {
            float charge = 0;
            if (j == 0)
              charge = +1;
            else
              charge = -1;

            //...Build a track...
            unsigned threshold2 = threshold / 2;
            AList<TLink> bads;
            TTrack* s = buildCurl(* list[i], pln, charge,
                                  threshold2, bads);
            if (! s) {

              //...Remove hits from Hough plane...
              houghTransformation(bads, _circleHough, +1., pln, -1);
              houghTransformation(bads, _circleHough, -1., pln, -1);
              planeModified = true;
              continue;
            }
            if (s->quality() & TrackQuality2D) {
              tracks2D.append(s);
              s->name("HoughTrack2D_" + itostring(tracks2D.length()));
            } else {
              tracks.append(s);
              s->name("HoughTrack_" + itostring(tracks.length()));
            }

            //...Remove from the Hough plane...
            AList<TLink> axial = TLink::axialHits(s->links());
            houghTransformation(axial, _circleHough, +1., pln, -1);
            houghTransformation(axial, _circleHough, -1., pln, -1);

#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "track made:" << s->name()
                      << std::endl;
#endif
          }
        }
        if (list.length()) HepAListDeleteAll(list);
      }

//  std::cout << "THoughFinder::curlSearch ... track building loops="
//      << nLoops << std::endl;
//  std::cout << "    sizeof(unsigned)=" << sizeof(unsigned) << std::endl;

      //...Finished ?...
      if ((nAxials == (unsigned) _axial.length()) && (! planeModified)) {
        finished = true;
      } else {
        nAxials = _axial.length();
        planeModified = false;
      }

      if (nLoops > 100) finished = true;
    }

    //...For debug...
    if (debugLevel() > 1) {
      std::cout << name() << "::curlSearch ... processed"
                << " axial=" << _axial.length()
                << ",stereo=" << _stereo.length()
                << ",tracks=" << tracks.length()
                << std::endl;
    }

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return 0;
  }

  bool
  THoughFinder::goodTrackHoughMatch(const THoughPlane& hp,
                                    const THoughPlane& hm,
                                    const THoughTransformation& trans,
                                    const TTrack& t,
                                    unsigned threshold,
                                    const TPoint2D& o) const
  {

#ifdef TRASAN_DEBUG
    const std::string stage = "goodTrackHoughMatch";
    EnterStage(stage);
#endif

    const TPoint2D& center = t.center() - o;
    float phi = atan2(center.y(), center.x());
    if (phi < 0) phi += 2 * M_PI;
    const float r = center.mag();
    const TPoint2D c(phi, r);
    const TPoint2D p = trans.convert(c);
    const unsigned id = hp.serialID(p);

    //...Check id...
    if (id == hp.nX() * hp.nY()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << ">>> track rejected:out of hough plane"
                << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return false;
    }

    bool ok = false;
    if ((hp.entry(id) > threshold) || (hm.entry(id) > threshold))
      ok = true;

    //...Check entry around the center...
    if (! ok) {
      CList<unsigned> neighbors = hp.neighbors(id, 2);
      for (unsigned i = 0; i < (unsigned) neighbors.length(); i++) {
        if ((hp.entry(* neighbors[i]) > threshold) ||
            (hm.entry(* neighbors[i]) > threshold)) {
          ok = true;
          break;
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    if (! ok) {
      std::cout << Tab() << ">>> track rejected" << std::endl;
      std::cout << Tab() << "    threshold=" << threshold << ",hp.entry="
                << hp.entry(id) << ",hm.entry=" << hm.entry(id) << std::endl;
      std::cout << Tab() << "    p=" << p << ",id=" << id << std::endl;

      CList<unsigned> neighbors = hp.neighbors(id, 2);
      for (unsigned i = 0; i < (unsigned) neighbors.length(); i++) {
        std::cout << Tab() << "    id=" << * neighbors[i] << ",hp.entry="
                  << hp.entry(* neighbors[i]) << ",hm.entry="
                  << hm.entry(* neighbors[i]) << std::endl;
      }
    }
#endif
#ifdef TRASAN_WINDOW
    AList<TPoint2D> tmp;
    tmp.append((TPoint2D&) p);
    if (ok) {
      HPWindow.drawOver(hp, tmp, 0.03, leda_green);
      HMWindow.drawOver(hm, tmp, 0.03, leda_green);
    } else {
      HPWindow.draw(hp, tmp, 0.06, leda_red);
      HMWindow.draw(hm, tmp, 0.06, leda_red);
      HPWindow.drawOver(hp, tmp, 0.03, leda_red);
      HMWindow.drawOver(hm, tmp, 0.03, leda_red);
      Window.append(t, leda_red);
      Window.text("goodTrack:2D track is failed");
      Window.wait();
    }
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return ok;
  }

  void
  THoughFinder::init(void)
  {
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    const int nlayers(cdc.nLayers());
    _planeHP = new THoughPlaneMulti("circle Hough:high pt and pluse charge",
                                    _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
                                    3, nlayers);
    _planeHM = new THoughPlaneMulti("circle Hough:high pt and minus charge",
                                    _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
                                    3, nlayers);
    _tmp = new THoughPlaneMulti("circle Hough:high pt and minus charge",
                                _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
                                3, nlayers);
    _planeHP2 = new THoughPlaneMulti2("circle Hough:high pt and pluse charge",
                                      _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
                                      3, nlayers);
    _planeHM2 = new THoughPlaneMulti2("circle Hough:high pt and minus charge",
                                      _meshX, 0, 2 * M_PI, _meshY, _ptBoundaryInHough,
                                      3, nlayers);
    _planes0[0] = _planeHP;
    _planes0[1] =  _planeHM;
    _planes[0] =  _planeHP;
    _planes[1] =  _planeHM;

    _planes02[0] =  _planeHP2;
    _planes02[1] =  _planeHM2;
    _planes2[0] =  _planeHP2;
    _planes2[1] =  _planeHM2;


    _nsl = cdc.nSuperLayers();
    if (_nHits2D)
      delete[] _nHits2D;
    _nHits2D = new unsigned[_nsl];
    if (_nHits3D)
      delete[] _nHits3D;
    _nHits3D = new unsigned[_nsl];
    if (_nHitsG)
      delete[] _nHitsG;
    _nHitsG = new unsigned[_nsl];

    for (unsigned i = 0; i < cdc.nLayers(); i++) {
      const Belle2::TRGCDCLayer* l = cdc.layer(i);
      const unsigned nWires = l->nCells();
      if (! nWires) continue;
      _planeHP2->preparePatterns(i, nWires);
      _planeHM2->preparePatterns(i, nWires);
      for (unsigned j = 0; j < nWires; j++) {
//      const Belle2::TRGCDCWire & w = * (* l)[j];
        const Belle2::TRGCDCWire& w = * cdc.wire(i, j);
        const float x = w.xyPosition().x();
        const float y = w.xyPosition().y();

        _planeHP2->clear();
        _planeHP2->vote(x, y, +1, _circleHough, i, 1);
        _planeHP2->registerPattern(i, j);

        _planeHM2->clear();
        _planeHM2->vote(x, y, -1, _circleHough, i, 1);
        _planeHM2->registerPattern(i, j);

#ifdef TRASAN_WINDOW_GTK_HOUGH
//      if (j == 0) {
//    _planeHP2.merge();
//    TWindowGTKHough & hp = Trasan::getTrasan()->hp();
//    hp.clear();
//    hp.append(& _planeHP2);
//    hp.show();
//    hp.run();
//      }
#endif

      }
    }

#if 0
    unsigned** patterns = new unsigned * [nWires];
    unsigned* nPatterns = new unsigned [nWires];
    for (unsigned j = 0; j < nWires; j++) {
//      const float phi = float(j) / float(nWires) * 2 * M_PI;
//      const float x = r * cos(phi);
//      const float y = r * sin(phi);
      const Belle2::TRGCDCWire& w = * (* l)[j];
      const float x = w.xyPosition().x();
      const float y = w.xyPosition().y();
      _tmp->clear();
      _tmp->vote(x, y, +1, _circleHough, 1, i);
      _tmp->registerPattern(i);
      const unsigned nCells = _tmp->nActiveCellsInPattern(i);
      patterns[j] = new unsigned[nCells];
      nPatterns[j] = nCells;
      unsigned n = 0;
      unsigned offset = 0; _tmp->merge();
      for (unsigned k = 0; k < _tmp->nX() * _tmp->nY(); k++) {
        if (_tmp->entry(k)) {
          if (n == 0) {
            offset = _tmp->nY() * (k / _tmp->nY());
//      std::cout << "offset=" << offset << std::endl;
          }
          patterns[j][n++] = k - offset;

//        std::cout << n - 1 << "_" << k << "_" << patterns[j][n - 1]
//         << ",";

        }
      }
//      std::cout << std::endl;

//      std::cout << "L" << i << " W" << j << " phi=" << phi << " #patterns="
//       << nCells << std::endl;

      _tmp->vote(r, 0, +1, _circleHough, 1, i);
      _tmp->merge();

#ifdef TRASAN_WINDOW_GTK_HOUGH
//      if (j == 0) {
//    TWindowGTKHough & hp = Trasan::getTrasan()->hp();
//    hp.clear();
//    hp.append(& _tmp);
//    hp.show();
//    hp.run();
//      }
#endif

    }

    //...Pattern check...
    bool* checked = new bool[nWires];
    for (unsigned j = 0; j < nWires; j++) {
      checked[j] = false;
    }
    for (unsigned j = 0; j < nWires - 1; j++) {
      const unsigned np0 = nPatterns[j];
      for (unsigned k = j + 1; k < nWires; k++) {
        const unsigned np1 = nPatterns[k];
        if (np0 != np1) continue;
        bool diff = false;
        for (unsigned l = 0; l < np0; l++) {
          if (patterns[j][l] != patterns[k][l]) diff = true;
          if (diff) break;
        }
        if (! diff) checked[k] = true;
      }
    }
    unsigned n = 0;
    for (unsigned j = 0; j < nWires; j++)
      if (! checked[j]) ++n;

    std::cout << "#wires=" << nWires << std::endl;
    std::cout << "    #patterns=" << n << std::endl;

    for (unsigned j = 0; j < nWires; j++)
      delete [] patterns[j];
    delete [] patterns;
    delete [] nPatterns;
    delete checked;

    unsigned** patterns = new unsigned * [nWires];
    unsigned* nPatterns = new unsigned [nWires];
    for (unsigned j = 0; j < nWires; j++) {
//      const float phi = float(j) / float(nWires) * 2 * M_PI;
//      const float x = r * cos(phi);
//      const float y = r * sin(phi);
      const Belle2::TRGCDCWire& w = * (* l)[j];
      const float x = w.xyPosition().x();
      const float y = w.xyPosition().y();
      _tmp->clear();
      _tmp->vote(x, y, +1, _circleHough, 1, i);
      _tmp->registerPattern(i);
      const unsigned nCells = _tmp->nActiveCellsInPattern(i);
      patterns[j] = new unsigned[nCells];
      nPatterns[j] = nCells;
      unsigned n = 0;
      unsigned offset = 0;
      _tmp->merge();
      for (unsigned k = 0; k < _tmp->nX() * _tmp->nY(); k++) {
        if (_tmp->entry(k)) {
          if (n == 0) {
            offset = _tmp->nY() * (k / _tmp->nY());
//      std::cout << "offset=" << offset << std::endl;
          }
          patterns[j][n++] = k - offset;

//        std::cout << n - 1 << "_" << k << "_" << patterns[j][n - 1]
//         << ",";

        }
      }
//      std::cout << std::endl;

//      std::cout << "L" << i << " W" << j << " phi=" << phi << " #patterns="
//       << nCells << std::endl;

      _tmp->vote(r, 0, +1, _circleHough, 1, i);
      _tmp->merge();

#ifdef TRASAN_WINDOW_GTK_HOUGH
//      if (j == 0) {
//    TWindowGTKHough & hp = Trasan::getTrasan()->hp();
//    hp.clear();
//    hp.append(& _tmp);
//    hp.show();
//    hp.run();
//      }
#endif

    }

    //...Pattern check...
    bool* checked = new bool[nWires];
    for (unsigned j = 0; j < nWires; j++) {
      checked[j] = false;
    }
    for (unsigned j = 0; j < nWires - 1; j++) {
      const unsigned np0 = nPatterns[j];
      for (unsigned k = j + 1; k < nWires; k++) {
        const unsigned np1 = nPatterns[k];
        if (np0 != np1) continue;
        bool diff = false;
        for (unsigned l = 0; l < np0; l++) {
          if (patterns[j][l] != patterns[k][l]) diff = true;
          if (diff) break;
        }
        if (! diff) checked[k] = true;
      }
    }
    unsigned n = 0;
    for (unsigned j = 0; j < nWires; j++)
      if (! checked[j]) ++n;

    std::cout << "#wires=" << nWires << std::endl;
    std::cout << "    #patterns=" << n << std::endl;

    for (unsigned j = 0; j < nWires; j++)
      delete [] patterns[j];
    delete [] patterns;
    delete [] nPatterns;
    delete checked;
  }
#endif

#if 0
  for (unsigned i = 0; i < cdc.nLayers(); i++)
  {
    const Belle2::TRGCDCLayer* l = cdc.layer(i);
    if (! l->nCells()) continue;
    const float r = l->wire(0)->xyPosition().perp();
    _planeHP.vote(r, 0, +1, _circleHough, 1, i);
    _planeHP.pattern(i);
    _planeHP.clear();
    _planeHM.vote(r, 0, -1, _circleHough, 1, i);
    _planeHM.pattern(i);
    _planeHM.clear();

    const unsigned nWires = l->nCells();
    unsigned** patterns = new unsigned * [nWires];
    unsigned* nPatterns = new unsigned [nWires];
    for (unsigned j = 0; j < nWires; j++) {
//      const float phi = float(j) / float(nWires) * 2 * M_PI;
//      const float x = r * cos(phi);
//      const float y = r * sin(phi);
      const Belle2::TRGCDCWire& w = * (* l)[j];
      const float x = w.xyPosition().x();
      const float y = w.xyPosition().y();
      _tmp->clear();
      _tmp->vote(x, y, +1, _circleHough, 1, i);
      _tmp->pattern(i);
      const unsigned nCells = _tmp->nActiveCellsInPattern(i);
      patterns[j] = new unsigned[nCells];
      nPatterns[j] = nCells;
      unsigned n = 0;
      unsigned offset = 0;
      _tmp->merge();
      for (unsigned k = 0; k < _tmp->nX() * _tmp->nY(); k++) {
        if (_tmp->entry(k)) {
          if (n == 0) {
            offset = _tmp->nY() * (k / _tmp->nY());
//      std::cout << "offset=" << offset << std::endl;
          }
          patterns[j][n++] = k - offset;

//        std::cout << n - 1 << "_" << k << "_" << patterns[j][n - 1]
//         << ",";

        }
      }
//      std::cout << std::endl;

//      std::cout << "L" << i << " W" << j << " phi=" << phi << " #patterns="
//       << nCells << std::endl;

      _tmp->vote(r, 0, +1, _circleHough, 1, i);
      _tmp->merge();

#ifdef TRASAN_WINDOW_GTK_HOUGH
      if (j == 0) {
        TWindowGTKHough& hp = Trasan::getTrasan()->hp();
        hp.clear();
        hp.append(& _tmp);
        hp.show();
        hp.run();
      }
#endif

    }

    //...Pattern check...
    bool* checked = new bool[nWires];
    for (unsigned j = 0; j < nWires; j++) {
      checked[j] = false;
    }
    for (unsigned j = 0; j < nWires - 1; j++) {
      const unsigned np0 = nPatterns[j];
      for (unsigned k = j + 1; k < nWires; k++) {
        const unsigned np1 = nPatterns[k];
        if (np0 != np1) continue;
        bool diff = false;
        for (unsigned l = 0; l < np0; l++) {
          if (patterns[j][l] != patterns[k][l]) diff = true;
          if (diff) break;
        }
        if (! diff) checked[k] = true;
      }
    }
    unsigned n = 0;
    for (unsigned j = 0; j < nWires; j++)
      if (! checked[j]) ++n;

    std::cout << "#wires=" << nWires << std::endl;
    std::cout << "    #patterns=" << n << std::endl;

    for (unsigned j = 0; j < nWires; j++)
      delete [] patterns[j];
    delete [] patterns;
    delete [] nPatterns;
    delete checked;

  }
#endif

}

// void
// THoughFinder::houghTransformation2(const AList<TLink> & hits,
//           THoughPlaneMulti & plane,
//           int weight) const {
//     const unsigned n = hits.length();
//     for (unsigned i = 0; i < n; i++) {
//  const TLink & l = * hits[i];
//  const unsigned layerId = l.wire()->layerId();
//  float phi = l.xyPosition().phi();
//  if (phi < 0) phi += 2 * M_PI;
//  phi /= 2 * M_PI;
//  plane.vote(phi, layerId, weight);
//     }
//     plane.merge();
// }

void
THoughFinder::houghTransformation2(const AList<TLink> & hits,
                                   THoughPlaneMulti2& plane,
                                   int weight) const
{
  const unsigned n = hits.length();
  for (unsigned i = 0; i < n; i++) {
    const TLink& l = * hits[i];
    const unsigned layerId = l.wire()->layerId();
    const unsigned localId = l.wire()->localId();
    plane.vote(layerId, localId, weight);
  }
  plane.merge();
}

int
THoughFinder::doit2(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                    const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                    AList<TTrack> & tracks,
                    AList<TTrack> & tracks2D)
{

  //...For debug...
  if (debugLevel() > 1) {
    std::cout << name() << "(doit2) ... processing"
              << " axial=" << axialHits.length()
              << ",stereo=" << stereoHits.length()
              << ",tracks=" << tracks.length()
              << std::endl;
  }

#ifdef TRASAN_DEBUG
  const std::string stage = "THoughFinder::doit0";
  EnterStage(stage);
#endif

  //...Select good hits and make TLinks...
  selectGoodHits(axialHits, stereoHits);

#ifdef TRASAN_WINDOW_GTK_HOUGH
  TWindowGTKConformal& w = Trasan::getTrasan()->w();
  w.clear();
//  w.skip(false);
  w.stage("Hough Finder : good hit selection");
  w.information("gray:all hits, green:selected");
  w.append(axialHits, Gdk::Color("gray"));
  w.append(stereoHits, Gdk::Color("gray"));
  w.append(_all, Gdk::Color("green"));
  w.run();
#endif

//     //...Select all axial hits...
//     AList<TLink> outerHits = _axial;

  //...Main loop...
  unsigned nAxials = _axial.length();
  bool finished = (nAxials < _minThreshold);
  while (! finished) {

    for (unsigned pt = 0; pt < 1; pt++) {

      _planeHP2->clear();
      _planeHM2->clear();
      houghTransformation2(_axial, *_planeHP2);
      houghTransformation2(_axial, *_planeHM2);

      //...Determine threshold...
      const unsigned maxEntry = unsigned(
                                  std::max(_planes2[0]->maxEntry(),
                                           _planes2[1]->maxEntry()));
      unsigned threshold = unsigned(float(maxEntry) * _threshold);

#ifdef TRASAN_DEBUG_DETAIL
//      _planes2[0]->dump("merged", Tab());
//      _planes2[1]->dump("merged", Tab());
      std::cout << Tab() << "threshold=" << threshold
                << ",peakThreshold=" << _minThreshold << std::endl;
#endif
      if (threshold < _minThreshold) continue;

      //...Peak finding...
      _planes2[0]->clearRegion();
      _planes2[1]->clearRegion();
      AList<TPoint2D> listP = _peakFinder.peaks5(* _planes2[0],
                                                 threshold);
      AList<TPoint2D> listM = _peakFinder.peaks5(* _planes2[1],
                                                 threshold);
      AList<TPoint2D> list;
      list.append(listP);
      list.append(listM);

      //...Sort peaks by R...
      list.sort(SortByY);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "#peaks=" << list.length() << ":plus peaks="
                << listP.length() << ",minus peaks=" << listM.length()
                << std::endl;
#endif

      //...Track building...
      unsigned lastAxialN = _axial.length();
      const unsigned n = list.length();
      for (unsigned i = 0; i < n; i++) {

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "peak loop " << i << "/" << n << std::endl;
        _planes2[0]->dump("region", Tab());
//    _planes2[0]->dump("merged", Tab());
        _planes2[1]->dump("region", Tab());
//    _planes2[1]->dump("merged", Tab());
#endif

//...tset...
        if (i) {
          unsigned axialN = _axial.length();
          if (axialN != lastAxialN) {
            _planeHP2->clearCells();
            _planeHM2->clearCells();
            houghTransformation2(_axial, *_planeHP2);
            houghTransformation2(_axial, *_planeHM2);
            lastAxialN = axialN;
          }
        }
//...test end...

        //...Track charge...
        float charge = 0;
        if (listP.hasMember(list[i]))
          charge = +1;
        else
          charge = -1;

//        static unsigned nDump = 0;
//        const std::string mess = "#dump" + itostring(nDump);
//        _planeHP2.dump(mess);
//        _planeHM2.dump(mess);
//        ++nDump;

        //...Check threshold again...
        if (charge > 0) {
          const int maxEntry = (* _planes2[0]).maxEntryInRegion(
                                 (* _planes2[0]).serialID(* list[i]));
          if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "peak loop " << i << ":pt="
                      << pt << ",charge=" << charge << " skipped"
                      << std::endl;
            std::cout << Tab() << "    maxEntryInRegion="
                      << maxEntry << ",threshold=" << threshold
                      << std::endl;
#endif
            continue;
          }
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "peak loop " << i << ":pt="
                    << pt << ",charge=" << charge << " accepted"
                    << std::endl;
          std::cout << Tab() << "    maxEntryInRegion="
                    << maxEntry << ",threshold=" << threshold
                    << std::endl;
#endif
        } else {
          const int maxEntry = (* _planes2[1]).maxEntryInRegion(
                                 (* _planes2[1]).serialID(* list[i]));
          if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "peak loop " << i << ":pt="
                      << pt << ",charge=" << charge << " skipped"
                      << std::endl;
            std::cout << Tab() << "    maxEntryInRegion="
                      << maxEntry << ",threshold=" << threshold
                      << std::endl;
#endif
            continue;
          }
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "peak loop " << i << ":pt="
                    << pt << ",charge=" << charge << " accepted"
                    << std::endl;
          std::cout << Tab() << "    maxEntryInRegion="
                    << maxEntry << ",threshold=" << threshold
                    << std::endl;
#endif
        }

#ifdef TRASAN_WINDOW_GTK_HOUGH
        TWindowGTKHough& hp = Trasan::getTrasan()->hp();
        TWindowGTKHough& hm = Trasan::getTrasan()->hm();
        hp.stage("Trasan Hough: Tracking not started yet");
        hm.stage("Trasan Hough: Tracking not started yet");
        hp.clear();
        hm.clear();
        hp.append(_planes2[0]);
        hm.append(_planes2[1]);
//    hm.append(& _tmp);
        hp.show();
        hm.show();
        hp.run();
#endif

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "pt=" << pt << ",charge=" << charge
                  << std::endl;
#endif

        //...Build a track...
        unsigned threshold2 = threshold / 2;
        TTrack* s = build0(* list[i],
                           _planes02,
                           charge,
                           pt,
                           threshold2);
        if (! s) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "3D failure (bad quality)" << std::endl;
#endif
          continue;
        }
        s->finder(TrackHoughFinder);
        if (s->quality() & TrackQuality2D) {
          tracks2D.append(s);
          s->name("HoughTrack2D_" + itostring(tracks2D.length()));
        } else {
          tracks.append(s);
          s->name("HoughTrack_" + itostring(tracks.length()));
        }

        //...Remove from the Hough plane (outer only)...
        _axial.remove(s->links());

//...test...
//      AList<TLink> axial = TLink::axialHits(s->links());
//      houghTransformation2(axial, _planeHP2, -1);
//      houghTransformation2(axial, _planeHM2, -1);
//...test end...

//    std::cout << "#outerHits      removing " << axial.length() << std::endl;

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "track made:" << s->name()
                  << std::endl;
#endif
      }

      if (list.length()) HepAListDeleteAll(list);
    }

    //...Finished ?...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "loop end check:nAxials=" << nAxials
              << ",_axial.length=" << _axial.length() << std::endl;
#endif

    if (nAxials == (unsigned) _axial.length())
      finished = true;
    else
      nAxials = _axial.length();
  }

  //...Curl search...
  if (_doCurlSearch)
    curlSearch(tracks, tracks2D);

  //...For debug...
  if (debugLevel() > 1) {
    std::cout << name() << " ... processed"
              << " axial=" << axialHits.length()
              << ",stereo=" << stereoHits.length()
              << ",tracks=" << tracks.length()
              << std::endl;
  }

#ifdef TRASAN_DEBUG_DETAIL
  std::cout << Tab() << "THoughFinder finished" << std::endl;
#endif

#ifdef TRASAN_DEBUG
  LeaveStage(stage);
#endif

  return 0;
}

int
THoughFinder::doit3(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                    const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                    AList<TTrack> & tracks,
                    AList<TTrack> & tracks2D)
{

  //...For debug...
  if (debugLevel() > 1) {
    std::cout << name() << " ... processing"
              << " axial=" << axialHits.length()
              << ",stereo=" << stereoHits.length()
              << ",tracks=" << tracks.length()
              << std::endl;
  }

#ifdef TRASAN_DEBUG
  const std::string stage = "THoughFinder::doit3";
  EnterStage(stage);
#endif

  //...Select good hits and make TLinks...
  selectGoodHits(axialHits, stereoHits);

#ifdef TRASAN_WINDOW_GTK_HOUGH
  TWindowGTKConformal& w = Trasan::getTrasan()->w();
  w.clear();
//  w.skip(false);
  w.stage("Hough Finder : good hit selection");
  w.information("gray:all hits, green:selected");
  w.append(axialHits, Gdk::Color("gray"));
  w.append(stereoHits, Gdk::Color("gray"));
  w.append(_all, Gdk::Color("green"));
  w.run();
#endif

  //...Main loop...
  unsigned nAxials = _axial.length();
  bool finished = (nAxials < _minThreshold);
  while (! finished) {

    for (unsigned pt = 0; pt < 1; pt++) {

      _planeHP2->clear();
      _planeHM2->clear();
      houghTransformation2(_axial, *_planeHP2);
      houghTransformation2(_axial, *_planeHM2);

      //...Determine threshold...
      const unsigned maxEntry = unsigned(
                                  std::max(_planes2[0]->maxEntry(),
                                           _planes2[1]->maxEntry()));
      unsigned threshold = unsigned(float(maxEntry) * _threshold);

#ifdef TRASAN_DEBUG_DETAIL
//      _planes2[0]->dump("merged", Tab());
//      _planes2[1]->dump("merged", Tab());
      std::cout << Tab() << "threshold=" << threshold
                << ",peakThreshold=" << _minThreshold << std::endl;
#endif
      if (threshold < _minThreshold) continue;

      //...Peak finding...
      _planes2[0]->clearRegion();
      _planes2[1]->clearRegion();
      AList<TPoint2D> listP = _peakFinder.peaks5(* _planes2[0],
                                                 threshold);
      AList<TPoint2D> listM = _peakFinder.peaks5(* _planes2[1],
                                                 threshold);
      AList<TPoint2D> list;
      list.append(listP);
      list.append(listM);

      //...Sort peaks by R...
      list.sort(SortByY);

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "#peaks=" << list.length() << ":plus peaks="
                << listP.length() << ",minus peaks=" << listM.length()
                << std::endl;
#endif

      //...Track building...
      unsigned lastAxialN = _axial.length();
      const unsigned n = list.length();
      for (unsigned i = 0; i < n; i++) {

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "peak loop " << i << "/" << n << std::endl;
        _planes2[0]->dump("region", Tab());
//    _planes2[0]->dump("merged", Tab());
        _planes2[1]->dump("region", Tab());
//    _planes2[1]->dump("merged", Tab());
#endif

//...tset...
        if (i) {
          unsigned axialN = _axial.length();
          if (axialN != lastAxialN) {
            _planeHP2->clearCells();
            _planeHM2->clearCells();
            houghTransformation2(_axial, *_planeHP2);
            houghTransformation2(_axial, *_planeHM2);
            lastAxialN = axialN;
          }
        }
//...test end...

        //...Track charge...
        float charge = 0;
        if (listP.hasMember(list[i]))
          charge = +1;
        else
          charge = -1;

        //...Check threshold again...
        if (charge > 0) {
          const int maxEntry = (* _planes2[0]).maxEntryInRegion(
                                 (* _planes2[0]).serialID(* list[i]));
          if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "peak loop " << i << ":pt="
                      << pt << ",charge=" << charge << " skipped"
                      << std::endl;
            std::cout << Tab() << "    maxEntryInRegion="
                      << maxEntry << ",threshold=" << threshold
                      << std::endl;
#endif
            continue;
          }
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "peak loop " << i << ":pt="
                    << pt << ",charge=" << charge << " accepted"
                    << std::endl;
          std::cout << Tab() << "    maxEntryInRegion="
                    << maxEntry << ",threshold=" << threshold
                    << std::endl;
#endif
        } else {
          const int maxEntry = (* _planes2[1]).maxEntryInRegion(
                                 (* _planes2[1]).serialID(* list[i]));
          if (maxEntry < (int) threshold) {
#ifdef TRASAN_DEBUG_DETAIL
            std::cout << Tab() << "peak loop " << i << ":pt="
                      << pt << ",charge=" << charge << " skipped"
                      << std::endl;
            std::cout << Tab() << "    maxEntryInRegion="
                      << maxEntry << ",threshold=" << threshold
                      << std::endl;
#endif
            continue;
          }
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "peak loop " << i << ":pt="
                    << pt << ",charge=" << charge << " accepted"
                    << std::endl;
          std::cout << Tab() << "    maxEntryInRegion="
                    << maxEntry << ",threshold=" << threshold
                    << std::endl;
#endif
        }

#ifdef TRASAN_WINDOW_GTK_HOUGH
        TWindowGTKHough& hp = Trasan::getTrasan()->hp();
        TWindowGTKHough& hm = Trasan::getTrasan()->hm();
        hp.stage("Trasan Hough: Tracking not started yet");
        hm.stage("Trasan Hough: Tracking not started yet");
        hp.clear();
        hm.clear();
        hp.append(_planes2[0]);
        hm.append(_planes2[1]);
//    hm.append(& _tmp);
        hp.show();
        hm.show();
        hp.run();
#endif

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "pt=" << pt << ",charge=" << charge
                  << std::endl;
#endif

        //...Build a track...
        unsigned threshold2 = threshold / 2;
        TTrack* s = build3(* list[i],
//                         _planes02,
                           charge,
                           pt,
                           threshold2);
        if (! s) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << Tab() << "3D failure (bad quality)" << std::endl;
#endif
          continue;
        }
        s->finder(TrackHoughFinder);
        if (s->quality() & TrackQuality2D) {
          tracks2D.append(s);
          s->name("HoughTrack2D_" + itostring(tracks2D.length()));
        } else {
          tracks.append(s);
          s->name("HoughTrack_" + itostring(tracks.length()));
        }

        //...Remove from the Hough plane (outer only)...
        _axial.remove(s->links());

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << Tab() << "track made:" << s->name()
                  << std::endl;
        std::cout << Tab() << "current _axial" << std::endl;
        TLink::dump(_axial, Tab());
#endif
      }

      if (list.length()) HepAListDeleteAll(list);
    }

    //...Finished ?...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "loop end check:nAxials=" << nAxials
              << ",_axial.length=" << _axial.length() << std::endl;
#endif

    if (nAxials == (unsigned) _axial.length())
      finished = true;
    else
      nAxials = _axial.length();
  }

  //...Curl search...
  if (_doCurlSearch)
    curlSearch(tracks, tracks2D);

  //...For debug...
  if (debugLevel() > 1) {
    std::cout << name() << " ... processed"
              << " axial=" << axialHits.length()
              << ",stereo=" << stereoHits.length()
              << ",tracks=" << tracks.length()
              << std::endl;
  }

#ifdef TRASAN_DEBUG_DETAIL
  std::cout << Tab() << "THoughFinder finished" << std::endl;
#endif

#ifdef TRASAN_DEBUG
  LeaveStage(stage);
#endif

  return 0;
}

} // namespace Belle

