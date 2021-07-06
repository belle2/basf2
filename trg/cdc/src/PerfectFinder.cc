/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//-----------------------------------------------------------------------------
// Description : A class to find 2D tracks usning MC information
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <map>
#include "cdc/dataobjects/CDCSimHit.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/PerfectFinder.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"

#ifdef TRGCDC_DISPLAY
#include "trg/cdc/DisplayRphi.h"
namespace Belle2_TRGCDC {
  extern Belle2::TRGCDCDisplayRphi* D;
}
using namespace Belle2_TRGCDC;
#endif

using namespace std;

namespace Belle2 {

  string
  TRGCDCPerfectFinder::version(void) const
  {
    return string("TRGCDCPerfectFinder 5.24");
  }

  TRGCDCPerfectFinder::TRGCDCPerfectFinder(const string& name,
                                           const TRGCDC& TRGCDC)
    : _name(name), _cdc(TRGCDC)
  {
  }

  TRGCDCPerfectFinder::~TRGCDCPerfectFinder()
  {
  }

  int
  TRGCDCPerfectFinder::doit(vector<TCTrack*>& trackListClone, vector<TCTrack*>& trackList)
  {
    int result = doitPerfectly(trackList);
    trackListClone = trackList;
    return result;
  }

  int
  TRGCDCPerfectFinder::doitPerfectly(vector<TRGCDCTrack*>& trackList)
  {

    TRGDebug::enterStage("Perfect Finder");

    //...TS hit loop...
    _mcList.clear();
    map<int, vector<const TCSegment*> *> trackMap;
    const vector<const TCSHit*> hits = _cdc.segmentHits();
    for (unsigned i = 0; i < hits.size(); i++) {
      const TCSHit& ts = * hits[i];
      if (! ts.signal().active()) continue;
      if (ts.segment().stereo()) continue;
      //const TCWHit * wh = ts.segment().center().hit();
      const TCWHit* wh = ts.segment().priority().hit();
      if (! wh) continue;
      const CDCSimHit& sh = * wh->simHit();
      const int trackId = sh.getTrackId();
      if (! trackMap[trackId]) {
        trackMap[trackId] = new vector<const TCSegment*>();
        _mcList.push_back(trackId);
      }
      trackMap[trackId]->push_back(& ts.segment());
    }

    if (TRGDebug::level()) {
      cout << TRGDebug::tab() << "#tracksInMC=" << trackMap.size() << endl;
      map<int, vector<const TCSegment*> *>::iterator it = trackMap.begin();
      while (it != trackMap.end()) {
        cout << TRGDebug::tab(4) << it->first << ":";
        const vector<const TCSegment*>& l = * it->second;
        for (unsigned i = 0; i < l.size(); i++)
          cout << l[i]->name() << ",";
        cout << endl;
        ++it;
      }
    }

    //...Make circles...
    map<int, vector<const TCSegment*> *>::iterator it = trackMap.begin();
    unsigned n = 0;
    while (it != trackMap.end()) {

      //...Make links...
      const vector<const TCSegment*>& l = * it->second;
      vector<TCLink*> links;
      for (unsigned i = 0; i < l.size(); i++) {
        TCLink* link = new TCLink(0,
                                  l[i]->hit(),
                                  l[i]->hit()->cell().xyPosition());
        links.push_back(link);
      }

      //...Requires all axial super layer hits...
      const unsigned nSuperLayers = TCLink::nSuperLayers(links);
      if (nSuperLayers < 5) {
        ++it;
        continue;
      }

      //...Check uniquness...
      vector<TCLink*> layers[9];
      vector<TCLink*> forCircle;
      TCLink::separate(links, 9, layers);
      for (unsigned i = 0; i < 9; i++) {
        if (layers[i].size() < 1) continue;
        if (layers[i].size() < 2) {
          forCircle.push_back(layers[i][0]);
          continue;
        }
        TCLink* best = 0;
        //int timeMin = 99999;
        float timeMin = 99999;
        bool bestCenterHit = 0;
        for (unsigned j = 0; j < layers[i].size(); j++) {
          //const TRGTime & t = * (layers[i][j]->cell()->signal())[0];
          const float tsDrift = layers[i][j]->cell()->hit()->drift();
          const TRGCDCSegment* t_cell = static_cast<const TRGCDCSegment*>(layers[i][j]->cell());
          bool centerHit = (t_cell->priorityPosition() == 3);
          //cout<<"PF2D ["<<layers[i][j]->cell()->superLayerId()<<"-"<<layers[i][j]->cell()->localId()<<"] Tick: "<<t.time()<<" Drift: "<<tsDrift<<endl;
          //if (t.time() < timeMin) {
          //    timeMin = t.time();
          if (centerHit == 1 && bestCenterHit == 0) {
            timeMin = tsDrift;
            best = layers[i][j];
            bestCenterHit = 1;
          } else if (centerHit == 0 && bestCenterHit == 1) {
          } else {
            if (tsDrift < timeMin) {
              timeMin = tsDrift;
              best = layers[i][j];
            }
          }
        }
        forCircle.push_back(best);
      }

      if (TRGDebug::level())
        TCLink::dump(forCircle,
                     "",
                     TRGDebug::tab() + "track_" + TRGUtil::itostring(n));

      //...Make a circle...
      TCCircle c = TCCircle(forCircle);
      c.fit();
      c.name("CircleFitted_" + TRGUtil::itostring(n));

      //...Make a track...
      TCTrack& t = * new TCTrack(c);
      t.name("Track_" + TRGUtil::itostring(n));
      trackList.push_back(& t);

      if (TRGDebug::level()) {
        c.dump("detail");
        t.dump("detail");
      }

      //...Incriment for next loop...
      ++it;
      ++n;

#ifdef TRGCDC_DISPLAY_HOUGH
      vector<const TCCircle*> cc;
      cc.push_back(& c);
      vector<const TCTrack*> tt;
      tt.push_back(& t);
      string stg = "2D : Perfect Finder circle fit";
      string inf = "   ";
      D->clear();
      D->stage(stg);
      D->information(inf);
      D->area().append(cc, Gdk::Color("#FF0066009900"));
//      D->area().append(tt, Gdk::Color("#990066009900"));
      D->area().append(_cdc.hits());
      D->area().append(_cdc.segmentHits());
      D->show();
      D->run();
#endif

    }

    if (TRGDebug::level()) {
      cout << TRGDebug::tab() << "#tracksMade=" << trackList.size() << endl;
    }

    TRGDebug::leaveStage("Perfect Finder");
    return 0;
  }

} // namespace Belle2
