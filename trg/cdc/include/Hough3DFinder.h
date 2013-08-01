//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Hough3DFinder.h
// Section  : TRG CDC
// Owner    : Jaebak Kim
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to find 3D tracks using Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#ifndef TRGCDCHough3DFinder_FLAG_
#define TRGCDCHough3DFinder_FLAG_

#include <string>
#include "trg/cdc/Hough3DUtility.h"

#ifdef TRGCDC_SHORT_NAMES
#define TCH3DFinder TRGCDCHough3DFinder
#endif

namespace Belle2 {

class TRGCDC;
class TRGCDCTrack;
class TRGCDCSegmentHit;
class TRGCDCLink;

class TRGCDCHough3DFinder {
  public:
    // Contructor.
    TRGCDCHough3DFinder(const TRGCDC &, bool makeRootFile, int finderMode);
    ~TRGCDCHough3DFinder();
    // Member functions.
    void doit(std::vector<TRGCDCTrack *> & trackList);
    void doit(std::vector<TRGCDCTrack *> & trackList, int m_eventNum);
    void perfectFinder(std::vector<TRGCDCTrack *> &trackList, unsigned j, std::vector<const TRGCDCSegmentHit*> &mcTSList);
    void doitPerfectly(std::vector<TRGCDCTrack *> & trackList);
    void doitFind(std::vector<TRGCDCTrack *> & trackList);
    void terminate(void);
    // Members.
    const TRGCDC & _cdc;
    int m_eventNum;
    int m_nWires[4];
    double m_rr[4];
    double m_axR[5];
    int m_axNWires[5];
    double m_ztostraw[4];
    double m_anglest[4];
    double m_Trg_PI;
    bool m_makeRootFile;
    // 0: perfect finder, 1: Hough3DFinder, 2: GeoFinder, 3: VHDL GeoFinder
    int m_finderMode;
    // Hough Variables.
    Hough3DFinder* m_Hough3DFinder;
    double m_cotStart;
    double m_cotEnd;
    double m_z0Start;
    double m_z0End;
    int m_nCotSteps;
    int m_nZ0Steps;
    double m_cotStepSize;
    double m_z0StepSize;
    // For root file variables.
    std::string m_rootHough3DFilename;
    TFile* m_fileHough3D;
    TTree* m_treeTrackHough3D;
    TTree* m_treeEventHough3D;
    TTree* m_treeConstantsHough3D;
    // Stores stereo phi values.
    TClonesArray* m_st0TSsTrackHough3D;
    TClonesArray* m_st1TSsTrackHough3D;
    TClonesArray* m_st2TSsTrackHough3D;
    TClonesArray* m_st3TSsTrackHough3D;
    // Stores drift stereo phi values.
    TClonesArray* m_driftSt0TSsTrackHough3D;
    TClonesArray* m_driftSt1TSsTrackHough3D;
    TClonesArray* m_driftSt2TSsTrackHough3D;
    TClonesArray* m_driftSt3TSsTrackHough3D;
    // Stores 2D fit values. [pT, phi0, charge].
    TClonesArray* m_fit2DTrackHough3D;
    // Stores best z0 and cot values. [z0, cot, #votes, minDiff]
    TClonesArray* m_bestFitTrackHough3D;
    // Stores MC true z0 and cot values. [mc z0, mc cot]
    TClonesArray* m_mcTrackHough3D;
    // Stores MC status values. [statusbit, pdg, charge]
    TClonesArray* m_mcStatusTrackHough3D;
    // Stores MC vertex.
    TClonesArray* m_mcVertexTrackHough3D;
    // Stores MC 4-vector
    TClonesArray* m_mc4VectorTrackHough3D;
    // Stores best phi values. [st0, st1, st2, st3]
    TClonesArray* m_bestTSsTrackHough3D;
    // Stores true phi values. [st0, st1, st2, st3]
    TClonesArray* m_mcTSsTrackHough3D;
    TClonesArray* m_mcSt0TSsTrackHough3D;
    TClonesArray* m_mcSt1TSsTrackHough3D;
    TClonesArray* m_mcSt2TSsTrackHough3D;
    TClonesArray* m_mcSt3TSsTrackHough3D;
    // Stores performance values. [purity, Efficiency, Track #]
    TClonesArray* m_performanceTrackHough3D;
    // Stores deltaWireDiff
    TClonesArray* m_deltaWireDiffTrackHough3D;
    // Stores fitz
    TClonesArray* m_fitZTrackHough3D;
    // Stores mc stereo's axial position.
    // 4*2 = 8
    // st1_x, st1_y, st2_x, st2_y, st3_x, st3_y, st4_x, st4_y
    TClonesArray* m_mcStAxXYTrackHough3D;
    // Stores performance values. [Total # tracks from 2D finder]
    TVectorD* m_performanceEventHough3D;
    // Stores geometry values. [rr0, rr1, rr2, rr3, anglest0, anglest1, anglest2, anglest3, ztostraw0, ztostraw1, ztostraw2, ztostraw3]
    TVectorD* m_geometryHough3D;
    TVectorD* m_modeHough3D;
    TVectorD* m_initVariablesHough3D;
    // Stores geoFinder candidate phi
    TClonesArray* m_st0GeoCandidatesPhiTrackHough3D;
    TClonesArray* m_st1GeoCandidatesPhiTrackHough3D;
    TClonesArray* m_st2GeoCandidatesPhiTrackHough3D;
    TClonesArray* m_st3GeoCandidatesPhiTrackHough3D;
    // Stores geoFinder candidate diff wires
    TClonesArray* m_st0GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray* m_st1GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray* m_st2GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray* m_st3GeoCandidatesDiffStWiresTrackHough3D;
    // Stores geoFinder stAx phi [ for each stereo layer ]
    TClonesArray* m_stAxPhiTrackHough3D;
    // Stores event and track ID;
    TClonesArray* m_eventTrackIDTrackHough3D;

};

} // namespace Belle2

#endif
