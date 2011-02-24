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
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/TrackSegment.h"

#ifdef TRGCDC_DISPLAY
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
    extern Belle2::TRGCDCDisplayRphi * D;
    Belle2::TRGCDCDisplayHough * H0 = 0;
    Belle2::TRGCDCDisplayHough * H1 = 0;
}
#endif

using namespace std;
#ifdef TRGCDC_DISPLAY
using namespace Belle2_TRGCDC;
#endif

namespace Belle2 {

string
TRGCDCHoughFinder::version(void) const {
    return string("0.01");
}

TRGCDCHoughFinder::TRGCDCHoughFinder(const string & name,
                                     const TRGCDC & TRGCDC,
                                     unsigned nX,
                                     unsigned nY)
    : _name(name),
      _cdc(TRGCDC),
      _circleH("CircleHough"),
      _peakFinder("PeakFinder") {

    //...Make Hough planes...
    _plane[0] = new TCHPlaneMulti2("circle hough plus",
                                   nX,
                                   0,
                                   2 * M_PI,
                                   nY,
                                   0.7,
                                   3,
                                   5);
    _plane[1] = new TCHPlaneMulti2("circle hough minus",
                                   nX,
                                   0,
                                   2 * M_PI,
                                   nY,
                                   0.7,
                                   3,
                                   5);

#ifdef TRGCDC_DISPLAY
    if (! H0)
        H0 = new TCDisplayHough("Plus");
    H0->clear();
    H0->show();
    if (! H1)
        H1 = new TCDisplayHough("Minus");
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
            _plane[0]->vote(x, y, +1, _circleH, axialSuperLayerId, 1);
            _plane[0]->registerPattern(axialSuperLayerId, j);
                
            _plane[1]->clear();
            _plane[1]->vote(x, y, -1, _circleH, axialSuperLayerId, 1);
            _plane[1]->registerPattern(axialSuperLayerId, j);

#ifdef TRGCDC_DISPLAY
            string stg = "Hough Pattern Regstration";
            string inf = "   ";

            _plane[0]->dump();
            _plane[0]->merge();
            _plane[1]->dump();
            _plane[1]->merge();
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
            H1->run();
#endif
        }
        ++axialSuperLayerId;
    }
}

TRGCDCHoughFinder::~TRGCDCHoughFinder() {
#ifdef TRGCDC_DISPLAY
    if (H0)
        delete H0;
    if (H1)
        delete H1;
#endif
}

int
TRGCDCHoughFinder::doit(void) {

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

    //...Look for peaks which have 5 hits...
    vector<TRGPoint2D *> peaks0 = _peakFinder.doit(* _plane[0], 5, false);
    vector<TRGPoint2D *> peaks1 = _peakFinder.doit(* _plane[1], 5, false);
    vector<TRGPoint2D *> * peaks[2] = {& peaks0, & peaks1};

    //...Loop over charge + and -...
    for (unsigned c = 0; c < 2; c++) {

         //...Loop over all peaks...
         for (unsigned i = 0; i < peaks[c]->size(); i++) {

             //...Circle center...
             const TRGPoint2D center =
                 _circleH.circleCenter(* (* peaks[c])[i]);
             const double ConstantAlpha = 222.376063; // for 1.5T
             const double pt = center.mag() / ConstantAlpha;

             //...Results...
#ifdef TRGCDC_DEBUG
             string charge = "+ charge";
             if (c == 1)
                 charge = "- charge";
             cout << name() << " ... Peak finding 5" << endl;
             cout << name() << "     " << charge
                  << ", track# " << i << endl;
             cout << name() << "     center = " << center
                  << ", pt=" << pt << endl;
#endif
          }
    }

#ifdef TRGCDC_DISPLAY
    string stg = "2D : Peak Finding";
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

    return 0;
}

} // namespace Belle2
