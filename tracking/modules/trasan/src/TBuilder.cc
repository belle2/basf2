//-----------------------------------------------------------------------------
// $Id: TBuilder.cc 10640 2008-09-12 05:11:38Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TBuilder.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to build a track.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.82  2006/02/22 16:44:05  katayama
// gcc 3/Linux fixes
//
// Revision 1.81  2005/11/03 23:20:11  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.79  2005/04/18 23:41:45  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.78  2005/03/11 03:57:46  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.77  2004/04/23 09:48:20  yiwasaki
// Trasan 3.12 : curlVersion=2 is default
//
// Revision 1.76  2004/04/15 05:34:08  yiwasaki
// Trasan 3.11 : trkmgr supports tracks found by other than trasan, Hough finder updates
//
// Revision 1.75  2004/03/26 06:07:03  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.74  2003/12/25 12:03:30  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.73  2002/02/26 22:38:58  yiwasaki
// bug fixes in debug mode
//
// Revision 1.72  2002/02/13 20:22:11  yiwasaki
// Trasan 3.02 : bug fixes in debug mode
//
// Revision 1.71  2001/12/23 09:58:43  katayama
// removed Strings.h
//
// Revision 1.70  2001/12/19 02:59:43  katayama
// Uss find,istring
//
// Revision 1.69  2001/04/25 02:35:59  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.68  2001/04/11 23:20:23  yiwasaki
// Trasan 3.00 RC3 : a bug in stereo mode 1 and 2 is fixed
//
// Revision 1.67  2001/04/11 01:09:07  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.66  2001/01/29 09:27:44  yiwasaki
// Trasan 2.20 release : 2D & cosmic tracks are output, curler decision is made by dr and dz
//
// Revision 1.65  2000/04/14 05:20:44  yiwasaki
// Trasan 2.00rc30 : memory leak fixed, multi-track assignment to a hit fixed, helix parameter checks added
//
// Revision 1.64  2000/04/04 07:40:05  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.63  2000/03/24 10:22:52  yiwasaki
// Trasan 2.00RC20 : track manager bug fix
//
// Revision 1.62  2000/03/07 03:00:55  yiwasaki
// Trasan 2.00RC15 : only debug info. changed
//
// Revision 1.61  2000/03/01 04:51:06  yiwasaki
// Trasan 2.00RC14 : stereo bug fix, curl updates
//
// Revision 1.60  2000/02/29 07:16:14  yiwasaki
// Trasan 2.00RC13 : default stereo param. changed
//
// Revision 1.59  2000/02/25 12:55:39  yiwasaki
// Trasan 2.00RC10 : stereo improved
//
// Revision 1.58  2000/02/25 08:09:54  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.57  2000/02/23 08:45:04  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.56  2000/02/17 13:24:18  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.55  2000/02/15 13:46:38  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.54  2000/02/10 13:11:37  yiwasaki
// Trasan 2.00RC1 : conformal bug fix, parameters added
//
// Revision 1.53  2000/02/09 03:27:38  yiwasaki
// Trasan 1.68l : curl and new conf stereo updated
//
// Revision 1.52  2000/02/06 22:55:39  katayama
// Missing ifdef, missing type
//
// Revision 1.51  2000/02/03 06:18:02  yiwasaki
// Trasan 1.68k : slow finder in conf. finder temporary off
//
// Revision 1.50  2000/02/01 11:24:40  yiwasaki
// Trasan 1.68j : curl finder modified, new stereo finder modified
//
// Revision 1.49  2000/01/30 08:17:04  yiwasaki
// Trasan 1.67i = Trasan 2.00 RC1 : new conf modified
//
// Revision 1.48  2000/01/28 06:30:19  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.47  2000/01/23 08:23:03  yiwasaki
// Trasan 1.67g : slow finder added
//
// Revision 1.46  2000/01/18 07:00:28  yiwasaki
// Trasan 1.67e : TWindow modified
//
// Revision 1.45  1999/11/19 09:13:04  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.44  1999/10/30 10:12:08  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.43  1999/10/21 15:45:12  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.41  1999/09/21 02:01:32  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.39  1999/09/10 09:19:56  yiwasaki
// Trasan 1.61b release : new parameters added, TTrackMC bug fixed
//
// Revision 1.38  1999/08/25 06:25:47  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.37  1999/08/04 01:01:40  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.36  1999/07/27 08:01:35  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.35  1999/07/23 03:53:22  yiwasaki
// Trasan 1.57b release : minor changes only
//
// Revision 1.34  1999/07/09 01:47:18  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.33  1999/07/01 08:15:21  yiwasaki
// Trasan 1.51a release : builder bug fix, TRGCDC bug fix again, T0 determination has more parameters
//
// Revision 1.32  1999/06/15 06:33:40  yiwasaki
// Trasan 1.43 release : minor changes in TRGCDCClust and TBuilder
//
// Revision 1.31  1999/06/14 12:40:20  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.30  1999/06/10 09:44:50  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.29  1999/03/21 15:45:37  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.28  1999/03/11 23:27:09  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.27  1999/03/09 13:42:59  yiwasaki
// Trasan 1.22 release : TrackManager bug fix
//
// Revision 1.26  1999/03/08 05:47:51  yiwasaki
// Trasan 1.20 release : Fitter in TBuilder is modified
//
// Revision 1.25  1999/02/04 02:12:21  yiwasaki
// Trasan 1.15 release : bug fix in cluster table output from S.Suzuki
//
// Revision 1.24  1999/02/03 06:23:06  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.23  1999/01/11 03:03:04  yiwasaki
// Fitters added
//
// Revision 1.22  1998/12/24 08:46:53  yiwasaki
// stereo building modified by J.Suzuki
//
// Revision 1.21  1998/11/10 09:08:55  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.20  1998/10/09 17:35:31  yiwasaki
// Trasan 1.1 beta 6 release : TBuilder::buildStereo bug, introduced by y.iwasaki, removed.
//
// Revision 1.19  1998/10/09 03:01:04  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.18  1998/10/06 02:30:01  yiwasaki
// Trasan 1.1 beta 3 relase : only minor change
//
// Revision 1.17  1998/09/29 01:24:18  yiwasaki
// Trasan 1.1 beta 1 relase : TBuilderCurl added
//
// Revision 1.16  1998/08/31 05:15:39  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.15  1998/08/17 01:00:30  yiwasaki
// TSegmentLinker::roughSelection added, TRASAN_DEBUG option works
//
// Revision 1.14  1998/08/12 16:32:39  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.13  1998/07/29 04:34:46  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.9  1998/07/06 15:48:45  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.8  1998/07/02 09:04:35  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.7  1998/06/24 06:54:58  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.6  1998/06/21 18:38:14  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.5  1998/06/19 12:17:10  yiwasaki
// Trasan 1 beta 4.1 release, TBuilder::buildStereo updated
//
// Revision 1.4  1998/06/17 20:22:55  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.3  1998/06/17 20:12:33  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.2  1998/06/15 09:58:13  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.1  1998/06/14 11:09:47  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
//-----------------------------------------------------------------------------


#include <math.h>
#include <cfloat>
#include <map>

#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/TBuilder.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TLine0.h"
#include "tracking/modules/trasan/TLine.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TSegment.h"
#include "tracking/modules/trasan/TPoint2D.h"
#include "tracking/modules/trasan/TLine2D.h"
#include "tracking/modules/trasan/TRobustLineFitter.h"
#include "tracking/modules/trasan/THoughTransformationLine.h"
#include "tracking/modules/trasan/THoughPlane.h"
#include "tracking/modules/trasan/THoughPlaneMulti2.h"
#include "tracking/modules/trasan/TPeakFinder.h"
#ifdef TRASAN_DEBUG
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif
#ifdef TRASAN_WINDOW_GTK
#include "tracking/modules/trasan/Trasan.h"
#include "tracking/modules/trasan/TWindowGTKHough.h"
#endif
#ifdef TRASAN_WINDOW
#include "tracking/modules/trasan/TWindow.h"
#include "tracking/modules/trasan/TWindowHough.h"
TWindow sz("sz");
extern TWindowHough LWindow;
#endif

namespace Belle {

  TBuilder::TBuilder(const std::string& a,
                     float maxSigma,
                     float maxSigmaStereo,
                     float salvageLevel,
                     float szLinkDistance,
                     unsigned fittingFlag)
    : _name(a),
      _fitter("TBuilder Fitter"),
      _maxSigma(maxSigma),
      _maxSigmaStereo(maxSigmaStereo),
      _salvageLevel(sqrt(salvageLevel)),
      _szLinkDistance(szLinkDistance),
      _minNCores(3)
  {
    if (fittingFlag & 1) _fitter.sag(true);
    if (fittingFlag & 2) _fitter.propagation(true);
    if (fittingFlag & 4) _fitter.tof(true);
    if (fittingFlag & 8) _fitter.freeT0(true);
  }

  TBuilder::~TBuilder()
  {
  }

  void
  TBuilder::dump(const std::string&, const std::string&) const
  {
  }

  TTrack*
  TBuilder::buildRphi(const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::buildRphi";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "building rphi by links:#links=" << list.length()
              << std::endl;
#endif

    //...Classify TLink's...
    AList<TLink> cores;
    AList<TLink> nonCores;
    TLink::separateCores(list, cores, nonCores);

    //...Check # of links...
    unsigned nCores = cores.length();
    if (nCores < _minNCores) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout  << Tab() << "building rphi failure:#cores(" << nCores
                 << ") is less then " << _minNCores << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Make a track...
    TTrack* t = buildRphiFast(cores);
    bool trySlow = false;
    if (t)
      trySlow = (float(t->links().length()) / float(nCores) < 0.5);
    else
      trySlow = true;
    if (trySlow) {
      if (t) delete t;
      t = buildRphiSlow(cores);
    }

#ifdef TRASAN_DEBUG_DETAIL
    if (t)
      t->dump("breif", Tab() + "2d trk> ");
    else
      std::cout << Tab() << "failed to build 2d track" << std::endl;
#endif

    if (! t) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Try to append non-core hits...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "appending non-core hits:#=" << nonCores.length()
              << std::endl;
#endif
    t->appendByApproach(nonCores, _salvageLevel);
#ifdef TRASAN_DEBUG
    t->fitted(true);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    t->dump("hits sort flag", Tab() + "final 2d> ");
#endif

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return t;
  }

  void
  TBuilder::salvage(TTrack& t, AList<TLink> & hits) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::salvage";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#given hits=" << hits.length();
    std::cout << ",salvage level=" << _salvageLevel << std::endl;
    TLink::dump(hits, "hits sort flag", Tab());
#endif

    unsigned nHits = hits.length();
    if (nHits == 0) {
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return;
    }

    //...Try to append this hit...
    t.appendByApproach(hits, _salvageLevel);
    _fitter.fit(t);
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  TTrack*
  TBuilder::build(TTrack& t, const TLine& l) const
  {
    AList<TLink> links = l.links();
    AList<TLink> toRemove;
    unsigned n = links.length();
    for (unsigned i = 1; i < n; i++) {
      if (links[i - 1]->link() == links[i]->link()) {
        toRemove.append(links[i]);
        continue;
      }
      if (i < 2)
        continue;
      if (links[i - 2]->link() == links[i]->link())
        toRemove.append(links[i]);
    }
    links.remove(toRemove);

    //...Pick up links...
    n = links.length();
    for (unsigned i = 0; i < n; i++)
      t.append(* links[i]->link());

    CLHEP::HepVector a(5);
    a = t.helix().a();
    a[3] = l.b();
    a[4] = t.charge() * l.a();
    t._helix->a(a);

#ifdef TRASAN_DEBUG_DETAIL
    static unsigned nTrk = 0;
    ++nTrk;
//      for (unsigned i = 0; i < t.nCores(); i++)
//    if (t.cores()[i]->wire()->name() == "43=220") {
//        std::cout << "43=220 removed" << std::endl;
//        t.remove(* t.cores()[i]);
//    }
//cnv    HepGeom::Vector3D<double> pp;
//cnv    if (links[0]->hit()->mc())
//cnv pp = links[0]->hit()->mc()->hep()->p().vect();
//cnv    const HepGeom::Vector3D<double> p0 = pp;
//      if (nTrk == 1) {
//    THelix tmph(Point3D(0, 0, 0), p0, +1);
//    * t._helix = tmph;
//      }
    t.dump("breif", "before fit");
//cnv    std::cout << "Pdif mag=" << (t.p() - p0).mag() << std::endl;
//    t.links().removeAll();
//    for (unsigned i = 0; i < BsCouTab(RECCDC_WIRHIT
//    }
#endif

    //...Refine...
    AList<TLink> bad;
    _fitter.fit(t);

#ifdef TRASAN_DEBUG_DETAIL
    t.dump("breif", "after fit");
//cnv    std::cout << "Pdif mag=" << (t.p() - p0).mag() << std::endl;
#endif

    t.refine(bad, _maxSigmaStereo * 100.);
    _fitter.fit(t);
    t.refine(bad, _maxSigmaStereo * 10.);
    _fitter.fit(t);
    t.refine(bad, _maxSigmaStereo);
#ifdef TRASAN_DEBUG_DETAIL
//    if (nTrk == 2) {
//    THelix tmph(Point3D(0, 0, 0), p0, +1);
//    * t._helix = tmph;
//    std::cout << "initial mom" << p0 << " is set" << std::endl;
//    }
#endif
    _fitter.fit(t);

#ifdef TRASAN_DEBUG_DETAIL
    t.dump("breif", "         ");
//    std::cout << "Pdif mag=" << (t.p() - p0).mag() << std::endl;
#endif

    //...Termination...
    return & t;
  }

  TTrack*
  TBuilder::buildStereoOld(TTrack& track, const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::buildStereo";
    EnterStage(stage);
#endif

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "given #links = " << list.length() << std::endl;
#endif

    //...Classify TLink's...
    AList<TLink> cores;
    AList<TLink> nonCores;
    TLink::separateCores(list, cores, nonCores);

    //...Check # of links...
    unsigned nCores = cores.length();
    if (nCores < minNCores()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "stereo building failure:#cores(="
                << nCores << ") is less then " << _minNCores << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...Deteremine initial lines...
    AList<TLine> lines = initialLines(track, cores);
    const unsigned nLines = lines.length();
    if (! nLines) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "initial line failure:#cores=" << nCores
                << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...Initial line loop...
    TTrack* t = NULL;
//    for (unsigned i = 0; i < nLines; i++) {
    for (unsigned i = 0; i < 1; i++) {

      //...Pick up good links...
      AList<TLink> szLinks = selectStereoHits(track, * lines[i], list);
      const unsigned n = szLinks.length();

      //...3D fit...
      t = new TTrack(track);
      for (unsigned j = 0; j < n; j++)
        t->append(* szLinks[j]->link());
      CLHEP::HepVector a(5);
      a = t->helix().a();
      a[3] = lines[i]->b();
      a[4] = t->charge() * lines[i]->a();
      t->_helix->a(a);

      //...Refine...
      AList<TLink> bad;

      // int err = helixFitter().fit(* t);
      // t->refine(bad, maxSigmaStereo() * 100.);
      // err = helixFitter().fit(* t);
      // t->refine(bad, maxSigmaStereo() * 10.);
      // err = helixFitter().fit(* t);
      // t->refine(bad, maxSigmaStereo());
      // err = helixFitter().fit(* t);

      helixFitter().fit(* t);
      t->refine(bad, maxSigmaStereo() * 100.);
      helixFitter().fit(* t);
      t->refine(bad, maxSigmaStereo() * 10.);
      helixFitter().fit(* t);
      t->refine(bad, maxSigmaStereo());
      helixFitter().fit(* t);

#ifdef TRASAN_WINDOW
      sz.clear();
      sz.mode(2);
      sz.append(* lines[i], leda_green);
      sz.append(szLinks, leda_green);
      sz.text("stereo finished");
//  sz.oneShot(* t, leda_blue);
      sz.append(* t, leda_blue);
      sz.draw();
#endif
      HepAListDeleteAll(szLinks);
    }

    //...Termination...
    for (unsigned i = 0; i < nLines; i++)
      HepAListDeleteAll((AList<TLink> &) lines[i]->links());
    HepAListDeleteAll(lines);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return t;
  }

  AList<TLine>
  TBuilder::initialLinesOld(const TTrack& t,
                            const AList<TLink> & lList) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::initialLines";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "initlialLine:#links=" << lList.length()
              << std::endl;
#endif

    //...Static object...
    static TRobustLineFitter fitter("Robust Line Fitter");

    //...Make SZ links...
    const unsigned n = lList.length();
    AList<TLink> szLinks;
    float maxR = 0;
    for (unsigned i = 0; i < n; i++) {
      for (unsigned j = 0; j < 2; j++) {
        TLink& tt = * new TLink(* lList[i]);
        tt.leftRight(j);
        int err = t.szPosition(tt);
        if (err) {
          delete & tt;
          continue;
        }
        szLinks.append(tt);
        float r =
          tt.position().x() * tt.position().x() +
          tt.position().y() * tt.position().y();
        if (r > maxR) maxR = r;
      }
    }
    maxR *= 1.1;
    maxR = sqrt(maxR);

    //...Line Hough transformation...
    THoughTransformationLine lineHough("Line Hough in TBuilder");
    THoughPlane plane("Line Hough with sz links",
                      200, 0, 2 * M_PI,
                      200, 0, maxR);
//          500, - maxR, maxR);
    houghTransformation(szLinks, lineHough, plane);

    //...Peak finding...
    const unsigned threshold = plane.maxEntry() / 2 + 1;
    TPeakFinder pf;
    AList<TPoint2D> list = pf.peaks(plane, threshold, 2, 10);

#ifdef TRASAN_WINDOW
    LWindow.clear();
    LWindow.draw(plane);
    LWindow.draw(plane, list, 0.06);
//    LWindow.wait();
#endif

    //...Peak loop...
    const unsigned nl = list.length();
    AList<TLine> lines;
    for (unsigned i = 0; i < nl; i++) {

      //...Make a line...
      const float a = - 1 / tan(list[i]->x());
      const float b = list[i]->y() / sin(list[i]->x());
      TLine* line = new TLine();
      line->property(a, b, 0);
      lines.append(line);
#ifdef TRASAN_DEBUG
      line->fitted(true);
#endif
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "line " << i << ":a=" << a << ",b=" << b
                << ":phi=" << list[i]->x() << ",r=" << list[i]->y()
                << std::endl;
#endif
    }

#ifdef TRASAN_WINDOW
    sz.clear();
    sz.mode(2);
    for (unsigned i = 0; i < nl; i++)
      sz.append(* lines[i], leda_black);
    sz.append(szLinks, leda_brown);
    sz.text("initial lines");
//    sz.wait();
#endif

    //...Termination...
    HepAListDeleteAll(szLinks);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return lines;
  }

  void
  TBuilder::houghTransformation(const AList<TLink> & hits,
                                const THoughTransformation& trans,
                                THoughPlane& plane) const
  {
    const unsigned n = hits.length();
    for (unsigned i = 0; i < n; i++) {
      const TLink& l = * hits[i];
      const float x = l.position().x();
      const float y = l.position().y();
      plane.vote(x, y, trans);
    }
  }

  AList<TLink>
  TBuilder::selectStereoHits(const TTrack& track,
                             const TLine& line,
                             const AList<TLink> & links) const
  {
    AList<TLink> goodLinks;

    const unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      const TLink& t = * links[i];
      TLink* tt[2] = {NULL, NULL};
      float distance[2] = {FLT_MAX, FLT_MAX};
      for (unsigned j = 0; j < 2; j++) {
        tt[j] = new TLink(t);
        tt[j]->leftRight(j);
        int err = track.szPosition(* tt[j]);
        if (! err) {
          distance[j] = line.distance(* tt[j]);
        }
      }

      if ((distance[0] < distance[1]) &&
          (distance[0] < szLinkDistance())) {
        tt[0]->link(& (TLink&) t);
        goodLinks.append(tt[0]);
        delete tt[1];
      } else if ((distance[1] < distance[0]) &&
                 (distance[1] < szLinkDistance())) {
        tt[1]->link(& (TLink&) t);
        goodLinks.append(tt[1]);
        delete tt[0];
      } else {
        delete tt[0];
        delete tt[1];
      }
    }

    return goodLinks;
  }

  TLine*
  TBuilder::initialLine(const TTrack& t,
                        const AList<TLink> & lList) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    ... initlialLine : # of links = " << lList.length();
    std::cout << std::endl;
#endif

    //...Static object...
    static TRobustLineFitter fitter("Robust Line Fitter");

    //...Make SZ links...
    const unsigned n = lList.length();
    AList<TLink> szLinks;
    for (unsigned i = 0; i < n; i++) {
      TLink& tt = * new TLink(* lList[i]);
      for (unsigned j = 0; j < 2; j++) {
        tt.leftRight(j);
        int err = t.szPosition(tt);
        if (err) {
          delete & tt;
          continue;
        }
        szLinks.append(tt);
      }
    }

    //...Line Hough transformation...
    THoughTransformationLine lineHough("Line Hough in TBuilder");
    THoughPlane plane("Line Hough with sz links",
                      100, 0, 2 * M_PI,
                      100, 0, 200);
    houghTransformation(szLinks, lineHough, plane);

//     //...Peak finding...
//     AList<TPoint2D> list = _peakFinder.peaks(cPlane);

//     //...Peak loop...
//     const unsigned n = list.length();
//     for (unsigned i = 0; i < n; i++) {

//  //...Make a line...
//  const float a = 1 / tan(list[0]->x());
//  const float b = list[0]->x() / sin(list[0]->y());
//     }




    TLine* line = new TLine(szLinks);
//     int err = fitter.fit(* line);
//     if (err) {
//  HepAListDeleteAll(szLinks);
//  delete line;
//  return NULL;
//     }
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "        initial line" << std::endl;
    line->dump("detail position", "    ");
#endif
    return line;
  }

  void
  TBuilder::refine(TTrack& t, AList<TLink> & list, double maxSigma) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::refine";
    EnterStage(stage);
#endif

    //...First trial...
    const unsigned n0 = t.links().length();
    AList<TLink> bad;
    for (unsigned i = 0; i < n0; i++) {
      if ((t.links())[i]->pull() < maxSigma) continue;
      bad.append((t.links())[i]);
    }
//  float badFraction = float(bad.length()) / float(t.links().length());
    float badFraction = 1;
    if (n0) badFraction = float(bad.length()) / float(n0);
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "bad fraction=" << badFraction << std::endl;
#endif
    if (badFraction < 0.5) {
      list.append(bad);
      t.remove(bad);
      _fitter.fit(t);
#ifdef TRASAN_DEBUG_DETAIL
      TLink::dump(bad, "sort pull mc", Tab() + "bads=");
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return;
    }

    bool finished = false;
    while (! finished) {
      unsigned n = t.links().length();

      //...Search for maximum sigma...
      float worst = 0;
      float iWorst = 999;
      for (unsigned i = 0; i < n; i++) {
        if ((t.links())[i]->pull() < maxSigma) continue;
        if ((t.links())[i]->pull() > worst) {
          worst = (t.links())[i]->pull();
          iWorst = i;
        }
      }

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "removed hits" << std::endl;
      TLink::dump(list, "pull mc", Tab());
#endif

      if (iWorst == 999) {
#ifdef TRASAN_DEBUG
        LeaveStage(stage);
#endif
        return;
      }

      //...Remove the worst, and fit it again...
      list.append((t.links())[(int) iWorst]);
      t.remove(* (t.links())[(int) iWorst]);
      _fitter.fit(t);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "removed hits" << std::endl;
    TLink::dump(list, "sort pull mc", Tab());
#endif
#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  TTrack*
  TBuilder::buildRphiSlow(AList<TLink> & list) const
  {

#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::buildRphiSlow";
    EnterStage(stage);
#endif

    //...Select hits which is isolated...
    AList<TLink> good;
    AList<TLink> normal;
    for (unsigned i = 0; i < (unsigned) list.length(); i++) {
      unsigned state = list[i]->hit()->state();
      if ((state & CellHitIsolated) && (state & CellHitContinuous))
        good.append(list[i]);
      else
        normal.append(list[i]);
    }

    //...Make a circle...
    TCircle c(good);
    int err = c.fit();
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "slow building rphi failure:circle fit error="
                << err << std::endl;
//  std::cout << Tab() << "retry with IP constraint" << std::endl;
#endif
//  err = c.fitForCurl(1);
//  if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
//      std::cout << Tab() << "retry with IP constraint failed" << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
//  }
    }
#ifdef TRASAN_DEBUG_DETAIL
    c.dump("breif", Tab() + "circle> ");
#endif

    //...Make a track...
    TTrack* t = new TTrack(c);
    err = _fitter.fit(* t);
    if (err < 0) {
      delete t;
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Refine...
    AList<TLink> bad;
//     t->refine(bad, _maxSigma * 300.);
//     err = _fitter.fit(* t);
//     t->refine(bad, _maxSigma * 100.);
//     err = _fitter.fit(* t);
//     t->refine(bad, _maxSigma * 10.);
//     err = _fitter.fit(* t);

    //...Final refine...
//  t->refine(bad, _maxSigma);
//  err = _fitter.fit(* t);
    refine(* t, bad, _maxSigma);

    //...Try to append remaining hits...
    t->appendByApproach(normal, _salvageLevel);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return t;
  }

  TTrack*
  TBuilder::buildRphiFast(AList<TLink> & list) const
  {

#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::buildRphiFast";
    EnterStage(stage);
#endif

    //...Make a circle...
    TCircle c(list);
    int err = c.fit();
    if (err < 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "fast building rphi failure:circle fit error="
                << err << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }
#ifdef TRASAN_DEBUG_DETAIL
    c.dump("breif", Tab() + "circle> ");
#endif

    //...Make a track...
    TTrack* t = new TTrack(c);
    err = _fitter.fit(* t);
    if (err < 0) {
      delete t;
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return 0;
    }

    //...Refine...
    AList<TLink> bad;
    t->refine(bad, _maxSigma * 300.);
    err = _fitter.fit(* t);
    t->refine(bad, _maxSigma * 100.);
    err = _fitter.fit(* t);
    t->refine(bad, _maxSigma * 10.);
    err = _fitter.fit(* t);

    //...Final refine...
//  t->refine(bad, _maxSigma);
//  err = _fitter.fit(* t);
    refine(* t, bad, _maxSigma);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return t;
  }

  TTrack*
  TBuilder::buildStereo(TTrack& track, const AList<TLink> & list) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::buildStereo";
    EnterStage(stage);
#endif

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "given #links = " << list.length() << std::endl;
#endif

    //...Classify TLink's...
    AList<TLink> cores;
    AList<TLink> nonCores;
    TLink::separateCores(list, cores, nonCores);

    //...Check # of links...
    unsigned nCores = cores.length();
    if (nCores < minNCores()) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "stereo building failure:#cores(="
                << nCores << ") is less then " << _minNCores << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...Deteremine initial lines...
//  AList<TLine> lines = initialLines(track, cores);
    AList<TLine> lines = initialLines2(track, cores);
    const unsigned nLines = lines.length();
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#initial lines=" << nLines << std::endl;
#endif
    if (! nLines) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "initial line failure:#cores=" << nCores
                << std::endl;
#endif
#ifdef TRASAN_DEBUG
      LeaveStage(stage);
#endif
      return NULL;
    }

    //...Initial line loop...
    AList<TTrack> trackList;
#ifdef TRASAN_WINDOW_GTK_SZ
    std::map<TTrack*, TLine*> tl;
#endif
    for (unsigned i = 0; i < nLines; i++) {

      //...Pick up good links...
      AList<TLink> szLinks = selectStereoHits(track, * lines[i], list);
      const unsigned n = szLinks.length();

      //...3D fit...
      TTrack* t = new TTrack(track);
      for (unsigned j = 0; j < n; j++)
        t->append(* szLinks[j]->link());
      CLHEP::HepVector a(5);
      a = t->helix().a();
      a[3] = lines[i]->b();
      a[4] = t->charge() * lines[i]->a();
      t->_helix->a(a);

      //...Refine...
      AList<TLink> bad;
      helixFitter().fit(* t);
      t->refine(bad, maxSigmaStereo() * 100.);
      helixFitter().fit(* t);
      t->refine(bad, maxSigmaStereo() * 10.);
      helixFitter().fit(* t);
      t->refine(bad, maxSigmaStereo());
      helixFitter().fit(* t);

#ifdef TRASAN_WINDOW
      sz.clear();
      sz.mode(2);
      sz.append(* lines[i], leda_green);
      sz.append(szLinks, leda_green);
      sz.text("stereo finished");
//  sz.oneShot(* t, leda_blue);
      sz.append(* t, leda_blue);
      sz.draw();
#endif
#ifdef TRASAN_WINDOW_GTK_SZ
      tl[t] = lines[i];
#endif
      HepAListDeleteAll(szLinks);
      trackList.append(t);
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "#tracks found=" << trackList.length() << std::endl;
#endif
    //...Select the best one...
    unsigned maxHits = 0;
    TTrack* best = 0;
    for (unsigned i = 0; i < (unsigned) trackList.length(); i++) {
      if (maxHits < (unsigned) trackList[i]->links().length()) {
        maxHits = trackList[i]->links().length();
        best = trackList[i];
      }
    }

    if (best) {
      trackList.remove(best);
      HepAListDeleteAll(trackList);
    }

#ifdef TRASAN_WINDOW_GTK_SZ
    TWindowGTKSZ& sz = Trasan::getTrasan()->sz();
    sz.clear();
    sz.stage("3D track found");
    sz.information("blue:best line, green:candidate hits, grey:candidate lines [one hit two entries]");
    AList<TLink> tb;
    AList<TLink> tbBest;
    if (best) {
	for (unsigned i = 0; i < unsigned(cores.length()); i++) {
        for (unsigned j = 0; j < 2; j++) {
          TLink* a = new TLink(* cores[i]);
          a->leftRight(j);
          int err = best->szPosition(* a);
          if (err) {
            delete a;
            continue;
          }
          tb.append(* a);
          if (best->links().hasMember(cores[i]))
            tbBest.append(* a);
        }
      }
    }
    sz.append(tb, Gdk::Color("green"));
    sz.append(lines);
    if (tl[best])
      sz.append(* tl[best], Gdk::Color("blue"));
    sz.append(tbBest, Gdk::Color("blue"));
    sz.show();
    sz.run();
    HepAListDeleteAll(tb);
#endif

    //...Termination...
    for (unsigned i = 0; i < nLines; i++)
      HepAListDeleteAll((AList<TLink> &) lines[i]->links());
    HepAListDeleteAll(lines);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return best;
  }

  AList<TLine>
  TBuilder::initialLines(const TTrack& t,
                         const AList<TLink> & lList) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::initialLines";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "initlialLine:#links=" << lList.length() << std::endl;
#endif

    //...Make SZ links...
    const unsigned n = lList.length();
    AList<TLink> szLinks;
    float maxR = 0;
    for (unsigned i = 0; i < n; i++) {
      for (unsigned j = 0; j < 2; j++) {
        TLink& tt = * new TLink(* lList[i]);
        tt.leftRight(j);
        int err = t.szPosition(tt);
        if (err) {
          delete & tt;
          continue;
        }
        szLinks.append(tt);
        float r =
          tt.position().x() * tt.position().x() +
          tt.position().y() * tt.position().y();
        if (r > maxR) maxR = r;
      }
    }
    maxR *= 1.1;
    maxR = sqrt(maxR);

    //...Line Hough transformation...
    THoughTransformationLine lineHough("Line Hough in TBuilder");
    THoughPlane plane("Line Hough with sz links",
//          100, 0, 2 * M_PI,
//          100, 0, maxR);
                      200, 0, 2 * M_PI,
                      200, 0, maxR);
// //         500, - maxR, maxR);
    houghTransformation(szLinks, lineHough, plane);

    //...Peak finding...
    const unsigned threshold = plane.maxEntry() / 2 + 1;
    TPeakFinder pf;
    AList<TPoint2D> list = pf.peaks5(plane, threshold);

#ifdef TRASAN_WINDOW_GTK_SZ
    TWindowGTKHough& hl = Trasan::getTrasan()->hl();
    hl.clear();
    hl.append(& plane);
    hl.show();
    hl.run();
#endif
#ifdef TRASAN_WINDOW
    LWindow.clear();
    LWindow.draw(plane);
    LWindow.draw(plane, list, 0.06, leda_blue);
    AList<TPoint2D> tmp;
    if (list.length()) tmp.append(list[0]);
    LWindow.draw(plane, tmp, 0.06);
//    LWindow.wait();
#endif

    //...Peak sort...
    const unsigned nl = list.length();
    if (nl) {
      for (unsigned i = 0; i < nl - 1; i++) {
        float ni = plane.entry(plane.serialID(* list[i]));
        for (unsigned j = i + 1; j < nl; j++) {
          float nj = plane.entry(plane.serialID(* list[j]));
          if (ni < nj) {
            list.swap(i, j);
            ni = nj;
          }
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << nl << " peaks" << std::endl;
    if (nl) {
      for (unsigned i = 0; i < nl; i++)
        std::cout << Tab() << i << ":"
                  << plane.entry(plane.serialID(* list[i])) << std::endl;
    }
#endif

    //...Peak loop...
    AList<TLine> lines;
    for (unsigned i = 0; i < nl; i++) {

      //...Make a line...
      const float a = - 1 / tan(list[i]->x());
      const float b = list[i]->y() / sin(list[i]->x());
      TLine* line = new TLine();
      line->property(a, b, 0);
      lines.append(line);
#ifdef TRASAN_DEBUG
      line->fitted(true);
#endif
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "line " << i << ":a=" << a << ",b=" << b
                << ":phi=" << list[i]->x() << ",r=" << list[i]->y()
                << std::endl;
#endif
    }

#ifdef TRASAN_WINDOW_GTK_SZ
    TWindowGTKSZ& sz = Trasan::getTrasan()->sz();
    sz.clear();
    sz.append(szLinks, Gdk::Color("green"));
    sz.append(lines);
    sz.show();
    sz.run();
#endif
#ifdef TRASAN_WINDOW
    sz.clear();
    sz.mode(2);
    for (unsigned i = 0; i < nl; i++)
      sz.append(* lines[i], leda_black);
    sz.append(szLinks, leda_brown);
    sz.text("initial lines");
//    sz.wait();
#endif

    //...Termination...
    HepAListDeleteAll(szLinks);
    HepAListDeleteAll(list);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return lines;
  }

  AList<TLine>
  TBuilder::initialLines2(const TTrack& t,
                          const AList<TLink> & lList) const
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TBldr::initialLines2";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "initlialLine:#links=" << lList.length() << std::endl;
#endif

    //...Make SZ links...
    const unsigned n = lList.length();
    AList<TLink> szLinks;
//  float maxR = 0;
    for (unsigned i = 0; i < n; i++) {
      for (unsigned j = 0; j < 2; j++) {
        TLink& tt = * new TLink(* lList[i]);
        tt.leftRight(j);
        int err = t.szPosition(tt);
        if (err) {
          delete & tt;
          continue;
        }
        szLinks.append(tt);
//        float r =
//      tt.position().x() * tt.position().x() +
//      tt.position().y() * tt.position().y();
//        if (r > maxR) maxR = r;
      }
    }
//  maxR = sqrt(maxR * 1.1);

//  std::cout << "maxR=" << maxR << std::endl;
//  maxR = 180;
    const float maxR = 112.72;

    //...Line Hough transformation...
    THoughTransformationLine lineHough("Line Hough in TBuilder");
    static THoughPlaneMulti2 plane("Line Hough with sz links",
                                   200, 0, 2 * M_PI,
                                   200, 0, maxR,
                                   50);
//  plane.yMax(maxR);
    plane.clear();
//  plane.dump();
    for (unsigned i = 0; i < (unsigned) szLinks.length(); i++) {
      const TLink& l = * szLinks[i];
      const float x = l.position().x();
      const float y = l.position().y();
      plane.vote(x, y, lineHough, l.wire()->layerId());
    }
    plane.merge();
    plane.clearRegion();

    //...Peak finding...
    const unsigned threshold = plane.maxEntry() / 2 + 1;
    TPeakFinder pf;
    AList<TPoint2D> list = pf.peaks5(plane, threshold);

#ifdef TRASAN_WINDOW_GTK_SZ
    TWindowGTKHough& hl = Trasan::getTrasan()->hl();
    hl.clear();
    hl.append(& plane);
    hl.show();
    hl.run();
#endif
#ifdef TRASAN_WINDOW
    LWindow.clear();
    LWindow.draw(plane);
    LWindow.draw(plane, list, 0.06, leda_blue);
    AList<TPoint2D> tmp;
    if (list.length()) tmp.append(list[0]);
    LWindow.draw(plane, tmp, 0.06);
//    LWindow.wait();
#endif

    //...Peak sort...
    const unsigned nl = list.length();
    if (nl) {
      for (unsigned i = 0; i < nl - 1; i++) {
        float ni = plane.entry(plane.serialID(* list[i]));
        for (unsigned j = i + 1; j < nl; j++) {
          float nj = plane.entry(plane.serialID(* list[j]));
          if (ni < nj) {
            list.swap(i, j);
            ni = nj;
          }
        }
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << nl << " peaks" << std::endl;
    if (nl) {
      for (unsigned i = 0; i < nl; i++)
        std::cout << Tab() << i << ":"
                  << plane.entry(plane.serialID(* list[i])) << " entires,"
                  << "x_h=" << list[i]->x() << ",y_h=" << list[i]->y()
                  << std::endl;
    }
#endif

    //...Peak loop...
    AList<TLine> lines;
    for (unsigned i = 0; i < nl; i++) {

      //...Make a line...
      const float a = - 1 / tan(list[i]->x());
      const float b = list[i]->y() / sin(list[i]->x());
      TLine* line = new TLine();
      line->property(a, b, 0);
      lines.append(line);
#ifdef TRASAN_DEBUG
      line->fitted(true);
#endif
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << Tab() << "line " << i << ":a=" << a << ",b=" << b
                << ":phi=" << list[i]->x() << ",r=" << list[i]->y()
                << std::endl;
#endif
    }

#ifdef TRASAN_WINDOW_GTK_SZ
    TWindowGTKSZ& sz = Trasan::getTrasan()->sz();
    sz.clear();
    sz.append(szLinks, Gdk::Color("green"));
    sz.append(lines);
    sz.show();
    sz.run();
#endif
#ifdef TRASAN_WINDOW
    sz.clear();
    sz.mode(2);
    for (unsigned i = 0; i < nl; i++)
      sz.append(* lines[i], leda_black);
    sz.append(szLinks, leda_brown);
    sz.text("initial lines");
//    sz.wait();
#endif

    //...Termination...
    HepAListDeleteAll(szLinks);
    HepAListDeleteAll(list);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif

    return lines;
  }

} // namespace Belle

