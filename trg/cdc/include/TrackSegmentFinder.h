//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : TrackSegmentFinder.h
// Section  : TRG CDC
// Owner    : Jaebak Kim
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to find Track Segments
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCTrackSegmentFinder_FLAG_
#define TRGCDCTrackSegmentFinder_FLAG_

#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>

#ifdef TRGCDC_SHORT_NAMES
#define TSFinder TRGCDCTrackSegmentFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCSegment;
  class TRGCDCSegmentHit;

  class TRGCDCTrackSegmentFinder {
    public:

    // Constructor.
    TRGCDCTrackSegmentFinder(const TRGCDC& , bool makeRootFile, bool logicLUTFlag);
    // Destructor.
    ~TRGCDCTrackSegmentFinder();

    // Member functions.
    void doit(std::vector<TRGCDCSegment* >& tss, const bool trackSegmentClockSimulation,
              std::vector<TRGCDCSegmentHit* >& segmentHits, std::vector<TRGCDCSegmentHit* >* segmentHitsSL);
    void terminate(void);
    void saveTSInformation(std::vector<TRGCDCSegment* >& tss);
    void saveTSFResults(std::vector<TRGCDCSegmentHit* >* segmentHitsSL);
    void saveNNTSInformation(std::vector<TRGCDCSegment* >& tss);

    // Members.
    const TRGCDC& _cdc;
    double m_Trg_PI;
    // 0 is Logic. 1 is LUT.
    bool m_logicLUTFlag;

    // ROOT variables.
    std::string m_rootTSFFilename;
    TFile* m_fileTSF;

    TTree* m_treeInputTSF;
    //// [TODO] Stores hitpattern information. [superlayer id, hitpattern, mc l/r, validation mc l/r, mc fine phi]
    //// validation mc l/r => 0: not valid, 1: priority, 2: secondary right, 3: secondary left
    // Stores hitpattern information. 9 components
    // [mc particle id, superlayer id, hitpattern, priorityLR, priorityPhi, secondPriorityRLR, secondPriorityRPhi, secondPriorityLLR, secondPriorityLPhi]
    TClonesArray* m_hitPatternInformation;

    TTree* m_treeOutputTSF;
    // [Efficiency, Pt, # MC TS]
    // Efficiency = -1 means that # MC TS is 0.
    TClonesArray* m_particleEfficiency;
    // [SuperLayer Id, Wire Id, Priority Timing]
    TClonesArray* m_tsInformation;

    TTree* m_treeNNTSF;
    // [superlayer id, lrDriftTime, timeWire0, timeWire1, ..., ...]
    TClonesArray* m_nnPatternInformation;

    bool m_makeRootFile;

  };

} // namespace Belle2

#endif
