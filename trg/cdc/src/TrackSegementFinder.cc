//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegmentFinder.cc
// Section  : TRG CDC
// Owner    : Jaebak Kim
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to find Track Segments
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include "trg/trg/Debug.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/TrackSegmentFinder.h"

using namespace std;

namespace Belle2 {


  TRGCDCTrackSegmentFinder::TRGCDCTrackSegmentFinder(const TRGCDC & TRGCDC, bool makeRootFile)
    : _cdc(TRGCDC), m_makeRootFile(makeRootFile) {

      // For ROOT file
      if(m_makeRootFile) m_fileTSF = new TFile("TSF.root","RECREATE");
      m_treeTSF = new TTree("m_treeTSF", "TSF");
      m_treeNNTSF = new TTree("m_treeNNTSF", "NNTSF");

  }

  TRGCDCTrackSegmentFinder::~TRGCDCTrackSegmentFinder() {

    delete m_treeNNTSF;
    delete m_treeTSF;
    if(m_makeRootFile) delete m_fileTSF;

  }

  void TRGCDCTrackSegmentFinder::terminate(void) {
    if(m_makeRootFile) {
      m_fileTSF->Write();
      m_fileTSF->Close();
    }
  }

  void TRGCDCTrackSegmentFinder::doit(std::vector<TRGCDCSegment* >& tss, const bool trackSegmentClockSimulation,
            std::vector<TRGCDCSegmentHit* >& segmentHits, std::vector<TRGCDCSegmentHit* >* segmentHitsSL) {
    TRGDebug::enterStage("Track Segment Finder");

    //...Store TS hits...
    const unsigned n = tss.size();
    for (unsigned i = 0; i < n; i++) {
      TCSegment & s = * tss[i];
      s.simulate(trackSegmentClockSimulation);
      if (s.signal().active()) {
        TCSHit * th = new TCSHit(s);
        s.hit(th);
        segmentHits.push_back(th);
        segmentHitsSL[s.layerId()].push_back(th);
      }
    }

    if (TRGDebug::level() > 1) {
      cout << TRGDebug::tab() << "TS hit list" << endl;
      string dumpOption = "trigger";
      if (TRGDebug::level() > 2)
        dumpOption = "detail";
      for (unsigned i = 0; i < _cdc.nSegments(); i++) {
        const TCSegment & s = _cdc.segment(i);
        if (s.signal().active())
          s.dump(dumpOption, TRGDebug::tab(4));
      }

      cout << TRGDebug::tab() << "TS hit list (2)" << endl;
      if (TRGDebug::level() > 2)
        dumpOption = "detail";
      for (unsigned i = 0; i < segmentHits.size(); i++) {
        const TCSHit & s = * segmentHits[i];
        s.dump(dumpOption, TRGDebug::tab(4));
      }

      cout << TRGDebug::tab() << "TS hit list (3)" << endl;
      if (TRGDebug::level() > 2)
        dumpOption = "detail";
      for (unsigned j = 0; j < _cdc.nSuperLayers(); j++) {
        for (unsigned i = 0; i < segmentHitsSL[j].size(); i++) {
          const vector<TCSHit*> & s = segmentHitsSL[j];
          for (unsigned k = 0; k < s.size(); k++)
            s[k]->dump(dumpOption, TRGDebug::tab(4));
        }
      }
    }

    TRGDebug::leaveStage("Track Segment Finder");
  }

} // namespace Belle2
