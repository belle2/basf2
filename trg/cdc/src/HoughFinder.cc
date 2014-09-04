//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughFinder.cc
// Section  : TRG CDC
// Owner    : Yoshihito Iwasaki
// Email    : yoshihito.iwasaki@kek.jp
//-----------------------------------------------------------------------------
// Description : A class to find tracks usning Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <stdlib.h>
#include <map>
#include "cdc/dataobjects/CDCSimHit.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Relation.h"

#ifdef TRGCDC_DISPLAY_HOUGH
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
    Belle2::TRGCDCDisplayHough * H0 = 0;
    Belle2::TRGCDCDisplayHough * H1 = 0;
}
#endif

using namespace std;
#ifdef TRGCDC_DISPLAY_HOUGH
using namespace Belle2_TRGCDC;
#endif

namespace Belle2 {

string
TRGCDCHoughFinder::version(void) const {
    return string("TRGCDCHoughFinder 5.24");
}

TRGCDCHoughFinder::TRGCDCHoughFinder(const string & name,
                                     const TRGCDC & TRGCDC,
                                     unsigned nX,
                                     unsigned nY,
				     unsigned peakMin)
    : _name(name),
      _cdc(TRGCDC),
      _circleH("CircleHough"),
      _peakFinder("PeakFinder"),
      _peakMin(peakMin) {

    //...Make Hough lanes...
    _plane[0] = new TCHPlaneMulti2("circle hough plus",
                                   _circleH,
                                   nX,
                                   0,
                                   2 * M_PI,
                                   nY,
                                   0.9615,
                                   3.204,
                                   5);
    _plane[1] = new TCHPlaneMulti2("circle hough minus",
                                   _circleH,
                                   nX,
                                   0,
                                   2 * M_PI,
                                   nY,
                                   0.9615,
                                   3.204,
                                   5);

    //...Set charge...
    _plane[0]->charge(1);
    _plane[1]->charge(-1);

#ifdef TRGCDC_DISPLAY_HOUGH
    if (! H0)
        H0 = new TCDisplayHough("Plus");
    H0->link(* D);
    H0->clear();
    H0->show();
    H0->move(630, 0);
    if (! H1)
        H1 = new TCDisplayHough("Minus");
    H1->link(* D);
    H1->clear();
    H1->show();
    H0->move(1260, 0);
#endif

    //...Create patterns...
    unsigned axialSuperLayerId = 0;
    for (unsigned i = 0; i < _cdc.nSegmentLayers(); i++) {
        const Belle2::TRGCDCLayer * l = _cdc.segmentLayer(i);
        const unsigned nWires = l->nCells();
            
        if (! nWires) continue;
        if ((* l)[0]->stereo()) continue;
            
        _plane[0]->preparePatterns(axialSuperLayerId, nWires);
        _plane[1]->preparePatterns(axialSuperLayerId, nWires);
        for (unsigned j = 0; j < nWires; j++) {
            const TCCell & w = * (* l)[j];
            const float x = w.xyPosition().x();
            const float y = w.xyPosition().y();
                
            _plane[0]->clear();
            _plane[0]->vote(x, y, +1, axialSuperLayerId, 1);
            _plane[0]->registerPattern(axialSuperLayerId, j);
                
            _plane[1]->clear();
            _plane[1]->vote(x, y, -1, axialSuperLayerId, 1);
            _plane[1]->registerPattern(axialSuperLayerId, j);

#ifdef TRGCDC_DISPLAY_HOUGH
//             string stg = "Hough Pattern Regstration";
//             string inf = "   ";
//             _plane[0]->merge();
//             _plane[1]->merge();
//             H0->stage(stg);
//             H0->information(inf);
//             H0->clear();
//             H0->area().append(_plane[0]);
//             H0->show();
//             H1->stage(stg);
//             H1->information(inf);
//             H1->clear();
//             H1->area().append(_plane[1]);
//             H1->show();
//             H1->run();
#endif
        }
        ++axialSuperLayerId;
    }
}

TRGCDCHoughFinder::~TRGCDCHoughFinder() {
#ifdef TRGCDC_DISPLAY_HOUGH
    if (H0)
        delete H0;
    if (H1)
        delete H1;
    cout << "TRGCDCHoughFinder ... Hough displays deleted" << endl;
#endif
}

int
TRGCDCHoughFinder::doit(vector<TCTrack *> & trackList) {

    TRGDebug::enterStage("Hough Finder");

    //...Initialization...
    _plane[0]->clear();
    _plane[1]->clear();

    //...Voting...
    unsigned nLayers = _cdc.nAxialSuperLayers();
    for (unsigned i = 0; i < nLayers; i++) {
	const vector<const TCSHit *> hits = _cdc.axialSegmentHits(i);
	for (unsigned j = 0; j < hits.size(); j++) {
	    _plane[0]->vote(i, hits[j]->cell().localId());
	    _plane[1]->vote(i, hits[j]->cell().localId());
	}
    }
    _plane[0]->merge();
    _plane[1]->merge();

#ifdef TRGCDC_DISPLAY_HOUGH
    string stg = "2D : Hough : Results of Peak Finding";
    string inf = "   ";
    H0->stage(stg);
    H0->information(inf);
    H0->clear();
    H0->area().append(_plane[0]);
    H0->show();
    H1->stage(stg);
    H1->information(inf);
    H1->clear();
    H1->area().append(_plane[1]);
    H1->show();
#endif

    //...Look for peaks which have 5 hits...
    vector<unsigned> serialIds[2];

//...Temporary 
    _peakFinder.doit(* _plane[0], _peakMin, false, serialIds[0]);
    _peakFinder.doit(* _plane[1], _peakMin, false, serialIds[1]);

    //...Peak loop...
    unsigned nCircles = 0;
    for (unsigned pm = 0; pm < 2; pm++) {
	for (unsigned i = 0; i < serialIds[pm].size(); i++) {
	    const unsigned peakId = serialIds[pm][i];

	    //...Get segment hits...
	    vector<TCLink *> links;
	    vector<const TCSegment *> segments;
	    const unsigned nLayers = _plane[pm]->nLayers();
	    for (unsigned j = 0; j < nLayers; j++) {
		const vector<unsigned> & ptn =
		    _plane[pm]->patternId(j, peakId);
		for (unsigned k = 0; k < ptn.size(); k++) {
		    const TCSegment & s = _cdc.axialSegment(j, ptn[k]);
		    segments.push_back(& s);
		    if (s.hit()) {
			TCLink * l = new TCLink(0, s.hit());
			links.push_back(l);
		    }
		}
	    }

	    //...Select best hits in each layer...
	    const vector<TCLink *> bests = selectBestHits(links);

	    //...Make a circle...
	    TCCircle * cptr = new TCCircle(bests);
	    TCCircle & c = * cptr;
	    c.fit();
	    c.name("CircleFitted_" + TRGUtil::itostring(nCircles));
	    ++nCircles;

	    if (TRGDebug::level()) {
		cout << TRGDebug::tab() << "peak#" << nCircles << ":"
		     << "plane" << pm << ",serialId=" << peakId << endl;
		cout << TRGDebug::tab() << "segments below" << endl;
		cout << TRGDebug::tab(4);
		for (unsigned j = 0; j < segments.size(); j++) {
		    cout << segments[j]->name();
		    if (j != (segments.size() - 1))
			cout << ",";
		}
		cout << endl;
		cout << TRGDebug::tab() << "best links below" << endl;
		TCLink::dump(bests, "", TRGDebug::tab(4));
 		c.dump("detail", TRGDebug::tab() + "Circle> ");
	    }

	    //...Make a track...
	    TCTrack & t = * new TCTrack(c);
	    t.name("Track_" + TRGUtil::itostring(i));
	    trackList.push_back(& t);
	    if (TRGDebug::level()) {
		t.relation().dump("", "        MCInfo");
		t.dump("detail");
	    }
#ifdef TRGCDC_DISPLAY_HOUGH
	    vector<const TCCircle *> cc;
	    cc.push_back(& c);
	    stg = "2D : Peak Finding & circle making";
	    inf = "   ";
	    D->clear();
	    D->stage(stg);
	    D->information(inf);
	    D->area().append(cc, Gdk::Color("#FF0066009900"));
	    D->area().append(_cdc.hits());
	    D->area().append(_cdc.segmentHits());
	    D->show();
	    D->run();
#endif

 	    //...Delete a circle...
 	    delete cptr;

	}
    }

    TRGDebug::leaveStage("Hough Finder");
    return 0;
}

vector<TCLink *>
TRGCDCHoughFinder::selectBestHits(const vector<TCLink *> & links) const {
    vector<TCLink *> bests;
    vector<TCLink *> layers[9];
    TCLink::separate(links, 9, layers);

    for (unsigned i = 0; i < 9; i++) {
	cout << TRGDebug::tab() << "layer " << i << endl;
	TCLink::dump(layers[i], "", TRGDebug::tab(4));
    }

    //...Select links to be removed...
    for (unsigned i = 0; i < 9; i++) {
	if (layers[i].size() == 0) continue;
	if (layers[i].size() == 1) {
	    bests.push_back(layers[i][0]);
	    continue;
	}

	TCLink * best = layers[i][0];
	int timeMin = (layers[i][0]->cell()->signal())[0]->time();
	for (unsigned j = 1; j < layers[i].size(); j++) {
	    const TRGTime & t = * (layers[i][j]->cell()->signal())[0];
	    if (t.time() < timeMin) {
		timeMin = t.time();
		best = layers[i][j];
	    }
	}

	bests.push_back(best);
    }

    return bests;
}

} // namespace Belle2
