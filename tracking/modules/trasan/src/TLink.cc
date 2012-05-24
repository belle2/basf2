//-----------------------------------------------------------------------------
// $Id: TLink.cc 11153 2010-04-28 03:36:53Z yiwasaki $
//-----------------------------------------------------------------------------
// Filename : TLink.cc
// Section  : Tracking
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to relate TRGCDCWireHit and TTrack objects.
//               See http://bsunsrv1.kek.jp/~yiwasaki/tracking/
//-----------------------------------------------------------------------------
// $Log$
// Revision 1.46  2005/11/03 23:20:12  yiwasaki
// Trasan 3.18 : Hough finder 0.12
//
// Revision 1.45  2005/04/18 23:41:46  yiwasaki
// Trasan 3.17 : Only Hough finder is modified
//
// Revision 1.44  2005/03/11 03:57:51  yiwasaki
// Trasan 3.15 : Only Hough finder is modified
//
// Revision 1.43  2005/01/14 00:55:40  katayama
// uninitialized variable
//
// Revision 1.42  2004/03/26 06:07:04  yiwasaki
// Trasan 3.10 : junk track rejection introduced in curl finder tagir option
//
// Revision 1.41  2003/12/25 12:03:34  yiwasaki
// Trasan 3.07 : small cell cdc available in the curl finder; many minor modifications not to produce warning message in compiling
//
// Revision 1.40  2003/12/19 07:36:03  yiwasaki
// Trasan 3.06 : small cell cdc available in the conformal finder; Tagir modification is applied to the curl finder; the hough finder is added;
//
// Revision 1.39  2002/02/13 20:22:12  yiwasaki
// Trasan 3.02 : bug fixes in debug mode
//
// Revision 1.38  2001/12/23 09:58:48  katayama
// removed Strings.h
//
// Revision 1.37  2001/12/19 02:59:47  katayama
// Uss find,istring
//
// Revision 1.36  2001/04/25 02:36:00  yiwasaki
// Trasan 3.00 RC6 : helix speed up, chisq_max parameter added
//
// Revision 1.35  2001/04/11 23:20:24  yiwasaki
// Trasan 3.00 RC3 : a bug in stereo mode 1 and 2 is fixed
//
// Revision 1.34  2000/10/05 23:54:23  yiwasaki
// Trasan 2.09 : TLink has drift time info.
//
// Revision 1.33  2000/06/13 05:19:02  yiwasaki
// Trasan 2.02 : RECCDC_MCTRK sorting
//
// Revision 1.32  2000/04/15 13:40:42  katayama
// Add/remove const so that they compile
//
// Revision 1.31  2000/04/13 02:53:39  yiwasaki
// Trasan 2.00rc29 : minor changes
//
// Revision 1.30  2000/04/11 13:05:45  katayama
// Added std:: to cout, cerr, endl etc.
//
// Revision 1.29  2000/03/30 08:30:31  katayama
// mods for CC5.0
//
// Revision 1.28  2000/02/25 08:09:56  yiwasaki
// Trasan 2.00RC9 : stereo bug fix
//
// Revision 1.27  2000/02/23 08:45:06  yiwasaki
// Trasan 2.00RC5
//
// Revision 1.26  2000/02/15 13:46:41  yiwasaki
// Trasan 2.00RC2 : curl bug fix, new conf modified
//
// Revision 1.25  2000/01/28 06:30:24  yiwasaki
// Trasan 1.67h : new conf modified
//
// Revision 1.24  2000/01/12 14:25:00  yiwasaki
// Trasan 1.67d : tuned trasan, bug in masking fixed, sakura modified, mitsuo added
//
// Revision 1.23  1999/11/19 09:13:09  yiwasaki
// Trasan 1.65d : new conf. finder updates, no change in default conf. finder
//
// Revision 1.22  1999/10/30 10:12:21  yiwasaki
// Trasan 1.65c : new conf finder with 3D
//
// Revision 1.21  1999/09/21 02:01:34  yiwasaki
// Trasan 1.63b release : conformal finder minor changes, TWindow added for debug
//
// Revision 1.20  1999/07/09 01:47:21  yiwasaki
// Trasan 1.53a release : cathode updates by T.Matsumoto, minor change of Conformal finder
//
// Revision 1.19  1999/06/16 08:29:51  yiwasaki
// Trasan 1.44 release : new THelixFitter
//
// Revision 1.18  1999/06/10 09:44:52  yiwasaki
// Trasan 1.40 release : minor changes only
//
// Revision 1.17  1999/05/18 04:44:30  yiwasaki
// Trasan 1.33 release : bugs in salvage and masking are fixed, T0 calculation option is added
//
// Revision 1.16  1999/03/12 13:11:58  yiwasaki
// Trasan 1.26 : bug fix in RECCDC_TRK output
//
// Revision 1.15  1999/03/11 23:27:17  yiwasaki
// Trasan 1.24 release : salvaging improved
//
// Revision 1.14  1999/03/10 12:55:04  yiwasaki
// Trasan 1.23 release : curl finder updated
//
// Revision 1.13  1999/02/03 06:23:09  yiwasaki
// Trasan 1.14 release : bugs in curl finder and trasan fixed, new salvage installed
//
// Revision 1.12  1999/01/25 03:16:11  yiwasaki
// salvage improved, movePivot problem fixed again
//
// Revision 1.11  1998/11/27 08:15:32  yiwasaki
// Trasan 1.1 RC 3 release : salvaging improved
//
// Revision 1.10  1998/07/29 04:33:18  yiwasaki
// Trasan 1.06 release : back to Trasan 1.02
//
// Revision 1.8  1998/06/14 11:09:51  yiwasaki
// Trasan beta 3 release, TBuilder and TSelector added
//
// Revision 1.7  1998/06/11 08:14:09  yiwasaki
// Trasan 1 beta 1 release
//
// Revision 1.6  1998/06/08 14:37:53  yiwasaki
// Trasan 1 alpha 8 release, Stereo append bug fixed, TCurlFinder added
//
// Revision 1.5  1998/05/22 08:23:20  yiwasaki
// Trasan 1 alpha 4 release, TCluster added, TConformalLink no longer used
//
// Revision 1.4  1998/04/23 17:21:42  yiwasaki
// Trasan 1 alpha 1 release
//
// Revision 1.3  1998/04/16 16:49:34  yiwasaki
// minor changes
//
// Revision 1.2  1998/04/10 09:36:27  yiwasaki
// TTrack added, TRGCDC becomes Singleton
//
// Revision 1.1  1998/04/10 00:50:16  yiwasaki
// TCircle, TConformalFinder, TConformalLink, TFinderBase, THistogram, TLink, TTrackBase classes added
//
//-----------------------------------------------------------------------------



#define HEP_SHORT_NAMES
#include "tracking/modules/trasan/ConstAList.h"
#include "tracking/modules/trasan/Strings.h"
#include "tracking/modules/trasan/TLink.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"

#include <string.h>
#include "trg/cdc/TrackMC.h"
#include "tracking/modules/trasan/TTrack.h"

namespace Belle {

#ifdef TRASAN_DEBUG
  unsigned TLink::_nTLinks = 0;
  unsigned TLink::_nTLinksMax = 0;

  unsigned
  TLink::nTLinks(void)
  {
    return _nTLinks;
  }

  unsigned
  TLink::nTLinksMax(void)
  {
    return _nTLinksMax;
  }
#endif

  bool TLink::ms_smallcell(false);
  bool TLink::ms_superb(false);
  unsigned TLink::_nL = 0;
  unsigned TLink::_nSL = 0;
  unsigned TLink::_nSLA = 0;
  unsigned* TLink::_nHitsSL = 0;

  TLink::TLink(TTrack* t, const Belle2::TRGCDCWireHit* h, const HepGeom::Point3D<double> & p)
    : _track(t),
      _hit(h),
      _position(p),
      _dPhi(0),
      _leftRight(0),
      _pull(0),
      _link(0),
      _usecathode(0),
      _fit2D(0)
  {
    if (h) {
      _drift[0] = h->drift(0);
      _drift[1] = h->drift(1);
      _dDrift[0] = h->dDrift(0);
      _dDrift[1] = h->dDrift(1);
    } else {
      _drift[0] = 0.;
      _drift[1] = 0.;
      _dDrift[0] = 0.;
      _dDrift[1] = 0.;
    }

    for (unsigned i = 0; i < 7; ++i)
      _neighbor[i] = NULL;

    if (h) {
      _onTrack = _onWire = h->xyPosition();
    }

#ifdef TRASAN_DEBUG
    ++_nTLinks;
    if (_nTLinks > _nTLinksMax)
      _nTLinksMax = _nTLinks;
#endif
  }

  TLink::TLink(const TLink& l)
    : _track(l._track),
      _hit(l._hit),
      _onTrack(l._onTrack),
      _onWire(l._onWire),
      _position(l._position),
      _dPhi(l._dPhi),
      _leftRight(l._leftRight),
      _zStatus(l._zStatus),
      _zPair(l._zPair),
      _pull(l._pull),
      _link(l._link),
      // addition by matsu ( 1999/07/05 )
      _usecathode(l._usecathode),
//  _mclust(l._mclust),
      // end of addition
      _fit2D(l._fit2D)
  {
    _drift[0] = l._drift[0];
    _drift[1] = l._drift[1];
    _dDrift[0] = l._dDrift[0];
    _dDrift[1] = l._dDrift[1];
    for (unsigned i = 0; i < 7; ++i)
      _neighbor[i] = l._neighbor[i];
    for (unsigned i = 0; i < 4; ++i)
      _arcZ[i] = l._arcZ[i];
#ifdef TRASAN_DEBUG
    ++_nTLinks;
    if (_nTLinks > _nTLinksMax)
      _nTLinksMax = _nTLinks;
#endif
  }

  TLink::~TLink()
  {
#ifdef TRASAN_DEBUG
    --_nTLinks;
#endif
  }

  void
  TLink::dump(const std::string& msg, const std::string& pre) const
  {

    //...Basic options...
    bool track = (msg.find("track") != std::string::npos);
    bool mc = (msg.find("mc") != std::string::npos);
    bool pull = (msg.find("pull") != std::string::npos);
    bool flag = (msg.find("flag") != std::string::npos);
    bool stereo = (msg.find("stereo") != std::string::npos);
    bool pos = (msg.find("position") != std::string::npos);

    //...Strong options...
    bool breif = (msg.find("breif") != std::string::npos);
    bool detail = (msg.find("detail") != std::string::npos);
    if (detail)
      track = mc = pull = flag = stereo = pos = true;
    if (breif)
      mc = pull = flag = true;

    //...Output...
    std::cout << pre;
    std::cout << wire()->name();
//     if (mc) {
//  if (_hit) {
//      if (_hit->mc()) {
//    if (_hit->mc()->hep())
//        std::cout << "(mc" << _hit->mc()->hep()->id() << ")";
//    else
//        std::cout << "(mc?)";
//      }
//      else {
//    std::cout << "(mc?)";
//      }
//  }
//     }
    if (pull)
      std::cout << "[pul=" << this->pull() << "]";
    if (flag) {
      if (_hit) {
        if (_hit->state() & CellHitFindingValid)
          std::cout << "o";
        if (_hit->state() & CellHitFittingValid)
          std::cout << "+";
        if (_hit->state() & CellHitInvalidForFit)
          std::cout << "x";
      }
    }
    if (stereo) {
      std::cout << "{" << leftRight() << "," << zStatus() << "}";
    }
    if (pos) {
      std::cout << ",pos=" << position();
      std::cout << ",drift=" << drift(0) << "," << drift(1);
    }
    if (! breif)
      std::cout << std::endl;
  }

  unsigned
  TLink::nLayers(const AList<TLink> & list)
  {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nLayers() > 64)
      std::cout << "TLink::nLayers !!! #layers should be less than 64"
                << std::endl;
#endif

    unsigned l0 = 0;
    unsigned l1 = 0;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->wire()->layerId();
      if (id < 32) l0 |= (1 << id);
      else         l1 |= (1 << (id - 32));
    }

    unsigned l = 0;
    for (unsigned i = 0; i < 32; i++) {
      if (l0 & (1 << i)) ++l;
      if (l1 & (1 << i)) ++l;
    }
    return l;
  }

  void
  TLink::nHits(const AList<TLink> & links, unsigned* nHits)
  {
    for (unsigned i = 0; i < _nL; i++)
      nHits[i] = 0;
    unsigned nLinks = links.length();
    for (unsigned i = 0; i < nLinks; i++) {
      ++nHits[links[i]->wire()->layerId()];

#ifdef TRASAN_DEBUG
      if (links[i]->wire()->layerId() >= _nL)
        std::cout << "TLink::nHits !!! layer ID("
                  << links[i]->wire()->layerId() << ") is larger than "
                  << "size of buffer(" << _nL << ")" << std::endl;
#endif
    }
  }

  void
  TLink::nHitsSuperLayer(const AList<TLink> & links, unsigned* nHits)
  {
    for (unsigned i = 0; i < _nSL; i++)
      nHits[i] = 0;
    const unsigned nLinks = links.length();
    for (unsigned i = 0; i < nLinks; i++) {
      ++nHits[links[i]->wire()->superLayerId()];

#ifdef TRASAN_DEBUG
      if (links[i]->wire()->superLayerId() >= _nSL)
        std::cout << "TLink::nHitsSuperLayer !!! super layer ID("
                  << links[i]->wire()->superLayerId() << ") is larger than "
                  << "size of buffer(" << _nSL << ")" << std::endl;
#endif
    }
  }

  void
  TLink::dump(const CAList<TLink> & links,
              const std::string& msg,
              const std::string& pre)
  {

    //...Basic options...
    bool mc = (msg.find("mc") != std::string::npos);
    bool sort = (msg.find("sort") != std::string::npos);
    bool flag = (msg.find("flag") != std::string::npos);

    //...Strong options...
    bool breif = (msg.find("breif") != std::string::npos);
    bool detail = (msg.find("detail") != std::string::npos);
    if (detail)
      mc = flag = true;
    if (breif)
      mc = true;

    CAList<TLink> tmp = links;
    if (sort)
      tmp.sort(TLink::sortByWireId);
    unsigned n = tmp.length();
    unsigned nForFit = 0;
#define MCC_MAX 1000
    unsigned MCC0[MCC_MAX];
    unsigned MCC1[MCC_MAX];
    memset((char*) MCC0, 0, sizeof(unsigned) * MCC_MAX);
    memset((char*) MCC1, 0, sizeof(unsigned) * MCC_MAX);
    bool MCCOverFlow = false;

    for (unsigned i = 0; i < n; i++) {
      const TLink& l = * tmp[i];

//  if (mc) {
//      unsigned mcId = 999;
//      if (l.hit()) {
//    if (l.hit()->mc())
//        if (l.hit()->mc()->hep())
//      mcId = l.hit()->mc()->hep()->id();
//    if (mcId < MCC_MAX) {
//        ++MCC0[mcId];
//        if (l.hit()->state() & CellHitFittingValid) {
//      if (! (l.hit()->state() & CellHitInvalidForFit))
//          ++MCC1[mcId];
//        }
//    }
//    else {
//        MCCOverFlow = true;
//    }
//      }
//  }
      if (flag) {
        if (l.hit()) {
          if (l.hit()->state() & CellHitFittingValid) {
            if (!(l.hit()->state() & CellHitInvalidForFit))
              ++nForFit;
          }
        }
      }
      if (i == 0) {
        std::cout << pre;
        if (! breif)
          std::cout << itostring(i) << " ";
      } else {
        if (breif)
          std::cout << ",";
        else
          std::cout << pre << itostring(i) << " ";
      }
      l.dump(msg);
    }
    std::cout << pre << ",Total " << n << " links";
    if (flag) std::cout << ",fv " << nForFit << " l(s)";
    if (mc) {
      unsigned nMC = 0;
      std::cout << ", mc";
      for (unsigned i = 0; i < MCC_MAX; i++) {
        if (MCC0[i] > 0) {
          ++nMC;
          std::cout << i << ":" << MCC0[i] << ",";
        }
      }
      std::cout << "total " << nMC << " mc contributions";
      if (flag) {
        nMC = 0;
        std::cout << ", fv mc";
        for (unsigned i = 0; i < MCC_MAX; i++) {
          if (MCC1[i] > 0) {
            ++nMC;
            std::cout << i << ":" << MCC1[i] << ",";
          }
        }
        std::cout << " total " << nMC << " mc fit valid contribution(s)";
      }

      if (MCCOverFlow)
        std::cout << "(counter overflow)";
    }
    std::cout << std::endl;

    //...Parent...
    if (mc) {
      std::vector<const Belle2::TRGCDCTrackMC*> list = Belle2::TRGCDCTrackMC::list();
      if (! list.size()) return;
      std::cout << pre;
//cnv unsigned nMC = 0;
      for (unsigned i = 0; i < MCC_MAX; i++) {
        if (MCC0[i] > 0) {
          const Belle2::TRGCDCTrackMC* h = list[i];
//    std::cout << ", mc" << i << "(" << h->pType() << ")";
          std::cout << ", mc" << i << "(";
          if (h)
            std::cout << h->pType() << ")";
          else
            std::cout << "?)";
          while (h) {
            const Belle2::TRGCDCTrackMC* m = h->mother();
            if (m) {
              std::cout << "<-mc" << m->id();
              h = m;
            } else {
              break;
            }
          }
        }
      }
      if (MCCOverFlow)
        std::cout << "(counter overflow)";
      std::cout << std::endl;
    }
  }

  void
  TLink::dump(const TLink& link,
              const std::string& msg,
              const std::string& pre)
  {
    CAList<TLink> tmp;
    tmp.append(link);
    dump(tmp, msg, pre);
  }

  unsigned
  TLink::nStereoHits(const AList<TLink> & links)
  {
    unsigned nLinks = links.length();
    unsigned n = 0;
    for (unsigned i = 0; i < nLinks; i++)
      if (links[i]->wire()->stereo())
        ++n;
    return n;
  }

  unsigned
  TLink::nAxialHits(const AList<TLink> & links)
  {
    unsigned nLinks = links.length();
    unsigned n = 0;
    for (unsigned i = 0; i < nLinks; i++)
      if (links[i]->wire()->axial())
        ++n;
    return n;
  }

  AList<TLink>
  TLink::axialHits(const AList<TLink> & links)
  {
    AList<TLink> a;
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (links[i]->wire()->axial())
        a.append(links[i]);
    }
    return a;
  }

  AList<TLink>
  TLink::stereoHits(const AList<TLink> & links)
  {
    AList<TLink> a;
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++) {
      if (! links[i]->wire()->axial())
        a.append(links[i]);
    }
    return a;
  }

  TLink*
  TLink::innerMost(const AList<TLink> & a)
  {
    unsigned n = a.length();
    unsigned minId = 19999;
    TLink* t = 0;
    for (unsigned i = 0; i < n; i++) {
      unsigned id = a[i]->wire()->id();
      if (id < minId) {
        minId = id;
        t = a[i];
      }
    }
    return t;
  }

  TLink*
  TLink::outerMost(const AList<TLink> & a)
  {
    unsigned n = a.length();
    unsigned maxId = 0;
    TLink* t = 0;
    for (unsigned i = 0; i < n; i++) {
      unsigned id = a[i]->wire()->id();
      if (id >= maxId) {
        maxId = id;
        t = a[i];
      }
    }
    return t;
  }

  void
  TLink::separateCores(const AList<TLink> & input,
                       AList<TLink> & cores,
                       AList<TLink> & nonCores)
  {
    unsigned n = input.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& t = * input[i];
      const Belle2::TRGCDCWireHit& h = * t.hit();
      if (h.state() & CellHitFittingValid)
        cores.append(t);
      else
        nonCores.append(t);
    }
  }

  AList<TLink>
  TLink::cores(const AList<TLink> & input)
  {
    AList<TLink> a;
    unsigned n = input.length();
    for (unsigned i = 0; i < n; i++) {
      TLink& t = * input[i];
      const Belle2::TRGCDCWireHit& h = * t.hit();
      if (h.state() & CellHitFittingValid)
        a.append(t);
    }
    return a;
  }

#if defined(__GNUG__)
  int
  TLink::sortByWireId(const TLink** a, const TLink** b)
  {
    if ((* a)->wire()->id() > (* b)->wire()->id()) return 1;
    else if
    ((* a)->wire()->id() == (* b)->wire()->id()) return 0;
    else return -1;
  }

  int
  TLink::sortByX(const TLink** a, const TLink** b)
  {
    if ((* a)->position().x() > (* b)->position().x()) return 1;
    else if ((* a)->position().x() == (* b)->position().x()) return 0;
    else return -1;
  }

#else
  extern "C" int
  SortByWireId(const void* av, const void* bv)
  {
    const TLink** a((const TLink**)av);
    const TLink** b((const TLink**)bv);
    if ((* a)->wire()->id() > (* b)->wire()->id()) return 1;
    else if
    ((* a)->wire()->id() == (* b)->wire()->id()) return 0;
    else return -1;
  }

  extern "C" int
  SortByX(const void* av, const void* bv)
  {
    const TLink** a((const TLink**)av);
    const TLink** b((const TLink**)bv);
    if ((* a)->position().x() > (* b)->position().x()) return 1;
    else if ((* a)->position().x() == (* b)->position().x()) return 0;
    else return -1;
  }
#endif

  unsigned
  TLink::width(const AList<TLink> & list)
  {
    const unsigned n = list.length();
    if (n < 2) return n;

    const Belle2::TRGCDCWire* const w0 = list[0]->wire();
//cnv    const unsigned sId = w0->superLayerId();
    unsigned nWires = w0->layer().nCells();
    unsigned center = w0->localId();

    if (ms_smallcell && w0->layerId() < 3) {
      nWires /= 2;
      center /= 2;
    }

#ifdef TRASAN_DEBUG_DETAIL
//     std::cout << "    ... Width : sid=" << sId << ",lid=" << w0->layerId()
//        << ",nWires=" << nWires << ",center=" << center << " : "
//        << std::endl;
//     dump(list);
#endif

    unsigned left = 0;
    unsigned right = 0;
    for (unsigned i = 1; i < n; i++) {
      const Belle2::TRGCDCWire* const w = list[i]->wire();
      unsigned id = w->localId();

      if (ms_smallcell && w->layerId() < 3)
        id /= 2;

      unsigned distance0, distance1;
      if (id > center) {
        distance0 = id - center;
        distance1 = nWires - distance0;
      } else {
        distance1 = center - id;
        distance0 = nWires - distance1;
      }

      if (distance0 < distance1) {
        if (distance0 > right) right = distance0;
      } else {
        if (distance1 > left) left = distance1;
      }

#ifdef TRASAN_DEBUG_DETAIL
//  std::cout << "        ... lyr=" << w->layerId()
//      << ",true id=" << w->localId() << ",id=" << id
//      << ",distance0,1=" << distance0 << "," << distance1
//      << std::endl;
//  if (w->superLayerId() != sId)
//      std::cout << "::width !!! super layer assumption violation"
//          << std::endl;
#endif
    }

    return right + left + 1;
  }

  AList<TLink>
  TLink::edges(const AList<TLink> & list)
  {
    AList<TLink> a;

    unsigned n = list.length();
    if (n < 2) return a;
    else if (n == 2) return list;

    const Belle2::TRGCDCWire* w = list[0]->wire();
    unsigned nWires = w->layer().nCells();
    unsigned center = w->localId();

    unsigned left = 0;
    unsigned right = 0;
    TLink* leftL = list[0];
    TLink* rightL = list[0];
    for (unsigned i = 1; i < n; i++) {
      w = list[i]->wire();
      unsigned id = w->localId();

      unsigned distance0, distance1;
      if (id > center) {
        distance0 = id - center;
        distance1 = nWires - distance0;
      } else {
        distance1 = center - id;
        distance0 = nWires - distance1;
      }

      if (distance0 < distance1) {
        if (distance0 > right) {
          right = distance0;
          rightL = list[i];
        }
      } else {
        if (distance1 > left) {
          left = distance1;
          leftL = list[i];
        }
      }
    }

    a.append(leftL);
    a.append(rightL);
    return a;
  }

  AList<TLink>
  TLink::sameLayer(const AList<TLink> & list, const TLink& a)
  {
    AList<TLink> same;
    unsigned id = a.wire()->layerId();
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->wire()->layerId() == id) same.append(list[i]);
    }
    return same;
  }

  AList<TLink>
  TLink::sameSuperLayer(const AList<TLink> & list, const TLink& a)
  {
    AList<TLink> same;
    unsigned id = a.wire()->superLayerId();
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->wire()->superLayerId() == id) same.append(list[i]);
    }
    return same;
  }

  AList<TLink>
  TLink::sameLayer(const AList<TLink> & list, unsigned id)
  {
    AList<TLink> same;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->wire()->layerId() == id) same.append(list[i]);
    }
    return same;
  }

  AList<TLink>
  TLink::sameSuperLayer(const AList<TLink> & list, unsigned id)
  {
    AList<TLink> same;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      if (list[i]->wire()->superLayerId() == id) same.append(list[i]);
    }
    return same;
  }

  AList<TLink>
  TLink::inOut(const AList<TLink> & list)
  {
    AList<TLink> inners;
    AList<TLink> outers;
    unsigned n = list.length();
    unsigned innerMostLayer = 999;
    unsigned outerMostLayer = 0;
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->wire()->layerId();
      if (id < innerMostLayer) innerMostLayer = id;
      else if (id > outerMostLayer) outerMostLayer = id;
    }
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->wire()->layerId();
      if (id == innerMostLayer) inners.append(list[i]);
      else if (id == outerMostLayer) outers.append(list[i]);
    }
    inners.append(outers);
    return inners;
  }

  unsigned
  TLink::superLayer(const AList<TLink> & list)
  {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nSuperLayers() > 32)
      std::cout << "TLink::superLayer !!! #super layers should be less than 32"
                << std::endl;
#endif

    unsigned sl = 0;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++)
      sl |= (1 << (list[i]->wire()->superLayerId()));
    return sl;
  }

  unsigned
  TLink::superLayer(const AList<TLink> & links, unsigned minN)
  {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nSuperLayers() > 32)
      std::cout
          << "#super layers should be less than 32" << std::endl;
#endif

    clearBufferSL();
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++)
      ++_nHitsSL[links[i]->wire()->superLayerId()];
    unsigned sl = 0;
    for (unsigned i = 0; i < _nSL; i++)
      if (_nHitsSL[i] >= minN)
        sl |= (1 << i);
    return sl;
  }

  unsigned
  TLink::nSuperLayers(const AList<TLink> & list)
  {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nSuperLayers() > 32)
      std::cout
          << "#super layers should be less than 32" << std::endl;
#endif

    unsigned l0 = 0;
    unsigned n = list.length();
    for (unsigned i = 0; i < n; i++) {
      unsigned id = list[i]->wire()->superLayerId();
      l0 |= (1 << id);
    }

    unsigned l = 0;
    for (unsigned i = 0; i < _nSL; i++) {
      if (l0 & (1 << i)) ++l;
    }
    return l;
  }

  unsigned
  TLink::nSuperLayers(const AList<TLink> & links, unsigned minN)
  {
    clearBufferSL();
    unsigned n = links.length();
    for (unsigned i = 0; i < n; i++)
      ++_nHitsSL[links[i]->wire()->superLayerId()];
    unsigned sl = 0;
    for (unsigned i = 0; i < _nSL; i++)
      if (_nHitsSL[i] >= minN)
        ++sl;
    return sl;
  }

  unsigned
  TLink::nMissingAxialSuperLayers(const AList<TLink> & links)
  {
    clearBufferSL();
    const unsigned n = links.length();
//  unsigned nHits[6] = {0, 0, 0, 0, 0, 0};
    for (unsigned i = 0; i < n; i++)
      if (links[i]->wire()->axial())
        ++_nHitsSL[links[i]->wire()->axialStereoSuperLayerId()];
//      ++nHits[links[i]->wire()->superLayerId() / 2];
    unsigned j = 0;
    while (_nHitsSL[j] == 0) ++j;
    unsigned nMissing = 0;
    unsigned nMax = 0;
    for (unsigned i = j; i < _nSLA; i++) {
      if (+_nHitsSL[i] == 0) ++nMissing;
      else {
        if (nMax < nMissing) nMax = nMissing;
        nMissing = 0;
      }
    }
    return nMax;
  }

  const Belle2::TRGCDCTrackMC&
  TLink::links2HEP(const AList<TLink> & links)
  {
    const Belle2::TRGCDCTrackMC* best = NULL;
    const std::vector<const Belle2::TRGCDCTrackMC*> list = Belle2::TRGCDCTrackMC::list();
    unsigned nHep = list.size();

    if (! nHep) return * best;

    unsigned* N;
    if (NULL == (N = (unsigned*) malloc(nHep * sizeof(unsigned)))) {
//      perror("$Id: TLink.cc 11153 2010-04-28 03:36:53Z yiwasaki $:N:malloc");
      exit(1);
    }
    for (unsigned i = 0; i < nHep; i++) N[i] = 0;

    for (unsigned i = 0; i < (unsigned) links.length(); i++) {
      const TLink& l = * links[i];
      const Belle2::TRGCDCTrackMC& hep = * l.hit()->mc()->hep();
      for (unsigned j = 0; j < nHep; j++)
        if (list[j] == & hep)
          ++N[j];
    }

    unsigned nMax = 0;
    for (unsigned i = 0; i < nHep; i++) {
      if (N[i] > nMax) {
        best = list[i];
        nMax = N[i];
      }
    }

    return * best;
  }

  void
  TLink::nHitsSuperLayer(const AList<TLink> & links, AList<TLink> * list)
  {
    const unsigned nLinks = links.length();
    for (unsigned i = 0; i < nLinks; i++)
      list[links[i]->wire()->superLayerId()].append(links[i]);
  }

  std::string
  TLink::layerUsage(const AList<TLink> & links)
  {
//  unsigned n[11];
    static unsigned* n = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
    nHitsSuperLayer(links, n);
    std::string nh;
    for (unsigned i = 0; i < _nSL; i++) {
      nh += itostring(n[i]);
      if (i % 2) nh += ",";
      else if (i < 10) nh += "-";
    }
    return nh;
  }

  void
  TLink::remove(AList<TLink> & list, const AList<TLink> & links)
  {
    const unsigned n = list.length();
    const unsigned m = links.length();
    AList<TLink> toBeRemoved;

    for (unsigned i = 0; i < n; i++) {
      for (unsigned j = 0; j < m; j++) {
        if (list[i]->wire()->id() == links[j]->wire()->id())
          toBeRemoved.append(list[i]);
      }
    }

    list.remove(toBeRemoved);
  }

  void
  TLink::initializeBuffers(void)
  {
    static bool first = true;
    if (first) {
      const Belle2::TRGCDC& cdc = * Belle2::TRGCDC::getTRGCDC();
      _nL = cdc.nLayers();
      _nSL = cdc.nSuperLayers();
      _nSLA = cdc.nAxialSuperLayers();
      _nHitsSL = new unsigned[_nSL];
      first = false;
    }
  }

} // namespace Belle
