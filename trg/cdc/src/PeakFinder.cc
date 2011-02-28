//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : PeakFinder.cc
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

#include <cstdlib>
#include "trg/trg/Debug.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/PeakFinder.h"
#include "trg/cdc/HoughPlane.h"
#include "trg/cdc/HoughTransformationCircle.h"

using namespace std;

namespace Belle2 {

string
TRGCDCPeakFinder::version(void) const {
    return string("0.01");
}

TRGCDCPeakFinder::TRGCDCPeakFinder(const string & name)
    : _name(name) {
}

TRGCDCPeakFinder::~TRGCDCPeakFinder() {
}

void
TRGCDCPeakFinder::doit(vector<TRGCDCCircle *> & circles,
                       TRGCDCHoughPlane & hp,
                       const unsigned threshold,
                       const bool centerIsPeak) const {
    return peaks5(circles, hp, threshold, centerIsPeak);
}
    
void
TRGCDCPeakFinder::peaks5(vector<TRGCDCCircle *> & circles,
                         TRGCDCHoughPlane & hp,
                         const unsigned threshold,
                         const bool centerIsPeak) const {
#ifdef TRG_DEBUG
    TRGDebug::enterStage("Peak Finding (peaks5)");
#endif
#ifdef TRGCDC_DEBUG_HOUGH
    const unsigned nCircles = circles.size();
#endif

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
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

        //...Calculate size and center of a region...
        const vector<unsigned> & r = * regions[i];
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
//          std::cout << Tab() << "region " << i << ":x=" << x << ",y=" << y
//                    << std::endl;
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

        //...Store a circle...
        const TCHTransformationCircle * tc =
            dynamic_cast<const TCHTransformationCircle *>
            (& hp.transformation());
        TRGPoint2D center =
            tc->circleCenter(hp.position(ncX, ncY));
        TRGPoint2D centerR(center.y(), center.x());
        circles.push_back(new TCCircle(centerR, hp.charge(), hp));

#ifdef TRGCDC_DEBUG_HOUGH
        cout << TRGDebug::tab() << "A circle made" << endl;
        circles.back()->dump("", TRGDebug::tab() + "    ");
//         cout << TRGDebug::tab() << "region " << i << " final center:x="
//              << hp.position(ncX, ncY).x() << ",y="
//              << hp.position(ncX, ncY).y() << endl;
#endif            
    }

#ifdef TRGCDC_DEBUG_HOUGH
//     cout << Tab() << "Peak finding:threshold=" << threshold << ",nActive="
//             << nActive << ",regions=" << hp.regions().length()
//             << "," << hp.name() << endl;
//     for (unsigned i = 0; i < (unsigned) hp.regions().length(); i++) {
//          const CList<unsigned> & region = * (hp.regions())[i];
//          for (unsigned j = 0; j < (unsigned) region.length(); j++)
//              cout << Tab() << "    " << * region[j] << "="
//                     << hp.entry(* region[j]) << endl;
//     }
#endif

#ifdef TRGCDC_DEBUG_HOUGH
    cout << TRGDebug::tab() << circles.size() - nCircles << " circle(s)"
         << " made in total" << endl;
#endif
#ifdef TRG_DEBUG
    TRGDebug::leaveStage("Peak Finding (peaks5)");
#endif

    return;
}

} // namespace Belle2
