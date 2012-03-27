//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Link.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to relate TRGCDCWireHit and TRGCDCTrack objects.
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <string>
#include <cstring>
#include "trg/trg/Constants.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Link.h"

using namespace std;

namespace Belle2 {

#ifdef TRASAN_DEBUG
unsigned TRGCDCLink::_nTRGCDCLinks = 0;
unsigned TRGCDCLink::_nTRGCDCLinksMax = 0;

unsigned
TRGCDCLink::nTRGCDCLinks(void) {
    return _nTRGCDCLinks;
}

unsigned
TRGCDCLink::nTRGCDCLinksMax(void) {
    return _nTRGCDCLinksMax;
}
#endif

bool TRGCDCLink::ms_smallcell(false);
bool TRGCDCLink::ms_superb(false);
unsigned TRGCDCLink::_nL = 0;
unsigned TRGCDCLink::_nSL = 0;
unsigned TRGCDCLink::_nSLA = 0;
unsigned * TRGCDCLink::_nHitsSL = 0;

TRGCDCLink::TRGCDCLink(TCTrack * t,
		       const Belle2::TCWHit * h,
		       const HepGeom::Point3D<double> & p)
: _track(t),
  _hit(h),
  _position(p),
  _dPhi(0),
  _leftRight(0),
  _pull(0),
  _link(0),
  _fit2D(0) {
    if (h) {
	_drift[0] = h->drift(0);
	_drift[1] = h->drift(1);
	_dDrift[0] = h->dDrift(0);
	_dDrift[1] = h->dDrift(1);
    }
    else {
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
    ++_nTRGCDCLinks;
    if (_nTRGCDCLinks > _nTRGCDCLinksMax)
	_nTRGCDCLinksMax = _nTRGCDCLinks;
#endif
}

TRGCDCLink::TRGCDCLink(const TRGCDCLink & l)
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
  _fit2D(l._fit2D) {
    _drift[0] = l._drift[0];
    _drift[1] = l._drift[1];
    _dDrift[0] = l._dDrift[0];
    _dDrift[1] = l._dDrift[1];
    for (unsigned i = 0; i < 7; ++i)
	_neighbor[i] = l._neighbor[i];
    for (unsigned i = 0; i < 4; ++i)
      _arcZ[i] = l._arcZ[i];
#ifdef TRASAN_DEBUG
    ++_nTRGCDCLinks;
    if (_nTRGCDCLinks > _nTRGCDCLinksMax)
	_nTRGCDCLinksMax = _nTRGCDCLinks;
#endif
}

TRGCDCLink::~TRGCDCLink() {
#ifdef TRASAN_DEBUG
    --_nTRGCDCLinks;
#endif
}

void
TRGCDCLink::dump(const string & msg, const string & pre) const {

    //...Basic options...
    bool track = (msg.find("track") != string::npos);
    bool mc = (msg.find("mc") != string::npos);
    bool pull = (msg.find("pull") != string::npos);
    bool flag = (msg.find("flag") != string::npos);
    bool stereo = (msg.find("stereo") != string::npos);
    bool pos = (msg.find("position") != string::npos);

    //...Strong options...
    bool breif = (msg.find("breif") != string::npos);
    bool detail = (msg.find("detail") != string::npos);
    if (detail)
	track = mc = pull = flag = stereo = pos = true;
    if (breif)
	mc = pull = flag = true;

    //...Output...
    std::cout << pre;
    std::cout << wire()->name();
//     if (mc) {
// 	if (_hit) {
// 	    if (_hit->mc()) {
// 		if (_hit->mc()->hep())
// 		    std::cout << "(mc" << _hit->mc()->hep()->id() << ")";
// 		else
// 		    std::cout << "(mc?)";
// 	    }
// 	    else {
// 		std::cout << "(mc?)";
// 	    }
// 	}
//     }
    if (pull)
	std::cout << "[pul=" << this->pull() << "]";
    if (flag) {
	if (_hit) {
	    if (_hit->state() & WireHitFindingValid)
		std::cout << "o";
	    if (_hit->state() & WireHitFittingValid)
		std::cout << "+";
	    if (_hit->state() & WireHitInvalidForFit)
		std::cout << "x";
	}
    }
    if (stereo) {
	std::cout << "{" << leftRight() << "," << _zStatus << "}";
    }
    if (pos) {
	std::cout << ",pos=" << position();
	std::cout << ",drift=" << drift(0) << "," << drift(1);
    }
    if (! breif)
	std::cout << std::endl;
}

unsigned
TRGCDCLink::nLayers(const vector<TRGCDCLink *> & list) {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nLayers() > 64)
	std::cout << "TRGCDCLink::nLayers !!! #layers should be less than 64"
	       << std::endl;
#endif

    unsigned l0 = 0;
    unsigned l1 = 0;
    unsigned n = list.size();
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
TRGCDCLink::nHits(const vector<TRGCDCLink *> & links, unsigned * nHits) {
    for (unsigned i = 0; i < _nL; i++)
	nHits[i] = 0;
    unsigned nLinks = links.size();
    for (unsigned i = 0; i < nLinks; i++) {
	++nHits[links[i]->wire()->layerId()];

#ifdef TRASAN_DEBUG
	if (links[i]->wire()->layerId() >= _nL)
	    std::cout << "TRGCDCLink::nHits !!! layer ID("
		   << links[i]->wire()->layerId() << ") is larger than "
		   << "size of buffer(" << _nL << ")" << std::endl;
#endif
    }
}

void
TRGCDCLink::nHitsSuperLayer(const vector<TRGCDCLink *> & links, unsigned * nHits) {
    for (unsigned i = 0; i < _nSL; i++)
	nHits[i] = 0;
    const unsigned nLinks = links.size();
    for (unsigned i = 0; i < nLinks; i++) {
	++nHits[links[i]->wire()->superLayerId()];

#ifdef TRASAN_DEBUG
	if (links[i]->wire()->superLayerId() >= _nSL)
	    std::cout << "TRGCDCLink::nHitsSuperLayer !!! super layer ID("
		   << links[i]->wire()->superLayerId() << ") is larger than "
		   << "size of buffer(" << _nSL << ")" << std::endl;
#endif
    }
}

void
TRGCDCLink::dump(const vector<const TRGCDCLink *> & links,
		 const string & msg,
		 const string & pre) {

    //...Basic options...
    bool mc = (msg.find("mc") != string::npos);
//  bool sort = (msg.find("sort") != string::npos);
    bool flag = (msg.find("flag") != string::npos);

    //...Strong options...
    bool breif = (msg.find("breif") != string::npos);
    bool detail = (msg.find("detail") != string::npos);
    if (detail)
	mc = flag = true;
    if (breif)
	mc = true;

    vector<const TRGCDCLink *> tmp = links;
//  if (sort)
//      tmp.sort(TRGCDCLink::sortByWireId);
    unsigned n = tmp.size();
    unsigned nForFit = 0;
#define MCC_MAX 1000
    unsigned MCC0[MCC_MAX];
    unsigned MCC1[MCC_MAX];
    memset((char *) MCC0, 0, sizeof(unsigned) * MCC_MAX);
    memset((char *) MCC1, 0, sizeof(unsigned) * MCC_MAX);
    bool MCCOverFlow = false;

    for (unsigned i = 0; i < n; i++) {
	const TRGCDCLink & l = * tmp[i];

// 	if (mc) {
// 	    unsigned mcId = 999;
// 	    if (l.hit()) {
// 		if (l.hit()->mc())
// 		    if (l.hit()->mc()->hep())
// 			mcId = l.hit()->mc()->hep()->id();
// 		if (mcId < MCC_MAX) {
// 		    ++MCC0[mcId];
// 		    if (l.hit()->state() & WireHitFittingValid) {
// 			if (! (l.hit()->state() & WireHitInvalidForFit))
// 			    ++MCC1[mcId];
// 		    }
// 		}
// 		else {
// 		    MCCOverFlow = true;
// 		}
// 	    }
// 	}
	if (flag) {
	    if (l.hit()) {
		if (l.hit()->state() & WireHitFittingValid) {
		    if (! (l.hit()->state() & WireHitInvalidForFit))
			++nForFit;
		}
	    }
	}
	if (i == 0) {
	    std::cout << pre;
	    if (! breif)
		std::cout << TRGUtil::itostring(i) << " ";
	}
	else {
	    if (breif)
		std::cout << ",";
	    else
		std::cout << pre << TRGUtil::itostring(i) << " ";
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
	std::vector<const Belle2::TRGCDCTrackMC *> list = Belle2::TRGCDCTrackMC::list();
	if (! list.size()) return;
	std::cout << pre;
//cnv	unsigned nMC = 0;
	for (unsigned i = 0; i < MCC_MAX; i++) {
	    if (MCC0[i] > 0) {
		const Belle2::TRGCDCTrackMC * h = list[i];
//		std::cout << ", mc" << i << "(" << h->pType() << ")";
		std::cout << ", mc" << i << "(";
		if (h)
		    std::cout << h->pType() << ")";
		else
		    std::cout << "?)";
		while (h) {
		    const Belle2::TRGCDCTrackMC * m = h->mother();
		    if (m) {
			std::cout << "<-mc" << m->id();
			h = m;
		    }
		    else {
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
TRGCDCLink::dump(const TRGCDCLink & link,
		 const string & msg,
		 const string & pre) {
    vector<const TRGCDCLink *> tmp;
    tmp.push_back(& link);
    dump(tmp, msg, pre);
}

unsigned
TRGCDCLink::nStereoHits(const vector<TRGCDCLink *> & links) {
    unsigned nLinks = links.size();
    unsigned n = 0;
    for (unsigned i = 0; i < nLinks; i++)
	if (links[i]->wire()->stereo())
	    ++n;
    return n;
}

unsigned
TRGCDCLink::nAxialHits(const vector<TRGCDCLink *> & links) {
    unsigned nLinks = links.size();
    unsigned n = 0;
    for (unsigned i = 0; i < nLinks; i++)
	if (links[i]->wire()->axial())
	    ++n;
    return n;
}

vector<TRGCDCLink *>
TRGCDCLink::axialHits(const vector<TRGCDCLink *> & links) {
    vector<TRGCDCLink *> a;
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++) {
	if (links[i]->wire()->axial())
	    a.push_back(links[i]);
    }
    return a;
}

vector<TRGCDCLink *>
TRGCDCLink::stereoHits(const vector<TRGCDCLink *> & links) {
    vector<TRGCDCLink *> a;
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++) {
	if (! links[i]->wire()->axial())
	    a.push_back(links[i]);
    }
    return a;
}

TRGCDCLink *
TRGCDCLink::innerMost(const vector<TRGCDCLink *> & a) {
    unsigned n = a.size();
    unsigned minId = 19999;
    TRGCDCLink * t = 0;
    for (unsigned i = 0; i < n; i++) {
	unsigned id = a[i]->wire()->id();
	if (id < minId) {
	    minId = id;
	    t = a[i];
	}
    }
    return t;
}

TRGCDCLink *
TRGCDCLink::outerMost(const vector<TRGCDCLink *> & a) {
    unsigned n = a.size();
    unsigned maxId = 0;
    TRGCDCLink * t = 0;
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
TRGCDCLink::separateCores(const vector<TRGCDCLink *> & input,
			  vector<TRGCDCLink *> & cores,
			  vector<TRGCDCLink *> & nonCores) {
    unsigned n = input.size();
    for (unsigned i = 0; i < n; i++) {
	TRGCDCLink & t = * input[i];
	const Belle2::TCWHit & h = * t.hit();
	if (h.state() & WireHitFittingValid)
	    cores.push_back(& t);
	else
	    nonCores.push_back(& t);
    }
}

vector<TRGCDCLink *>
TRGCDCLink::cores(const vector<TRGCDCLink *> & input) {
    vector<TRGCDCLink *> a;
    unsigned n = input.size();
    for (unsigned i = 0; i < n; i++) {
	TRGCDCLink & t = * input[i];
	const Belle2::TCWHit & h = * t.hit();
	if (h.state() & WireHitFittingValid)
	    a.push_back(& t);
    }
    return a;
}

#if defined(__GNUG__)
int
TRGCDCLink::sortByWireId(const TRGCDCLink ** a, const TRGCDCLink ** b) {
    if ((* a)->wire()->id() > (* b)->wire()->id()) return 1;
    else if
	((* a)->wire()->id() == (* b)->wire()->id()) return 0;
    else return -1;
}

int
TRGCDCLink::sortByX(const TRGCDCLink ** a, const TRGCDCLink ** b) {
    if ((* a)->position().x() > (* b)->position().x()) return 1;
    else if ((* a)->position().x() == (* b)->position().x()) return 0;
    else return -1;
}

#else
extern "C" int
SortByWireId(const void * av, const void * bv) {
  const TRGCDCLink ** a((const TRGCDCLink**)av);
  const TRGCDCLink ** b((const TRGCDCLink**)bv);
    if ((* a)->wire()->id() > (* b)->wire()->id()) return 1;
    else if
	((* a)->wire()->id() == (* b)->wire()->id()) return 0;
    else return -1;
}

extern "C" int
SortByX(const void* av, const void* bv) {
  const TRGCDCLink ** a((const TRGCDCLink**)av);
  const TRGCDCLink ** b((const TRGCDCLink**)bv);
    if ((* a)->position().x() > (* b)->position().x()) return 1;
    else if ((* a)->position().x() == (* b)->position().x()) return 0;
    else return -1;
}
#endif

unsigned
TRGCDCLink::width(const vector<TRGCDCLink *> & list) {
    const unsigned n = list.size();
    if (n < 2) return n;

    const Belle2::TRGCDCWire * const w0 = list[0]->wire();
//cnv    const unsigned sId = w0->superLayerId();
    unsigned nWires = w0->layer().nWires();
    unsigned center = w0->localId();

    if (ms_smallcell && w0->layerId() < 3) {
	nWires /= 2;
	center /= 2;
    }

#ifdef TRASAN_DEBUG_DETAIL
//     std::cout << "    ... Width : sid=" << sId << ",lid=" << w0->layerId()
// 	      << ",nWires=" << nWires << ",center=" << center << " : " 
// 	      << std::endl;
//     dump(list);
#endif

    unsigned left = 0;
    unsigned right = 0;
    for (unsigned i = 1; i < n; i++) {
	const Belle2::TRGCDCWire * const w = list[i]->wire();
	unsigned id = w->localId();

	if (ms_smallcell && w->layerId() < 3)
	    id /= 2;
	
	unsigned distance0, distance1;
	if (id > center) {
	    distance0 = id - center;
	    distance1 = nWires - distance0;
	}
	else {
	    distance1 = center - id;
	    distance0 = nWires - distance1;
	}

	if (distance0 < distance1) {
	    if (distance0 > right) right = distance0;
	}
	else {
	    if (distance1 > left) left = distance1;
	}

#ifdef TRASAN_DEBUG_DETAIL
// 	std::cout << "        ... lyr=" << w->layerId()
// 		  << ",true id=" << w->localId() << ",id=" << id
// 		  << ",distance0,1=" << distance0 << "," << distance1
// 		  << std::endl;
// 	if (w->superLayerId() != sId)
// 	    std::cout << "::width !!! super layer assumption violation"
// 		      << std::endl;
#endif
    }

    return right + left + 1;
}

vector<TRGCDCLink *>
TRGCDCLink::edges(const vector<TRGCDCLink *> & list) {
    vector<TRGCDCLink *> a;

    unsigned n = list.size();
    if (n < 2) return a;
    else if (n == 2) return list;

    const Belle2::TRGCDCWire * w = list[0]->wire();
    unsigned nWires = w->layer().nWires();
    unsigned center = w->localId();

    unsigned left = 0;
    unsigned right = 0;
    TRGCDCLink * leftL = list[0];
    TRGCDCLink * rightL = list[0];
    for (unsigned i = 1; i < n; i++) {
	w = list[i]->wire();
	unsigned id = w->localId();

	unsigned distance0, distance1;
	if (id > center) {
	    distance0 = id - center;
	    distance1 = nWires - distance0;
	}
	else {
	    distance1 = center - id;
	    distance0 = nWires - distance1;
	}

	if (distance0 < distance1) {
	    if (distance0 > right) {
		right = distance0;
		rightL = list[i];
	    }
	}
	else {
	    if (distance1 > left) {
		left = distance1;
		leftL = list[i];
	    }
	}
    }

    a.push_back(leftL);
    a.push_back(rightL);
    return a;
}

vector<TRGCDCLink *>
TRGCDCLink::sameLayer(const vector<TRGCDCLink *> & list, const TRGCDCLink & a) {
    vector<TRGCDCLink *> same;
    unsigned id = a.wire()->layerId();
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
	if (list[i]->wire()->layerId() == id) same.push_back(list[i]);
    }
    return same;
}

vector<TRGCDCLink *>
TRGCDCLink::sameSuperLayer(const vector<TRGCDCLink *> & list, const TRGCDCLink & a) {
    vector<TRGCDCLink *> same;
    unsigned id = a.wire()->superLayerId();
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
	if (list[i]->wire()->superLayerId() == id) same.push_back(list[i]);
    }
    return same;
}

vector<TRGCDCLink *>
TRGCDCLink::sameLayer(const vector<TRGCDCLink *> & list, unsigned id) {
    vector<TRGCDCLink *> same;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
	if (list[i]->wire()->layerId() == id) same.push_back(list[i]);
    }
    return same;
}

vector<TRGCDCLink *>
TRGCDCLink::sameSuperLayer(const vector<TRGCDCLink *> & list, unsigned id) {
    vector<TRGCDCLink *> same;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++) {
	if (list[i]->wire()->superLayerId() == id) same.push_back(list[i]);
    }
    return same;
}

vector<TRGCDCLink *>
TRGCDCLink::inOut(const vector<TRGCDCLink *> & list) {
    vector<TRGCDCLink *> inners;
    vector<TRGCDCLink *> outers;
    unsigned n = list.size();
    unsigned innerMostLayer = 999;
    unsigned outerMostLayer = 0;
    for (unsigned i = 0; i < n; i++) {
	unsigned id = list[i]->wire()->layerId();
	if (id < innerMostLayer) innerMostLayer = id;
	else if (id > outerMostLayer) outerMostLayer = id;
    }
    for (unsigned i = 0; i < n; i++) {
	unsigned id = list[i]->wire()->layerId();
	if (id == innerMostLayer) inners.push_back(list[i]);
	else if (id == outerMostLayer) outers.push_back(list[i]);
    }
//  inners.push_back(outers);
    inners.insert(inners.end(), outers.begin(), outers.end());
    return inners;
}

unsigned
TRGCDCLink::superLayer(const vector<TRGCDCLink *> & list) {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nSuperLayers() > 32)
	std::cout << "TRGCDCLink::superLayer !!! #super layers should be less than 32"
	       << std::endl;
#endif

    unsigned sl = 0;
    unsigned n = list.size();
    for (unsigned i = 0; i < n; i++)
	sl |= (1 << (list[i]->wire()->superLayerId()));
    return sl;
}

unsigned
TRGCDCLink::superLayer(const vector<TRGCDCLink *> & links, unsigned minN) {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nSuperLayers() > 32)
	std::cout
	    << "#super layers should be less than 32" << std::endl;
#endif

    clearBufferSL();
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++)
	++_nHitsSL[links[i]->wire()->superLayerId()];
    unsigned sl = 0;
    for (unsigned i = 0; i < _nSL; i++)
	if (_nHitsSL[i] >= minN)
	    sl |= (1 << i);
    return sl;
}

unsigned
TRGCDCLink::nSuperLayers(const vector<TRGCDCLink *> & list) {
#ifdef TRASAN_DEBUG
    const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
    if (cdc.nSuperLayers() > 32)
	std::cout
	    << "#super layers should be less than 32" << std::endl;
#endif

    unsigned l0 = 0;
    unsigned n = list.size();
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
TRGCDCLink::nSuperLayers(const vector<TRGCDCLink *> & links, unsigned minN) {
    clearBufferSL();
    unsigned n = links.size();
    for (unsigned i = 0; i < n; i++)
	++_nHitsSL[links[i]->wire()->superLayerId()];
    unsigned sl = 0;
    for (unsigned i = 0; i < _nSL; i++)
	if (_nHitsSL[i] >= minN)
	    ++sl;
    return sl;
}

unsigned
TRGCDCLink::nMissingAxialSuperLayers(const vector<TRGCDCLink *> & links) {
    clearBufferSL();
    const unsigned n = links.size();
//  unsigned nHits[6] = {0, 0, 0, 0, 0, 0};
    for (unsigned i = 0; i < n; i++)
	if (links[i]->wire()->axial())
	    ++_nHitsSL[links[i]->wire()->axialStereoSuperLayerId()];
//	    ++nHits[links[i]->wire()->superLayerId() / 2];
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

const Belle2::TRGCDCTrackMC &
TRGCDCLink::links2HEP(const vector<TRGCDCLink *> & ) {
    const Belle2::TRGCDCTrackMC * best = NULL;
    const std::vector<const Belle2::TRGCDCTrackMC *> list = Belle2::TRGCDCTrackMC::list();
    unsigned nHep = list.size();

    if (! nHep) return * best;

    unsigned * N;
    if (NULL == (N= (unsigned *) malloc(nHep * sizeof(unsigned)))) {
//      perror("$Id: TRGCDCLink.cc 11153 2010-04-28 03:36:53Z yiwasaki $:N:malloc");
      exit(1);
    }  
    for (unsigned i = 0; i < nHep; i++) N[i] = 0;

//     for (unsigned i = 0; i < (unsigned) links.size(); i++) {
// 	const TRGCDCLink & l = * links[i];
// 	const Belle2::TRGCDCTrackMC & hep = * l.hit()->mc()->hep();
// 	for (unsigned j = 0; j < nHep; j++)
// 	    if (list[j] == & hep)
// 		++N[j];
//     }

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
TRGCDCLink::nHitsSuperLayer(const vector<TRGCDCLink *> & links, vector<TRGCDCLink *> * list) {
    const unsigned nLinks = links.size();
    for (unsigned i = 0; i < nLinks; i++)
	list[links[i]->wire()->superLayerId()].push_back(links[i]);
}

string
TRGCDCLink::layerUsage(const vector<TRGCDCLink *> & links) {
//  unsigned n[11];
    static unsigned * n = new unsigned[Belle2::TRGCDC::getTRGCDC()->nSuperLayers()];
    nHitsSuperLayer(links, n);
    string nh;
    for (unsigned i = 0; i < _nSL; i++) {
	nh += TRGUtil::itostring(n[i]);
	if (i % 2) nh += ",";
	else if (i < 10) nh += "-";
    }
    return nh;
}

void
TRGCDCLink::remove(vector<TRGCDCLink *> & list,
		   const vector<TRGCDCLink *> & links) {
    const unsigned n = list.size();
    const unsigned m = links.size();
    vector<TRGCDCLink *> toBeRemoved;

    for (unsigned i = 0; i < n; i++) {
	for (unsigned j = 0; j < m; j++) {
	    if (list[i]->wire()->id() == links[j]->wire()->id())
//		toBeRemoved.push_back(list[i]);

		cout << "TCLink::remove !!! not implemented yet" << endl;
	}
    }

//  list.remove(toBeRemoved);
}

void
TRGCDCLink::initializeBuffers(void) {
    static bool first = true;
    if (first) {
	const Belle2::TRGCDC & cdc = * Belle2::TRGCDC::getTRGCDC();
	_nL = cdc.nLayers();
	_nSL = cdc.nSuperLayers();
	_nSLA = cdc.nAxialSuperLayers();
	_nHitsSL = new unsigned[_nSL];
	first = false;
    }
}

} // namespace Belle
