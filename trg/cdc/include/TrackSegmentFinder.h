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
    TRGCDCTrackSegmentFinder(const TRGCDC& , bool makeRootFile);
    // Destructor.
    ~TRGCDCTrackSegmentFinder();

    // Member functions.
    void doit(std::vector<TRGCDCSegment* >& tss, const bool trackSegmentClockSimulation,
              std::vector<TRGCDCSegmentHit* >& segmentHits, std::vector<TRGCDCSegmentHit* >* segmentHitsSL);
    void terminate(void);

    // Members.
    const TRGCDC& _cdc;

    // ROOT variables.
    std::string m_rootTSFFilename;
    TFile* m_fileTSF;
    TTree* m_treeTSF;
    TTree* m_treeNNTSF;

    bool m_makeRootFile;

  };

} // namespace Belle2

#endif
