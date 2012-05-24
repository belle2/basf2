//-----------------------------------------------------------------------------
// $Id: TFastFinder.cc 10129 2007-05-18 12:44:41Z katayama $
//-----------------------------------------------------------------------------
// Filename : TFastFinder.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks with the conformal method.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.11  2003/12/25 12:03:33  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.10  2001/12/23 09:58:47  katayama
// removed Strings.h
//
// Revision 1.9  2001/12/19 02:59:46  katayama
// Uss find,istring
//
// Revision 1.8  2001/04/11 01:09:10  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.7  2000/04/11 13:05:43  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.6  2000/03/21 07:01:27  yiwasaki
// tmp updates
//
// Revision 1.5  2000/03/17 07:00:50  katayama
// Module function modified
//
// Revision 1.4  2000/01/28 06:30:22  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.3  1999/10/30 10:12:19  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.2  1999/07/15 08:43:19  yiwasaki
// Trasan 1.55b release : Curl finder # of hits protection, bug in TManager for MC, helix fitter # of hits protection, fast finder improved
//
// Revision 1.1  1999/06/29 00:03:02  yiwasaki
// Trasan 1.48a release : TFastFinder added
//
//
//-----------------------------------------------------------------------------




#include "tracking/modules/trasan/TFastFinder.h"
#include "tracking/modules/trasan/TConformalFinder0.h"
#include "tracking/modules/trasan/TLink.h"
#include "tracking/modules/trasan/THistogram.h"

#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

  extern const HepGeom::Point3D<double>  ORIGIN;

  TFastFinder::TFastFinder() : _builder("fast find builder", 30)
  {
    _selector.nLinks(4);
    _selector.nSuperLayers(2);
    _selector.minPt(0.05);
    _selector.maxImpact(100.);
    _selector.maxSigma(30.);
    _selector.nLinksStereo(3);
    _selector.maxDistance(30.);
    _builder.trackSelector(_selector);
  }

  TFastFinder::~TFastFinder()
  {
  }

  std::string
  TFastFinder::version(void) const
  {
    return "0.00";
  }

  void
  TFastFinder::dump(const std::string& msg, const std::string& pre) const
  {
    std::cout << pre;
    TFinderBase::dump(msg);
  }

  void
  TFastFinder::clear(void)
  {
    _axialHits.removeAll();
    _stereoHits.removeAll();
    HepAListDeleteAll(_axialLinks);
    HepAListDeleteAll(_stereoLinks);
  }

  int
  TFastFinder::doit(const AList<Belle2::TRGCDCWireHit> & axialHits,
                    const AList<Belle2::TRGCDCWireHit> & stereoHits,
                    AList<TTrack> & tracks)
  {

    //...Select good hits...
    selectHits2(axialHits, stereoHits);

    //...Conformal transformation with IP constraint...
    TConformalFinder0::conformalTransformationRphi(ORIGIN,
                                                   _axialHits,
                                                   _axialLinks);
    TConformalFinder0::conformalTransformationRphi(ORIGIN,
                                                   _stereoHits,
                                                   _stereoLinks);

    //...Make a histogram...
    THistogram hist(288);
    hist.fillX(_axialLinks);
    AList<TSegment0> clusters = hist.clusters0();

    //...Cluster loop...
    unsigned n = clusters.length();
    for (unsigned i = 0; i < n; i++) {

      //...2D track...
      TTrack* t = _builder.buildRphi(clusters[i]->links());
      if (t == NULL) continue;
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TFastFinder::doit ... 2D track found" << std::endl;
#endif

      //...Make it 3D...
      TTrack* ts = t;
      ts = _builder.buildStereo(* t,
                                findCloseHits(_stereoLinks,
                                              * t));
      if (ts == NULL) continue;
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TFastFinder::doit ... 3D track found" << std::endl;
#endif

      //...OK...
      t->assign(CellHitFastFinder);
      t->finder(TrackFastFinder);
//  t->assign(CellHitFastFinder, TrackFastFinder);
      tracks.append(t);
      _stereoLinks.remove(t->links());
    }

    //...Termination...
    HepAListDeleteAll(clusters);
    return 0;
  }

  void
  TFastFinder::selectHits(const AList<Belle2::TRGCDCWireHit> & axialHits,
                          const AList<Belle2::TRGCDCWireHit> & stereoHits)
  {
    unsigned n = axialHits.length();
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWireHit& h = * axialHits[i];
      if ((h.state() & CellHitIsolated) && (h.state() & CellHitContinuous))
        _axialHits.append((Belle2::TRGCDCWireHit&) h);
    }
    n = stereoHits.length();
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWireHit& h = * stereoHits[i];
      if ((h.state() & CellHitIsolated) && (h.state() & CellHitContinuous))
        _stereoHits.append((Belle2::TRGCDCWireHit&) h);
    }
  }

  void
  TFastFinder::selectHits2(const AList<Belle2::TRGCDCWireHit> & axialHits,
                           const AList<Belle2::TRGCDCWireHit> & stereoHits)
  {
    selectSimpleSegments(axialHits, _axialHits);
    selectSimpleSegments(stereoHits, _stereoHits);
  }

  AList<TLink>
  TFastFinder::findCloseHits(const AList<TLink> & links,
                             const TTrack& track) const
  {
    //
    // Coded by J.Suzuki
    //
    AList<TLink> list;

    //...Check condition...
    if (track.links().length() == 0) {
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TConformalFinder::findCloseHits !!! ";
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

  void
  TFastFinder::selectSimpleSegments(const AList<Belle2::TRGCDCWireHit> & in,
                                    AList<Belle2::TRGCDCWireHit> & out) const
  {
    AList<Belle2::TRGCDCWireHit> hits = in;
    while (hits.last()) {
      Belle2::TRGCDCWireHit& h = * hits.last();

      //...Start clustering...
      // AList<Belle2::TRGCDCWireHit> & cluster = * new AList<Belle2::TRGCDCWireHit>();
      AList<Belle2::TRGCDCWireHit> cluster;
      AList<Belle2::TRGCDCWireHit> toBeChecked;
      bool ok = true;
      toBeChecked.append(h);
      while (toBeChecked.length()) {
        Belle2::TRGCDCWireHit& a = * toBeChecked.last();
        toBeChecked.remove(a);
        if (cluster.hasMember(a)) continue;

        //...Check hit...
        unsigned state = a.state();
        if (!(state & CellHitIsolated)) ok = false;
        if (!(state & CellHitContinuous)) ok = false;

        //...Append...
        cluster.append(a);

        //...Neighbor hit...
        unsigned ptn =
          (state & CellHitNeighborPatternMask) >> CellHitNeighborHit;
        for (unsigned i = 0; i < 7; i++) {
          if ((ptn >> i) % 2) {
            const Belle2::TRGCDCWireHit& b = * a.wire().neighbor(i)->hit();
            toBeChecked.append((Belle2::TRGCDCWireHit&) b);
          }
        }
      }

      //...Check cluster size...
      if ((cluster.length() < 4) || (cluster.length() > 8)) ok = false;

      //...OK. Good hits...
      if (ok) out.append(cluster);

      //...Remove cluster...
      hits.remove(cluster);

      //...For debug...
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << "TFastFinder::selectSimpleSegment ... cluster : ok = ";
      std::cout << ok << " : ";
#endif
      for (unsigned i = 0; i < (unsigned) cluster.length(); i++) {
        Belle2::TRGCDCWireHit& h = * cluster[i];
#ifdef TRASAN_DEBUG_DETAIL
        std::cout << h.wire().name() << ",";
#endif
        if (! ok) {
          unsigned state = h.state();
          if (state & CellHitIsolated) state ^= CellHitIsolated;
          if (state & CellHitContinuous) state ^= CellHitContinuous;
          h.state(state);
        }
      }
#ifdef TRASAN_DEBUG_DETAIL
      std::cout << std::endl;
#endif

//    remove:
      hits.remove(h);
    }
  }

} // namespace Belle

