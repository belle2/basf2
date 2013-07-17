//-----------------------------------------------------------------------------
// $Id: TTrackBase.cc 10678 2008-10-07 05:11:49Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TTrackBase.h
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A virtual class for a track class in tracking.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.53  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.52  2005/04/18 23:41:47  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.51  2005/03/11 03:57:52  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.50  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.49  2003/12/25 12:03:35  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.48  2001/12/23 09:58:49  katayama
// removed Strings.h
//
// Revision 1.47  2001/12/19 02:59:48  katayama
// Uss find,istring
//
// Revision 1.46  2001/04/11 23:20:24  yiwasaki
// Trasan 3.00 RC3 : a bug in stereo mode 1 and 2 is fixed
//
// Revision 1.45  2001/04/11 01:09:12  yiwasaki
// Trasan 3.00 RC2.1 : bugs in RC2 fixed
//
// Revision 1.44  2001/01/11 04:40:23  yiwasaki
// minor changes
//
// Revision 1.43  2000/10/05 23:54:27  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.42  2000/04/13 02:53:41  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.41  2000/04/11 13:05:48  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.40  2000/04/04 07:52:37  yiwasaki
// Trasan 2.00RC26 : additions
//
// Revision 1.39  2000/04/04 07:40:07  yiwasaki
// Trasan 2.00RC26 : pm finder update, salvage in conf. finder modified
//
// Revision 1.38  2000/03/21 07:01:29  yiwasaki
// tmp updates
//
// Revision 1.37  2000/03/07 03:00:56  yiwasaki
// Trasan 2.00RC15 : only debug info. changed
//
// Revision 1.36  2000/03/01 04:51:10  yiwasaki
// Trasan 2.00RC14 : stereo bug fix, curl updates
//
// Revision 1.35  2000/02/17 13:24:20  yiwasaki
// Trasan 2.00RC3 : bug fixes
//
// Revision 1.34  1999/10/30 10:12:24  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.33  1999/10/21 15:45:15  yiwasaki
// Trasan 1.65b : T3DLine, T3DLineFitter, TConformalFinder0 added : no change in Trasan outputs
//
// Revision 1.32  1999/08/25 06:25:50  yiwasaki
// Trasan 1.60b release : curl finder updated, conformal accepts fitting flags
//
// Revision 1.31  1999/07/09 01:47:22  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.30  1999/06/10 09:44:53  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.29  1999/05/26 05:03:49  yiwasaki
// Trasan 1.34 release : Track merge option added my T.Matsumoto, masking bug fixed, RecCDC_trk.stat is filled
//
// Revision 1.28  1999/04/07 06:14:11  yiwasaki
// Trasan 1.30 release : curl finder mask updated
//
// Revision 1.27  1999/03/12 13:11:59  yiwasaki
// Trasan 1.26 : bug fix in RECCDC_TRK output
//
// Revision 1.26  1999/03/11 23:27:20  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.25  1999/03/10 12:55:06  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.24  1999/03/09 13:43:00  yiwasaki
// Trasan 1.22 release : TrackManager bug fix
//
// Revision 1.23  1999/01/20 01:02:47  yiwasaki
// Trasan 1.12 release : movePivot problem avoided temporary, new sakura
//
// Revision 1.22  1999/01/11 03:03:17  yiwasaki
// Fitters added
//
// Revision 1.21  1998/12/14 16:59:09  yiwasaki
// bad hits removed from outputs
//
// Revision 1.20  1998/11/27 08:15:35  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.19  1998/11/12 12:27:29  yiwasaki
// Trasan 1.1 RC 1 release : salvaging installed, basf_if/refit.cc added
//
// Revision 1.18  1998/11/10 09:09:10  yiwasaki
// Trasan 1.1 beta 8 release : negative sqrt fixed, curl finder updated by j.tanaka, TRGCDC classes modified by y.iwasaki
//
// Revision 1.17  1998/09/24 22:56:40  yiwasaki
// Trasan 1.1 alpha 2 release
//
// Revision 1.16  1998/08/31 05:15:52  yiwasaki
// Trasan 1.09 release : curl finder updated by J.Tanaka, MC classes updated by Y.Iwasaki
//
// Revision 1.15  1998/08/17 01:00:34  yiwasaki
// TSegmentLinker::roughSelection added, TRASAN_DEBUG option works
//
// Revision 1.14  1998/08/12 16:32:50  yiwasaki
// Trasan 1.08 release : stereo finder updated by J.Suzuki, new MC classes added by Y.Iwasaki
//
// Revision 1.13  1998/07/29 04:34:57  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.10  1998/06/17 20:23:00  yiwasaki
// Trasan 1 beta 4 release again, KS effi. improved?
//
// Revision 1.9  1998/06/17 20:12:38  yiwasaki
// Trasan 1 beta 4 release, KS effi. improved?
//
// Revision 1.8  1998/06/15 09:58:17  yiwasaki
// Trasan 1 beta 3.2, bug fixed
//
// Revision 1.7  1998/06/14 11:09:53  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.6  1998/06/11 08:14:11  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.5  1998/05/22 08:23:21  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.4  1998/05/08 09:45:45  yiwasaki
// Trasan 1 alpha 2 relase, stereo recon. added, off-vtx recon. added
//
// Revision 1.3  1998/04/23 17:21:45  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.2  1998/04/10 09:36:28  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:50:16  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------

#define HEP_SHORT_NAMES



#include "tracking/modules/trasan/CList.h"
#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/TTrackBase.h"
#include "tracking/modules/trasan/TLink.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"

#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/TFitter.h"
#ifdef TRASAN_DEBUG
#include "tracking/modules/trasan/TTrack.h"
#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/TDebugUtilities.h"
#endif

namespace Belle {

  TTrackBase::TTrackBase(const AList<TLink> & a)
    : _links(a),
      _fitted(false),
      _mc(0),
//    _superLayer(0),
//    _nSuperLayers(0),
      _updated(false),
      _fitter(0),
      _time(0.)
  {
//      _layer[0] = 0;
//      _layer[1] = 0;
//      _nLayers[0] = 0;
//      _nLayers[1] = 0;
//    update();
  }

  TTrackBase::TTrackBase()
    : _fitted(false),
      _mc(0),
//    _superLayer(0),
//    _nSuperLayers(0),
      _updated(true),
      _fitter(0),
      _time(0.)
  {
//      _layer[0] = 0;
//      _layer[1] = 0;
//      _nLayers[0] = 0;
//      _nLayers[1] = 0;
  }

  TTrackBase::~TTrackBase()
  {
  }

  void
  TTrackBase::dump(const std::string& msg, const std::string& pre) const
  {
    if (! _updated) update();
    TLink::dump(_links, msg, pre);
  }

  void
  TTrackBase::update(void) const
  {
    _cores.removeAll();
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++) {
      TLink* l = _links[i];
      const Belle2::TRGCDCWireHit& h = * l->hit();
      if (h.state() & CellHitInvalidForFit) continue;
      if (!(h.state() & CellHitFittingValid)) continue;
      _cores.append(l);
    }
    _updated = true;
  }

  double
  TTrackBase::distance(const TLink&) const
  {
    std::cout << "TTrackBase::distance !!! not implemented" << std::endl;
    return 0.;
  }

  int
  TTrackBase::approach(TLink&) const
  {
    std::cout << "TTrackBase::approach !!! not implemented" << std::endl;
    return -1;
  }

  void
  TTrackBase::appendByApproach(AList<TLink> & list, double maxSigma)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TTrkBs::appendByApproach";
    EnterStage(stage);
#endif
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "sigma=" << maxSigma << std::endl;
#endif

    AList<TLink> unused;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * list[i];

      if ((_links.hasMember(l)) || (l.hit()->state() & CellHitUsed))
        continue;

      //...Calculate closest approach...
      int err = approach(l);
      if (err < 0) {
        unused.append(l);
        continue;
      }

      //...Calculate sigma...
      float distance = (l.positionOnWire() - l.positionOnTrack()).mag();
      float diff = fabs(distance - l.drift());
      float sigma = diff / l.dDrift();

      //...For debug...
#ifdef TRASAN_DEBUG_DETAIL
      l.dump("breif mc", Tab());
      std::cout << ",sigma=" << sigma
                << ",dist=" << distance
                << ",diff=" << diff
                << ",err=" << l.hit()->dDrift() << ",";
      if (sigma < maxSigma) std::cout << "ok,";
      else                  std::cout << "X,";
      std::cout << std::endl;
#endif

      //...Make sigma cut...
      if (sigma > maxSigma) {
        unused.append(l);
        continue;
      }

      //...OK...
      _links.append(l);
      _updated = false;
      _fitted = false;
    }
    list.remove(unused);

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  void
  TTrackBase::appendByDistance(AList<TLink> & list, double)
  {
    std::cout << "TTrackBase::appendByDistance !!! not implemented" << std::endl;
    list.removeAll();
  }

  AList<TLink>
  TTrackBase::refineMain(double sigma)
  {
    AList<TLink> bad;
    unsigned n = _links.length();
    for (unsigned i = 0; i < n; i++)
      if (_links[i]->pull() > sigma)
        bad.append(_links[i]);
    return bad;
  }

  void
  TTrackBase::refine(AList<TLink> & list, double sigma)
  {
#ifdef TRASAN_DEBUG
    const std::string stage = "TTrkBs::refine";
    EnterStage(stage);
#endif

    AList<TLink> bad = refineMain(sigma);

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << Tab() << "sigma=" << sigma << ",#rejected hits=";
    std::cout << bad.length() << std::endl;
    TLink::dump(bad, "breif sort pull mc", Tab());
#endif

    if (bad.length()) {
      _links.remove(bad);
      list.append(bad);
      _fitted = false;
      _updated = false;
    }

#ifdef TRASAN_DEBUG
    LeaveStage(stage);
#endif
  }

  void
  TTrackBase::refine(double sigma)
  {
    AList<TLink> bad = refineMain(sigma);
//      for (unsigned i = 0; i < bad.length(); i++) {
//    const Belle2::TRGCDCWireHit * hit = bad[i]->hit();
//    hit->state(hit->state() | CellHitInvalidForFit);
//      }

#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    refine ... sigma=" << sigma << std::endl;
    TLink::dump(bad, "detail sort", "        ");
#endif

    if (bad.length()) {
      _fitted = false;
      _updated = false;
    }
  }

  unsigned
  TTrackBase::testByApproach(const AList<TLink> & list, double maxSigma) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TTrackBase::testByApproach ... sigma=" << maxSigma << std::endl;
#endif

    unsigned nOK = 0;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& l = * list[i];
      nOK += testByApproach(l, maxSigma);
    }
    return nOK;
  }

  unsigned
  TTrackBase::testByApproach(const TLink& link, double maxSigma) const
  {
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    TTrackBase::testByApproach ... sigma=" << maxSigma << std::endl;
#endif
    TLink l = link;

    //...Calculate closest approach...
    int err = approach(l);
    if (err < 0) return 0;

    //...Calculate sigma...
    float distance = l.distance();
    float diff = fabs(distance - l.hit()->drift());
    float sigma = diff / l.hit()->dDrift();
    l.pull(sigma * sigma);

    //...For debug...
#ifdef TRASAN_DEBUG_DETAIL
    std::cout << "    sigma=" << sigma;
    std::cout << ",dist=" << distance;
    std::cout << ",diff=" << diff << ",";
    if (sigma < maxSigma) std::cout << "ok,";
    else                  std::cout << "X,";
    l.dump("mc");
#endif

    //...Make sigma cut...
    if (sigma < maxSigma) return 1;

    return 0;
  }

//  unsigned
//  TTrackBase::nAxialHits(void) const {
//      unsigned n = _links.length();
//      unsigned a = 0;
//      for (unsigned i = 0; i < n; i++) {
//    if (_links[i]->wire()->axial()) ++a;
//      }
//      return a;
//  }

  const AList<TLink> &
  TTrackBase::links(unsigned mask) const
  {
    if (mask == 0) return _links;

    std::cout << "TTrackBase::links !!! mask is not supportted yet" << std::endl;
    return _links;
  }

  unsigned
  TTrackBase::nLinks(unsigned mask) const
  {
    unsigned n = _links.length();
    if (mask == 0) return n;
    unsigned nn = 0;
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCWireHit& h = * _links[i]->hit();
      if (h.state() & mask) ++nn;
    }
    return nn;
  }

  const AList<TLink> &
  TTrackBase::cores(unsigned mask) const
  {
    if (mask)
      std::cout << "TTrackBase::cores !!! mask is not supported" << std::endl;
    if (! _updated) update();
    return _cores;
  }

  unsigned
  TTrackBase::nCores(unsigned mask) const
  {
    if (mask)
      std::cout << "TTrackBase::nCores !!! mask is not supported" << std::endl;
    if (! _updated) update();
    return _cores.length();
  }

//  TLink *
//  TTrackBase::innerMostLink(unsigned mask) const {
//      if (mask == 0) return InnerMost(_links);
//      unsigned n = _links.length();
//      AList<TLink> tmp;
//      for (unsigned i = 0; i < n; i++) {
//    const Belle2::TRGCDCWireHit & h = * _links[i]->hit();
//    if (h.state() & mask) tmp.append(_links[i]);
//      }
//      return InnerMost(tmp);
//  }

//  TLink *
//  TTrackBase::outerMostLink(unsigned mask) const {
//      if (mask == 0) return OuterMost(_links);
//      unsigned n = _links.length();
//      AList<TLink> tmp;
//      for (unsigned i = 0; i < n; i++) {
//    const Belle2::TRGCDCWireHit & h = * _links[i]->hit();
//    if (h.state() & mask) tmp.append(_links[i]);
//      }
//      return OuterMost(tmp);
//  }

  int
  TTrackBase::fit(void)
  {
    return _fitter->fit(* this);
  }

  void
  TTrackBase::append(TLink& a)
  {
#ifdef TRASAN_DEBUG
    if ((a.hit()->state() & CellHitUsed)) {
      std::cout << "TTrackBase::append !!! " << a.wire()->name()
                << " is already used by another track!" << std::endl;
    }
    for (unsigned i = 0; i < (unsigned) _links.length(); i++) {
      if (a.hit() == _links[i]->hit()) {
        std::cout << "TTrackBase::append !!! " << a.wire()->name()
                  << " is already used in this track!" << std::endl;
        std::cout << "    a.hit()=" << a.hit()->wire().name() << ",_links["
                  << i << "]->hit()=" << _links[i]->hit()->wire().name()
                  << std::endl;
      }
    }
    if (_links.hasMember(a))
      std::cout << "TTrackBase::append !!! link ignored" << std::endl;
#endif

    if (_links.hasMember(a))
      return;

    _links.append(a);
    _updated = false;
    _fitted = false;
    _fittedWithCathode = false; // added by matsu ( 1999/05/24 )
  }

  void
  TTrackBase::append(const AList<TLink> & a)
  {
    for (unsigned i = 0; i < (unsigned) a.length(); i++)
      append(* a[i]);
  }

  const Belle2::TRGCDCTrackMC*
  TTrackBase::hep(void) const
  {
    unsigned n = _links.length();
    CAList<Belle2::TRGCDCTrackMC> hepList;
    CList<unsigned> hepCounter;
    for (unsigned i = 0; i < n; i++) {
      const Belle2::TRGCDCTrackMC* hep = _links[i]->hit()->mc()->hep();
      unsigned nH = hepList.length();
      bool found = false;
      for (unsigned j = 0; j < nH; j++) {
        if (hepList[j] == hep) {
          found = true;
          ++(* hepCounter[j]);
        }
      }

      if (! found) {
        hepList.append(hep);
        unsigned c = 0;
        hepCounter.append(c);
      }
    }

    _nHeps = hepList.length();
    _hep = 0;
    unsigned max = 0;
    for (unsigned i = 0; i < _nHeps; i++) {
      if ((* hepCounter[i]) > max) {
        max = (* hepCounter[i]);
        _hep = hepList[i];
      }
    }

    return _hep;
  }

  unsigned
  TTrackBase::nHeps(void) const
  {
    hep();
    return _nHeps;
  }

  float
  TTrackBase::fractionUsedLayers(void) const
  {
    if (! _links.length()) return 0;

    const unsigned inner = TLink::innerMost(_links)->wire()->layerId();
    const unsigned outer = TLink::outerMost(_links)->wire()->layerId();
    const unsigned used = TLink::nLayers(_links);
    const unsigned expected = outer - inner + 1;

    return float(used) / float(expected);
  }

  float
  TTrackBase::fractionUsedSuperLayers(void) const
  {
    if (! _links.length()) return 0;

    const unsigned inner = TLink::innerMost(_links)->wire()->superLayerId();
    const unsigned outer = TLink::outerMost(_links)->wire()->superLayerId();
    const unsigned used = TLink::nSuperLayers(_links);
    const unsigned expected = outer - inner + 1;

    return float(used) / float(expected);
  }

  unsigned
  TTrackBase::nMissingSuperLayers(void) const
  {
    const unsigned nLinks = _links.length();
    if (! nLinks) return 0;

    const unsigned inner = TLink::innerMost(_links)->wire()->superLayerId();
    const unsigned outer = TLink::outerMost(_links)->wire()->superLayerId();

    static unsigned* nHits = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
    TLink::nHitsSuperLayer(_links, nHits);
//     unsigned nHits[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//     for (unsigned i = 0; i < nLinks; i++)
//  ++nHits[_links[i]->wire()->superLayerId()];

    unsigned nMissing = 0;
    for (unsigned i = inner; i < outer; i++) {
      if (nHits[i] == 0)
        ++nMissing;
    }

    return nMissing;
  }

  void
  Dump(const AList<TTrackBase> & list,
       const std::string& msg,
       const std::string& pre)
  {
    std::string tab;
    for (unsigned i = 0; i < pre.size(); i++)
      tab += " ";
    for (unsigned i = 0; i < (unsigned) list.length(); i++) {
      std::cout << pre << "[" << itostring(i) << "]";
      list[i]->dump(msg, tab);
    }
  }

} // namespace Belle

