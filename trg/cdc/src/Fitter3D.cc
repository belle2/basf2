//-----------------------------------------------------------------------------
// $Id$
//-----------------------------------------------------------------------------
// Filename : Fitter3D.cc
// Section  : TRG CDC
// Owner    : KyungTae KIM (K.U.)
// Email    : ktkim@hep.korea.ac.kr
//-----------------------------------------------------------------------------
// Description : A class to fit tracks in 3D
//-----------------------------------------------------------------------------
// $Log$
//-----------------------------------------------------------------------------

#define TRG_SHORT_NAMES
#define TRGCDC_SHORT_NAMES

#include <iostream>
#include "trg/trg/Debug.h"
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/Segment.h"
#include "trg/cdc/Track.h"
#include "trg/cdc/Link.h"
#include <cstdlib>
#include <bitset>

#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
#include "cdc/dataobjects/CDCHit.h"
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
#include "trg/trg/Channel.h"
#include "trg/trg/Utilities.h"
#include "trg/cdc/TRGCDC.h"
#include "trg/cdc/Wire.h"
#include "trg/cdc/Layer.h"
#include "trg/cdc/WireHit.h"
#include "trg/cdc/WireHitMC.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/cdc/TrackMC.h"
#include "trg/cdc/Relation.h"
#include "mdst/dataobjects/MCParticle.h"
#include "trg/cdc/FrontEnd.h"
#include "trg/cdc/Merger.h"
#include "trg/cdc/HoughFinder.h"
#include "trg/cdc/LUT.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/trg/Constants.h"
#include "trg/cdc/Helix.h"
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/EventTime.h"

using namespace std;
namespace Belle2 {

  TRGCDCFitter3D::TRGCDCFitter3D(const std::string & name,
      const std::string & rootFitter3DFile,
      const TRGCDC &TRGCDC,
      const std::map<std::string, bool> &flags)
      : m_name(name), m_cdc(TRGCDC),
       m_mBool(flags), m_rootFitter3DFileName(rootFitter3DFile){
  }

  TRGCDCFitter3D::~TRGCDCFitter3D() {
  }

  void TRGCDCFitter3D::initialize(){
    // [TODO] move to TRGCDC steering file
    m_mBool["fFloatInt"] = 0;
    m_mBool["fNonTSStudy"] = 0;
    m_mBool["fMc"] = 1;

    
    // Init values
    m_mConstD["Trg_PI"] = 3.141592653589793;
    m_mConstV["wirePhi2DError"] = vector<double>  ({0.0085106, 0.0039841, 0.0025806, 0.0019084, 0.001514});
    m_mConstV["driftPhi2DError"] = vector<double>  ({0.0085106, 0.0039841, 0.0025806, 0.0019084, 0.001514});
    m_mConstV["wireZError"] = vector<double>  ({0.0581, 0.0785, 0.0728, 0.0767});
    m_mConstV["driftZError"] = vector<double>  ({0.00388, 0.00538, 0.00650, 0.00842});

    // Get rr,zToStarw,angleSt,nWire
    CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    m_mConstV["rr"] = vector<double> (9);
    m_mConstV["nWires"] = vector<double> (9);
    m_mConstV["zToStraw"] = vector<double> (4);
    m_mConstV["angleSt"] = vector<double> (4);
    for(unsigned iSL=0; iSL<9; iSL++){
      unsigned t_layerId = m_cdc.segment(iSL,0).center().layerId();
      m_mConstV["rr"][iSL] = cdcp.senseWireR(t_layerId);
      m_mConstV["nWires"][iSL] = cdcp.nWiresInLayer(t_layerId)*2;
    }
    for(int iSt=0;iSt<4;iSt++){
      unsigned t_layerId = m_cdc.stereoSegment(iSt,0).center().layerId();
      m_mConstV["zToStraw"][iSt]=cdcp.senseWireBZ(t_layerId);
      m_mConstV["angleSt"][iSt]=2*m_mConstV["rr"][2*iSt+1]*sin(m_mConstD["Trg_PI"]*cdcp.nShifts(t_layerId)/(2*cdcp.nWiresInLayer(t_layerId)))/(cdcp.senseWireFZ(t_layerId)-cdcp.senseWireBZ(t_layerId));
    }
    m_mConstV["rr2D"] = vector<double> ({m_mConstV["rr"][0], m_mConstV["rr"][2], m_mConstV["rr"][4], m_mConstV["rr"][6], m_mConstV["rr"][8]}); 
    m_mConstV["rr3D"] = vector<double> ({m_mConstV["rr"][1], m_mConstV["rr"][3], m_mConstV["rr"][5], m_mConstV["rr"][7]}); 
    

    // [TODO] initialize LUT values
  
  }

  int TRGCDCFitter3D::doit(const std::vector<TRGCDCTrack *> & trackListIn,
             std::vector<TRGCDCTrack *> & trackListOut){

    TRGDebug::enterStage("Fitter 3D");


    if(m_mBool["fEvtTime"]==1) m_mDouble["eventTime"]=0;
    else m_mDouble["eventTime"] = m_cdc.getEventTime();
    if(m_mBool["fRootFile"]) m_mDouble["iSave"] = 0;

    // Loop over all tracks
    for(unsigned iTrack=0; iTrack<trackListIn.size(); iTrack++){

      if(m_mBool["fRootFile"]==1 && m_mDouble["iSave"]==0) initializeRoot("event");

      TCTrack & aTrack = * trackListIn[iTrack];

      ///////////////////////////////////////
      // Check if all superlayers have one TS
      bool trackFull=1;
      for (unsigned iSL = 0; iSL < m_cdc.nSuperLayers(); iSL++) {
        // Check if all superlayers have one TS
        const vector<TCLink *> & links = aTrack.links(iSL);
        const unsigned nSegments = links.size();
        if(nSegments != 1) {
          if (nSegments == 0){
            trackFull = 0;
            //cout<<"Fitter3D::doit() => Not enough TS."<<endl;
          } else {
            cout<<"Fitter3D::doit() => multiple TS are assigned."<<endl;
          }
        }
      } // End superlayer loop
      if(trackFull == 0){
         TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5,0), CLHEP::HepSymMatrix(5,0));
         CLHEP::HepVector helixParameters(5);
         helixParameters = aTrack.helix().a();
         aTrack.setFitted(0);
         aTrack.setHelix(helix);
         trackListOut.push_back(&aTrack);
         continue;
      }

      /////////////////////////////////
      // Get input values for 3D fitter
      // Get event and track ID
      m_mDouble["eventNumber"] = m_cdc.getEventNumber();
      m_mDouble["trackId"] = aTrack.getTrackID();
      // Get MC values for 3D fitter
      if(m_mBool["fMc"]) getMCValues(&aTrack);
      // Get wirePhi, LR, mcLR, Drift information
      m_mVector["wirePhi"] = vector<double> (9);
      m_mVector["lutLR"] = vector<double> (9);
      m_mVector["LR"] = vector<double> (9);
      m_mVector["driftLength"] = vector<double> (9);
      for (unsigned iSL = 0; iSL < 9; iSL++) {
        const vector<TCLink *> & links = aTrack.links(iSL);
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
        m_mVector["wirePhi"][iSL] = (double) t_segment->localId()/m_mConstV["nWires"][iSL]*4*m_mConstD["Trg_PI"];
//        m_mVector["lutLR"][iSL] = t_segment->LUT()->getLRLUT(t_segment->hitPattern(),iSL);
        m_mVector["lutLR"][iSL] = t_segment->LUT()->getValue(t_segment->hitPattern());
        m_mVector["mcLR"][iSL] = t_segment->hit()->mcLR();
        m_mVector["driftLength"][iSL] = t_segment->hit()->drift();
        if(m_mBool["fmcLR"]==1) m_mVector["LR"][iSL] = m_mVector["mcLR"][iSL];
        else if(m_mBool["fLRLUT"]==1) m_mVector["LR"][iSL] = m_mVector["lutLR"][iSL];
        else m_mVector["LR"][iSL] = 3;
      } // End superlayer loop

      ////////////////////
      // Get 2D fit values
      // Get 2D fit values from IW 2D fitter
      m_mDouble["phi02D"] = aTrack.helix().phi0();
      m_mDouble["pt2D"] = aTrack.helix().curv()*0.01*0.3*1.5;
      if(aTrack.charge()<0) {
        m_mDouble["phi02D"] -= m_mConstD["Trg_PI"];
        if (m_mDouble["phi02D"] < 0) m_mDouble["phi02D"] += 2 * m_mConstD["Trg_PI"];
        m_mDouble["pt2D"] = m_mDouble["pt2D"] * -1;
      }
      m_mDouble["dr2D"] = aTrack.helix().dr()*0.01;
      // Get 2D fit values from JB 2D fitter
      // Currently using JB fitter for 3D fitting
      m_mDouble["charge"] = double(aTrack.charge());
      // Set phi2DError for 2D fit
      m_mVector["phi2DError"] = vector<double> (5);
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        if(m_mVector["LR"][2*iAx] != 3) m_mVector["phi2DError"][iAx] = m_mConstV["driftPhi2DError"][iAx];
        else m_mVector["phi2DError"][iAx] = m_mConstV["wirePhi2DError"][iAx];
      }
      // Calculate phi2D using driftTime.
      m_mVector["phi2D"] = vector<double> (5);
      for (unsigned iAx = 0; iAx < 5; iAx++) m_mVector["phi2D"][iAx] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iAx*2], m_mVector["driftLength"][iAx*2], m_mDouble["eventTime"], m_mConstV["rr"][iAx*2], m_mVector["LR"][iAx*2]);
      // Fit2D
      m_mDouble["rho"] = 0;
      m_mDouble["phi0"] = 0;
      Fitter3DUtility::rPhiFit(&m_mConstV["rr2D"][0],&m_mVector["phi2D"][0],&m_mVector["phi2DError"][0],m_mDouble["rho"], m_mDouble["phi0"]); 
      m_mDouble["pt"] = 0.3*1.5*m_mDouble["rho"]/100;


      //////////////////////
      // Start of 3D fitter
      if( m_mBool["fFloatInt"] == 0) {
        ////////////////////
        // Float 3D fitter
        // Calculate phi3D using driftTime.
        m_mVector["phi3D"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["phi3D"][iSt] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iSt*2+1], m_mVector["driftLength"][iSt*2+1], m_mDouble["eventTime"], m_mConstV["rr3D"][iSt], m_mVector["LR"][iSt*2+1]);
        // Calculate zz
        m_mVector["zz"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge"], m_mConstV["angleSt"][iSt], m_mConstV["zToStraw"][iSt], m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
        // Calculate arcS
        m_mVector["arcS"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["arcS"][iSt] = Fitter3DUtility::calS(m_mDouble["rho"], m_mConstV["rr3D"][iSt]);
        // Get zerror for 3D fit
        m_mVector["zError"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          if(m_mVector["LR"][2*iSt+1] != 2) m_mVector["zError"][iSt] = m_mConstV["driftZError"][iSt];
          else m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
        }
        // Get inverse zerror ^ 2
        m_mVector["iZError2"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["iZError2"][iSt] = 1/pow(m_mVector["zError"][iSt],2);
        // Fit3D
        m_mDouble["z0"] = 0;
        m_mDouble["cot"] = 0;
        m_mDouble["zChi2"] = 0;
        Fitter3DUtility::rSFit(&m_mVector["iZError2"][0], &m_mVector["arcS"][0], &m_mVector["zz"][0], m_mDouble["z0"], m_mDouble["cot"], m_mDouble["zChi2"]);
        // Change to deg
        m_mDouble["theta"] = m_mConstD["Trg_PI"]/2 - atan(m_mDouble["cot"]);
        m_mDouble["theta"] = 180 / m_mConstD["Trg_PI"];
      } else {
        ////////////////////////
        // [TODO] Int 3D fitter
        // Calculate phi3D using driftTime.
        // Calculate zz
        // Calculate arcS
        // Fit3D
        // Change to deg
      }

      ///////////////
      // Save values
      if(m_mBool["fRootFile"]) {
        if(m_fileFitter3D==0) initializeRoot();
        saveTrackValues();
      }

    } // End track loop

    // Save values to file
    if(m_mBool["fRootFile"]) m_treeTrackFitter3D->Fill();

    TRGDebug::leaveStage("Fitter 3D");
    return 1;

  };

  /////////////////////////////////////////////////////////////////////////////////////////////
  ////// Function for saving
  /////////////////////////////////////////////////////////////////////////////////////////////
  // Functions for saving.
  void TRGCDCFitter3D::initializeRoot(string type){

    if(type == ""){
      // Add file
      m_fileFitter3D = new TFile(m_rootFitter3DFileName.c_str(),"RECREATE");

      // Add Track tree
      m_treeTrackFitter3D = new TTree("m_treeTrackFitter3D","track");
      // Make storage for saving m_mDouble
      for(map<string, double>::iterator it=m_mDouble.begin(); it!=m_mDouble.end(); it++){
        m_mTClonesArray[(*it).first+"TrackFitter3D"] = new TClonesArray("TVectorD");
      }
      // Make storage for saving m_mVector
      for(map<string, vector<double> >::iterator it=m_mVector.begin(); it!=m_mVector.end(); it++){
        m_mTClonesArray[(*it).first+"TrackFitter3D"] = new TClonesArray("TVectorD");
      }
      // Adding branch
      for(map<string,TClonesArray*>::iterator it=m_mTClonesArray.begin(); it!=m_mTClonesArray.end(); it++){
        m_treeTrackFitter3D->Branch(it->first.c_str(),&it->second, 32000,0);
      }

      // Add Constant tree
      m_treeConstantsFitter3D = new TTree("m_treeConstantsFitter3D","constants");
      // Make storage for saving m_mConstD. And fill it.
      for(map<string, double>::iterator it=m_mConstD.begin(); it!=m_mConstD.end(); it++){
        m_mTVectorD[(*it).first+"Fitter3D"] = new TVectorD(1, &(*it).second);
      }
      // Make storage for saving m_mConstV. And fill it.
      for(map<string, vector<double> >::iterator it=m_mConstV.begin(); it!=m_mConstV.end(); it++){
        int t_vectorSize = (*it).second.size();
        m_mTVectorD[(*it).first+"Fitter3D"] = new TVectorD(t_vectorSize, &((*it).second)[0]);
      }
      // Adding branch
      for(map<string,TVectorD*>::iterator it=m_mTVectorD.begin(); it!=m_mTVectorD.end(); it++){
        m_treeConstantsFitter3D->Branch(it->first.c_str(),&it->second, 32000,0);
      }
      // Fill tree
      m_treeConstantsFitter3D->Fill();

    } else if(type == "event"){
      for(map<string,TClonesArray*>::iterator it=m_mTClonesArray.begin(); it!=m_mTClonesArray.end(); it++){
        it->second->Clear();
      }
    } else{
      cout<<"Error in Fitter3D::initalizationRoot"<<endl;
    }
  }

  void TRGCDCFitter3D::saveTrackValues() {

    // Save m_mDouble
    for(map<string, double >::iterator it=m_mDouble.begin(); it!=m_mDouble.end(); it++){
      new((*m_mTClonesArray[(*it).first+"TrackFitter3D"])[m_mDouble["iSave"]]) TVectorD(1, &(*it).second);
    }
    // Save m_mVector
    for(map<string, vector<double> >::iterator it=m_mVector.begin(); it!=m_mVector.end(); it++){
      unsigned t_vectorSize = it->second.size();
      new((*m_mTClonesArray[(*it).first+"TrackFitter3D"])[m_mDouble["iSave"]]) TVectorD(t_vectorSize, &((*it).second)[0]);
    }

    m_mDouble["iSave"]++;

  }


  void TRGCDCFitter3D::getMCValues( TRGCDCTrack* aTrack ){
    // Access to track's MC particle.
    const TCRelation & trackRelation = aTrack->relation();
    // Biggest contibutor is 0. Next is 1 and so on.
    const MCParticle & trackMCParticle = trackRelation.mcParticle(0);
    // Access track's particle parameters
    m_mDouble["mcPt"] = trackMCParticle.getMomentum().Pt();
    m_mDouble["mcPhi0"] = 0;
    if(trackMCParticle.getCharge()>0) m_mDouble["mcPhi0"]=trackMCParticle.getMomentum().Phi()-m_mConstD["Trg_PI"]/2;
    if(trackMCParticle.getCharge()<0) m_mDouble["mcPhi0"]=trackMCParticle.getMomentum().Phi()+m_mConstD["Trg_PI"]/2;
    // Change range to [0,2pi]
    if(m_mDouble["mcPhi0"]<0) m_mDouble["mcPhi0"]+=2*m_mConstD["Trg_PI"];
    m_mDouble["mcZ0"] = trackMCParticle.getVertex().Z();
    m_mDouble["mcCot"]=trackMCParticle.getMomentum().Pz()/trackMCParticle.getMomentum().Pt();
    m_mDouble["mcCharge"] = trackMCParticle.getCharge();

    // mcStatus[0]: statusbit, mcStatus[1]: pdg, mcStatus[2]: charge
    TVectorD mcStatus(3);
    m_mDouble["mcStatus"] = trackMCParticle.getStatus();
    m_mDouble["pdgId"] = trackMCParticle.getPDG();

    TVector3 vertex = trackMCParticle.getVertex();
    TLorentzVector vector4 = trackMCParticle.get4Vector();
    TVector2 helixCenter;
    TVector3 impactPosition;
    Fitter3DUtility::findImpactPosition(&vertex, &vector4, int(m_mDouble["mcCharge"]), helixCenter, impactPosition);
    m_mVector["mcVertex"] = vector<double> ({vertex.X(), vertex.Y(), vertex.Z()});
    m_mVector["mcMomentum"] = vector<double> ({vector4.Px(), vector4.Py(), vector4.Pz()});
    m_mVector["helixCenter"] = vector<double> ({helixCenter.X(), helixCenter.Y()});
    m_mVector["impactPosition"] = vector<double> ({impactPosition.X(), impactPosition.Y(), impactPosition.Z()});

    // Find position of track for each super layer
    //...G4 trackID...
    unsigned id = trackRelation.contributor(0);
    vector<const TCSHit*> mcAllTSList[9];
    vector<const TCSHit*> mcTSList(9);
    //...Segment loop...
    const vector<const TCSHit*> hits = m_cdc.segmentHits();
    for (unsigned i = 0; i < hits.size(); i++) {
      const TCSHit& ts = * hits[i];
      if (! ts.signal().active()) continue;
      const TCWHit* wh = ts.segment().center().hit();
      if (! wh) continue;
      const unsigned trackId = wh->iMCParticle();
      if (id == trackId)
        mcAllTSList[wh->wire().superLayerId()].push_back(& ts);
    }
    //...Select best one in each super layer...
    for (unsigned i = 0; i < 9; i++) {
      const TCSHit* best = 0;
      if (mcAllTSList[i].size() == 0) {
      } else if (mcAllTSList[i].size() == 1) {
        best = mcAllTSList[i][0];
      } else {
        int timeMin = 99999;
        for (unsigned k = 0; k < mcAllTSList[i].size(); k++) {
          const TRGSignal& timing = mcAllTSList[i][k]->signal();
          const TRGTime& t = * timing[0];
          if (t.time() < timeMin) {
            timeMin = t.time();
            best = mcAllTSList[i][k];
          }
        }
      }
      mcTSList[i] = best;
    }
    // Get mc track positions. Unit is cm.
    m_mVector["mcPosX"] = vector<double> ({9999,9999,9999,9999,9999,9999,9999,9999,9999});
    m_mVector["mcPosY"] = vector<double> ({9999,9999,9999,9999,9999,9999,9999,9999,9999});
    m_mVector["mcPosZ"] = vector<double> ({9999,9999,9999,9999,9999,9999,9999,9999,9999});
    for(unsigned iSL=0; iSL<9; iSL++){
      if(mcTSList[iSL]!=0) {
        TVector3 posTrack = mcTSList[iSL]->simHit()->getPosTrack();
        m_mVector["mcPosX"][iSL] = posTrack.X();
        m_mVector["mcPosY"][iSL] = posTrack.X();
        m_mVector["mcPosZ"][iSL] = posTrack.X();
      }
    }
    // Get mc LR
    m_mVector["mcLR"] = vector<double> (9);
    for(unsigned iSL=0; iSL<9; iSL++){
      if(mcTSList[iSL]!=0) {
        m_mVector["mcLR"][iSL] = mcTSList[iSL]->simHit()->getPosFlag();
      }
    }

    // Find helix parameters and mc position at R
    for(unsigned iSL=0; iSL<9; iSL++){
      if(mcTSList[iSL]!=0) {
        TVectorD t_helixParameters; 
        TVector3 t_positionAtR;
        TVector3 t_momentumAtR;
        Fitter3DUtility::calHelixParameters(mcTSList[iSL]->simHit()->getPosIn(), mcTSList[iSL]->simHit()->getMomentum(),trackMCParticle.getCharge(),t_helixParameters);
        //cout<<"dr: "<<helixParameters[0]<<" phi0: "<<helixParameters[1]<<" R: "<<1/helixParameters[2]/0.3/1.5*100<<" dz: "<<helixParameters[3]<<" tanLambda: "<<helixParameters[4]<<endl;
        //calVectorsAtR(t_helixParameters, trackMCParticle.getCharge(), m_rr[iSL]*100, t_positionAtR, t_momentumAtR);
        //cout<<" x: "<<t_positionAtR.X()<<"  y: "<<t_positionAtR.Y()<<"  z: "<<t_positionAtR.Z()<<endl;
        //cout<<"Px: "<<t_momentumAtR.X()<<" Py: "<<t_momentumAtR.Y()<<" Pz: "<<t_momentumAtR.Z()<<endl;
      }
    }
  }


  void TRGCDCFitter3D::terminate(){
    if(m_mBool["fRootFile"]) terminateRoot();
  }

  void TRGCDCFitter3D::terminateRoot(){
      m_fileFitter3D->Write();
      m_fileFitter3D->Close();
  }

  string TRGCDCFitter3D::version(void) const {
    return string("TRGCDCFitter3D 6.0");
  }

  std::string TRGCDCFitter3D::name(void) const {
    return m_name;
  }

} // namespace Belle2
