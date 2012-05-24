//-----------------------------------------------------------------------------
// $Id: TConformalFinder0.cc 10509 2008-05-01 14:25:46Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TConformalFinder0.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the conformal method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// TConformalLink removed. TSegment added.
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.10  2001/12/23 09:58:46  katayama
// removed Strings.h
//
// Revision 1.9  2001/12/19 02:59:45  katayama
// Uss find,istring
//
// Revision 1.8  2000/04/29 03:45:55  yiwasaki
// old conf. fixed, new conf. unchanged
//
// Revision 1.7  2000/04/11 13:05:42  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.6  2000/03/21 07:01:26  yiwasaki
// tmp updates
//
// Revision 1.5  2000/03/17 11:01:38  yiwasaki
// Trasan 2.00RC16 : updates for new tracking scheme
//
// Revision 1.4  2000/01/28 06:30:22  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.3  1999/12/15 08:39:24  yiwasaki
// memory leak stopped
//
// Revision 1.2  1999/10/30 10:12:17  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.1  1999/10/21 15:45:15  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.59  1999/09/21 02:01:33  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.57  1999/08/25 06:25:49  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.56  1999/08/04 01:01:41  yiwasaki
// Trasan 1.59b release : putting parameters from basf enabled
//
// Revision 1.55  1999/07/27 08:01:37  yiwasaki
// Trasan 1.58b release : protection for fpe error(not completed yet)
//
// Revision 1.54  1999/07/23 04:47:23  yiwasaki
// comment out debug option
//
// Revision 1.53  1999/07/23 03:53:23  yiwasaki
// Trasan 1.57b release : minor changes only
//
// Revision 1.52  1999/07/09 01:47:20  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.51  1999/06/26 07:05:43  yiwasaki
// Trasan 1.47a release : hit and tracking efficiency improved
//
// Revision 1.50  1999/06/15 06:33:41  yiwasaki
// Trasan 1.43 release : minor changes in TRGCDCClust and TBuilder
//
// Revision 1.49  1999/06/14 12:40:21  yiwasaki
// Trasan 1.42 release : bug in findCloseHits fixed, sakura 1.06
//
// Revision 1.48  1999/06/10 09:44:51  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.47  1999/06/10 00:27:27  yiwasaki
// Trasan 1.39 release : TTrack::approach bug fix
//
// Revision 1.46  1999/05/18 04:44:29  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.45  1999/03/21 15:45:40  yiwasaki
// Trasan 1.28 release : TrackManager bug fix, CosmicFitter added in BuilderCosmic, parameter salvage level added
//
// Revision 1.44  1999/03/11 23:27:13  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.43  1999/03/09 06:29:23  yiwasaki
// Trasan 1.21 release : conformal finder bug fix
//
// Revision 1.42  1999/02/03 06:23:07  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.40  1999/01/11 03:09:08  yiwasaki
// Trasan 1.11 release
//
// Revision 1.39  1999/01/11 03:03:10  yiwasaki
// Fitters added
//
// Revision 1.38  1998/11/27 08:15:30  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.37  1998/11/12 12:27:25  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.36  1998/11/11 07:26:45  yiwasaki
// Trasan 1.1 beta 9 release : more protections for negative sqrt and zero division
//
// Revision 1.35  1998/11/10 09:09:05  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.34  1998/10/13 04:04:45  yiwasaki
// Trasan 1.1 beta 7 release : memory leak fixed by J.Tanaka, TCurlFinderParameters.h added by J.Tanaka
//
// Revision 1.33  1998/10/09 03:01:06  yiwasaki
// Trasan 1.1 beta 4 release : memory leak stopped, and minor changes
//
// Revision 1.32  1998/09/28 16:52:10  yiwasaki
// TBuilderCosmic added
//
// Revision 1.31  1998/09/24 22:56:37  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.30  1998/08/12 16:32:47  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.29  1998/08/03 15:01:00  yiwasaki
// Trasan 1.07 release : cluster finder from S.Suzuki-san added
//
// Revision 1.28  1998/07/29 04:34:50  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.25  1998/07/06 15:48:47  yiwasaki
// Trasan 1.01 release:CurlFinder default on, bugs in TLine and TTrack::fit fixed
//
// Revision 1.24  1998/07/02 09:04:38  yiwasaki
// Trasan 1.0 official release
//
// Revision 1.23  1998/06/29 02:15:07  yiwasaki
// Trasan 1 release candidate 1
//
// Revision 1.22  1998/06/24 06:55:01  yiwasaki
// Trasan 1 beta 5.2 release, memory leaks fixed
//
// Revision 1.21  1998/06/21 18:38:19  yiwasaki
// Trasan 1 beta 5 release, rphi improved?
//
// Revision 1.20  1998/06/17 20:22:58  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.19  1998/06/17 20:12:36  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.18  1998/06/15 09:58:14  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.17  1998/06/15 03:34:17  yiwasaki
// Trasan 1 beta 3.1, compiler error fixed
//
// Revision 1.16  1998/06/14 11:09:50  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.15  1998/06/11 12:25:15  yiwasaki
// TConformalLink removed
//
// Revision 1.14  1998/06/11 08:14:06  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.13  1998/06/08 14:37:52  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.12  1998/06/03 17:17:37  yiwasaki
// const added to TRGCDC::hits,axialHits,stereoHits,hitsMC, symbols WireHitNeghborHit* added in TRGCDCWireHit, TSegment::innerWidth,outerWidth,innerMostLayer,outerMostLayer,type,split,split2,widht,outer,updateType added, TLink::conf added, TTrack::appendStereo3,refineStereo2,aa,bb,Zchisqr added
//
// Revision 1.11  1998/05/26 05:10:17  yiwasaki
// cvs repair
//
// Revision 1.10  1998/05/22 18:20:56  yiwasaki
// Range moved to com-cdc
//
// Revision 1.9  1998/05/22 08:23:18  yiwasaki
// Trasan 1 alpha 4 release, TSegment added, TConformalLink no longer used
//
// Revision 1.8  1998/05/18 08:08:53  yiwasaki
// preparation for alpha 3
//
// Revision 1.7  1998/05/11 10:16:56  yiwasaki
// TTrack::assign -> TTrack::assert, WireHitUsedMask is set in TRGCDCWireHit
//
// Revision 1.6  1998/05/08 09:45:43  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.5  1998/04/23 17:21:37  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.4  1998/04/16 16:49:33  yiwasaki
// minor changes
//
// Revision 1.3  1998/04/14 01:04:48  yiwasaki
// TRGCDCWireHitMC added
//
// Revision 1.2  1998/04/10 09:36:27  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:50:14  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------





#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "tracking/modules/trasan/TConformalFinder0.h"
#include "tracking/modules/trasan/TBuilderCosmic.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/THistogram.h"
#include "tracking/modules/trasan/TCircle.h"
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/TSegment0.h"
#include "tracking/modules/trasan/Range.h"

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  std::string
  TConformalFinder0::version(void) const
  {
    return "1.63";
  }

  TConformalFinder0::TConformalFinder0(float maxSigma,
                                       float fraction,
                                       float stereoZ3,
                                       float stereoZ4,
                                       float stereoChisq3,
                                       float stereoChisq4,
                                       float stereoMaxSigma,
                                       unsigned fittingCorrections,
                                       float salvageLevel,
                                       bool cosmic)
    : TFinderBase(),
      _builder(0),
      _fraction(fraction),
      _doStereo(true),
      _doSalvage(false)
  {

    //...Parameters for a track...
    _trackSelector.nLinks(4);
    _trackSelector.nSuperLayers(2);
    _trackSelector.minPt(0.05);
    _trackSelector.maxImpact(100.);
    _trackSelector.maxSigma(maxSigma);
    _trackSelector.nLinksStereo(3);
    _trackSelector.maxDistance(30.);

    //...Make a builder...
    if (cosmic) _builder = new TBuilderCosmic("cosmic builder", salvageLevel);
    else        _builder = new TBuilder0("conformal builder",
                                           stereoZ3,
                                           stereoZ4,
                                           stereoChisq3,
                                           stereoChisq4,
                                           stereoMaxSigma,
                                           fittingCorrections,
                                           salvageLevel);

    //...Set up TBuilder...
    _builder->trackSelector(_trackSelector);
  }

  TConformalFinder0::~TConformalFinder0()
  {
    delete _builder;
  }

  void
  TConformalFinder0::dump(const std::string& msg, const std::string& pre) const
  {
    std::cout << pre;
    TFinderBase::dump(msg);
    std::cout << pre;
    if (msg.find("state") != std::string::npos) {
      std::cout << "#axialConfPos=" << _axialConfLinks.length();
      std::cout << ",#stereoConfPos=" << _stereoConfLinks.length();
    }
  }

  void
  TConformalFinder0::clear(void)
  {
    HepAListDeleteAll(_axialConfLinks);
    HepAListDeleteAll(_stereoConfLinks);
    _unusedAxialConfLinks.removeAll();
    _unusedStereoConfLinks.removeAll();
    _goodAxialConfLinks.removeAll();
    HepAListDeleteAll(_circles);
    _tracks.removeAll();
  }

  void
  TConformalFinder0::conformalTransformation(const HepGeom::Point3D<double> & center,
                                             const CAList<Belle2::TRGCDCWireHit> & hits,
                                             AList<TLink> & links)
  {

    unsigned nHits = hits.length();
    if (center == ORIGIN) {
      for (unsigned i = 0; i < nHits; i++) {
        const Belle2::TRGCDCWireHit* h = hits[i];
        const HepGeom::Point3D<double> & p = h->xyPosition();
        HepGeom::Point3D<double> cp(2. * p.x() / p.mag2(), 2. * p.y() / p.mag2(), 0);
        links.append(new TLink(0, h, cp));
      }
    } else {
      for (unsigned i = 0; i < nHits; i++) {
        const Belle2::TRGCDCWireHit* h = hits[i];
        HepGeom::Point3D<double> p(h->xyPosition() - center);
        HepGeom::Point3D<double> cp(2. * p.x() / p.mag2(), 2. * p.y() / p.mag2(), 0);
        links.append(new TLink(0, h, cp));
      }
    }
  }

  void
  TConformalFinder0::conformalTransformationRphi(const HepGeom::Point3D<double> & center,
                                                 const CAList<Belle2::TRGCDCWireHit> & hits,
                                                 AList<TLink> & links)
  {

    unsigned nHits = hits.length();
    if (center == ORIGIN) {
      for (unsigned i = 0; i < nHits; i++) {
        const Belle2::TRGCDCWireHit* h = hits[i];
        const HepGeom::Point3D<double> & p = h->xyPosition();
        HepGeom::Point3D<double> cp(2. * p.x() / p.mag2(), 2. * p.y() / p.mag2(), 0);
        double r = log(cp.mag()) + 4.;
        double phi = atan2(cp.y(), cp.x()) + M_PI;
        HepGeom::Point3D<double> cpt(phi, r, 0.);
        links.append(new TLink(0, h, cpt));
      }
    } else {
      for (unsigned i = 0; i < nHits; i++) {
        const Belle2::TRGCDCWireHit* h = hits[i];
        HepGeom::Point3D<double> p(h->xyPosition() - center);
        HepGeom::Point3D<double> cp(2. * p.x() / p.mag2(), 2. * p.y() / p.mag2(), 0);
        double r = log(cp.mag()) + 4.;
        double phi = atan2(cp.y(), cp.x()) + M_PI;
        HepGeom::Point3D<double> cpt(phi, r, 0.);
        links.append(new TLink(0, h, cpt));
      }
    }
  }

  AList<TSegment0>
  TConformalFinder0::findClusters(const THistogram& hist) const
  {

    //...Obtain raw clusters...
    AList<TSegment0> list = hist.clusters0();
    unsigned n = list.length();
    if (n == 0) return list;

#ifdef TRASAN_DEBUG_DETAIL
    // static TChecker chk0("clusters before splitting");
    // chk0.check(list);
    // chk0.dump("detail", "    ");
#endif

    //...Examine each cluster...
    AList<TSegment0> splitted;
    for (unsigned i = 0; i < n; i++) {
      TSegment0* c = list[i];

      AList<TSegment0> newClusters = c->split();
      if (newClusters.length() == 0) {
        c->solveDualHits();
        continue;
      }

      list.append(newClusters);
      splitted.append(c);
#ifdef TRASAN_DEBUG_DETAIL
      c->dump("hits", "    ");
      std::cout << "    ... splitted as" << std::endl;
      for (unsigned j = 0; j < (unsigned) newClusters.length(); j++) {
        std::cout << "    " << j << " : ";
        newClusters[j]->dump("hits");
      }
#endif
    }
    list.remove(splitted);
    HepAListDeleteAll(splitted);

#ifdef TRASAN_DEBUG_DETAIL
    // static TChecker chk1("clusters after splitting");
    // chk1.check(list);
    // chk1.dump("detail", "    ");
#endif

    return list;
  }

  AList<TSegment0>
  TConformalFinder0::findClusters2(const THistogram& hist) const
  {

    //...Obtain raw clusters...
    AList<TSegment0> list = hist.clusters0();
    unsigned n = list.length();
    if (n == 0) return list;

#ifdef TRASAN_DEBUG_DETAIL
    // static TChecker chk0("clusters before splitting (2)");
    // chk0.check(list);
    // chk0.dump("detail", "    ");
#endif

    //...Examine each cluster...
    for (unsigned i = 0; i < n; i++) {
      TSegment0* c = list[i];
      unsigned type = c->clusterType();

      if ((type == 1) || (type == 2)) {
        c->dump("hits mc", "    ");
        c->solveDualHits();
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    // static TChecker chk1("clusters after splitting (2)");
    // chk1.check(list);
    // chk1.dump("detail", "    ");
#endif

    return list;
  }

  AList<TLink>
  TConformalFinder0::findCloseHits(const AList<TLink> & links,
                                   const TTrack& track) const
  {
    //
    // Coded by J.Suzuki
    //
    AList<TLink> list;

    //...Check condition...
    if (track.links().length() == 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TConformalFinder0::findCloseHits !!! ";
      std::cout << " no links found in a track : This should not be happened";
      std::cout << std::endl;
#endif

      return list;
    }

    //...Parameters...
    //    float dRcut[11] = {0, 3.5, 0., 5.5, 0., 6.5, 0., 7.5, 0., 9.5, 0.};
    float dRcut[11] = {0., 4.3, 0., 6.5, 0., 7.5, 0., 8.0, 0., 9.5, 0.};

    //...Select Stereo hits associated to the current r-phi curve...
    double R0 = track.helix().curv();
    double xInnerWire = track.links()[0]->wire()->xyPosition().x();
    double yInnerWire = track.links()[0]->wire()->xyPosition().y();
    unsigned nall = links.length();
    for (unsigned j = 0; j < nall; j++) {
      TLink& t = * links[j];
      const Belle2::TRGCDCWire& w = * t.wire();
      Vector3D X = w.xyPosition() - track.helix().center();
      double Rmag2 = X.mag2();
      double DR = fabs(sqrt(Rmag2) - fabs(R0));
      t.zStatus(-10);
      t.zPair(0);
      if (DR < dRcut[w.superLayerId()] &&
          (xInnerWire * w.xyPosition().x() + yInnerWire * w.xyPosition().y()) > 0.) {
        list.append(t);
      }
    }

    return list;
  }

  TSegment0*
  TConformalFinder0::findBestLink(const TSegment0& base,
                                  const AList<TSegment0> & candidates) const
  {
    //...Parameters...
    double minAngle = 0.80;
    double maxDistance = 0.3;

    //...Candidate loop...
    unsigned n = candidates.length();
    double minDistance = 999.;
    TSegment0* best = NULL;
    for (unsigned j = 0; j < n; j++) {
      TSegment0* current = candidates[j];
      if (current->nLinks() < 2) continue;

      float angle = base.direction().dot(current->direction());
#ifdef TRASAN_DEBUG_DETAIL
      current->dump("vector hits mc", "    ");
      std::cout << "        angle=" << angle;
      if (angle < minAngle) std::cout << std::endl;
#endif
      if (angle < minAngle) continue;

      float distance = base.distance(* current);
      if (distance < minDistance) {
        minDistance = distance;
        best = current;
      }
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << ",dist=" << distance << std::endl;
#endif
    }

    if (minDistance < maxDistance) return best;
    return NULL;
  }

  TSegment0*
  TConformalFinder0::appendCluster(TTrack& t, AList<TSegment0> & list) const
  {

    //...Candidate loop...
    unsigned n = list.length();
    TSegment0* best = NULL;
    unsigned nBest = 0;
    for (unsigned j = 0; j < n; j++) {
      TSegment0* c = list[j];

      unsigned nOk = t.testByApproach(c->links(),
                                      _trackSelector.maxSigma());
      if (nOk > nBest) {
        nBest = nOk;
        best = c;
      }
    }

    //...Try to append...
    if (best) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "    ... appending a cluster" << std::endl;
      best->dump("hits mc", "        ");
#endif
      AList<TLink> links(best->links());
      t.appendByApproach(links, _trackSelector.maxSigma());
      return best;
    }

    return NULL;
  }

  AList<TSegment0>
  TConformalFinder0::findClusterLink(TSegment0& base,
                                     const AList<TSegment0> * const list) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... finding cluster linkage" << std::endl;
    if (base.links().length() == 0)
      std::cout << name() << " !!! base doesn't have any TLink." << std::endl;
    std::cout << "... base cluster" << std::endl;
    base.dump("cluster hits mc", "  ->");
#endif

    //...Preparation of return value...
    AList<TSegment0> seeds;
    seeds.append(base);

    //...Which super layer?...
    unsigned outerMost = (base.links())[0]->wire()->superLayerId() / 2;

    //...Inner super layer loop...
    int next = outerMost;
    TSegment0* last = & base;
    while (next) {
      --next;
      const AList<TSegment0> & candidates = list[next];
      if (candidates.length() == 0) continue;

#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "... clusters in super layer " << next << std::endl;
#endif

      //...Find best match...
      TSegment0* best = findBestLink(* last, candidates);
      if (best != NULL) {
        seeds.append(best);
        last = best;
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << "  ->Best is ";
        std::cout << best->position() << " ";
        best->dump("hits mc");
#endif
      }
    }

    return seeds;
  }

  TTrack*
  TConformalFinder0::makeTrack(const AList<TSegment0> & list) const
  {
    AList<TLink> links;
    for (unsigned i = 0; i < (unsigned) list.length(); i++) {
      const AList<TLink> & tmp = list[i]->links();
      unsigned n = tmp.length();
      for (unsigned j = 0; j < n; j++) {
        if (tmp[j]->hit()->track()) continue;
        links.append(tmp[j]);
      }
    }

    TTrack* t = _builder->buildRphi(links);

    return t;
  }

  AList<TSegment0>
  TConformalFinder0::findCloseClusters(const TTrack& track,
                                       const AList<TSegment0> & list,
                                       double maxDistance) const
  {

    //...Cal. direction of rotation of track...
    double radius = fabs(track.helix().radius());
    const HepGeom::Point3D<double> & center = track.helix().center();
    int rotation =
      (center.cross(track.links()[0]->xyPosition()).z() > 0.) ? 1 : -1;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... finding close clusters:maxDistance=";
    std::cout << maxDistance << std::endl;
    std::cout << "    radius,center,rotation=" << radius << ",";
    std::cout << center << "," << rotation << std::endl;
#endif

    //...Cluster loop...
    AList<TSegment0> close;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TSegment0& c = * list[i];

      //...Cal. position of cluster in the real plane...
      Point3D position(ORIGIN);
      unsigned m = c.links().length();
      for (unsigned j = 0; j < m; j++) {
        position += c.links()[j]->xyPosition();
      }
      position *= 1. / double(m);

#ifdef TRASAN_DEBUG_DETAIL
      c.dump("cluster hits mc", "    ");
      std::cout << "        position=" << position;
      std::cout << ",diff=" << (position - center).mag() - radius << std::endl;
#endif
      //...Cal. distance to a track...
      Vector3D diff = position - center;
      if ((diff.mag() - radius) < maxDistance) {

        //...Same side?...
        int direction = (center.cross(position).z() > 0.) ? 1 : -1;
        if (direction == rotation) close.append(c);
      }
    }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    found clusters" << std::endl;
    for (unsigned i = 0; i < (unsigned) close.length(); i++) {
      close[i]->dump("hits mc", "    ");
    }
#endif
    return close;
  }

  void
  TConformalFinder0::appendClusters2(TTrack& track,
                                     AList<TSegment0> & list) const
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... appending clusters remained" << std::endl;
    std::cout << "    clusters to be tested : " << std::endl;
    for (unsigned i = 0; i < (unsigned) list.length(); i++) {
      list[i]->dump("cluster hits mc", "        ");
    }
#endif

    unsigned n = list.length();
    if (n == 0) return;

    AList<TSegment0> closer;
    closer.append(findCloseClusters(track, list, 1.));

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    found clusters" << std::endl;
    for (unsigned i = 0; i < (unsigned) closer.length(); i++) {
      closer[i]->dump("cluster hits mc", "        ");
    }
#endif
    n = closer.length();
    if (closer.length() == 0) return;

    //...Append them...
    AList<TLink> candidates;
    for (unsigned i = 0; i < n; i++)
      candidates.append(closer[i]->links());
    _builder->appendClusters(track, candidates);

    //...Remove TLinks from clusters...
    for (unsigned i = 0; i < n; i++) {
      closer[i]->TTrackBase::remove(track.links());
      if (closer[i]->nLinks() == 0) list.remove(closer[i]);
    }
  }

  int
  TConformalFinder0::doit(const CAList<Belle2::TRGCDCWireHit> & axialHits,
                          const CAList<Belle2::TRGCDCWireHit> & stereoHits,
                          AList<TTrack> & tracks,
                          AList<TTrack> &)
  {

    //...For debug...
    if (debugLevel()) {
      std::cout << name() << " ... processing" << std::endl;
      std::cout << "    axialHits=" << axialHits.length();
      std::cout << ",stereoHits=" << stereoHits.length();
      std::cout << ",tracks=" << tracks.length();
      std::cout << std::endl;

      if (debugLevel() > 1)
        std::cout << name() << " ... conformal transformation" << std::endl;
    }

    //...Conformal transformation with IP constraint...
    conformalTransformationRphi(ORIGIN, axialHits, _axialConfLinks);
    conformalTransformationRphi(ORIGIN, stereoHits, _stereoConfLinks);
    _unusedAxialConfLinks.append(_axialConfLinks);
    _unusedStereoConfLinks.append(_stereoConfLinks);
    AList<TLink> unusedConfLinks;
    if (_doSalvage) {
      unusedConfLinks.append(_axialConfLinks);
      unusedConfLinks.append(_stereoConfLinks);
    }

    //...For debug...
    if (debugLevel() > 1)
      std::cout << name() << " ... selecting good hits" << std::endl;

    //...Select good axial hits...
    AList<TLink> goodHits;
    unsigned nLinks = _axialConfLinks.length();
    for (unsigned i = 0; i < nLinks; i++) {
      TLink* l = _axialConfLinks[i];
      const Belle2::TRGCDCWireHit& h = * l->hit();
      if ((h.state() & CellHitIsolated) &&
          (h.state() & CellHitContinuous))
        goodHits.append(l);
    }

    //...Main algorithm...
    standardFinding(goodHits, unusedConfLinks, _fraction);

    //...Main algorithm for second trial...
    specialFinding(goodHits, unusedConfLinks, _fraction);

    //...For debug...
    if (debugLevel()) {
      std::cout << name() << " ... processed : ";
      std::cout << "good hits=" << goodHits.length();
      std::cout << ",tracks=" << _tracks.length();
      std::cout << std::endl;
    }

    tracks.append(_tracks);
    return 0;
  }

  void
  TConformalFinder0::standardFinding(AList<TLink> & list,
                                     AList<TLink> & unusedLinks,
                                     double fraction)
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... standard finding with salvage : given hits :";
    TLink::dump(list, "sort");
#endif

    //...Find segments...
    AList< AList<TSegment0> > segments = findSegments(list);
    AList<TSegment0> segmentList[6];
    AList<TSegment0> original[6];
    for (unsigned i = 0; i < 6; i++) {
      segmentList[i] = * segments[i];
      original[i] = * segments[i];
    }

#ifdef TRASAN_DEBUG_DETAIL
    for (unsigned i = 0; i < 6; i++) {
      std::cout << "... clusters in super layer " << i << std::endl;
      for (unsigned j = 0; j < (unsigned) segmentList[i].length(); j++) {
        segmentList[i][j]->dump("", "    ");
        segmentList[i][j]->dump("hits mc", "      ");
      }
    }
#endif

    //...Main loop...
    AList<TSegment0> retryList;
    AList<TTrack> salvageList;
    unsigned outerMost = 5;
    while (outerMost) {

      while (TSegment0* base = segmentList[outerMost][0]) {

        //...Get linked clusters...
        AList<TSegment0> clusters = findClusterLink(* base, segmentList);

        //...Make a track...
        TTrack* t = makeTrack(clusters);  // don't change 'clusters'
        if (t == NULL) {
          retryList.append(base);
          segmentList[outerMost].remove(base);
          continue;
        }

        //...Check track quality...
        // double f = float(t->nLinks()) / float(nTLinks(clusters));
        double f = float(t->nCores()) / float(NCoreLinks(clusters));
        if (f < fraction) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... fraction too low:" << f << std::endl;
          std::cout << "    used cores=" << t->nCores();
          std::cout << ", candidate cores=" << NCoreLinks(clusters);
          std::cout << " ... retry later" << std::endl;
#endif
          retryList.append(base);
          segmentList[outerMost].remove(base);
          delete t;
          continue;
        }

        //...Append other hits...
        appendClusters2(* t, retryList);

#ifdef TRASAN_DEBUG_DETAIL
        std::cout << name() << " ... 2D result :" << std::endl;
        t->dump("detail", "    ");
#endif

        //...Make it 3D...
        TTrack* ts = t;
        if (_doStereo) {
          ts = _builder->buildStereo(* t,
                                     findCloseHits(_unusedStereoConfLinks, * t));
        }

        //...Check track quality...
        if (! ts) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... failed to make a track 3D" << std::endl;
#endif
          retryList.append(base);
          segmentList[outerMost].remove(base);
          delete t;
          continue;
        }

        //...Salvaging...
        _builder->salvage(* t, unusedLinks);

        //...OK...
        t->assign(CellHitConformalFinder);
        t->finder(TrackOldConformalFinder);
//          TrackOldConformalFinder | TrackValid | Track3D);
        _tracks.append(t);

        //...Remove used links...
        const AList<TLink> & usedLinks = t->links();
        list.remove(usedLinks);
        unusedLinks.remove(usedLinks);
        _unusedStereoConfLinks.remove(usedLinks);
        for (unsigned i = 0; i <= outerMost; i++)
          segmentList[i].remove(clusters);

        //...For debug...
        if (debugLevel() > 1) {
          std::cout << name() << " ... track # " << _tracks.length() - 1;
          std::cout << " found" << std::endl;
          t->dump("detail", "    ");
        }
      }

      //...Loop termination...
      --outerMost;
    }

    //...Termination...
    for (unsigned i = 0; i < 6; i++) {
      HepAListDeleteAll(original[i]);
      delete segments[i];
    }
  }

  void
  TConformalFinder0::specialFinding(AList<TLink> & list,
                                    AList<TLink> & unusedLinks,
                                    double fraction)
  {

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... standard finding with salvage : given hits :";
    TLink::dump(list, "sort");
#endif

    //...Find segments...
    AList< AList<TSegment0> > segments = findSegments(list);
    AList<TSegment0> segmentList[6];
    AList<TSegment0> original[6];
    for (unsigned i = 0; i < 6; i++) {
      segmentList[i] = * segments[i];
      original[i] = * segments[i];
    }

#ifdef TRASAN_DEBUG_DETAIL
    for (unsigned i = 0; i < 6; i++) {
      std::cout << "... clusters in super layer " << i << std::endl;
      for (unsigned j = 0; j < (unsigned) segmentList[i].length(); j++) {
        segmentList[i][j]->dump("", "    ");
        segmentList[i][j]->dump("hits mc", "      ");
      }
    }
#endif

    //...Main loop...
    AList<TSegment0> retryList;
    unsigned outerMost = 5;
    while (outerMost) {

      while (TSegment0* base = segmentList[outerMost][0]) {

        //...Get linked clusters...
        AList<TSegment0> clusters = findClusterLink(* base, segmentList);

again:;

        //...Check # of clusters...
        if (clusters.length() < 2) {
          segmentList[outerMost].remove(base);
          continue;
        }

        //...Make a track...
        TTrack* t = makeTrack(clusters);  // don't change 'clusters'
        if (t == NULL) {
          clusters.remove(clusters.last());
          goto again;
        }

        //...Check track quality...
        // double f = float(t->nLinks()) / float(nTLinks(clusters));
        double f = float(t->nCores()) / float(NCoreLinks(clusters));
        if (f < fraction) {
#ifdef TRASAN_DEBUG_DETAIL
          std::cout << "... fraction too low:" << f << std::endl;
          std::cout << "    retry later" << std::endl;
#endif
          delete t;
          clusters.remove(clusters.last());
          goto again;
        }

        //...Append other hits...
        appendClusters2(* t, retryList);

        //...Make it 3D...
        TTrack* ts = t;
        if (_doStereo) {
          ts = _builder->buildStereo(* t,
                                     findCloseHits(_unusedStereoConfLinks, * t));
        }

        //...Check track quality...
        if (! ts) {
          clusters.remove(clusters.last());
          delete t;
          goto again;
        }

        //...Salvaging...
        _builder->salvage(* t, unusedLinks);

        //...OK...
        t->assign(CellHitConformalFinder);
        t->finder(TrackOldConformalFinder);
//          TrackOldConformalFinder | TrackValid | Track3D);
        _tracks.append(t);

        //...Remove used links...
        const AList<TLink> & usedLinks = t->links();
        list.remove(usedLinks);
        unusedLinks.remove(usedLinks);
        _unusedStereoConfLinks.remove(usedLinks);
        for (unsigned i = 0; i <= outerMost; i++)
          segmentList[i].remove(clusters);

        //...For debug...
        if (debugLevel() > 1) {
          std::cout << name() << " ... track # " << _tracks.length() - 1;
          std::cout << " found" << std::endl;
          t->dump("detail", "    ");
        }
      }

      //...Loop termination...
      --outerMost;
    }

    //...Termination...
    for (unsigned i = 0; i < 6; i++) {
      HepAListDeleteAll(original[i]);
      delete segments[i];
    }
  }

  AList< AList<TSegment0> >
  TConformalFinder0::findSegments(const AList<TLink> & in) const
  {
    AList< AList<TSegment0> > a;

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << name() << " ... finding segments : given hits =" << std::endl;
    TLink::dump(in, "sort");
#endif

    //...Create lists of links for each super layer...
    AList<TLink> links[6];
    unsigned n = in.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * in[i];
      links[l.wire()->superLayerId() / 2].append(l);
    }

    //...Create phi hists and clusters for each super layer...
    THistogram* hist[6];
    hist[0] = new THistogram(64);
    hist[1] = new THistogram(96);
    hist[2] = new THistogram(144);
    hist[3] = new THistogram(192);
    hist[4] = new THistogram(240);
    hist[5] = new THistogram(288);
    for (unsigned i = 0; i < 6; i++) {
      hist[i]->fillX(links[i]);
      AList<TSegment0> * b = new AList<TSegment0>();
      a.append(b);
      b->append(findClusters(* hist[i]));
      delete hist[i];
    }

    return a;
  }

} // namespace Belle

