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
#include "trg/trg/Board.h"
#include "trg/trg/SignalVector.h"
#include "trg/trg/SignalBundle.h"

#ifdef TRGCDC_SHORT_NAMES
#define TSFinder TRGCDCTrackSegmentFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCSegment;
  class TRGCDCSegmentHit;
  class TRGCDCMerger;


  class TRGCDCTrackSegmentFinder 
      : public TRGBoard,
	public std::vector <const TRGCDCMerger *>{

    public:
	enum boardType{
	  innerType = 0,
	  outerType = 1,
	  unknown   = 999};

    // Constructor.
    TRGCDCTrackSegmentFinder(const TRGCDC& , bool makeRootFile, bool logicLUTFlag);

    TRGCDCTrackSegmentFinder(const  TRGCDC&,
			     const std::string & name,
		 	     boardType type,
			     const TRGClock & systemClock,
			     const TRGClock & dataClock,
			     const TRGClock & userClockInput,
			     const TRGClock & userClockOutput,
			     std::vector<TCSegment *> & tsSL);
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

    public:

    /// return version
    static std::string version(void);

//    /// Make bit pattern for Inner-most Tracker
//    static TRGState packerInnerTracker(const TRGState & input);
//
//    /// Make bit pattern for Outer Tracker
//    static TRGState packerOuterTracker(const TRGState & input);
//    
//    /// Make bit pattern for Inner-most EvtTime & Low Pt Tracker
//    static TRGState packerInnerEvt(const TRGState & input);
//
//    /// Make bit pattern for Outer EvtTime & Low Pt Tracker
//    static TRGState packerOuterEvt(const TRGState & input);

    /// Use LUT for find TSHit
    //void findTSHit(void);
    vector<TRGSignalVector*> findTSHit(TRGSignalVector * eachInput, int);
  
    /// Packing output for tracker
    TRGSignalVector* packerOuterTracker(vector<TRGSignalVector*>, vector<int>, int);

    /// Packing output for evtTime & Low pT
    TRGSignalVector* packerOuterEvt(vector<TRGSignalVector*>, vector<int>, int);
    ///
    boardType type(void) const;

    TRGSignalBundle * outputE(void) {return _tosbE;};
    TRGSignalBundle * outputT(void) {return _tosbT;};

    void push_back(const TRGCDCMerger *);

    /// firmware simulation.
    void simulateBoard(void);
    //void simulateBoard(std::vector <TRGCDCSegment * > & tsSL);

    double mkint(TRGState );

    vector<bool> mkbool(int, int);
    private:


    /// Unit type.
    boardType _type;

    /// Input signal bundle.
    //TRGSignalBundle * _tisb;

    /// Output signal bundle.
    TRGSignalBundle * _tosbE;
    TRGSignalBundle * _tosbT;
    std::vector<TCSegment *> _tsSL;

  };

} // namespace Belle2

#endif
