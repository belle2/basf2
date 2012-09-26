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
    TRGCDCHough3DFinder(const TRGCDC &);
    ~TRGCDCHough3DFinder();
    // Member functions.
    void doit(std::vector<TRGCDCTrack *> & trackList);
    void perfectFinder(std::vector<TRGCDCTrack *> &trackList, unsigned j, std::vector<const TRGCDCSegmentHit*> &mcTSList);
    void doitPerfectly(std::vector<TRGCDCTrack *> & trackList);
    void doitVersion1(std::vector<TRGCDCTrack *> & trackList);
    void terminate(void);
    // Members.
    const TRGCDC & _cdc;
    int m_nWires[4];
    double m_rr[4];
    double m_ztostraw[4];
    double m_anglest[4];
    double m_Trg_PI;
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
    float ***m_houghMeshLayerDiff;
    bool ***m_houghMeshLayer;
    int **m_houghMesh;
    float **m_houghMeshDiff;
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
    // Stores performance values. [purity, Track #]
    TClonesArray* m_performanceTrackHough3D;
    // Stores performance values. [Total # tracks from 2D finder]
    TVectorD* m_performanceEventHough3D;
    // Stores geometry values. [rr0, rr1, rr2, rr3, anglest0, anglest1, anglest2, anglest3, ztostraw0, ztostraw1, ztostraw2, ztostraw3]
    TVectorD* m_geometryHough3D;

};

} // namespace Belle2

#endif
