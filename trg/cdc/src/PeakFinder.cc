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
#include "trg/cdc/HoughPlaneMulti2.h"
#include "trg/cdc/HoughTransformationCircle.h"

using namespace std;

namespace Belle2 {

string
TRGCDCPeakFinder::version(void) const {
    return string("TRGCDCPeakFinder 5.04");
}

TRGCDCPeakFinder::TRGCDCPeakFinder(const string & name)
    : _name(name) {
}

TRGCDCPeakFinder::~TRGCDCPeakFinder() {
}

void
TRGCDCPeakFinder::doit(vector<TCCircle *> & circles,
                       TCHPlane & hp,
                       const unsigned threshold,
                       const bool centerIsPeak) const {
    return peaks5(circles, hp, threshold, centerIsPeak);
}
    
void
TRGCDCPeakFinder::peaks5(vector<TCCircle *> & circles,
                         TCHPlane & hp,
                         const unsigned threshold,
                         const bool centerIsPeak) const {

    TRGDebug::enterStage("Peak Finding (peaks5)");
    if (TRGDebug::level())
        cout << TRGDebug::tab() << "threshold=" << threshold
             << ",plane name=[" << hp.name() << "]" << endl;

    const unsigned nCircles = circles.size();

    //...Make connected regions (is this the best way???)...
    regions(hp, threshold);

    //...Determine peaks...
    const vector<vector<unsigned> *> & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

        TRGDebug::enterStage("Peak position determination");
        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "region " << i << " contents" << endl;

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

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab(4) << "x=" << x << ",y=" << y << endl;
        }
        const unsigned cX = minX + (maxX - minX) / 2;
        const unsigned cY = minY + (maxY - minY) / 2;

        //...Determine a center of a region...
        unsigned ncX = hp.nX() * hp.nY();
        unsigned ncY = ncX;
        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "center of region:x=" << cX << ",y="
                 << cY << endl;
        if (! centerIsPeak) {

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab() << "Searching a cell closest to the "
                     << "region center" << endl;
            
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
                }
                if (TRGDebug::level() > 1)
                    cout << TRGDebug::tab(4) << "x=" << ncX << ",y=" << ncY
                         << ":diff2=" << diff2 << endl;
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
        circles.push_back(new TCCircle(center.y(),
                                       center.x(),
                                       hp.charge(),
                                       hp));

        if (TRGDebug::level()) {
            cout << TRGDebug::tab() << "region " << i << " final center:x="
                 << ncX << ",y=" << ncY << endl
                 << TRGDebug::tab(4) << "position in HP:x="
                 << hp.position(ncX, ncY).x() << ",y="
                 << hp.position(ncX, ncY).y() << endl;
            cout << TRGDebug::tab() << "A circle made" << endl;
            circles.back()->dump("", TRGDebug::tab(4));
        }

        TRGDebug::leaveStage("Peak position determination");
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << circles.size() - nCircles << " circle(s)"
             << " made in total" << endl;

    TRGDebug::leaveStage("Peak Finding (peaks5)");
    return;
}

void
TRGCDCPeakFinder::regions(TCHPlane & hp, const unsigned threshold) const {

    TRGDebug::enterStage("Making regions");

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
            candidates[nActive] = hp.serialId(i, j);
            ++nActive;
        }
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << "Active cells=" << nActive << endl;

    //...Loop over active cells...
    const unsigned used = nCells;
    for (unsigned i = 0; i < nActive; i++) {
        if (candidates[i] == used) continue;
        const unsigned id0 = candidates[i];
        candidates[i] = used;

        //...Make a new region...
        vector<unsigned> * region = new vector<unsigned>;
        region->push_back(id0);
        if (TRGDebug::level() > 2)
            cout << TRGDebug::tab(4) << "new region made" << endl;

        //...Search neighbors...
        for (unsigned j = 0; j < nActive; j++) {
            if (candidates[j] == used) continue;
            const unsigned id1 = candidates[j];

            unsigned x1 = 0;
            unsigned y1 = 0;
            hp.id(id1, x1, y1);

            if (TRGDebug::level() > 2)
                cout << TRGDebug::tab(8) << "cell:x=" << x1 << ",y=" << y1
                     << endl;

            for (unsigned k = 0; k < unsigned(region->size()); k++) {
                unsigned id2 = (* region)[k];
                unsigned x2 = 0;
                unsigned y2 = 0;
                hp.id(id2, x2, y2);
                int difx = abs(int(x1) - int(x2));
                int dify = abs(int(y1) - int(y2));
                if (difx > (int) hp.nX() / 2) difx = hp.nX() - difx;
                if (dify > (int) hp.nY() / 2) dify = hp.nY() - dify;

                if (TRGDebug::level() > 2) {
                    cout << TRGDebug::tab(12) << "x=" << x2 << ",y=" << y2
                         << ":difx=" << difx << ",dify=" << dify;
                    if ((difx < 2) && (dify < 2))
                        cout << " ... connected" << endl;
                    else
                        cout << endl;
                }

                if ((difx < 2) && (dify < 2)) {
                    region->push_back(id1);
                    candidates[j] = used;
                    break;
                }

            }
        }
        hp.setRegion(region);
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << "Regions=" << hp.regions().size() << endl;

    TRGDebug::leaveStage("Making regions");
}

void
TRGCDCPeakFinder::doit(TCHPlaneMulti2 & hp,
                       const unsigned threshold,
                       const bool centerIsPeak,
		       vector<unsigned> & peakSerialIds) const {
    return peaks6(hp, threshold, centerIsPeak, peakSerialIds);
}
    
void
TRGCDCPeakFinder::peaks6(TCHPlaneMulti2 & hp,
                         const unsigned threshold,
                         const bool centerIsPeak,
			 vector<unsigned> & peakSerialIds) const {

    TRGDebug::enterStage("Peak Finding (peaks6)");
    if (TRGDebug::level())
        cout << TRGDebug::tab() << "threshold=" << threshold
             << ",plane name=[" << hp.name() << "]" << endl;

    //...Make connected regions (is this the best way???)...
    regions(hp, threshold);

    //...Determine peaks...
    const vector<vector<unsigned> *> & regions = hp.regions();
    for (unsigned i = 0; i < (unsigned) regions.size(); i++) {

        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "region " << i << " contents" << endl;

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

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab(4) << "x=" << x << ",y=" << y << endl;
        }
        const unsigned cX = minX + (maxX - minX) / 2;
        const unsigned cY = minY + (maxY - minY) / 2;

        //...Determine a center of a region...
        unsigned ncX = hp.nX() * hp.nY();
        unsigned ncY = ncX;
        if (TRGDebug::level() > 1)
            cout << TRGDebug::tab() << "center of region:x=" << cX << ",y="
                 << cY << endl;
        if (! centerIsPeak) {

            if (TRGDebug::level() > 1)
                cout << TRGDebug::tab() << "Searching a cell closest to the "
                     << "region center" << endl;
            
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
                }
                if (TRGDebug::level() > 1)
                    cout << TRGDebug::tab(4) << "x=" << ncX << ",y=" << ncY
                         << ":diff2=" << diff2 << endl;
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

	//...Store the center cell...
	const unsigned serialId = hp.serialId(ncX, ncY);
	peakSerialIds.push_back(serialId);

        if (TRGDebug::level()) {
            cout << TRGDebug::tab() << "region " << i << " final center:x="
                 << ncX << ",y=" << ncY << endl
                 << TRGDebug::tab(4) << "position in HP:x="
                 << hp.position(ncX, ncY).x() << ",y="
                 << hp.position(ncX, ncY).y() << endl;
        }
    }

    if (TRGDebug::level())
        cout << TRGDebug::tab() << peakSerialIds.size() << " peak(s)"
             << " found in total" << endl;

    TRGDebug::leaveStage("Peak Finding (peaks6)");
    return;
}

} // namespace Belle2
