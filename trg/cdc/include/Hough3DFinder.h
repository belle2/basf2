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
    // Destructor
    ~TRGCDCHough3DFinder();
    // Member functions.
    // Finds tracks using tracklist. Has parameter to choose between perfect and original finder.
    void doit(std::vector<TRGCDCTrack *> & trackList);
    // Finds tracks using tracklist. Saves event number. Has parameter to choose between perfect and original finder.
    void doit(std::vector<TRGCDCTrack *> & trackList, int m_eventNum);
    // Perfect 3D finder for a track.
    void perfectFinder(std::vector<TRGCDCTrack *> &trackList, unsigned j, std::vector<const TRGCDCSegmentHit*> &mcTSList);
    // Perfect 3D finder for a tracklist.
    void doitPerfectly(std::vector<TRGCDCTrack *> & trackList);
    // Finds tracks using tracklist.
    void doitFind(std::vector<TRGCDCTrack *> & trackList);
    // Termination method.
    void terminate(void);
    // Members.
    // Holds the trgcdc singleton.
    const TRGCDC & _cdc;
    // Holds the event number.
    int m_eventNum;
    // Holds the number of wire for stereo SL.
    int m_nWires[4];
    // Holds the radius of stereo SL in cm.
    double m_rr[4];
    // Holds the radius of axial SL in cm.
    double m_axR[5];
    // Holds the number of of axial SL.
    int m_axNWires[5];
    // Holds the ztostraw for stereo SL in cm.
    double m_ztostraw[4];
    // Holds the angle of shift for stereo SL in rad.
    double m_anglest[4];
    // PI used for 3D tracker.
    double m_Trg_PI;
    // Choose whether to save root file.
    bool m_makeRootFile;
    // 0: perfect finder, 1: Hough3DFinder, 2: GeoFinder, 3: VHDL GeoFinder
    // Choose what finder to use.
    int m_finderMode;
    // Hough Variables.
    // Pointer to hough 3D finder.
    Hough3DFinder* m_Hough3DFinder;
    // cot start size variable for Hough map for 3D.
    double m_cotStart;
    // cot end size variable for Hough map for 3D.
    double m_cotEnd;
    // z0 start size variable for Hough map for 3D.
    double m_z0Start;
    // z0 end size variable for Hough map for 3D.
    double m_z0End;
    // Number of mesh for cot for Hough map for 3D.
    int m_nCotSteps;
    // Number of mesh for z0 for Hough map for 3D.
    int m_nZ0Steps;
    // Size of mesh for cot for Hough map for 3D.
    double m_cotStepSize;
    // Size of mesh for z0 for Hough map for 3D.
    double m_z0StepSize;
    // For root file variables.
    std::string m_rootHough3DFilename;
    // File to save finder3D information.
    TFile* m_fileHough3D;
    // TTree for track information.
    TTree* m_treeTrackHough3D;
    // TTree for event information.
    TTree* m_treeEventHough3D;
    // TTree for constant information.
    TTree* m_treeConstantsHough3D;
    // Stores stereo SL1 phi values.
    TClonesArray* m_st0TSsTrackHough3D;
    // Stores stereo SL3 phi values.
    TClonesArray* m_st1TSsTrackHough3D;
    // Stores stereo SL5 phi values.
    TClonesArray* m_st2TSsTrackHough3D;
    // Stores stereo SL7 phi values.
    TClonesArray* m_st3TSsTrackHough3D;
    // Stores drift stereo SL1 phi values.
    TClonesArray* m_driftSt0TSsTrackHough3D;
    // Stores drift stereo SL3 phi values.
    TClonesArray* m_driftSt1TSsTrackHough3D;
    // Stores drift stereo SL5 phi values.
    TClonesArray* m_driftSt2TSsTrackHough3D;
    // Stores drift stereo SL7 phi values.
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
    // Stores mc ture stereo TSs for SL1 for track.
    TClonesArray* m_mcSt0TSsTrackHough3D;
    // Stores mc ture stereo TSs for SL3 for track.
    TClonesArray* m_mcSt1TSsTrackHough3D;
    // Stores mc ture stereo TSs for SL5 for track.
    TClonesArray* m_mcSt2TSsTrackHough3D;
    // Stores mc ture stereo TSs for SL7 for track.
    TClonesArray* m_mcSt3TSsTrackHough3D;
    // Stores performance values. [purity, Efficiency, Track #]
    TClonesArray* m_performanceTrackHough3D;
    // Stores deltaWireDiff
    TClonesArray* m_deltaWireDiffTrackHough3D;
    // Stores fitz
    TClonesArray* m_fitZTrackHough3D;
    // 4*2 = 8
    // st1_x, st1_y, st2_x, st2_y, st3_x, st3_y, st4_x, st4_y
    // Stores mc stereo's axial position.
    TClonesArray* m_mcStAxXYTrackHough3D;
    // Stores performance values. [Total # tracks from 2D finder]
    TVectorD* m_performanceEventHough3D;
    // Stores geometry values. [rr0, rr1, rr2, rr3, anglest0, anglest1, anglest2, anglest3, ztostraw0, ztostraw1, ztostraw2, ztostraw3]
    TVectorD* m_geometryHough3D;
    // To save mode of 3D tracker.
    TVectorD* m_modeHough3D;
    // To save inital variables for 3D tracker.
    TVectorD* m_initVariablesHough3D;
    // Stores geoFinder candidate phi for SL1.
    TClonesArray* m_st0GeoCandidatesPhiTrackHough3D;
    // Stores geoFinder candidate phi for SL3.
    TClonesArray* m_st1GeoCandidatesPhiTrackHough3D;
    // Stores geoFinder candidate phi for SL5.
    TClonesArray* m_st2GeoCandidatesPhiTrackHough3D;
    // Stores geoFinder candidate phi for SL7.
    TClonesArray* m_st3GeoCandidatesPhiTrackHough3D;
    // Stores geoFinder candidate diff wires for SL1.
    TClonesArray* m_st0GeoCandidatesDiffStWiresTrackHough3D;
    // Stores geoFinder candidate diff wires for SL3.
    TClonesArray* m_st1GeoCandidatesDiffStWiresTrackHough3D;
    // Stores geoFinder candidate diff wires for SL5.
    TClonesArray* m_st2GeoCandidatesDiffStWiresTrackHough3D;
    // Stores geoFinder candidate diff wires for SL7.
    TClonesArray* m_st3GeoCandidatesDiffStWiresTrackHough3D;
    // Stores geoFinder stAx phi [ for each stereo layer ]
    TClonesArray* m_stAxPhiTrackHough3D;
    // Stores event and track ID;
    TClonesArray* m_eventTrackIDTrackHough3D;

};

} // namespace Belle2

#endif
