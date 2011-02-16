//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : HoughFinder.h
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

#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/TrackSegment.h"

#ifdef TRGCDC_DISPLAY
#include "trg/cdc/DisplayRphi.h"
#include "trg/cdc/DisplayHough.h"
namespace Belle2_TRGCDC {
    Belle2::TRGCDCDisplayHough * H0 = 0;
    Belle2::TRGCDCDisplayHough * H1 = 0;
}
#endif

using namespace std;
#ifdef TRGCDC_DISPLAY
using namespace Belle2_TRGCDC;
#endif

namespace Belle2 {

TRGCDCHoughFinder::TRGCDCHoughFinder(const string & name,
                                     const TRGCDC & TRGCDC,
                                     unsigned nX,
                                     unsigned nY)
    : _name(name),
      _cdc(TRGCDC),
      _circleH("CircleHough") {

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
    vector<TRGPoint2D *> peaks0 = peaks5(* _plane[0], 5, false);
    vector<TRGPoint2D *> peaks1 = peaks5(* _plane[1], 5, false);
    vector<TRGPoint2D *> * peaks[2] = {& peaks0, & peaks1};


    //...Loop over charge + and -...
    for (unsigned c = 0; c < 2; c++) {

         //...Loop over all peaks...
         for (unsigned i = 0; peaks[c]->size(); i++) {

            //...Circle center...
            const TRGPoint2D center = _circleH.circleCenter(* (* peaks[c])[i]);
            const double ConstantAlpha = 222.376063; // for 1.5T
            const double pt = center.mag() / ConstantAlpha;
            
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

vector<TRGPoint2D *>
TRGCDCHoughFinder::peaks5(TRGCDCHoughPlane & hp,
                          const unsigned threshold,
                          bool centerIsPeak) const {
    vector<TRGPoint2D *> list;

    //...Search cells above threshold...
    unsigned nCells = hp.nX() * hp.nY();
    static unsigned * candidates =
        (unsigned *) malloc(nCells * sizeof(unsigned));
    unsigned nActive = 0;
    for (unsigned j = 0; j < hp.nY(); j++) {
        for (unsigned i = 0; i < hp.nX(); i++) {

            //...Threshold check...        
            const unsigned n = hp.entry(i, j);
            if (n < threshold) continue;
            candidates[nActive] = hp.serialID(i, j);
            ++nActive;
        }
    }

    //...Make connected regions (is this the best way???)...
    const unsigned used = nCells;
    for (unsigned i = 0; i < nActive; i++) {
        if (candidates[i] == used) continue;
        const unsigned id0 = candidates[i];
        candidates[i] = used;

        //...Make a new region...
        vector<unsigned> * region = new vector<unsigned>;
        region->push_back(id0);

        //...Search neighbors...
        for (unsigned j = 0; j < nActive; j++) {
            if (candidates[j] == used) continue;
            const unsigned id1 = candidates[j];

            unsigned x1 = 0;
            unsigned y1 = 0;
            hp.id(id1, x1, y1);

#ifdef TRGCDC_DEBUG_HOUGH
//              std::cout << Tab() << "    region:x=" << x1 << ",y=" << y1
//                        << std::endl;
#endif            
            for (unsigned k = 0; k < unsigned(region->size()); k++) {
                unsigned id2 = (* region)[k];
                unsigned x2 = 0;
                unsigned y2 = 0;
                hp.id(id2, x2, y2);
                int difx = abs(int(x1) - int(x2));
                int dify = abs(int(y1) - int(y2));
                if (difx > (int) hp.nX() / 2) difx = hp.nX() - difx;
                if (dify > (int) hp.nY() / 2) dify = hp.nY() - dify;
#ifdef TRGCDC_DEBUG_HOUGH
//                 std::cout << Tab() << "        :x=" << x2 << ",y=" << y2
//                        << ":difx=" << difx << ",dify=" << dify;
//                 if ((difx < 2) && (dify < 2))
//                     std::cout << " ... connected" << std::endl;
//                 else
//                     std::cout << std::endl;
#endif            
                if ((difx < 2) && (dify < 2)) {
                    region->push_back(id1);
                    candidates[j] = used;
                    break;
                }

            }
        }
        hp.setRegion(region);
    }

    //...Determine peaks...
    const vector<vector<unsigned> *> & regions = hp.regions();
//  const AList<CList<unsigned> > & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

        //...Calculate size and center of a region...
        const vector<unsigned> & r = * regions[i];
//        const CList<unsigned> & r = * regions[i];
        unsigned minX = hp.nX();
        unsigned maxX = 0;
        unsigned minY = hp.nY();
        unsigned maxY = 0;
        for (unsigned j = 0; j < (unsigned) r.size(); j++) {
            const unsigned s = r[j];
            unsigned x = 0;
            unsigned y = 0;
            hp.id(s, x, y);
            if (x < minX) minX = x;
            if (x > maxX) maxX = x;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

#ifdef TRGCDC_DEBUG_HOUGH
//              std::cout << Tab() << "region " << i << ":x=" << x << ",y=" << y
//                        << std::endl;
#endif            
        }
        const unsigned cX = minX + (maxX - minX) / 2;
        const unsigned cY = minY + (maxY - minY) / 2;

        //...Determine a center of a region...
        unsigned ncX = hp.nX() * hp.nY();
        unsigned ncY = ncX;
        if (! centerIsPeak) {

            //...Search for a cell which is the closest to the center...
            float minDiff2 = float(hp.nX() * hp.nX() + hp.nY() * hp.nY());
            for (unsigned j = 0; j < (unsigned) r.size(); j++) {
                const unsigned s = r[j];
                unsigned x = 0;
                unsigned y = 0;
                hp.id(s, x, y);
                
                const float diff2 = (float(x) - float(cX)) *
                    (float(x) - float(cX))
                    + (float(y) - float(cY)) *
                    (float(y) - float(cY));

                if (diff2 < minDiff2) {
                    minDiff2 = diff2;
                    ncX = x;
                    ncY = y;
#ifdef TRGCDC_DEBUG_HOUGH
//                     std::cout << Tab() << "region " << i << " center:x="
//                            << ncX << ",y=" << ncY << "(" << j << ")"
//                            << std::endl;
#endif            
                }
            }
        }
        else {

            //...Search for a peak...
            float max = 0;
            for (unsigned j = 0; j < (unsigned) r.size(); j++) {
                const unsigned s = r[j];
                const float entry = hp.entry(s);
                if (max < entry) {
                    max = entry;
                    unsigned x = 0;
                    unsigned y = 0;
                    hp.id(s, x, y);
                    ncX = x;
                    ncY = y;
                }
            }
        }

        //...Store the center position....
        list.push_back(new TRGPoint2D(hp.position(ncX, ncY)));
#ifdef TRGCDC_DEBUG_HOUGH
//         std::cout << Tab() << "region " << i << " final center:x="
//                << hp.position(ncX, ncY).x() << ",y="
//                << hp.position(ncX, ncY).y() << std::endl;
#endif            
    }

#ifdef TRGCDC_DEBUG_HOUGH
//     std::cout << Tab() << "Peak finding:threshold=" << threshold << ",nActive="
//             << nActive << ",regions=" << hp.regions().length()
//             << "," << hp.name() << std::endl;
//     for (unsigned i = 0; i < (unsigned) hp.regions().length(); i++) {
//          const CList<unsigned> & region = * (hp.regions())[i];
//          for (unsigned j = 0; j < (unsigned) region.length(); j++)
//              std::cout << Tab() << "    " << * region[j] << "="
//                     << hp.entry(* region[j]) << std::endl;
//     }
#endif

    return list;
}

} // namespace Belle2
