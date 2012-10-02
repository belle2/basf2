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
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "generators/dataobjects/MCParticle.h"
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

  TRGCDCHough3DFinder::TRGCDCHough3DFinder(const TRGCDC & TRGCDC)
    : _cdc(TRGCDC) {
      m_Trg_PI = 3.141592653589793;
      // Initialize rr, ztostraw, anglest, nWiresInStereoLayer.
      CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
      for(int stSuperLayer=0;stSuperLayer<4;stSuperLayer++){
        m_rr[stSuperLayer]=cdcp.senseWireR(12*stSuperLayer+10)*0.01;
        m_ztostraw[stSuperLayer]=cdcp.senseWireBZ(12*stSuperLayer+10)*0.01;
        m_anglest[stSuperLayer]=2*m_rr[stSuperLayer]*sin(m_Trg_PI*cdcp.nShifts(12*stSuperLayer+10)/(2*cdcp.nWiresInLayer(12*stSuperLayer+10)))/(cdcp.senseWireFZ(12*stSuperLayer+10)-cdcp.senseWireBZ(12*stSuperLayer+10))/0.01;
        m_nWires[stSuperLayer]=cdcp.nWiresInLayer(12*stSuperLayer+10)*2;
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
      // HoughMesh
      m_houghMeshLayerDiff = new float**[m_nCotSteps];
      m_houghMeshLayer = new bool**[m_nCotSteps];
      m_houghMesh = new int*[m_nCotSteps];
      m_houghMeshDiff = new float*[m_nCotSteps];
      for(int i=0; i<m_nCotSteps; i++){
        m_houghMeshLayerDiff[i] = new float*[m_nZ0Steps];
        m_houghMeshLayer[i] = new bool*[m_nZ0Steps];
        m_houghMesh[i] = new int[m_nZ0Steps];
        m_houghMeshDiff[i] = new float[m_nZ0Steps];
        for(int j=0; j<m_nZ0Steps; j++){
          m_houghMeshLayerDiff[i][j] = new float[4];
          m_houghMeshLayer[i][j] = new bool[4];
        }
      }

      // Save in root.
      m_fileHough3D = new TFile("Hough3D.root","RECREATE");
      m_treeTrackHough3D = new TTree("m_treeTrackHough3D","track");
      m_st0TSsTrackHough3D = new TClonesArray("TVectorD");
      m_st1TSsTrackHough3D = new TClonesArray("TVectorD");
      m_st2TSsTrackHough3D = new TClonesArray("TVectorD");
      m_st3TSsTrackHough3D = new TClonesArray("TVectorD");
      m_fit2DTrackHough3D = new TClonesArray("TVectorD");
      m_bestFitTrackHough3D = new TClonesArray("TVectorD");
      m_mcTrackHough3D = new TClonesArray("TVectorD");
      m_mcStatusTrackHough3D = new TClonesArray("TVectorD");
      m_mcVertexTrackHough3D = new TClonesArray("TVector3");
      m_mc4VectorTrackHough3D = new TClonesArray("TLorentzVector");
      m_bestTSsTrackHough3D = new TClonesArray("TVectorD");
      m_mcTSsTrackHough3D = new TClonesArray("TVectorD");
      m_performanceTrackHough3D = new TClonesArray("TVectorD");
      m_treeTrackHough3D->Branch("st0TSsTrackHough3D", &m_st0TSsTrackHough3D);
      m_treeTrackHough3D->Branch("st1TSsTrackHough3D", &m_st1TSsTrackHough3D);
      m_treeTrackHough3D->Branch("st2TSsTrackHough3D", &m_st2TSsTrackHough3D);
      m_treeTrackHough3D->Branch("st3TSsTrackHough3D", &m_st3TSsTrackHough3D);
      m_treeTrackHough3D->Branch("fit2DTrackHough3D", &m_fit2DTrackHough3D);
      m_treeTrackHough3D->Branch("bestFitTrackHough3D", &m_bestFitTrackHough3D);
      m_treeTrackHough3D->Branch("mcTrackHough3D", &m_mcTrackHough3D);
      m_treeTrackHough3D->Branch("mcStatusTrackHough3D", &m_mcStatusTrackHough3D);
      m_treeTrackHough3D->Branch("mcVertexTrackHough3D", &m_mcVertexTrackHough3D);
      m_treeTrackHough3D->Branch("mc4VectorTrackHough3D", &m_mc4VectorTrackHough3D);
      m_treeTrackHough3D->Branch("bestTSsTrackHough3D", &m_bestTSsTrackHough3D);
      m_treeTrackHough3D->Branch("mcTSsTrackHough3D", &m_mcTSsTrackHough3D);
      m_treeTrackHough3D->Branch("performanceTrackHough3D", &m_performanceTrackHough3D);

      m_treeEventHough3D = new TTree("m_treeEventHough3D","event");
      m_performanceEventHough3D = new TVectorD(1);
      m_treeEventHough3D->Branch("performanceEventHough3D", &m_performanceEventHough3D);

      m_treeConstantsHough3D = new TTree("m_treeConstantsHough3D", "constants");
      m_geometryHough3D = new TVectorD(12);
      m_treeConstantsHough3D->Branch("geometryHough3D", &m_geometryHough3D);

      // Save geometry to root file.
      TVectorD &geometryHough3D = *m_geometryHough3D;
      for(int i=0; i<4; i++){
        geometryHough3D[i] = m_rr[i];
        geometryHough3D[i+4] = m_anglest[i];
        geometryHough3D[i+8] = m_ztostraw[i];
      }

      m_treeConstantsHough3D->Fill();

      m_Hough3DFinder = new Hough3DFinder();
      // cotStart, cotEnd, z0Start, z0End, cotSteps, z0Steps
      float tempInitVariables[] = {-3,3,-2,2,1001,1001};
      vector<float > initVariables(tempInitVariables, tempInitVariables+sizeof(tempInitVariables) / sizeof(tempInitVariables[0]) );
      m_Hough3DFinder->initialize(geometryHough3D, initVariables);
    }

  TRGCDCHough3DFinder::~TRGCDCHough3DFinder(){
    m_Hough3DFinder->destruct();
    // Deallocate HoughMesh
    for(int i=0; i<m_nCotSteps; i++){
      for(int j=0; j<m_nZ0Steps; j++){
        delete [] m_houghMeshLayerDiff[i][j];
        delete [] m_houghMeshLayer[i][j];
      }
      delete [] m_houghMeshLayerDiff[i];
      delete [] m_houghMeshLayer[i];
      delete [] m_houghMesh[i];
      delete [] m_houghMeshDiff[i];
    }
    delete [] m_houghMeshLayerDiff;
    delete [] m_houghMeshLayer;
    delete [] m_houghMesh;
    delete [] m_houghMeshDiff;

    // Deallocate root variables
    delete m_fileHough3D;
    delete m_treeTrackHough3D;
    delete m_st0TSsTrackHough3D;
    delete m_st1TSsTrackHough3D;
    delete m_st2TSsTrackHough3D;
    delete m_st3TSsTrackHough3D;
    delete m_fit2DTrackHough3D;
    delete m_bestFitTrackHough3D;
    delete m_mcTrackHough3D;
    delete m_mcStatusTrackHough3D;
    delete m_mcVertexTrackHough3D;
    delete m_mc4VectorTrackHough3D;
    delete m_bestTSsTrackHough3D;
    delete m_mcTSsTrackHough3D;
    delete m_performanceTrackHough3D;
    delete m_treeEventHough3D;
    delete m_performanceEventHough3D;
    delete m_treeConstantsHough3D;
    delete m_geometryHough3D;

  }

  void TRGCDCHough3DFinder::terminate(void){
    m_fileHough3D->Write();
    m_fileHough3D->Close();
  }

  void TRGCDCHough3DFinder::doit(vector<TCTrack *> & trackList){
    int m_version=1;
    if(m_version==0) doitPerfectly(trackList);
    if(m_version==1) doitVersion1(trackList);
  }

  void TRGCDCHough3DFinder::doitVersion1(vector<TCTrack *> & trackList){
    TRGDebug::enterStage("Hough 3D finder");

    TClonesArray &st0TSsTrackHough3D = *m_st0TSsTrackHough3D;
    TClonesArray &st1TSsTrackHough3D = *m_st1TSsTrackHough3D;
    TClonesArray &st2TSsTrackHough3D = *m_st2TSsTrackHough3D;
    TClonesArray &st3TSsTrackHough3D = *m_st3TSsTrackHough3D;
    TClonesArray &fit2DTrackHough3D = *m_fit2DTrackHough3D;
    TClonesArray &bestFitTrackHough3D = *m_bestFitTrackHough3D;
    TClonesArray &mcTrackHough3D = *m_mcTrackHough3D;
    TClonesArray &mcStatusTrackHough3D = *m_mcStatusTrackHough3D;
    TClonesArray &mcVertexTrackHough3D = *m_mcVertexTrackHough3D;
    TClonesArray &mc4VectorTrackHough3D = *m_mc4VectorTrackHough3D;
    TClonesArray &bestTSsTrackHough3D = *m_bestTSsTrackHough3D;
    TClonesArray &mcTSsTrackHough3D = *m_mcTSsTrackHough3D;
    TClonesArray &performanceTrackHough3D = *m_performanceTrackHough3D;

    TVectorD &performanceEventHough3D = *m_performanceEventHough3D;

    st0TSsTrackHough3D.Clear();
    st1TSsTrackHough3D.Clear();
    st2TSsTrackHough3D.Clear();
    st3TSsTrackHough3D.Clear();
    fit2DTrackHough3D.Clear();
    bestFitTrackHough3D.Clear();
    mcTrackHough3D.Clear();
    mcStatusTrackHough3D.Clear();
    mcVertexTrackHough3D.Clear();
    mc4VectorTrackHough3D.Clear();
    bestTSsTrackHough3D.Clear();
    mcTSsTrackHough3D.Clear();
    performanceTrackHough3D.Clear();

    //performanceEventHough3D.Clear();

    // Generate arrays for TS candidates.
    vector<vector<double> > stTSs;
    vector<vector<const TCSHit *> > p_stTSs;

    vector<const TCSHit *> hits = _cdc.stereoSegmentHits(0);
    Int_t nHitsInLayer = (Int_t)hits.size();
    TVectorD st0TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
      if(hits[j]==0) {cout<<"[0] POINTER IS ZERO"<<endl; continue;}
      st0TSs[j] = (double)hits[j]->cell().localId()/m_nWires[0]*4*m_Trg_PI;
      stTSs[0].push_back(st0TSs[j]);
      p_stTSs[0].push_back(hits[j]);
    }

    hits = _cdc.stereoSegmentHits(1);
    nHitsInLayer = (Int_t)hits.size();
    TVectorD st1TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
      if(hits[j]==0) {cout<<"[1] POINTER IS ZERO"<<endl; continue;}
      st1TSs[j] = (double)hits[j]->cell().localId()/m_nWires[1]*4*m_Trg_PI;
      stTSs[1].push_back(st1TSs[j]);
      p_stTSs[1].push_back(hits[j]);
    }

    hits = _cdc.stereoSegmentHits(2);
    nHitsInLayer = (Int_t)hits.size();
    TVectorD st2TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
      if(hits[j]==0) {cout<<"[2] POINTER IS ZERO"<<endl; continue;}
      st2TSs[j] = (double)hits[j]->cell().localId()/m_nWires[2]*4*m_Trg_PI;
      stTSs[2].push_back(st2TSs[j]);
      p_stTSs[2].push_back(hits[j]);
    }

    hits = _cdc.stereoSegmentHits(3);
    nHitsInLayer = (Int_t)hits.size();
    TVectorD st3TSs(nHitsInLayer);
    stTSs.push_back(vector<double>());
    p_stTSs.push_back(vector<const TCSHit *>());
    for(unsigned j=0; j< hits.size(); j++) {
      if(hits[j]==0) {cout<<"[3] POINTER IS ZERO"<<endl; continue;}
      st3TSs[j] = (double)hits[j]->cell().localId()/m_nWires[3]*4*m_Trg_PI;
      stTSs[3].push_back(st3TSs[j]);
      p_stTSs[3].push_back(hits[j]);
    }

    // Loop over all the tracks.
    unsigned nTracks=trackList.size();
    for(unsigned iTrack=0; iTrack<nTracks; iTrack++){

      // Check if there is a TS in each super layer.
      if(stTSs[0].size()==0) {cout<<"[0] No TS"<<endl; continue;}
      if(stTSs[1].size()==0) {cout<<"[1] No TS"<<endl; continue;}
      if(stTSs[2].size()==0) {cout<<"[2] No TS"<<endl; continue;}
      if(stTSs[3].size()==0) {cout<<"[3] No TS"<<endl; continue;}

      // Get 2D track parameters.
      TCTrack& aTrack = *trackList[iTrack];
      double fitPt = aTrack.pt();
      double rho = fitPt/0.3/1.5;
      double fitPhi0;
      if (aTrack.charge() > 0) fitPhi0 = aTrack.p().phi() - M_PI / 2; 
      if (aTrack.charge() < 0) fitPhi0 = aTrack.p().phi() + M_PI / 2; 
      int charge = int(aTrack.charge());

      // Get MC track parameters.
      const TCRelation& trackRelation = aTrack.relation();
      const MCParticle& trackMCParticle = trackRelation.mcParticle(0);
      //double mcPt = trackMCParticle.getMomentum().Pt();
      //double mcPhi0;
      //if (trackMCParticle.getCharge() > 0) mcPhi0 = trackMCParticle.getMomentum().Phi() - M_PI / 2;
      //if (trackMCParticle.getCharge() < 0) mcPhi0 = trackMCParticle.getMomentum().Phi() + M_PI / 2;
      // Change range to [0,2pi]
      //if (mcPhi0 < 0) mcPhi0 += 2 * M_PI;
      // mcZ0 is not vertex point. It should be impact point.
      double mcZ0 = trackMCParticle.getVertex().Z()/100;
      double mcCot = trackMCParticle.getMomentum().Pz()/trackMCParticle.getMomentum().Pt();

      // Set input of finder
      double tempTrackVariables[] = {charge, rho, fitPhi0};
      vector<double > trackVariables(tempTrackVariables, tempTrackVariables+sizeof(tempTrackVariables) / sizeof(tempTrackVariables[0]) );

      // Run finder
      m_Hough3DFinder->runFinder(trackVariables, stTSs);

      // Get results of finder
      const TCSHit* p_bestTS[4]={0,0,0,0};
      double bestZ0,bestCot,houghMax,minDiffHough;
      double bestTS[4];
      vector<double> tempResult;
      m_Hough3DFinder->getValuesVersion1("bestTSIndex",tempResult);
      for(int iTS=0; iTS<4; iTS++) {
        p_bestTS[iTS] = p_stTSs[iTS][(int)tempResult[iTS]];
        bestTS[iTS] = stTSs[iTS][(int)tempResult[iTS]];
      }
      m_Hough3DFinder->getValuesVersion1("bestZ0", tempResult);
      bestZ0 = tempResult[0];
      m_Hough3DFinder->getValuesVersion1("bestCot", tempResult);
      bestCot = tempResult[0];
      m_Hough3DFinder->getValuesVersion1("houghMax", tempResult);
      houghMax = tempResult[0];
      m_Hough3DFinder->getValuesVersion1("minDiffHough", tempResult);
      minDiffHough = tempResult[0];

      // Find and append TS to track
      for(unsigned iLayer=0; iLayer<4; iLayer++){
        //cout<<bestTS[iLayer]<<endl;
        //cout<<"hough3d["<<iLayer<<"]"<<endl;
        //p_bestTS[iLayer]->dump("state","");
        // This makes it crash sometimes. Why is p_bestTS zero????
        //if(p_bestTS[iLayer]==0){ cout<<"p_bestTS is zero"<<endl; }
        //else {
        aTrack.append(new TCLink(0,p_bestTS[iLayer],p_bestTS[iLayer]->cell().xyPosition()));
        //}
      }
      //aTrack.dump("", "> ");

      // Save values to root
      new(st0TSsTrackHough3D[iTrack]) TVectorD(st0TSs);
      new(st1TSsTrackHough3D[iTrack]) TVectorD(st1TSs);
      new(st2TSsTrackHough3D[iTrack]) TVectorD(st2TSs);
      new(st3TSsTrackHough3D[iTrack]) TVectorD(st3TSs);
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
      // performance values [purity, Track#]
      TVectorD tempPerformanceTrack(2);
      const TCRelation& trackRelation3D = aTrack.relation3D();
      tempPerformanceTrack[0] = trackRelation3D.purity(0);
      tempPerformanceTrack[1] = iTrack;
      new(performanceTrackHough3D[iTrack]) TVectorD(tempPerformanceTrack);
      
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
        //cout<<mcTSList[iTS]->cell().localId()<<" "<<mcTSList[iTS]->cell().superLayerId()<<" ";
      }
      //cout<<endl;
      new(mcTSsTrackHough3D[iTrack]) TVectorD(tempMcTSs);

    } // End of loop over all the tracks.

    performanceEventHough3D[0] = nTracks;

    m_treeTrackHough3D->Fill();
    m_treeEventHough3D->Fill();

    TRGDebug::leaveStage("Hough 3D finder");
  }

  void TRGCDCHough3DFinder::perfectFinder(vector<TCTrack *> &trackList, unsigned j, vector<const TCSHit*> &mcTSList){

    //...G4 trackID...
    unsigned id = trackList[j]->relation().contributor(0);
    vector<const TCSHit*> tsList[9];

    //...Segment loop...
    const vector<const TCSHit*> hits = _cdc.segmentHits();
    for (unsigned i = 0; i < hits.size(); i++) {
      const TCSHit& ts = * hits[i];
      if (ts.segment().axial()) continue;
      if (! ts.timing().active()) continue;
      const TCWHit* wh = ts.segment().center().hit();
      if (! wh) continue;
      const unsigned trackId = wh->iMCParticle();

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
          const TRGSignal& timing = tsList[i][k]->timing();
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

