//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Hough3DFinder.cc
// Section  : TRG CDC
// Owner    : Jaebak Kim
// Email    : jbkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to find 3D tracks using Hough algorithm
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <stdlib.h>
#include <map>
#include "TFile.h"
#include "TTree.h"
#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "mdst/dataobjects/MCParticle.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Debug.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/Cell.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/Hough3DFinder.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/Circle.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include "trg/cdc/Relation.h"
#include "trg/cdc/Fitter3DUtility.h"

using namespace std;

namespace Belle2 {

TRGCDCHough3DFinder::TRGCDCHough3DFinder(const TRGCDC & TRGCDC, bool makeRootFile, int finderMode)
    : _cdc(TRGCDC), m_makeRootFile(makeRootFile) , m_finderMode(finderMode) {
    m_Trg_PI = 3.141592653589793;
    // Initialize rr, ztostraw, anglest, nWiresInStereoLayer.
    CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    for(int stSuperLayer=0;stSuperLayer<4;stSuperLayer++){
        m_rr[stSuperLayer]=cdcp.senseWireR(12*stSuperLayer+10)*0.01;
        m_ztostraw[stSuperLayer]=cdcp.senseWireBZ(12*stSuperLayer+10)*0.01;
        m_anglest[stSuperLayer]=2*m_rr[stSuperLayer]*sin(m_Trg_PI*cdcp.nShifts(12*stSuperLayer+10)/(2*cdcp.nWiresInLayer(12*stSuperLayer+10)))/(cdcp.senseWireFZ(12*stSuperLayer+10)-cdcp.senseWireBZ(12*stSuperLayer+10))/0.01;
        m_nWires[stSuperLayer]=cdcp.nWiresInLayer(12*stSuperLayer+10)*2;
    }
    m_axR[0] = cdcp.senseWireR(2)*0.01;
    m_axNWires[0]=cdcp.nWiresInLayer(2)*2;
    for(int axSuperLayer=0;axSuperLayer<4;axSuperLayer++){
        m_axR[axSuperLayer+1]=cdcp.senseWireR(12*axSuperLayer+16)*0.01;
        m_axNWires[axSuperLayer+1]=cdcp.nWiresInLayer(12*axSuperLayer+16)*2;
    }

    // Hough Variables.
    m_cotStart = -3;
    m_cotEnd = -m_cotStart;
    m_z0Start = -2;
    m_z0End = -m_z0Start;
    // Must be odd
    //m_nCotSteps = 101;
    //m_nZ0Steps = 501;
    m_nCotSteps = 1001;
    m_nZ0Steps = 1001;
    m_cotStepSize = m_cotEnd/((m_nCotSteps-1)/2);
    m_z0StepSize = m_z0End/((m_nZ0Steps-1)/2);

    // Save in root.
    if(m_makeRootFile) m_fileHough3D = new TFile("Hough3D.root","RECREATE");
    m_treeTrackHough3D = new TTree("m_treeTrackHough3D","track");
    m_st0TSsTrackHough3D = new TClonesArray("TVectorD");
    m_st1TSsTrackHough3D = new TClonesArray("TVectorD");
    m_st2TSsTrackHough3D = new TClonesArray("TVectorD");
    m_st3TSsTrackHough3D = new TClonesArray("TVectorD");
    m_driftSt0TSsTrackHough3D = new TClonesArray("TVectorD");
    m_driftSt1TSsTrackHough3D = new TClonesArray("TVectorD");
    m_driftSt2TSsTrackHough3D = new TClonesArray("TVectorD");
    m_driftSt3TSsTrackHough3D = new TClonesArray("TVectorD");
    m_fit2DTrackHough3D = new TClonesArray("TVectorD");
    m_bestFitTrackHough3D = new TClonesArray("TVectorD");
    m_mcTrackHough3D = new TClonesArray("TVectorD");
    m_mcStatusTrackHough3D = new TClonesArray("TVectorD");
    m_mcVertexTrackHough3D = new TClonesArray("TVector3");
    m_mc4VectorTrackHough3D = new TClonesArray("TLorentzVector");
    m_bestTSsTrackHough3D = new TClonesArray("TVectorD");
    m_mcTSsTrackHough3D = new TClonesArray("TVectorD");
    m_mcSt0TSsTrackHough3D = new TClonesArray("TVectorD");
    m_mcSt1TSsTrackHough3D = new TClonesArray("TVectorD");
    m_mcSt2TSsTrackHough3D = new TClonesArray("TVectorD");
    m_mcSt3TSsTrackHough3D = new TClonesArray("TVectorD");
    m_performanceTrackHough3D = new TClonesArray("TVectorD");
    m_deltaWireDiffTrackHough3D = new TClonesArray("TVectorD");
    m_fitZTrackHough3D = new TClonesArray("TVectorD");
    m_mcStAxXYTrackHough3D = new TClonesArray("TVectorD");
    m_st0GeoCandidatesPhiTrackHough3D = new TClonesArray("TVectorD");
    m_st1GeoCandidatesPhiTrackHough3D = new TClonesArray("TVectorD");
    m_st2GeoCandidatesPhiTrackHough3D = new TClonesArray("TVectorD");
    m_st3GeoCandidatesPhiTrackHough3D = new TClonesArray("TVectorD");
    m_st0GeoCandidatesDiffStWiresTrackHough3D = new TClonesArray("TVectorD");
    m_st1GeoCandidatesDiffStWiresTrackHough3D = new TClonesArray("TVectorD");
    m_st2GeoCandidatesDiffStWiresTrackHough3D = new TClonesArray("TVectorD");
    m_st3GeoCandidatesDiffStWiresTrackHough3D = new TClonesArray("TVectorD");
    m_stAxPhiTrackHough3D = new TClonesArray("TVectorD");
    m_eventTrackIDTrackHough3D = new TClonesArray("TVectorD");
    m_treeTrackHough3D->Branch("st0TSsTrackHough3D", &m_st0TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st1TSsTrackHough3D", &m_st1TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st2TSsTrackHough3D", &m_st2TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st3TSsTrackHough3D", &m_st3TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("driftSt0TSsTrackHough3D", &m_driftSt0TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("driftSt1TSsTrackHough3D", &m_driftSt1TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("driftSt2TSsTrackHough3D", &m_driftSt2TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("driftSt3TSsTrackHough3D", &m_driftSt3TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("fit2DTrackHough3D", &m_fit2DTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("bestFitTrackHough3D", &m_bestFitTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcTrackHough3D", &m_mcTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcStatusTrackHough3D", &m_mcStatusTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcVertexTrackHough3D", &m_mcVertexTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mc4VectorTrackHough3D", &m_mc4VectorTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("bestTSsTrackHough3D", &m_bestTSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcTSsTrackHough3D", &m_mcTSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcSt0TSsTrackHough3D", &m_mcSt0TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcSt1TSsTrackHough3D", &m_mcSt1TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcSt2TSsTrackHough3D", &m_mcSt2TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcSt3TSsTrackHough3D", &m_mcSt3TSsTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("performanceTrackHough3D", &m_performanceTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("deltaWireDiffTrackHough3D", &m_deltaWireDiffTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("fitZTrackHough3D", &m_fitZTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("mcStAxXYTrackHough3D", &m_mcStAxXYTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st0GeoCandidatesPhiTrackHough3D", &m_st0GeoCandidatesPhiTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st1GeoCandidatesPhiTrackHough3D", &m_st1GeoCandidatesPhiTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st2GeoCandidatesPhiTrackHough3D", &m_st2GeoCandidatesPhiTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st3GeoCandidatesPhiTrackHough3D", &m_st3GeoCandidatesPhiTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st0GeoCandidatesDiffStWiresTrackHough3D", &m_st0GeoCandidatesDiffStWiresTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st1GeoCandidatesDiffStWiresTrackHough3D", &m_st1GeoCandidatesDiffStWiresTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st2GeoCandidatesDiffStWiresTrackHough3D", &m_st2GeoCandidatesDiffStWiresTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("st3GeoCandidatesDiffStWiresTrackHough3D", &m_st3GeoCandidatesDiffStWiresTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("stAxPhiTrackHough3D", &m_stAxPhiTrackHough3D,32000,0);
    m_treeTrackHough3D->Branch("eventTrackIDTrackHough3D", &m_eventTrackIDTrackHough3D,32000,0);

    m_treeEventHough3D = new TTree("m_treeEventHough3D","event");
    m_performanceEventHough3D = new TVectorD(1);
    m_treeEventHough3D->Branch("performanceEventHough3D", &m_performanceEventHough3D,32000,0);

    m_treeConstantsHough3D = new TTree("m_treeConstantsHough3D", "constants");
    m_geometryHough3D = new TVectorD(16);
    m_modeHough3D = new TVectorD(1);
    m_initVariablesHough3D = new TVectorD(6);
    m_treeConstantsHough3D->Branch("geometryHough3D", &m_geometryHough3D,32000,0);
    m_treeConstantsHough3D->Branch("modeHough3D", &m_modeHough3D,32000,0);
    m_treeConstantsHough3D->Branch("initVariablesHough3D", &m_initVariablesHough3D,32000,0);

    // Save geometry to root file.
    TVectorD &geometryHough3D = *m_geometryHough3D;
    for(int i=0; i<4; i++){
        geometryHough3D[i] = m_rr[i];
        geometryHough3D[i+4] = m_anglest[i];
        geometryHough3D[i+8] = m_ztostraw[i];
        geometryHough3D[i+12] = m_nWires[i];
    }

    m_Hough3DFinder = new Hough3DFinder();
    // 1: Hough3DFinder 2: GeoFinder 3: VHDL GeoFinder
    m_Hough3DFinder->setMode(m_finderMode);
    // Set input file name for VHDL GeoFinder.
    m_Hough3DFinder->setInputFileName("GeoFinder.input");
    // For VHDL GEoFinder
    //m_Hough3DFinder->setInputFileName("GeoFinder.input");
    // cotStart, cotEnd, z0Start, z0End, cotSteps, z0Steps
    float tempInitVariables[] = {-3,3,-2,2,1001,1001};
    vector<float > initVariables(tempInitVariables, tempInitVariables+sizeof(tempInitVariables) / sizeof(tempInitVariables[0]) );
    // Save the init variables
    m_Hough3DFinder->initialize(geometryHough3D, initVariables);

    TVectorD &modeHough3D = *m_modeHough3D;
    modeHough3D[0] = m_Hough3DFinder->getMode();

    TVectorD &initVariablesHough3D = *m_initVariablesHough3D;
    initVariablesHough3D[0] = tempInitVariables[0];
    initVariablesHough3D[1] = tempInitVariables[1];
    initVariablesHough3D[2] = tempInitVariables[2];
    initVariablesHough3D[3] = tempInitVariables[3];
    initVariablesHough3D[4] = tempInitVariables[4];
    initVariablesHough3D[5] = tempInitVariables[5];

    m_treeConstantsHough3D->Fill();
}

TRGCDCHough3DFinder::~TRGCDCHough3DFinder(){
    m_Hough3DFinder->destruct();

    // Deallocate root variables
    if(m_makeRootFile) delete m_fileHough3D;
    delete m_treeTrackHough3D;
    delete m_st0TSsTrackHough3D;
    delete m_st1TSsTrackHough3D;
    delete m_st2TSsTrackHough3D;
    delete m_st3TSsTrackHough3D;
    delete m_driftSt0TSsTrackHough3D;
    delete m_driftSt1TSsTrackHough3D;
    delete m_driftSt2TSsTrackHough3D;
    delete m_driftSt3TSsTrackHough3D;
    delete m_fit2DTrackHough3D;
    delete m_bestFitTrackHough3D;
    delete m_mcTrackHough3D;
    delete m_mcStatusTrackHough3D;
    delete m_mcVertexTrackHough3D;
    delete m_mc4VectorTrackHough3D;
    delete m_bestTSsTrackHough3D;
    delete m_mcTSsTrackHough3D;
    delete m_mcSt0TSsTrackHough3D;
    delete m_mcSt1TSsTrackHough3D;
    delete m_mcSt2TSsTrackHough3D;
    delete m_mcSt3TSsTrackHough3D;
    delete m_performanceTrackHough3D;
    delete m_deltaWireDiffTrackHough3D;
    delete m_fitZTrackHough3D;
    delete m_mcStAxXYTrackHough3D;
    delete m_treeEventHough3D;
    delete m_performanceEventHough3D;
    delete m_treeConstantsHough3D;
    delete m_geometryHough3D;
    delete m_st0GeoCandidatesPhiTrackHough3D;
    delete m_st1GeoCandidatesPhiTrackHough3D;
    delete m_st2GeoCandidatesPhiTrackHough3D;
    delete m_st3GeoCandidatesPhiTrackHough3D;
    delete m_st0GeoCandidatesDiffStWiresTrackHough3D;
    delete m_st1GeoCandidatesDiffStWiresTrackHough3D;
    delete m_st2GeoCandidatesDiffStWiresTrackHough3D;
    delete m_st3GeoCandidatesDiffStWiresTrackHough3D;
    delete m_stAxPhiTrackHough3D;
    delete m_eventTrackIDTrackHough3D;

}

void TRGCDCHough3DFinder::terminate(void){
    if(m_makeRootFile) {
        m_fileHough3D->Write();
        m_fileHough3D->Close();
    }
}

void TRGCDCHough3DFinder::doit(vector<TCTrack *> & trackList, int eventNum){
    m_eventNum = eventNum;
    doit(trackList);
}

void TRGCDCHough3DFinder::doit(vector<TCTrack *> & trackList){

    // Assign track ID's.
    for(unsigned iTrack=0; iTrack<trackList.size(); iTrack++){
        TCTrack& aTrack = *trackList[iTrack];
        aTrack.setTrackID(iTrack+1);
    }

    if(m_finderMode==0) doitPerfectly(trackList);
    if(m_finderMode!=0) doitFind(trackList);
}

void TRGCDCHough3DFinder::doitFind(vector<TCTrack *> & trackList){
    TRGDebug::enterStage("3D finder");
    //cout<<"[JB] Event: "<<m_eventNum<<endl;

    // Call storage array.
    StoreArray<CDCSimHit> SimHits("CDCSimHits");
    StoreArray<CDCHit> CDCHits("CDCHits");
    RelationArray relationCDCHits(SimHits, CDCHits);

    TClonesArray &st0TSsTrackHough3D = *m_st0TSsTrackHough3D;
    TClonesArray &st1TSsTrackHough3D = *m_st1TSsTrackHough3D;
    TClonesArray &st2TSsTrackHough3D = *m_st2TSsTrackHough3D;
    TClonesArray &st3TSsTrackHough3D = *m_st3TSsTrackHough3D;
    TClonesArray &driftSt0TSsTrackHough3D = *m_driftSt0TSsTrackHough3D;
    TClonesArray &driftSt1TSsTrackHough3D = *m_driftSt1TSsTrackHough3D;
    TClonesArray &driftSt2TSsTrackHough3D = *m_driftSt2TSsTrackHough3D;
    TClonesArray &driftSt3TSsTrackHough3D = *m_driftSt3TSsTrackHough3D;
    TClonesArray &fit2DTrackHough3D = *m_fit2DTrackHough3D;
    TClonesArray &bestFitTrackHough3D = *m_bestFitTrackHough3D;
    TClonesArray &mcTrackHough3D = *m_mcTrackHough3D;
    TClonesArray &mcStatusTrackHough3D = *m_mcStatusTrackHough3D;
    TClonesArray &mcVertexTrackHough3D = *m_mcVertexTrackHough3D;
    TClonesArray &mc4VectorTrackHough3D = *m_mc4VectorTrackHough3D;
    TClonesArray &bestTSsTrackHough3D = *m_bestTSsTrackHough3D;
    TClonesArray &mcTSsTrackHough3D = *m_mcTSsTrackHough3D;
    TClonesArray &mcSt0TSsTrackHough3D = *m_mcSt0TSsTrackHough3D;
    TClonesArray &mcSt1TSsTrackHough3D = *m_mcSt1TSsTrackHough3D;
    TClonesArray &mcSt2TSsTrackHough3D = *m_mcSt2TSsTrackHough3D;
    TClonesArray &mcSt3TSsTrackHough3D = *m_mcSt3TSsTrackHough3D;
    TClonesArray &performanceTrackHough3D = *m_performanceTrackHough3D;
    TClonesArray &deltaWireDiffTrackHough3D = *m_deltaWireDiffTrackHough3D;
    TClonesArray &fitZTrackHough3D = *m_fitZTrackHough3D;
    TClonesArray &mcStAxXYTrackHough3D = *m_mcStAxXYTrackHough3D;
    TClonesArray &st0GeoCandidatesPhiTrackHough3D = *m_st0GeoCandidatesPhiTrackHough3D;
    TClonesArray &st1GeoCandidatesPhiTrackHough3D = *m_st1GeoCandidatesPhiTrackHough3D;
    TClonesArray &st2GeoCandidatesPhiTrackHough3D = *m_st2GeoCandidatesPhiTrackHough3D;
    TClonesArray &st3GeoCandidatesPhiTrackHough3D = *m_st3GeoCandidatesPhiTrackHough3D;
    TClonesArray &st0GeoCandidatesDiffStWiresTrackHough3D = *m_st0GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray &st1GeoCandidatesDiffStWiresTrackHough3D = *m_st1GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray &st2GeoCandidatesDiffStWiresTrackHough3D = *m_st2GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray &st3GeoCandidatesDiffStWiresTrackHough3D = *m_st3GeoCandidatesDiffStWiresTrackHough3D;
    TClonesArray &stAxPhiTrackHough3D = *m_stAxPhiTrackHough3D;
    TClonesArray &eventTrackIDTrackHough3D = *m_eventTrackIDTrackHough3D;

    TVectorD &performanceEventHough3D = *m_performanceEventHough3D;

    st0TSsTrackHough3D.Clear();
    st1TSsTrackHough3D.Clear();
    st2TSsTrackHough3D.Clear();
    st3TSsTrackHough3D.Clear();
    driftSt0TSsTrackHough3D.Clear();
    driftSt1TSsTrackHough3D.Clear();
    driftSt2TSsTrackHough3D.Clear();
    driftSt3TSsTrackHough3D.Clear();
    fit2DTrackHough3D.Clear();
    bestFitTrackHough3D.Clear();
    mcTrackHough3D.Clear();
    mcStatusTrackHough3D.Clear();
    mcVertexTrackHough3D.Clear();
    mc4VectorTrackHough3D.Clear();
    bestTSsTrackHough3D.Clear();
    mcTSsTrackHough3D.Clear();
    mcSt0TSsTrackHough3D.Clear();
    mcSt1TSsTrackHough3D.Clear();
    mcSt2TSsTrackHough3D.Clear();
    mcSt3TSsTrackHough3D.Clear();
    performanceTrackHough3D.Clear();
    deltaWireDiffTrackHough3D.Clear();
    fitZTrackHough3D.Clear();
    mcStAxXYTrackHough3D.Clear();
    st0GeoCandidatesPhiTrackHough3D.Clear();
    st1GeoCandidatesPhiTrackHough3D.Clear();
    st2GeoCandidatesPhiTrackHough3D.Clear();
    st3GeoCandidatesPhiTrackHough3D.Clear();
    st0GeoCandidatesDiffStWiresTrackHough3D.Clear();
    st1GeoCandidatesDiffStWiresTrackHough3D.Clear();
    st2GeoCandidatesDiffStWiresTrackHough3D.Clear();
    st3GeoCandidatesDiffStWiresTrackHough3D.Clear();
    stAxPhiTrackHough3D.Clear();
    eventTrackIDTrackHough3D.Clear();

    //performanceEventHough3D.Clear();

    // Generate arrays for TS candidates.
    vector<vector<double> > stTSs;
    vector<vector<const TCSHit *> > p_stTSs;

    vector<const TCSHit *> hits = _cdc.stereoSegmentHits(0);
    Int_t nHitsInLayer = (Int_t)hits.size();
    TVectorD st0TSs(nHitsInLayer);
    TVectorD driftSt0TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
        if(hits[j]==0) {cout<<"[0] POINTER IS ZERO"<<endl; continue;}
        st0TSs[j] = (double)hits[j]->cell().localId()/m_nWires[0]*4*m_Trg_PI;
        driftSt0TSs[j] = hits[j]->segment().phiPosition();
        //cout<<st0TSs[j]<<" "<<hits[j]->segment().phiPosition()<<endl;
        stTSs[0].push_back(st0TSs[j]);
        p_stTSs[0].push_back(hits[j]);
    }

    hits = _cdc.stereoSegmentHits(1);
    nHitsInLayer = (Int_t)hits.size();
    TVectorD st1TSs(nHitsInLayer);
    TVectorD driftSt1TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
        if(hits[j]==0) {cout<<"[1] POINTER IS ZERO"<<endl; continue;}
        st1TSs[j] = (double)hits[j]->cell().localId()/m_nWires[1]*4*m_Trg_PI;
        driftSt1TSs[j] = hits[j]->segment().phiPosition();
        stTSs[1].push_back(st1TSs[j]);
        p_stTSs[1].push_back(hits[j]);
    }

    hits = _cdc.stereoSegmentHits(2);
    nHitsInLayer = (Int_t)hits.size();
    TVectorD st2TSs(nHitsInLayer);
    TVectorD driftSt2TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
        if(hits[j]==0) {cout<<"[2] POINTER IS ZERO"<<endl; continue;}
        st2TSs[j] = (double)hits[j]->cell().localId()/m_nWires[2]*4*m_Trg_PI;
        driftSt2TSs[j] = hits[j]->segment().phiPosition();
        stTSs[2].push_back(st2TSs[j]);
        p_stTSs[2].push_back(hits[j]);
    }

    hits = _cdc.stereoSegmentHits(3);
    nHitsInLayer = (Int_t)hits.size();
    TVectorD st3TSs(nHitsInLayer);
    TVectorD driftSt3TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
        if(hits[j]==0) {cout<<"[3] POINTER IS ZERO"<<endl; continue;}
        st3TSs[j] = (double)hits[j]->cell().localId()/m_nWires[3]*4*m_Trg_PI;
        driftSt3TSs[j] = hits[j]->segment().phiPosition();
        stTSs[3].push_back(st3TSs[j]);
        p_stTSs[3].push_back(hits[j]);
    }

    // Find number of TS(only one for each layer) for each mc particle.
    // numberTSsForParticle[0] = MCID, numberTSsForParticle[1] = # Superlayer hits
    map<unsigned, unsigned> numberTSsForParticle;
    vector<unsigned> mcParticleList;
    for(unsigned iLayer=0; iLayer<4; iLayer++) {
        // Find what mc particles there are in a layer
        mcParticleList.clear();
        for(unsigned iTS = 0; iTS < p_stTSs[iLayer].size(); iTS++) {
            unsigned iMCParticle = p_stTSs[iLayer][iTS]->iMCParticle();
            if ( find(mcParticleList.begin(), mcParticleList.end(), iMCParticle) == mcParticleList.end() ) {
                mcParticleList.push_back(iMCParticle);
            }
        }
        // Loop over mcParticleList and add to numberTSsForParticle
        for( unsigned iMCPart = 0; iMCPart < mcParticleList.size(); iMCPart++ ) {
            map<unsigned, unsigned>::iterator it = numberTSsForParticle.find(mcParticleList[iMCPart]);
            if ( it != numberTSsForParticle.end() ) ++it->second;
            else numberTSsForParticle[mcParticleList[iMCPart]] = 1;
        }
    }

    //// Print result of numberTSsForParticle
    //map<unsigned, unsigned>::const_iterator it = numberTSsForParticle.begin();
    //while ( it != numberTSsForParticle.end() ) {
    //  cout << "IDParticle: "<<it->first << ", #: " << it->second << endl;
    //  ++it;
    //}


    // Loop over all the tracks.
    unsigned nTracks=trackList.size();
    for(unsigned iTrack=0; iTrack<nTracks; iTrack++){


        // Check if there is a TS in each super layer.
        if(stTSs[0].size()==0) {cout<<"[0] No TS"<<endl; continue;}
        if(stTSs[1].size()==0) {cout<<"[1] No TS"<<endl; continue;}
        if(stTSs[2].size()==0) {cout<<"[2] No TS"<<endl; continue;}
        if(stTSs[3].size()==0) {cout<<"[3] No TS"<<endl; continue;}

        // Set input for Hough 3D finder
        // Get 2D track parameters.
        TCTrack& aTrack = *trackList[iTrack];

        //cout<<"[JB]Track: "<<aTrack.getTrackID()<<endl;
        double fitPt = aTrack.pt();
        double rho = fitPt/0.3/1.5;
        double fitPhi0;
        if (aTrack.charge() > 0) fitPhi0 = aTrack.p().phi() - M_PI / 2; 
        if (aTrack.charge() < 0) fitPhi0 = aTrack.p().phi() + M_PI / 2; 
        // Need to do 2D fitting here one more time
        // Get phi and rr for Ax.
        double axPhi[5]={0,0,0,0,0};
        for (int iLayer=0; iLayer<5; iLayer++){
            const vector<TCLink *> & links = aTrack.links(2*iLayer);
            const unsigned nSegments = links.size();
            //...Presently nSegments should be 1...
            if (nSegments != 1) {
                if (nSegments==0){
                    cout << " !!! NO TS assigned" << endl;
                    break;
                }
                else{cout<< "!!!! multiple TS assigned"<< endl;}
            }
            const TCSegment * s = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
            axPhi[iLayer]=(double) s->localId()/m_axNWires[iLayer]*4*m_Trg_PI;
        } // End of layer loop

        // Do 2D fitting with fixed IP.
        double tempR[9], tempPhi[9];
        double phiError[5];
        phiError[0] = 0.0085106; phiError[1] = 0.0039841; phiError[2] = 0.0025806; phiError[3] = 0.0019084; phiError[4] = 0.001514;
        for(int iLayer=0; iLayer<5; iLayer++) {
            tempR[iLayer] = m_axR[iLayer];
            tempPhi[iLayer] = axPhi[iLayer];
        }
        for(int iLayer=0; iLayer<4; iLayer++) {
            tempR[iLayer+5] = 0;
            tempPhi[iLayer+5] = 0;
        }
        Fitter3DUtility::rPhiFit(tempR,tempPhi,phiError,rho,fitPhi0);
        fitPt = rho*0.3*1.5;

        int charge = int(aTrack.charge());
        // Get MC track parameters.
        const TCRelation& trackRelation = aTrack.relation();
        const MCParticle& trackMCParticle = trackRelation.mcParticle(0);
        //double mcPt = trackMCParticle.getMomentum().Pt();
        double mcPhi0;
        if (trackMCParticle.getCharge() > 0) mcPhi0 = trackMCParticle.getMomentum().Phi() - M_PI / 2;
        if (trackMCParticle.getCharge() < 0) mcPhi0 = trackMCParticle.getMomentum().Phi() + M_PI / 2;
        //Change range to [0,2pi]
        if (mcPhi0 < 0) mcPhi0 += 2 * M_PI;
        // mcZ0 is not vertex point. It should be impact point.
        double mcZ0 = trackMCParticle.getVertex().Z()/100;
        double mcCot = trackMCParticle.getMomentum().Pz()/trackMCParticle.getMomentum().Pt();
        // Set input of finder
        double tempTrackVariables[] = {double(charge), rho, fitPhi0};
        vector<double > trackVariables(tempTrackVariables, tempTrackVariables+sizeof(tempTrackVariables) / sizeof(tempTrackVariables[0]) );

        // Run finder
        m_Hough3DFinder->runFinder(trackVariables, stTSs);

        // Get results of finder
        const TCSHit* p_bestTS[4]={0,0,0,0};
        double bestZ0,bestCot,houghMax,minDiffHough = 0;
        double bestTS[4];
        vector<double> tempResult;
        m_Hough3DFinder->getValues("bestTSIndex",tempResult);
        for(int iTS=0; iTS<4; iTS++) {
            p_bestTS[iTS] = p_stTSs[iTS][(int)tempResult[iTS]];
            //bestTS[iTS] = stTSs[iTS][(int)tempResult[iTS]];
        }
        m_Hough3DFinder->getValues("bestTS",tempResult);
        for(int iLayer=0; iLayer<4; iLayer++) bestTS[iLayer] = tempResult[iLayer];
        if(m_Hough3DFinder->getMode()==1){
            m_Hough3DFinder->getValues("bestZ0", tempResult);
            bestZ0 = tempResult[0];
            m_Hough3DFinder->getValues("bestCot", tempResult);
            bestCot = tempResult[0];
            m_Hough3DFinder->getValues("houghMax", tempResult);
            houghMax = tempResult[0];
            m_Hough3DFinder->getValues("minDiffHough", tempResult);
            minDiffHough = tempResult[0];
        }
        if(m_Hough3DFinder->getMode()==2){
            m_Hough3DFinder->getValues("st0GeoCandidatesPhi", tempResult);
            TVectorD st0GeoCandidatesPhi(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st0GeoCandidatesPhi[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st1GeoCandidatesPhi", tempResult);
            TVectorD st1GeoCandidatesPhi(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st1GeoCandidatesPhi[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st2GeoCandidatesPhi", tempResult);
            TVectorD st2GeoCandidatesPhi(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st2GeoCandidatesPhi[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st3GeoCandidatesPhi", tempResult);
            TVectorD st3GeoCandidatesPhi(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st3GeoCandidatesPhi[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st0GeoCandidatesDiffStWires", tempResult);
            TVectorD st0GeoCandidatesDiffStWires(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st0GeoCandidatesDiffStWires[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st1GeoCandidatesDiffStWires", tempResult);
            TVectorD st1GeoCandidatesDiffStWires(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st1GeoCandidatesDiffStWires[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st2GeoCandidatesDiffStWires", tempResult);
            TVectorD st2GeoCandidatesDiffStWires(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st2GeoCandidatesDiffStWires[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("st3GeoCandidatesDiffStWires", tempResult);
            TVectorD st3GeoCandidatesDiffStWires(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                st3GeoCandidatesDiffStWires[iTS] = tempResult[iTS];
            }
            m_Hough3DFinder->getValues("stAxPhi", tempResult);
            TVectorD stAxPhi(tempResult.size());
            for(int iTS=0; iTS<(int)tempResult.size(); iTS++){
                stAxPhi[iTS] = tempResult[iTS];
            }
            // Save values to root
            // GeoCandidatesPhi
            new(st0GeoCandidatesPhiTrackHough3D[iTrack]) TVectorD(st0GeoCandidatesPhi); 
            new(st1GeoCandidatesPhiTrackHough3D[iTrack]) TVectorD(st1GeoCandidatesPhi); 
            new(st2GeoCandidatesPhiTrackHough3D[iTrack]) TVectorD(st2GeoCandidatesPhi); 
            new(st3GeoCandidatesPhiTrackHough3D[iTrack]) TVectorD(st3GeoCandidatesPhi); 
            // GeoCandidatesDiffStWires
            new(st0GeoCandidatesDiffStWiresTrackHough3D[iTrack]) TVectorD(st0GeoCandidatesDiffStWires); 
            new(st1GeoCandidatesDiffStWiresTrackHough3D[iTrack]) TVectorD(st1GeoCandidatesDiffStWires); 
            new(st2GeoCandidatesDiffStWiresTrackHough3D[iTrack]) TVectorD(st2GeoCandidatesDiffStWires); 
            new(st3GeoCandidatesDiffStWiresTrackHough3D[iTrack]) TVectorD(st3GeoCandidatesDiffStWires); 
            // stAxPhi
            new(stAxPhiTrackHough3D[iTrack]) TVectorD(stAxPhi);

        }

        // Find and append TS to track
        for(unsigned iLayer=0; iLayer<4; iLayer++){
            //p_bestTS[iLayer]->dump("state","");
            if(bestTS[iLayer]!=999){
                aTrack.append(new TCLink(0,p_bestTS[iLayer],p_bestTS[iLayer]->cell().xyPosition()));
            }
        }
        //aTrack.dump("", "> ");

        // Save values to root
        new(st0TSsTrackHough3D[iTrack]) TVectorD(st0TSs);
        new(st1TSsTrackHough3D[iTrack]) TVectorD(st1TSs);
        new(st2TSsTrackHough3D[iTrack]) TVectorD(st2TSs);
        new(st3TSsTrackHough3D[iTrack]) TVectorD(st3TSs);
        new(driftSt0TSsTrackHough3D[iTrack]) TVectorD(driftSt0TSs);
        new(driftSt1TSsTrackHough3D[iTrack]) TVectorD(driftSt1TSs);
        new(driftSt2TSsTrackHough3D[iTrack]) TVectorD(driftSt2TSs);
        new(driftSt3TSsTrackHough3D[iTrack]) TVectorD(driftSt3TSs);
        // fitPt, fitPhi0, fitCharge
        TVectorD tempFit2D(3);
        tempFit2D[0] = fitPt;
        tempFit2D[1] = fitPhi0;
        tempFit2D[2] = charge;
        new(fit2DTrackHough3D[iTrack]) TVectorD(tempFit2D);
        // z0, cot, #votes, minDiff
        //cout<<"Start saving"<<endl;
        TVectorD tempBestFit(4);
        tempBestFit[0] = bestZ0*m_z0StepSize;
        tempBestFit[1] = bestCot*m_cotStepSize+m_cotStart;
        tempBestFit[2] = houghMax;
        tempBestFit[3] = minDiffHough;
        new(bestFitTrackHough3D[iTrack]) TVectorD(tempBestFit);
        // mc z0, mc cot
        TVectorD tempMC(2);
        tempMC[0] = mcZ0;
        tempMC[1] = mcCot;
        new(mcTrackHough3D[iTrack]) TVectorD(tempMC);
        // statusbit, pdg, charge
        TVectorD mcStatus(3);
        mcStatus[0] = trackMCParticle.getStatus();
        mcStatus[1] = trackMCParticle.getPDG();
        mcStatus[2] = trackMCParticle.getCharge();
        new(mcStatusTrackHough3D[iTrack]) TVectorD(mcStatus);
        // vertex
        TVector3 tempVertex;
        tempVertex = trackMCParticle.getVertex();
        new(mcVertexTrackHough3D[iTrack]) TVector3(tempVertex);
        // 4-vector
        TLorentzVector temp4Vector;
        temp4Vector = trackMCParticle.get4Vector();
        new(mc4VectorTrackHough3D[iTrack]) TLorentzVector(temp4Vector);
        // best phi values [st0, st1, st2, st3, st4]
        TVectorD tempBestTSs(4,bestTS);
        new(bestTSsTrackHough3D[iTrack]) TVectorD(tempBestTSs);
        // performance values [purity, efficiency, Track#]
        TVectorD tempPerformanceTrack(3);
        const TCRelation& trackRelation3D = aTrack.relation3D();
        //cout<<"[Before] purity3D:   "<<trackRelation3D.purity(0)<<endl;
        //cout<<"[After] purity3D:    "<<trackRelation3D.purity3D(aTrack.relation2D().contributor(0))<<endl;
        tempPerformanceTrack[0] = trackRelation3D.purity3D(aTrack.relation2D().contributor(0));
        tempPerformanceTrack[1] = trackRelation3D.efficiency3D(aTrack.relation2D().contributor(0), numberTSsForParticle);
        tempPerformanceTrack[2] = iTrack;
        new(performanceTrackHough3D[iTrack]) TVectorD(tempPerformanceTrack);

        // Find one mc stereo track segment per layer.
        vector<const TCSHit*> mcTSList;
        perfectFinder(trackList, iTrack, mcTSList);
        TVectorD tempMcTSs(4);
        // Initialize tempMCTSs;
        for(unsigned iTS=0; iTS<4; iTS++){
            tempMcTSs[iTS]=999;
        }
        // Set TS values to root file.
        for(unsigned iTS=0; iTS<mcTSList.size(); iTS++){
            int iSuperLayer = (int)(double(mcTSList[iTS]->cell().superLayerId())-1)/2;
            tempMcTSs[iSuperLayer]=(double)mcTSList[iTS]->cell().localId()/m_nWires[iTS]*4*m_Trg_PI;
            //cout<<"[JB] "<<mcTSList[iTS]->cell().localId()<<" "<<mcTSList[iTS]->cell().superLayerId()<<" ";
        }
        //cout<<endl;
        new(mcTSsTrackHough3D[iTrack]) TVectorD(tempMcTSs);

        // Find multiple mc stereo track segments per layer.
        // Get tracks mc index
        //unsigned int mcParticleId = trackRelation3D.contributor(0);
        unsigned int mcParticleId = aTrack.relation2D().contributor(0);
        vector<vector<const TCSHit*> > p_mcStTSs;
        //cout<<mcParticleId<<endl;
        // Stereo Layer 0
        // Get TSs for layer
        hits = _cdc.stereoSegmentHits(0);
        nHitsInLayer = (Int_t)hits.size();
        vector<double> tempMcStTSs;
        tempMcStTSs.clear();
        // Find TS which match mc index
        //cout<<"mcID: "<<mcParticleId<<endl;
        for(unsigned j=0; j< hits.size(); j++) {
            //cout<<"[0] Hit["<<j<<"] "<<hits[j]->iMCParticle()<<endl;
            if(hits[j]->iMCParticle() == mcParticleId) {
                tempMcStTSs.push_back((double)hits[j]->cell().localId()/m_nWires[0]*4*m_Trg_PI);
            }
        }
        // Save mc TS to TVectorD
        TVectorD mcSt0TSs(tempMcStTSs.size());
        for(unsigned j=0; j<tempMcStTSs.size(); j++){
            mcSt0TSs[j] = tempMcStTSs[j];
            //cout<<"0: "<<j<<" "<<mcSt0TSs[j]<<endl;
        }
        // Stereo Layer 1
        // Get TSs for layer
        hits = _cdc.stereoSegmentHits(1);
        nHitsInLayer = (Int_t)hits.size();
        tempMcStTSs.clear();
        // Find TS which match mc index
        for(unsigned j=0; j< hits.size(); j++) {
            //cout<<"[1] Hit["<<j<<"] "<<hits[j]->iMCParticle()<<endl;
            if(hits[j]->iMCParticle() == mcParticleId) {
                tempMcStTSs.push_back((double)hits[j]->cell().localId()/m_nWires[1]*4*m_Trg_PI);
            }
        }
        // Save mc TS to TVectorD
        TVectorD mcSt1TSs(tempMcStTSs.size());
        for(unsigned j=0; j<tempMcStTSs.size(); j++){
            mcSt1TSs[j] = tempMcStTSs[j];
            //cout<<"1: "<<j<<" "<<mcSt1TSs[j]<<endl;
        }
        // Stereo Layer 2
        // Get TSs for layer
        hits = _cdc.stereoSegmentHits(2);
        nHitsInLayer = (Int_t)hits.size();
        tempMcStTSs.clear();
        // Find TS which match mc index
        for(unsigned j=0; j< hits.size(); j++) {
            //cout<<"[2] Hit["<<j<<"] "<<hits[j]->iMCParticle()<<endl;
            if(hits[j]->iMCParticle() == mcParticleId) {
                tempMcStTSs.push_back((double)hits[j]->cell().localId()/m_nWires[2]*4*m_Trg_PI);
            }
        }
        // Save mc TS to TVectorD
        TVectorD mcSt2TSs(tempMcStTSs.size());
        for(unsigned j=0; j<tempMcStTSs.size(); j++){
            mcSt2TSs[j] = tempMcStTSs[j];
            //cout<<"2: "<<j<<" "<<mcSt2TSs[j]<<endl;
        }
        // Stereo Layer 3
        // Get TSs for layer
        hits = _cdc.stereoSegmentHits(3);
        nHitsInLayer = (Int_t)hits.size();
        tempMcStTSs.clear();
        // Find TS which match mc index
        for(unsigned j=0; j< hits.size(); j++) {
            //cout<<"[3] Hit["<<j<<"] "<<hits[j]->iMCParticle()<<endl;
            if(hits[j]->iMCParticle() == mcParticleId) {
                tempMcStTSs.push_back((double)hits[j]->cell().localId()/m_nWires[3]*4*m_Trg_PI);
            }
        }
        // Save mc TS to TVectorD
        TVectorD mcSt3TSs(tempMcStTSs.size());
        for(unsigned j=0; j<tempMcStTSs.size(); j++){
            mcSt3TSs[j] = tempMcStTSs[j];
            //cout<<"3: "<<j<<" "<<mcSt3TSs[j]<<endl;
        } 
        // Save values to root
        new(mcSt0TSsTrackHough3D[iTrack]) TVectorD(mcSt0TSs);
        new(mcSt1TSsTrackHough3D[iTrack]) TVectorD(mcSt1TSs);
        new(mcSt2TSsTrackHough3D[iTrack]) TVectorD(mcSt2TSs);
        new(mcSt3TSsTrackHough3D[iTrack]) TVectorD(mcSt3TSs);

        // Save MC true CDC's hitPos
        TVectorD tempStAxialXY(8);
        for(unsigned iTS=0; iTS<mcTSList.size(); iTS++){
            unsigned iCDCSimHit = mcTSList[iTS]->iCDCSimHit();
            CDCSimHit* aCDCSimHit = SimHits[iCDCSimHit];
            TVector3 posWire = aCDCSimHit->getPosWire();
            tempStAxialXY(iTS*2) = posWire.X();
            tempStAxialXY(iTS*2+1) = posWire.Y();
        }
        new(mcStAxXYTrackHough3D[iTrack]) TVectorD(tempStAxialXY);

        // Calculate diff from perfect
        TVectorD deltaWireDiff(4);
        TVectorD fitZ(4);
        for(unsigned iLayer=0; iLayer<4; iLayer++){
            //cout<<"Layer["<<iLayer<<"] deltaPhi: "<<calDeltaPhi(charge, m_anglest[iLayer], m_ztostraw[iLayer], m_rr[iLayer], tempMcTSs[iLayer], rho, fitPhi0)<<" pt: "<<rho*1.5*0.3<<" phi0: "<<fitPhi0<<endl;
            // In case no wire hit in layer
            if(tempMcTSs[iLayer] == 999) {
                deltaWireDiff[iLayer] = 999;
                fitZ[iLayer] = 999;
            }
            else {
                deltaWireDiff[iLayer] = Fitter3DUtility::calDeltaPhi (charge, m_anglest[iLayer], m_ztostraw[iLayer],  m_rr[iLayer], tempMcTSs[iLayer], rho, fitPhi0) / 4 / m_Trg_PI * m_nWires[iLayer];
                fitZ[iLayer] = Fitter3DUtility::calZ (charge, m_anglest[iLayer], m_ztostraw[iLayer],  m_rr[iLayer], tempMcTSs[iLayer], rho, fitPhi0);
            }
        }
        new(deltaWireDiffTrackHough3D[iTrack]) TVectorD(deltaWireDiff);
        new(fitZTrackHough3D[iTrack]) TVectorD(fitZ);

        // Saves Event number and track number
        TVectorD eventTrackID(2);
        eventTrackID[0] = m_eventNum;
        eventTrackID[1] = aTrack.getTrackID();
        new(eventTrackIDTrackHough3D[iTrack]) TVectorD(eventTrackID);

    } // End of loop over all the tracks.

    performanceEventHough3D[0] = nTracks;

    m_treeTrackHough3D->Fill();
    m_treeEventHough3D->Fill();

    TRGDebug::leaveStage("3D finder");
}

void TRGCDCHough3DFinder::perfectFinder(vector<TCTrack *> &trackList, unsigned j, vector<const TCSHit*> &mcTSList){

    //Just a test
    StoreArray<CDCHit> CDCHits("CDCHits");
    StoreArray<CDCSimHit> SimHits("CDCSimHits");
    RelationArray rels(SimHits,CDCHits);

    //...G4 trackID...
    unsigned id = trackList[j]->relation().contributor(0);
    vector<const TCSHit*> tsList[9];
    //cout<<"[JB] id: "<<id<<endl;

    //...Segment loop...
    const vector<const TCSHit*> hits = _cdc.segmentHits();
    for (unsigned i = 0; i < hits.size(); i++) {
        const TCSHit& ts = * hits[i];
        if (ts.segment().axial()) continue;
        if (! ts.signal().active()) continue;
        const TCWHit* wh = ts.segment().center().hit();
        if (! wh) continue;
        const unsigned trackId = wh->iMCParticle();
        // Try to track down the mcParticle another way.
        //cout<<"[CDCTRG] trackId:"<<trackId<<" "<<ts.cell().name()<<endl;
//iw commented out because simind is not used
//    int ind=wh->iCDCHit();
//    int simind=rels[ind].getFromIndex();
        //CDCSimHit &h=*SimHits[simind];
        //cout<<"[CDCTRG] simTrackId: "<<wh->simHit()->getTrackId()<<endl;;
        //cout<<"[CDC]    simTrackId: "<<h.getTrackId()<<" from CDCHit: "<<CDCHits[ind]->getIWire()<<endl;;

        if (id == trackId)
            tsList[wh->wire().superLayerId()].push_back(& ts);
    }

    if (TRGDebug::level()) {
        for (unsigned k = 0; k < 9; k++) {
            if (k % 2) {
                cout << TRGDebug::tab(4) << "superlayer " << k << ":";
                for (unsigned l = 0; l < tsList[k].size(); l++) {
                    if (l)
                        cout << ",";
                    cout << tsList[k][l]->cell().name();
                }
                cout << endl;
            }
        }
    }

    //...Select best one in each super layer...
    for (unsigned i = 0; i < 9; i++) {
        const TCSHit* best = 0;
        if (tsList[i].size() == 0) {
            continue;
        } else if (tsList[i].size() == 1) {
            best = tsList[i][0];
        } else {
            int timeMin = 99999;
            for (unsigned k = 0; k < tsList[i].size(); k++) {
                const TRGSignal& timing = tsList[i][k]->signal();
                const TRGTime& t = * timing[0];
                if (t.time() < timeMin) {
                    timeMin = t.time();
                    best = tsList[i][k];
                }
            }
        }
        mcTSList.push_back(best);
    }

}


void TRGCDCHough3DFinder::doitPerfectly(vector<TCTrack *> & trackList){

    TRGDebug::enterStage("Perfect 3D Finder");
    if (TRGDebug::level())
        cout << TRGDebug::tab() << "givenTrk#=" << trackList.size() << endl;


    //...Track loop....
    for (unsigned j = 0; j < trackList.size(); j++) {
        //...G4 trackID...
        TCTrack* trk = trackList[j];

        vector<const TCSHit*> mcTSList;
        perfectFinder(trackList, j, mcTSList);
        for(unsigned iTS=0; iTS<mcTSList.size(); iTS++){
            trk->append(new TCLink(0, mcTSList[iTS], mcTSList[iTS]->cell().xyPosition()));
        }

        if (TRGDebug::level())
            trk->dump("", "> ");

    }

    TRGDebug::leaveStage("Perfect 3D Finder");

}

}

