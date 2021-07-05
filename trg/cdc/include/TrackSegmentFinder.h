/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
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
#define TCTSFinder TRGCDCTrackSegmentFinder
#define TSFinder TRGCDCTrackSegmentFinder
#endif

namespace Belle2 {

  class TRGCDC;
  class TRGCDCSegment;
  class TRGCDCSegmentHit;
  class TRGCDCMerger;

  /// a class of TrackSegmentFinder in TRGCDC
  class TRGCDCTrackSegmentFinder
    : public TRGBoard,
      public std::vector <const TRGCDCMerger*> {

  public:
    /// enum of boardType of TrackSegmentFinder
    enum boardType {
      innerType = 0,
      outerType = 1,
      unknown   = 999
    };

    /// Constructor.
    TRGCDCTrackSegmentFinder(const TRGCDC&,
                             bool makeRootFile,
                             bool logicLUTFlag);

    /// Constructor.
    TRGCDCTrackSegmentFinder(const  TRGCDC&,
                             const std::string& name,
                             boardType type,
                             const TRGClock& systemClock,
                             const TRGClock& dataClock,
                             const TRGClock& userClockInput,
                             const TRGClock& userClockOutput,
                             const std::vector<TRGCDCSegment*>& tsSL);

    /// Destructor.
    ~TRGCDCTrackSegmentFinder();


    /// Member functions of doing TSF.
    void doit(std::vector<TRGCDCSegment* >& tss, const bool trackSegmentClockSimulation,
              std::vector<TRGCDCSegmentHit* >& segmentHits, std::vector<TRGCDCSegmentHit* >* segmentHitsSL);
    /// terminate
    void terminate(void);
    /// save the TS info
    void saveTSInformation(std::vector<TRGCDCSegment* >& tss);
    /// save result of TSF
    void saveTSFResults(std::vector<TRGCDCSegmentHit* >* segmentHitsSL);
    /// Saves NNTS information. Only when ts is hit
    void saveNNTSInformation(std::vector<TRGCDCSegment* >& tss);


    /// Members.
    const TRGCDC& _cdc;
    /// PI
    double m_Trg_PI;
    /// 0 is Logic. 1 is LUT.
    bool m_logicLUTFlag;

    /// ROOT file name string.
    std::string m_rootTSFFilename;
    /// ROOT file
    TFile* m_fileTSF;

    /// ROOT TTree for input
    TTree* m_treeInputTSF = nullptr;
    //// [TODO] Stores hitpattern information. [superlayer id, hitpattern, mc l/r, validation mc l/r, mc fine phi]
    //// validation mc l/r => 0: not valid, 1: priority, 2: secondary right, 3: secondary left
    // Stores hitpattern information. 9 components
    // [mc particle id, superlayer id, hitpattern, priorityLR, priorityPhi, secondPriorityRLR, secondPriorityRPhi, secondPriorityLLR, secondPriorityLPhi]
    /// Stores hitpattern information
    TClonesArray* m_hitPatternInformation = nullptr;

    /// ROOT TTree for output
    TTree* m_treeOutputTSF = nullptr;
    /// [Efficiency, Pt, # MC TS]
    /// Efficiency = -1 means that # MC TS is 0.
    TClonesArray* m_particleEfficiency = nullptr;
    /// [SuperLayer Id, Wire Id, Priority Timing]
    TClonesArray* m_tsInformation = nullptr;

    /// ROOT Tree for NNTSF
    TTree* m_treeNNTSF = nullptr;
    /// [superlayer id, lrDriftTime, timeWire0, timeWire1, ..., ...]
    TClonesArray* m_nnPatternInformation = nullptr;

    /// make ROOT file or not
    bool m_makeRootFile = false;

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
    vector<TRGSignalVector*> findTSHit(TRGSignalVector* eachInput, int);

    /// Packing output for tracker
    TRGSignalVector* packerOuterTracker(vector<TRGSignalVector*>&,
                                        vector<int>&,
                                        const unsigned);

    /// Packing output for evtTime & Low pT
    TRGSignalVector* packerOuterEvt(vector<TRGSignalVector*>, vector<int>, int);
    /// board type of TSF
    boardType type(void) const;

    /// signal bundle of outputE
    TRGSignalBundle* outputE(void) {return _tosbE;};
    /// signal bundle of outputT
    TRGSignalBundle* outputT(void) {return _tosbT;};

    /// push back the Mergers of this TSF
    void push_back(const TRGCDCMerger*);

    /// firmware simulation.
    void simulateBoard(void);

    /// Firmware simulation. yi
    void simulate(void);

    /// Firmware simulation. Unified version of inner and outer : yi
    void simulate2(void);

    /// Creates the hit maps for the inner.
    void hitMapInner(void);

    /// Creates the hit maps for the outer.
    void hitMapOuter(void);

    /// Creates input signals to TSF for the inner.
    void inputInner(const unsigned id,
                    const unsigned nTSF,
                    TRGSignalVector* s);

    /// Creates input signals to TSF for the outer.
    void inputOuter(const unsigned id,
                    const unsigned nTSF,
                    TRGSignalVector* s);

    /// Simulate TSF response for the inner
    vector<TRGSignalVector*> simulateInner(const TRGSignalVector& in,
                                           unsigned id);

    /// Simulate TSF response for the inner
    vector<TRGSignalVector*> simulateOuter(TRGSignalVector* in,
                                           unsigned id);

    /// Simulate TSF response (unified version)
    vector<TRGSignalVector*> simulateTSFOld(TRGSignalVector* in,
                                            unsigned id);

    /// Simulate TSF response (unified version, 2016/07/12)
    vector<TRGSignalVector*> simulateTSF(TRGSignalVector* in,
                                         unsigned id);

    /// Simulate TSF response (unified version, state machine)
    vector<TRGSignalVector*> simulateTSF2(TRGSignalVector* in,
                                          unsigned id);

    /// Output packer for tracker
    TRGSignalVector* packerForTracker(vector<TRGSignalVector*>&,
                                      vector<int>&,
                                      const unsigned);

    /// Output packer for ETF.
    TRGSignalVector* packerForETF(vector<TRGSignalVector*>&,
                                  vector<int>&,
                                  const unsigned);
    /// tranformatoin into integer
    double mkint(TRGState);

    /// tranformatoin into bool
    vector<bool> mkbool(int, int);

  private:

    /// Firmware simulation for the outers. yi
    void simulateInner(void);

    /// Firmware simulation for the outers. yi
    void simulateOuter(void);

    /// Make TRGSignals for the priority timing bits.
    void priorityTiming(unsigned tsfID,
                        const unsigned nTSF,
                        TRGSignalVector& s,
                        const TRGSignal& center,
                        const TRGSignal& right,
                        const TRGSignal& left) const;

    /// Make TRGSignals for the fastest timing bits.
    void fastestTimingInner(unsigned tsfID,
                            const unsigned nTSF,
                            TRGSignalVector& s) const;

    /// Make TRGSignals for the fastest timing bits.
    void fastestTimingOuter(unsigned tsfID,
                            const unsigned nTSF,
                            TRGSignalVector& s) const;

    /// Add TSF ID to timing signal vector for the output.
    static void addID(TRGSignalVector& s, unsigned id);

  private:

    /// Unit type.
    boardType _type;

    /// Input signal bundle.
    //TRGSignalBundle * _tisb;

    /// Output signal bundle.
    TRGSignalBundle* _tosbE;
    /// Output signal bundle.
    TRGSignalBundle* _tosbT;
    /// list of TSF
    std::vector<TCSegment*> _tsSL;

    /// TSF input storage
    std::vector<TRGSignalVector*> _tsfIn;

    /// TSF response storeage
    std::vector<TRGSignalVector*> _tsfOut;

    /// One time info. to be deleted in next event;
    std::vector<TRGSignalVector*> _toBeDeleted;

    /// Internal data of wire hit map.
    std::vector<TRGSignal* > _hitMap[5];

    /// Internal data of the priority hit timing.
    std::vector<TRGSignal* > _priMap;

    /// Internal data of the fastest hit timing.
    std::vector<TRGSignal* > _fasMap;

    /// Internal data of the priority cell hit position flag.
    std::vector<TRGSignal* > _secMap;

    /// Internal data of the edge timing information.
    std::vector<TRGSignal* > _edg0Map;

    /// Internal data of the edge timing information.
    std::vector<TRGSignal* > _edg1Map;

    /// Internal data of the edge timing information.
    std::vector<TRGSignal* > _edg2Map;

    /// Internal data of the edge timing information.
    std::vector<TRGSignal* > _edg3Map;

    /// Internal data of the edge timing information.
    std::vector<TRGSignal* > _edg4Map;
  };

} // namespace Belle2

#endif
