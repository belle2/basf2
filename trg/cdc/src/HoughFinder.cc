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
#include "trg/trg/Debug.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/TrackSegment.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"

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
    return string("TRGCDCHoughFinder 5.04");
}

TRGCDCHoughFinder::TRGCDCHoughFinder(const string & name,
                                     const TRGCDC & TRGCDC,
                                     unsigned nX,
                                     unsigned nY)
    : _name(name),
      _cdc(TRGCDC),
      _perfect(false),
      _circleH("CircleHough"),
      _peakFinder("PeakFinder") {

    //...Make Hough planes...
    _plane[0] = new TCHPlaneMulti2("circle hough plus",
                                   _circleH,
                                   nX,
                                   0,
                                   2 * M_PI,
                                   nY,
                                   0.7,
                                   3,
                                   5);
    _plane[1] = new TCHPlaneMulti2("circle hough minus",
                                   _circleH,
                                   nX,
                                   0,
                                   2 * M_PI,
                                   nY,
                                   0.7,
                                   3,
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
    if (! H1)
        H1 = new TCDisplayHough("Minus");
    H1->link(* D);
    H1->clear();
    H1->show();
#endif

    //...Create patterns...
    unsigned axialSuperLayerId = 0;
    for (unsigned i = 0; i < _cdc.nTrackSegmentLayers(); i++) {
        const Belle2::TRGCDCLayer * l = _cdc.trackSegmentLayer(i);
        const unsigned nWires = l->nWires();
            
        if (! nWires) continue;
        if ((* l)[0]->stereo()) continue;
            
        _plane[0]->preparePatterns(axialSuperLayerId, nWires);
        _plane[1]->preparePatterns(axialSuperLayerId, nWires);
        for (unsigned j = 0; j < nWires; j++) {
            const Belle2::TRGCDCWire & w = * (* l)[j];
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

    if (_perfect)
        return doitPerfectly(trackList);

    TRGDebug::enterStage("Hough Finder");

    //...Initialization...
    _plane[0]->clear();
    _plane[1]->clear();

    //...Voting...
    unsigned axialSuperLayerId = 0;
    for (unsigned i = 0; i < _cdc.nTrackSegmentLayers(); i++) {
        const Belle2::TRGCDCLayer * l = _cdc.trackSegmentLayer(i);
        const unsigned nWires = l->nWires();
        if (! nWires) continue;
        if ((* l)[0]->stereo()) continue;

        for (unsigned j = 0; j < nWires; j++) {
            const TCTSegment & s = (TCTSegment &) * (* l)[j];

            //...Select hit TS only...
            if (s.triggerOutput().active()) {
                _plane[0]->vote(axialSuperLayerId, j);
                _plane[1]->vote(axialSuperLayerId, j);
            }
        }
        ++axialSuperLayerId;
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
    vector<TCCircle *> circles;
    _peakFinder.doit(circles, * _plane[0], 5, false);
    _peakFinder.doit(circles, * _plane[1], 5, false);

#ifdef TRGCDC_DISPLAY_HOUGH
    vector<const TCCircle *> cc;
    cc.assign(circles.begin(), circles.end());
    stg = "2D : Peak Finding";
    inf = "   ";
    D->clear();
    D->stage(stg);
    D->information(inf);
    D->area().append(cc, Gdk::Color("#FF0066009900"));
    D->area().append(_cdc.hits());
    D->area().append(_cdc.tsHits());
    D->show();
    D->run();
#endif

    //...Make circles tracks...
    

    //...2D fitting...

    //...Select stereo TSs...

    TRGDebug::leaveStage("Hough Finder");
    return 0;
}

int
TRGCDCHoughFinder::doitPerfectly(vector<TRGCDCTrack *> & trackList) {

    TRGDebug::enterStage("Perfect Finder");

    //...TS hit loop...
    //   Presently assuming single track event.
    //   Select the best TS(the fastest hit) in eash super layer.
    //

    vector<TCLink *> links;
    for (unsigned i = 0; i < _cdc.nTrackSegmentLayers(); i++) {
        const Belle2::TRGCDCLayer * l = _cdc.trackSegmentLayer(i);
        const unsigned nWires = l->nWires();
        if (! nWires) continue;

        int timeMin = 99999;
        const TCTSegment * best = 0;
        for (unsigned j = 0; j < nWires; j++) {
            const TCTSegment & s = (TCTSegment &) * (* l)[j];

            //...Select hit TS only...
            const TRGSignal & timing = s.triggerOutput();
            if (! timing.active())
                continue;

            //...Select TS with the shortest drift time.
            const TRGTime & t = * timing[0];
            if (t.time() < timeMin) {
                timeMin = t.time();
                best = & s;
            }
        }

        if (best) {
	    TCLink * link = new TCLink(0,
				       best->hit(),
				       best->hit()->wire().xyPosition());
	    links.push_back(link);
	}
    }

    //...Let's fit it...
    TCCircle c = TCCircle(links);
    c.fit();
    c.name("CircleFitted");

    //...Make a track...
    TCTrack * track = new TCTrack(c);
    trackList.push_back(track);

    if (TRGDebug::level())
 	track->dump("detail");

#ifdef TRGCDC_DISPLAY_HOUGH
    vector<const TCCircle *> cc;
    cc.push_back(& c);
    vector<const TCTrack *> tt;
    tt.push_back(track);
    string stg = "2D : Perfect Finder circle fit";
    string inf = "   ";
    D->clear();
    D->stage(stg);
    D->information(inf);
    D->area().append(cc, Gdk::Color("#FF0066009900"));
//  D->area().append(tt, Gdk::Color("#990066009900"));
    D->area().append(_cdc.hits());
    D->area().append(_cdc.tsHits());
    D->show();
    D->run();
#endif

    TRGDebug::leaveStage("Perfect Finder");
    return 0;
}

} // namespace Belle2
