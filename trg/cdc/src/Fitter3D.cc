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
#include <iomanip>

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
#include "trg/cdc/LUT.h"
#include "trg/cdc/SegmentHit.h"
#include "trg/trg/Constants.h"
#include "trg/cdc/Helix.h"
#include "trg/cdc/Fitter3DUtility.h"
#include "trg/cdc/EventTime.h"
#include "trg/cdc/JSignal.h"
#include "trg/cdc/JLUT.h"
#include "trg/cdc/JSignalData.h"
#include "trg/cdc/FpgaUtility.h"
#include "trg/cdc/HandleRoot.h"

using namespace std;
namespace Belle2 {

  TRGCDCFitter3D::TRGCDCFitter3D(const std::string & name,
      const std::string & rootFitter3DFile,
      const TRGCDC &TRGCDC,
      const std::map<std::string, bool> &flags)
      : m_name(name), m_cdc(TRGCDC),
       m_mBool(flags), m_rootFitter3DFileName(rootFitter3DFile){
         m_fileFitter3D = 0;
         m_commonData = 0;
  }

  TRGCDCFitter3D::~TRGCDCFitter3D() {
  }

  void TRGCDCFitter3D::initialize(){
    // If we are using Monte Carlo information.
    m_mBool["fMc"] = 1;
    m_mBool["fVerbose"] = 0;
    m_mBool["fIsPrintError"] = 0;
    m_mBool["fIsIntegerEffect"] = 1;
    // Init values
    m_mConstD["Trg_PI"] = 3.141592653589793;

    // Get rr,zToStraw,angleSt,nWire
    CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    m_mConstV["rr"] = vector<double> (9);
    m_mConstV["nWires"] = vector<double> (9);
    for(unsigned iSL=0; iSL<9; iSL++){
      unsigned t_layerId = m_cdc.segment(iSL,0).center().layerId();
      m_mConstV["rr"][iSL] = cdcp.senseWireR(t_layerId);
      m_mConstV["nWires"][iSL] = cdcp.nWiresInLayer(t_layerId)*2;
    }

    m_mConstV["zToStraw"] = vector<double> (4);
    m_mConstV["zToOppositeStraw"] = vector<double> (4);
    m_mConstV["angleSt"] = vector<double> (4);
    m_mConstV["nShift"] = vector<double> (4);
    for(int iSt=0;iSt<4;iSt++){
      unsigned t_layerId = m_cdc.stereoSegment(iSt,0).center().layerId();
      m_mConstV["zToStraw"][iSt] = cdcp.senseWireBZ(t_layerId);
      m_mConstV["zToOppositeStraw"][iSt] = cdcp.senseWireFZ(t_layerId);
      m_mConstV["angleSt"][iSt] = 2*m_mConstV["rr"][2*iSt+1]*sin(m_mConstD["Trg_PI"]*cdcp.nShifts(t_layerId)/(2*cdcp.nWiresInLayer(t_layerId)))/(cdcp.senseWireFZ(t_layerId)-cdcp.senseWireBZ(t_layerId));
      m_mConstV["nShift"][iSt] = cdcp.nShifts(t_layerId);
    }

    m_mConstV["rr2D"] = vector<double> (5);
    m_mConstV["rr3D"] = vector<double> (4);
    for(int iAx=0; iAx<5; iAx++) m_mConstV["rr2D"][iAx] = m_mConstV["rr"][iAx*2];
    for(int iSt=0; iSt<4; iSt++) m_mConstV["rr3D"][iSt] = m_mConstV["rr"][iSt*2+1];

    m_mConstV["wirePhi2DError"] = vector<double> (5);
    m_mConstV["driftPhi2DError"] = vector<double> (5);
    //// Ideal case of phi2DError. Not used. Fit phi_c becomes strange.
    //m_mConstD["driftResolution"] = 2 * 40 * 0.0001; // (cm)
    //m_mConstV["cellResolution"] = vector<double> (5);
    //for(unsigned iAx=0; iAx<5; iAx++){
    //  m_mConstV["cellResolution"][iAx] = m_mConstV["rr2D"][iAx] * 2 * m_mConstD["Trg_PI"] / (m_mConstV["nWires"][iAx*2]/2); // (cm)
    //}
    //for(unsigned iAx=0; iAx<5; iAx++){
    //  m_mConstV["wirePhi2DError"][iAx] = m_mConstV["cellResolution"][iAx]/m_mConstV["rr2D"][iAx] / sqrt(12);
    //}
    //for(unsigned iAx=0; iAx<5; iAx++){
    //  m_mConstV["driftPhi2DError"][iAx] = m_mConstD["driftResolution"]/m_mConstV["rr2D"][iAx] / sqrt(12);
    //}

    m_mConstV["wirePhi2DError"][0] = 0.00085106;
    m_mConstV["wirePhi2DError"][1] = 0.00039841;
    m_mConstV["wirePhi2DError"][2] = 0.00025806;
    m_mConstV["wirePhi2DError"][3] = 0.00019084; 
    m_mConstV["wirePhi2DError"][4] = 0.0001514;
    m_mConstV["driftPhi2DError"][0] = 0.00085106;
    m_mConstV["driftPhi2DError"][1] = 0.00039841;
    m_mConstV["driftPhi2DError"][2] = 0.00025806;
    m_mConstV["driftPhi2DError"][3] = 0.00019084;
    m_mConstV["driftPhi2DError"][4] = 0.0001514;

    // KT study
    //m_mConstV["wireZError"] = vector<double>  ({0.0581, 0.0785, 0.0728, 0.0767});
    //m_mConstV["driftZError"] = vector<double>  ({0.00388, 0.00538, 0.00650, 0.00842});
    // Orginal study
    m_mConstV["wireZError"] = vector<double> (4);
    m_mConstV["wireZError"][0] = 3.19263;
    m_mConstV["wireZError"][1] = 2.8765;
    m_mConstV["wireZError"][2] = 2.90057;
    m_mConstV["wireZError"][3] = 3.96206;
    m_mConstV["driftZError"] = vector<double> (4);
    m_mConstV["driftZError"][0] = 3.19263; 
    m_mConstV["driftZError"][1] = 2.8765; 
    m_mConstV["driftZError"][2] = 2.90057;
    m_mConstV["driftZError"][3] = 3.96206;
    

    if(m_mBool["fVerbose"]) {
      cout<<"fLRLUT:       "<<m_mBool["fLRLUT"]<<endl;
      cout<<"fEvtTime:     "<<m_mBool["fEvtTime"]<<endl;
      cout<<"fMc:          "<<m_mBool["fMc"]<<endl;
      cout<<"fVerbose:     "<<m_mBool["fVerbose"]<<endl;
      cout<<"fmcLR:        "<<m_mBool["fmcLR"]<<endl;
      cout<<"fRoot:        "<<m_mBool["fRootFile"]<<endl;
      cout<<"PI:           "<<m_mConstD["Trg_PI"]<<endl;
      cout<<"rr:           "<<m_mConstV["rr"][0]<<" "<<m_mConstV["rr"][1]<<" "<<m_mConstV["rr"][2]<<" "<<m_mConstV["rr"][3]<<" "<<m_mConstV["rr"][4]<<" "<<m_mConstV["rr"][5]<<" "<<m_mConstV["rr"][6]<<" "<<m_mConstV["rr"][7]<<" "<<m_mConstV["rr"][8]<<endl;
      cout<<"nWires:       "<<int(m_mConstV["nWires"][0])<<" "<<int(m_mConstV["nWires"][1])<<" "<<int(m_mConstV["nWires"][2])<<" "<<int(m_mConstV["nWires"][3])<<" "<<int(m_mConstV["nWires"][4])<<" "<<int(m_mConstV["nWires"][5])<<" "<<int(m_mConstV["nWires"][6])<<" "<<int(m_mConstV["nWires"][7])<<" "<<int(m_mConstV["nWires"][8])<<endl;
      cout<<"zToStraw:     "<<m_mConstV["zToStraw"][0]<<" "<<m_mConstV["zToStraw"][1]<<" "<<m_mConstV["zToStraw"][2]<<" "<<m_mConstV["zToStraw"][3]<<endl;
      cout<<"angleSt:      "<<m_mConstV["angleSt"][0]<<" "<<m_mConstV["angleSt"][1]<<" "<<m_mConstV["angleSt"][2]<<" "<<m_mConstV["angleSt"][3]<<endl;
      cout<<"wireZError:   "<<m_mConstV["wireZError"][0]<<" "<<m_mConstV["wireZError"][1]<<" "<<m_mConstV["wireZError"][2]<<" "<<m_mConstV["wireZError"][3]<<endl;
      cout<<"driftZError:  "<<m_mConstV["driftZError"][0]<<" "<<m_mConstV["driftZError"][1]<<" "<<m_mConstV["driftZError"][2]<<" "<<m_mConstV["driftZError"][3]<<endl;
      cout<<"wirePhiError: "<<m_mConstV["wirePhi2DError"][0]<<" "<<m_mConstV["wirePhi2DError"][1]<<" "<<m_mConstV["wirePhi2DError"][2]<<" "<<m_mConstV["wirePhi2DError"][3]<<" "<<m_mConstV["wirePhi2DError"][4]<<endl;
      cout<<"driftPhiError: "<<m_mConstV["driftPhi2DError"][0]<<" "<<m_mConstV["driftPhi2DError"][1]<<" "<<m_mConstV["driftPhi2DError"][2]<<" "<<m_mConstV["driftPhi2DError"][3]<<" "<<m_mConstV["driftPhi2DError"][4]<<endl;
    }
    

  }

  int TRGCDCFitter3D::doit(std::vector<TRGCDCTrack *> & trackList)
  {

    TRGDebug::enterStage("Fitter 3D");

    // Set values for saving data.
    if(m_mBool["fRootFile"]) m_mDouble["iSave"] = 0;
    if(m_mBool["fRootFile"]) {
      HandleRoot::initializeEvent(m_mTClonesArray);
    }

    // Get common values for event.
    if(m_mBool["fEvtTime"]==0) m_mDouble["eventTime"]=0;
    else m_mDouble["eventTime"] = m_cdc.getEventTime();


    // Fitter3D
    // Loop over all tracks
    for(unsigned iTrack=0; iTrack<trackList.size(); iTrack++){

      TCTrack & aTrack = * trackList[iTrack];

      ///////////////////////////////////////
      // Check if all superlayers have one TS for the track.
      //bool trackFull= isAxialTrackFull(aTrack) && isStereoTrackFull(aTrack);
      //if(trackFull == 0){
      //   aTrack.setFitted(0);
      //   continue;
      //}

      // Get MC values for fitter
      if(m_mBool["fMc"]) getMCValues(m_cdc, &aTrack, m_mConstD, m_mDouble, m_mVector);
      // Get event and track ID
      m_mDouble["eventNumber"] = m_cdc.getEventNumber();
      m_mDouble["trackId"] = aTrack.getTrackID();


      ///////////////////////////////////
      //// 2D Fitter
      int fit2DResult = do2DFit(aTrack, m_mBool, m_mConstD, m_mConstV, m_mDouble, m_mVector);
      if (fit2DResult != 0) continue;

      /////////////////////////////////
      // 3D Fitter
      // Check which stereo super layers should be used.
      bool useStSl[4];
      findHitStereoSuperlayers(aTrack, useStSl, m_mBool["fIsPrintError"]);
      removeImpossibleStereoSuperlayers(useStSl);
      // Check if number of stereo super layer hits is smaller or equal to 1.
      int nHitSl = (int)useStSl[0]+(int)useStSl[1]+(int)useStSl[2]+(int)useStSl[3];
      if(nHitSl <= 1) {
        aTrack.setFitted(0);
        continue;
      }

      // Fill information for stereo layers
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if(useStSl[iSt] == 1) {
          const vector<TCLink *> & links = aTrack.links(iSt*2+1);
          const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
          m_mVector["wirePhi"][iSt*2+1] = (double) t_segment->localId()/m_mConstV["nWires"][iSt*2+1]*4*m_mConstD["Trg_PI"];
          //m_mVector["lutLR"][iSL] = t_segment->LUT()->getLRLUT(t_segment->hitPattern(),iSL);
          m_mVector["lutLR"][iSt*2+1] = t_segment->LUT()->getValue(t_segment->lutPattern());
          // mcLR should be removed.
          m_mVector["mcLR"][iSt*2+1] = t_segment->hit()->mcLR();
          m_mVector["driftLength"][iSt*2+1] = t_segment->hit()->drift();
          m_mVector["tdc"][iSt*2+1] = t_segment->priorityTime();
          if(m_mBool["fmcLR"]==1) m_mVector["LR"][iSt*2+1] = m_mVector["mcLR"][iSt*2+1];
          else if(m_mBool["fLRLUT"]==1) m_mVector["LR"][iSt*2+1] = m_mVector["lutLR"][iSt*2+1];
          else m_mVector["LR"][iSt*2+1] = 3;
        } else {
          m_mVector["wirePhi"][iSt*2+1] = 9999;
          m_mVector["lutLR"][iSt*2+1] = 9999;
          // mcLR should be removed.
          m_mVector["mcLR"][iSt*2+1] = 9999;
          m_mVector["driftLength"][iSt*2+1] = 9999;
          m_mVector["tdc"][iSt*2+1] = 9999;
          if(m_mBool["fmcLR"]==1) m_mVector["LR"][iSt*2+1] = 9999;
          else if(m_mBool["fLRLUT"]==1) m_mVector["LR"][iSt*2+1] = 9999;
          else m_mVector["LR"][iSt*2+1] = 9999;
        }
      } // End superlayer loop


      //////////////////////
      // Start of 3D fitter
      // Calculate phi3D using driftTime.
      m_mVector["phi3D"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if(useStSl[iSt] == 1) {
          m_mVector["phi3D"][iSt] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iSt*2+1], m_mVector["tdc"][iSt*2+1], m_mDouble["eventTime"], m_mConstV["rr3D"][iSt], m_mVector["LR"][iSt*2+1]);
        } else {
          m_mVector["phi3D"][iSt] = 9999;
        }
      }
      // Get zerror for 3D fit
      m_mVector["zError"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if(useStSl[iSt] == 1) {
          if(m_mVector["LR"][2*iSt+1] != 2) m_mVector["zError"][iSt] = m_mConstV["driftZError"][iSt];
          else m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
        } else {
          m_mVector["zError"][iSt] = 9999;
        }
      }
      // Get inverse zerror ^ 2
      m_mVector["iZError2"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if(useStSl[iSt] == 1) {
          m_mVector["iZError2"][iSt] = 1/pow(m_mVector["zError"][iSt],2);
        } else {
          m_mVector["iZError2"][iSt] = 0;
        }
      }

      //Reject low pt. (Should not happen because 2D is requiring all TS hits.)
      //if(m_mDouble["rho"] > 67){

        // Calculate zz
        m_mVector["zz"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          if(useStSl[iSt] == 1) {
            m_mVector["zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge"], m_mConstV["angleSt"][iSt], m_mConstV["zToStraw"][iSt], m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
          } else {
            m_mVector["zz"][iSt] = 0;
          }
        }
        // Calculate arcS
        m_mVector["arcS"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          if(useStSl[iSt] == 1) {
            m_mVector["arcS"][iSt] = Fitter3DUtility::calS(m_mDouble["rho"], m_mConstV["rr3D"][iSt]);
          } else {
            m_mVector["arcS"][iSt] = 0; 
          }
        }
        // Fit3D
        m_mDouble["z0"] = 0;
        m_mDouble["cot"] = 0;
        m_mDouble["zChi2"] = 0;
        Fitter3DUtility::rSFit(&m_mVector["iZError2"][0], &m_mVector["arcS"][0], &m_mVector["zz"][0], m_mDouble["z0"], m_mDouble["cot"], m_mDouble["zChi2"]);
        // Change to deg
        m_mDouble["theta"] = m_mConstD["Trg_PI"]/2 - atan(m_mDouble["cot"]);
        m_mDouble["theta"] = 180 / m_mConstD["Trg_PI"];

      //} //Reject low pt.

        // Set track in trackList
        // Set Helix parameters 
        TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5,0), CLHEP::HepSymMatrix(5,0)); 
        CLHEP::HepVector a(5); 
        a = aTrack.helix().a(); 
        aTrack.setFitted(1); 
        if(m_mDouble["charge"]<0){
          a[1] = fmod(m_mDouble["phi0"] + m_mConstD["Trg_PI"],2*m_mConstD["Trg_PI"]);
        } else {
          a[1] = m_mDouble["phi0"];
        }
        a[2] = 1/m_mDouble["pt"]*m_mDouble["charge"];
        a[3] = m_mDouble["z0"];  
        a[4] = m_mDouble["cot"]; 
        helix.a(a); 
        aTrack.set2DFitChi2(m_mDouble["fit2DChi2"]);
        aTrack.set3DFitChi2(m_mDouble["zChi2"]);
        aTrack.setHelix(helix); 

        //cout<<"charge="<<m_mDouble["charge"]<<" pt="<<m_mDouble["pt"]<<" phi_c="<<m_mDouble["phi0"]<<" z0="<<m_mDouble["z0"]<<" cot="<<m_mDouble["cot"]<<endl;

      ///////////////
      // Save values
      if(m_mBool["fRootFile"]) {
        if(m_fileFitter3D==0) {
          m_fileFitter3D = new TFile(m_rootFitter3DFileName.c_str(),"RECREATE");
          HandleRoot::initializeRoot("fitter3D", &m_treeConstantsFitter3D, &m_treeTrackFitter3D,
            m_mTVectorD, m_mTClonesArray,
            m_mConstD, m_mConstV,
            m_mDouble, m_mVector
          );
        }
        HandleRoot::saveTrackValues("fitter3D", 
          m_mTClonesArray, m_mDouble, m_mVector
        );
      }

      if(m_mBool["fVerbose"]) {
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]evtTime: "<<m_mDouble["eventTime"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]wirePhi: "<<m_mVector["wirePhi"][0]<<" "<<m_mVector["wirePhi"][1]<<" "<<m_mVector["wirePhi"][2]<<" "<<m_mVector["wirePhi"][3]<<" "<<m_mVector["wirePhi"][4]<<" "<<m_mVector["wirePhi"][5]<<" "<<m_mVector["wirePhi"][6]<<" "<<m_mVector["wirePhi"][7]<<" "<<m_mVector["wirePhi"][8]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]LR:      "<<int(m_mVector["LR"][0])<<" "<<int(m_mVector["LR"][1])<<" "<<int(m_mVector["LR"][2])<<" "<<int(m_mVector["LR"][3])<<" "<<int(m_mVector["LR"][4])<<" "<<int(m_mVector["LR"][5])<<" "<<int(m_mVector["LR"][6])<<" "<<int(m_mVector["LR"][7])<<" "<<int(m_mVector["LR"][8])<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]drift:   "<<m_mVector["driftLength"][0]<<" "<<m_mVector["driftLength"][1]<<" "<<m_mVector["driftLength"][2]<<" "<<m_mVector["driftLength"][3]<<" "<<m_mVector["driftLength"][4]<<" "<<m_mVector["driftLength"][5]<<" "<<m_mVector["driftLength"][6]<<" "<<m_mVector["driftLength"][7]<<" "<<m_mVector["driftLength"][8]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]tdc:     "<<m_mVector["tdc"][0]<<" "<<m_mVector["tdc"][1]<<" "<<m_mVector["tdc"][2]<<" "<<m_mVector["tdc"][3]<<" "<<m_mVector["tdc"][4]<<" "<<m_mVector["tdc"][5]<<" "<<m_mVector["tdc"][6]<<" "<<m_mVector["tdc"][7]<<" "<<m_mVector["tdc"][8]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]Phi2D:   "<<m_mVector["phi2D"][0]<<" "<<m_mVector["phi2D"][1]<<" "<<m_mVector["phi2D"][2]<<" "<<m_mVector["phi2D"][3]<<" "<<m_mVector["phi2D"][4]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]Phi3D:   "<<m_mVector["phi3D"][0]<<" "<<m_mVector["phi3D"][1]<<" "<<m_mVector["phi3D"][2]<<" "<<m_mVector["phi3D"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]zz:      "<<m_mVector["zz"][0]<<" "<<m_mVector["zz"][1]<<" "<<m_mVector["zz"][2]<<" "<<m_mVector["zz"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]arcS:    "<<m_mVector["arcS"][0]<<" "<<m_mVector["arcS"][1]<<" "<<m_mVector["arcS"][2]<<" "<<m_mVector["arcS"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]zerror:  "<<m_mVector["zError"][0]<<" "<<m_mVector["zError"][1]<<" "<<m_mVector["zError"][2]<<" "<<m_mVector["zError"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]charge:  "<<int(m_mDouble["charge"])<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]pt:      "<<m_mDouble["pt"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]phi0:    "<<m_mDouble["phi0"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]z0:      "<<m_mDouble["z0"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]cot:     "<<m_mDouble["cot"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]chi2:    "<<m_mDouble["zChi2"]<<endl;
      }
      if(m_mBool["fVerbose"] && m_mBool["fMc"]) {
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]mcPosZ:  "<<m_mVector["mcPosZ"][1]<<" "<<m_mVector["mcPosZ"][3]<<" "<<m_mVector["mcPosZ"][5]<<" "<<m_mVector["mcPosZ"][7]<<endl;
      }

    } // End track loop

    // Save values to file
    // To prevent crash when there are no tracks in first event.
    // If there is no track in first case then the ROOT saving functions might fail.
    // This is due to bad software design.
    // The first event that have tracks will be event 0 in ROOT file.
    if(m_mBool["fRootFile"]) {
      if(m_fileFitter3D != 0) m_treeTrackFitter3D->Fill();
    }

    TRGDebug::leaveStage("Fitter 3D");
    return 1;

  }

  int TRGCDCFitter3D::doitComplex(std::vector<TRGCDCTrack *> & trackList)
  {

    TRGDebug::enterStage("Fitter 3D");

    // Set values for saving data.
    if(m_mBool["fRootFile"]) m_mDouble["iSave"] = 0;
    if(m_mBool["fRootFile"]) {
      HandleRoot::initializeEvent(m_mTClonesArray);
    }
    if(m_commonData==0){
      m_commonData = new Belle2::TRGCDCJSignalData();
    }

    // Get common values for event.
    if(m_mBool["fEvtTime"]==0) m_mDouble["eventTime"]=0;
    else m_mDouble["eventTime"] = m_cdc.getEventTime();

    // Fitter3D
    // Loop over all tracks
    for(unsigned iTrack=0; iTrack<trackList.size(); iTrack++){

      TCTrack & aTrack = * trackList[iTrack];

      ///////////////////////////////////////
      // Check if all superlayers have one TS for the track.
      bool trackFull= isAxialTrackFull(aTrack) && isStereoTrackFull(aTrack);
      if(trackFull == 0){
         aTrack.setFitted(0);
         continue;
      }

      /////////////////////////////////
      // Get MC values for 3D fitter
      if(m_mBool["fMc"]) getMCValues(m_cdc, &aTrack, m_mConstD,  m_mDouble, m_mVector);
      // Get input values for 3D fitter
      // Get event and track ID
      m_mDouble["eventNumber"] = m_cdc.getEventNumber();
      m_mDouble["trackId"] = aTrack.getTrackID();

      ////////////////
      // Do 2D Fit
      // Get wirePhi, LR, mcLR, Drift information
      m_mVector["wirePhi"] = vector<double> (9);
      m_mVector["lutLR"] = vector<double> (9);
      m_mVector["LR"] = vector<double> (9);
      m_mVector["driftLength"] = vector<double> (9);
      m_mVector["tdc"] = vector<double> (9);
      for (unsigned iSL = 0; iSL < 9; iSL++) {
        const vector<TCLink *> & links = aTrack.links(iSL);
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
        m_mVector["wirePhi"][iSL] = (double) t_segment->localId()/m_mConstV["nWires"][iSL]*4*m_mConstD["Trg_PI"];
        m_mVector["lutLR"][iSL] = t_segment->LUT()->getValue(t_segment->lutPattern());
        // mcLR should be removed.
        m_mVector["mcLR"][iSL] = t_segment->hit()->mcLR();
        m_mVector["driftLength"][iSL] = t_segment->hit()->drift();
        m_mVector["tdc"][iSL] = t_segment->priorityTime();
        if(m_mBool["fmcLR"]==1) m_mVector["LR"][iSL] = m_mVector["mcLR"][iSL];
        else if(m_mBool["fLRLUT"]==1) m_mVector["LR"][iSL] = m_mVector["lutLR"][iSL];
        else m_mVector["LR"][iSL] = 3;
      } // End superlayer loop

      ////////////////////
      // Get 2D fit values
      // Get 2D fit values from IW 2D fitter
      m_mDouble["phi02D"] = aTrack.helix().phi0();
      m_mDouble["pt2D"] = aTrack.pt();
      if(aTrack.charge()<0) {
        m_mDouble["phi02D"] -= m_mConstD["Trg_PI"];
        if (m_mDouble["phi02D"] < 0) m_mDouble["phi02D"] += 2 * m_mConstD["Trg_PI"];
      }
      m_mDouble["dr2D"] = aTrack.helix().dr()*0.01;
      // Get 2D fit values from JB 2D fitter
      // Currently using JB fitter for 3D fitting
      m_mDouble["charge"] = double(aTrack.charge());
      // Set phi2DError for 2D fit
      m_mVector["phi2DError"] = vector<double> (5);
      m_mVector["phi2DInvError"] = vector<double> (5);
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        if(m_mVector["LR"][2*iAx] != 3) m_mVector["phi2DError"][iAx] = m_mConstV["driftPhi2DError"][iAx];
        else m_mVector["phi2DError"][iAx] = m_mConstV["wirePhi2DError"][iAx];
        m_mVector["phi2DInvError"][iAx] = 1/m_mVector["phi2DError"][iAx];
      }
      // Calculate phi2D.
      m_mVector["phi2D"] = vector<double> (5);
      if (m_mBool["f2DFitDrift"] == 0) {
        for (unsigned iAx = 0; iAx < 5; iAx++) m_mVector["phi2D"][iAx] = m_mVector["wirePhi"][iAx*2];
      } else {
        for (unsigned iAx = 0; iAx < 5; iAx++) {
          m_mVector["phi2D"][iAx] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iAx*2], m_mVector["tdc"][iAx*2], m_mDouble["eventTime"], m_mConstV["rr"][iAx*2], m_mVector["LR"][iAx*2]);
        }
      }
      // Fit2D
      if (m_mBool["f2DFit"] == 0) {
        m_mDouble["rho"] = m_mDouble["pt2D"]/0.01/1.5/0.299792458;
        m_mDouble["pt"] = 0.299792458*1.5*m_mDouble["rho"]/100;
        m_mDouble["phi0"] = m_mDouble["phi02D"];
        m_mDouble["fit2DChi2"] = 9999;
      } else {
        m_mDouble["rho"] = 0;
        m_mDouble["phi0"] = 0;
        m_mDouble["fit2DChi2"] = 0;
        Fitter3DUtility::rPhiFitter(&m_mConstV["rr2D"][0],&m_mVector["phi2D"][0],&m_mVector["phi2DInvError"][0],m_mDouble["rho"], m_mDouble["phi0"],m_mDouble["fit2DChi2"]); 
        m_mDouble["pt"] = 0.3*1.5*m_mDouble["rho"]/100;
      }

      //////////////////////
      // Start of 3D fitter
      // Calculate phi3D using driftTime.
      m_mVector["phi3D"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["phi3D"][iSt] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iSt*2+1], m_mVector["tdc"][iSt*2+1], m_mDouble["eventTime"], m_mConstV["rr3D"][iSt], m_mVector["LR"][iSt*2+1]);
      // Get zerror for 3D fit
      m_mVector["zError"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if(m_mVector["LR"][2*iSt+1] != 2) m_mVector["zError"][iSt] = m_mConstV["driftZError"][iSt];
        else m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
      }
      // Get inverse zerror ^ 2
      m_mVector["iZError2"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["iZError2"][iSt] = 1/pow(m_mVector["zError"][iSt],2);

      // Simple version of Fitter3D
      //// Calculate zz
      //m_mVector["zz"] = vector<double> (4);
      //for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge"], m_mConstV["angleSt"][iSt], m_mConstV["zToStraw"][iSt], m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
      //// Calculate arcS
      //m_mVector["arcS"] = vector<double> (4);
      //for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["arcS"][iSt] = Fitter3DUtility::calS(m_mDouble["rho"], m_mConstV["rr3D"][iSt]);
      //// Fit3D
      //m_mDouble["z0"] = 0;
      //m_mDouble["cot"] = 0;
      //m_mDouble["zChi2"] = 0;
      //Fitter3DUtility::rSFit(&m_mVector["iZError2"][0], &m_mVector["arcS"][0], &m_mVector["zz"][0], m_mDouble["z0"], m_mDouble["cot"], m_mDouble["zChi2"]);
      //// Change to deg
      //m_mDouble["theta"] = m_mConstD["Trg_PI"]/2 - atan(m_mDouble["cot"]);
      //m_mDouble["theta"] = 180 / m_mConstD["Trg_PI"];

      double phiMax = m_mConstD["Trg_PI"];
      double phiMin = -m_mConstD["Trg_PI"];
      // pt = 0.3*1.5*rho*0.01;
      double rhoMin = 67;
      double rhoMax = 1600;
      double iError2Max = 0.120857;
      int phiBitSize = 13;
      int rhoBitSize = 12;
      int iError2BitSize = 8;
      // LUT values
      m_mConstD["JB"] = 0;
      m_mConstD["acosLUTOutBitSize"] = phiBitSize-1;
      m_mConstD["acosLUTInBitSize"] = rhoBitSize;
      m_mConstD["zLUTInBitSize"] = phiBitSize;
      m_mConstD["zLUTOutBitSize"] = 9;
      m_mConstD["iDenLUTInBitSize"] = 13;
      m_mConstD["iDenLUTOutBitSize"] = 11;
      ////Rotate phi0///////////////////////////////////////////////////////////
      // Rotate phi0 to -45 deg for + charge or +45 deg for - charge.
      // This will not be needed for firmware case. Range of phi will already be [-90, 90].
      // Find reference value. Set phi0 to be 45 deg or -45 deg depending on charge.
      if(m_mDouble["charge"] == -1) m_mDouble["relRefPhi"] = m_mDouble["phi0"] - m_mConstD["Trg_PI"]/4;
      else m_mDouble["relRefPhi"] = m_mDouble["phi0"] + m_mConstD["Trg_PI"]/4;
      // Rotate
      m_mDouble["relPhi0"] = Fitter3DUtility::rotatePhi(m_mDouble["phi0"], m_mDouble["relRefPhi"]);
      m_mVector["relPhi3D"] = vector<double> (4);
      for(unsigned iSt=0; iSt<4; iSt++) m_mVector["relPhi3D"][iSt] = Fitter3DUtility::rotatePhi(m_mVector["phi3D"][iSt], m_mDouble["relRefPhi"]);
      //Reject low pt.
      if(m_mDouble["rho"] > 67){
        
        // Constrain rho to 1600.
        if(m_mDouble["rho"] > rhoMax) {
          m_mDouble["rho"] = rhoMax;
          m_mDouble["pt"] = rhoMax * 0.3 * 1.5 * 0.01;
        }

        // Change to Signals.
        {
          vector<tuple<string, double, int, double, double, int> > t_values = {
            make_tuple("phi0", m_mDouble["relPhi0"], phiBitSize, phiMin, phiMax, 0),
            make_tuple("phi_0", m_mVector["relPhi3D"][0], phiBitSize, phiMin, phiMax, 0),
            make_tuple("phi_1", m_mVector["relPhi3D"][1], phiBitSize, phiMin, phiMax, 0),
            make_tuple("phi_2", m_mVector["relPhi3D"][2], phiBitSize, phiMin, phiMax, 0),
            make_tuple("phi_3", m_mVector["relPhi3D"][3], phiBitSize, phiMin, phiMax, 0),
            make_tuple("rho", m_mDouble["rho"], rhoBitSize, rhoMin, rhoMax, 0),
            make_tuple("iZError2_0", m_mVector["iZError2"][0], iError2BitSize, 0, iError2Max, 0),
            make_tuple("iZError2_1", m_mVector["iZError2"][1], iError2BitSize, 0, iError2Max, 0),
            make_tuple("iZError2_2", m_mVector["iZError2"][2], iError2BitSize, 0, iError2Max, 0),
            make_tuple("iZError2_3", m_mVector["iZError2"][3], iError2BitSize, 0, iError2Max, 0),
            make_tuple("charge",(int) (m_mDouble["charge"]==1 ? 1 : 0), 1, 0, 1, 0)
          };
          TRGCDCJSignal::valuesToMapSignals(t_values, m_commonData, m_mSignalStorage);
        }
        Fitter3DUtility::calZ(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
        Fitter3DUtility::calS(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
        Fitter3DUtility::rSFit(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
        // Change to values.
        vector<tuple<string, double, int, double, double, int> > resultValues;
        {
          vector<pair<string, int> > t_chooseSignals = {
            make_pair("z0", 0), make_pair("cot",0), make_pair("chi2Sum",0)
          };
          TRGCDCJSignal::mapSignalsToValues(m_mSignalStorage, t_chooseSignals, resultValues);
        }

        // Post handling of signals.
        // Name all signals.
        if((*m_mSignalStorage.begin()).second.getName() == ""){
          for(auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it){
            (*it).second.setName((*it).first);
          }
        }
        ////// Dump all signals.
        ////bool done = 0;
        ////if((*m_mSignalStorage.begin()).second.getName() != "" && done==0){
        ////  for(auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it){
        ////    (*it).second.dump();
        ////  }
        ////  done=1;
        ////}


        // Takes some time.
        // Save values.
        if(m_mBool["fRootFile"]) {
          // Check if there is a name.
          if((*m_mSignalStorage.begin()).second.getName() != ""){

            // Save values to root file.
            {
              vector<pair<string, int> > chooseValues = {
                make_pair("zz_0", m_mBool["fIsIntegerEffect"]),
                make_pair("zz_1", m_mBool["fIsIntegerEffect"]),
                make_pair("zz_2", m_mBool["fIsIntegerEffect"]),
                make_pair("zz_3", m_mBool["fIsIntegerEffect"]),
                make_pair("arcS_0", m_mBool["fIsIntegerEffect"]),
                make_pair("arcS_1", m_mBool["fIsIntegerEffect"]),
                make_pair("arcS_2", m_mBool["fIsIntegerEffect"]),
                make_pair("arcS_3", m_mBool["fIsIntegerEffect"]),
                make_pair("z0", m_mBool["fIsIntegerEffect"]),
                make_pair("cot", m_mBool["fIsIntegerEffect"]),
                make_pair("zChi2", m_mBool["fIsIntegerEffect"])
              };
              // outValues => [name, value, bitwidth, min, max, clock]
              vector<tuple<string, double, int, double, double, int> > outValues;
              TRGCDCJSignal::mapSignalsToValues(m_mSignalStorage, chooseValues, outValues);
              HandleRoot::convertSignalValuesToMaps(outValues, m_mDouble, m_mVector);
              // Save values to m_mDouble and m_mVector.
            }
          }
        }

        m_mBool["fVHDLFile"] = 0;
        if(m_mBool["fVHDLFile"]) {
          // Check if there is a name.
          if((*m_mSignalStorage.begin()).second.getName() != ""){
            // Saves to file only one time.
            saveVhdlAndCoe();
            // Saves values to memory. Wrote to file at terminate().
            saveAllSignals();
            saveIoSignals();
          }
        }

        // Set track in trackListOut.
        // Set Helix parameters 
        TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5,0), CLHEP::HepSymMatrix(5,0)); 
        CLHEP::HepVector a(5); 
        a = aTrack.helix().a(); 
        aTrack.setFitted(1); 
        if(m_mDouble["charge"]<0){
          a[1] = fmod(m_mDouble["phi0"] + m_mConstD["Trg_PI"],2*m_mConstD["Trg_PI"]);
        } else {
          a[1] = m_mDouble["phi0"];
        }
        a[2] = 1/m_mDouble["pt"]*m_mDouble["charge"];
        a[3] = m_mDouble["z0"];  
        a[4] = m_mDouble["cot"]; 
        helix.a(a); 
        aTrack.setHelix(helix); 
        aTrack.set2DFitChi2(m_mDouble["fit2DChi2"]);
        aTrack.set3DFitChi2(m_mDouble["zChi2"]);

      } else { //Reject low pt.
        aTrack.setFitted(0);
      }

      ///////////////
      // Save values
      if(m_mBool["fRootFile"]) {
        if(m_fileFitter3D==0) {
          m_fileFitter3D = new TFile(m_rootFitter3DFileName.c_str(),"RECREATE");
          HandleRoot::initializeRoot("fitter3D", &m_treeConstantsFitter3D, &m_treeTrackFitter3D,
            m_mTVectorD, m_mTClonesArray,
            m_mConstD, m_mConstV,
            m_mDouble, m_mVector
          );
        }
        HandleRoot::saveTrackValues("fitter3D", 
          m_mTClonesArray, m_mDouble, m_mVector
        );
      }

      if(m_mBool["fVerbose"]) {
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]evtTime: "<<m_mDouble["eventTime"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]wirePhi: "<<m_mVector["wirePhi"][0]<<" "<<m_mVector["wirePhi"][1]<<" "<<m_mVector["wirePhi"][2]<<" "<<m_mVector["wirePhi"][3]<<" "<<m_mVector["wirePhi"][4]<<" "<<m_mVector["wirePhi"][5]<<" "<<m_mVector["wirePhi"][6]<<" "<<m_mVector["wirePhi"][7]<<" "<<m_mVector["wirePhi"][8]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]LR:      "<<int(m_mVector["LR"][0])<<" "<<int(m_mVector["LR"][1])<<" "<<int(m_mVector["LR"][2])<<" "<<int(m_mVector["LR"][3])<<" "<<int(m_mVector["LR"][4])<<" "<<int(m_mVector["LR"][5])<<" "<<int(m_mVector["LR"][6])<<" "<<int(m_mVector["LR"][7])<<" "<<int(m_mVector["LR"][8])<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]drift:   "<<m_mVector["driftLength"][0]<<" "<<m_mVector["driftLength"][1]<<" "<<m_mVector["driftLength"][2]<<" "<<m_mVector["driftLength"][3]<<" "<<m_mVector["driftLength"][4]<<" "<<m_mVector["driftLength"][5]<<" "<<m_mVector["driftLength"][6]<<" "<<m_mVector["driftLength"][7]<<" "<<m_mVector["driftLength"][8]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]tdc:     "<<m_mVector["tdc"][0]<<" "<<m_mVector["tdc"][1]<<" "<<m_mVector["tdc"][2]<<" "<<m_mVector["tdc"][3]<<" "<<m_mVector["tdc"][4]<<" "<<m_mVector["tdc"][5]<<" "<<m_mVector["tdc"][6]<<" "<<m_mVector["tdc"][7]<<" "<<m_mVector["tdc"][8]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]Phi2D:   "<<m_mVector["phi2D"][0]<<" "<<m_mVector["phi2D"][1]<<" "<<m_mVector["phi2D"][2]<<" "<<m_mVector["phi2D"][3]<<" "<<m_mVector["phi2D"][4]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]Phi3D:   "<<m_mVector["phi3D"][0]<<" "<<m_mVector["phi3D"][1]<<" "<<m_mVector["phi3D"][2]<<" "<<m_mVector["phi3D"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]zz:      "<<m_mVector["zz"][0]<<" "<<m_mVector["zz"][1]<<" "<<m_mVector["zz"][2]<<" "<<m_mVector["zz"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]arcS:    "<<m_mVector["arcS"][0]<<" "<<m_mVector["arcS"][1]<<" "<<m_mVector["arcS"][2]<<" "<<m_mVector["arcS"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]zerror:  "<<m_mVector["zError"][0]<<" "<<m_mVector["zError"][1]<<" "<<m_mVector["zError"][2]<<" "<<m_mVector["zError"][3]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]charge:  "<<int(m_mDouble["charge"])<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]pt:      "<<m_mDouble["pt"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]phi0:    "<<m_mDouble["phi0"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]z0:      "<<m_mDouble["z0"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]cot:     "<<m_mDouble["cot"]<<endl;
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]chi2:    "<<m_mDouble["zChi2"]<<endl;
      }
      if(m_mBool["fVerbose"] && m_mBool["fMc"]) {
        cout<<"[E"<<int(m_mDouble["eventNumber"])<<"][T"<<iTrack<<"]mcPosZ:  "<<m_mVector["mcPosZ"][1]<<" "<<m_mVector["mcPosZ"][3]<<" "<<m_mVector["mcPosZ"][5]<<" "<<m_mVector["mcPosZ"][7]<<endl;
      }

    } // End track loop

    // Save values to file
    // To prevent crash when there are no tracks in first event.
    // If there is no track in first evnet then the HandleRoot saving functions will fail.
    // This is due to bad HandleRoot software design.
    // The first event that have tracks will be event 0 in ROOT file.
    if(m_mBool["fRootFile"]) {
      if(m_fileFitter3D != 0) m_treeTrackFitter3D->Fill();
    }

    TRGDebug::leaveStage("Fitter 3D");

    return 1;

  }

  double TRGCDCFitter3D::calPhi(TRGCDCSegmentHit const * segmentHit, double eventTime){
    CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    unsigned localId = segmentHit->segment().center().localId();
    unsigned layerId = segmentHit->segment().center().layerId();
    int nWires = cdcp.nWiresInLayer(layerId)*2;
    double rr = cdcp.senseWireR(layerId);
    //double driftLength = segmentHit->drift();
    double tdc = segmentHit->segment().priorityTime();
    int lr = segmentHit->segment().LUT()->getValue(segmentHit->segment().lutPattern());
    return Fitter3DUtility::calPhi(localId, nWires, tdc, eventTime, rr, lr);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  ////// Function for saving
  /////////////////////////////////////////////////////////////////////////////////////////////
  // Functions for saving.
  void TRGCDCFitter3D::saveVhdlAndCoe() {
    // Print Vhdl
    if(m_commonData->getPrintedToFile()==0){
      if(m_commonData->getPrintVhdl()==0){
        m_commonData->setVhdlOutputFile("Fitter3D.vhd");
        m_commonData->setPrintVhdl(1);
      } else {
        m_commonData->setPrintVhdl(0);
        m_commonData->entryVhdlCode();
        m_commonData->signalsVhdlCode();
        m_commonData->buffersVhdlCode();
        m_commonData->printToFile();
        // Print LUTs.
        for(map<string,TRGCDCJLUT*>::iterator it=m_mLutStorage.begin(); it!=m_mLutStorage.end(); ++it){
          //it->second->makeCOE("./VHDL/LutData/"+it->first+".coe");
          it->second->makeCOE(it->first+".coe");
        }
      }
    }
  }

  void TRGCDCFitter3D::saveAllSignals() {
    // Save all signals to m_mSavedSignals. Limit to 10 times.
    if((*m_mSavedSignals.begin()).second.size() < 10 || m_mSavedSignals.empty()) {
      for(auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it){
        m_mSavedSignals[(*it).first].push_back((*it).second.getInt()); 
      }
    }
  }

  void TRGCDCFitter3D::saveIoSignals() {
    // Save input output signals. Limit to 1024.
    if((*m_mSavedIoSignals.begin()).second.size() < 1025 || m_mSavedIoSignals.empty()) {
      m_mSavedIoSignals["phi0"].push_back(m_mSignalStorage["phi0"].getInt());
      for(unsigned iSt=0; iSt<4; iSt++) m_mSavedIoSignals["phi_"+to_string(iSt)].push_back(m_mSignalStorage["phi_"+to_string(iSt)].getInt());
      m_mSavedIoSignals["rho"].push_back(m_mSignalStorage["rho"].getInt());
      for(unsigned iSt=0; iSt<4; iSt++) m_mSavedIoSignals["iZError2_"+to_string(iSt)].push_back(m_mSignalStorage["iZError2_"+to_string(iSt)].getInt());
      m_mSavedIoSignals["charge"].push_back(m_mSignalStorage["charge"].getInt());
      m_mSavedIoSignals["z0"].push_back(m_mSignalStorage["z0"].getInt());
      m_mSavedIoSignals["cot"].push_back(m_mSignalStorage["cot"].getInt());
      m_mSavedIoSignals["chi2Sum"].push_back(m_mSignalStorage["chi2Sum"].getInt());
    }
  }


  void TRGCDCFitter3D::getMCValues(const TRGCDC & m_cdc, TRGCDCTrack* aTrack, std::map<std::string, double> & m_mConstD, std::map<std::string, double> & m_mDouble, std::map<std::string, std::vector<double> > & m_mVector){
    // Access to track's MC particle.
    const TCRelation & trackRelation = aTrack->relation();
    // Biggest contibutor is 0. Next is 1 and so on.
    const MCParticle & trackMCParticle = trackRelation.mcParticle(0);

    // Calculated impact position
    TVector3 vertex = trackMCParticle.getVertex();
    TLorentzVector vector4 = trackMCParticle.get4Vector();
    TVector2 helixCenter;
    TVector3 impactPosition;
    Fitter3DUtility::findImpactPosition(&vertex, &vector4, int(m_mDouble["mcCharge"]), helixCenter, impactPosition);
    m_mVector["mcVertex"] = vector<double> ({vertex.X(), vertex.Y(), vertex.Z()});
    m_mVector["mcMomentum"] = vector<double> ({vector4.Px(), vector4.Py(), vector4.Pz()});
    m_mVector["helixCenter"] = vector<double> ({helixCenter.X(), helixCenter.Y()});
    m_mVector["impactPosition"] = vector<double> ({impactPosition.X(), impactPosition.Y(), impactPosition.Z()});

    // Access track's particle parameters
    m_mDouble["mcPt"] = trackMCParticle.getMomentum().Pt();
    m_mDouble["mcPhi0"] = 0;
    if(trackMCParticle.getCharge()>0) m_mDouble["mcPhi0"]=trackMCParticle.getMomentum().Phi()-m_mConstD["Trg_PI"]/2;
    if(trackMCParticle.getCharge()<0) m_mDouble["mcPhi0"]=trackMCParticle.getMomentum().Phi()+m_mConstD["Trg_PI"]/2;
    // Change range to [0,2pi]
    if(m_mDouble["mcPhi0"]<0) m_mDouble["mcPhi0"]+=2*m_mConstD["Trg_PI"];
    //m_mDouble["mcZ0"] = trackMCParticle.getVertex().Z();
    m_mDouble["mcZ0"] = impactPosition.Z();
    m_mDouble["mcCot"]=trackMCParticle.getMomentum().Pz()/trackMCParticle.getMomentum().Pt();
    m_mDouble["mcCharge"] = trackMCParticle.getCharge();

    // mcStatus[0]: statusbit, mcStatus[1]: pdg, mcStatus[2]: charge
    TVectorD mcStatus(3);
    m_mDouble["mcStatus"] = trackMCParticle.getStatus();
    m_mDouble["pdgId"] = trackMCParticle.getPDG();


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
        m_mVector["mcPosY"][iSL] = posTrack.Y();
        m_mVector["mcPosZ"][iSL] = posTrack.Z();
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

  bool TRGCDCFitter3D::isAxialTrackFull( TRGCDCTrack & aTrack ){
    bool trackFull = 1;
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      // Check if all superlayers have one TS
      const vector<TCLink *> & links = aTrack.links(iAx*2);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if(nSegments != 1) {
        if (nSegments == 0){
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::isAxialTrackFull() => There are no TS."<<endl;
        } else {
          if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::isAxialTrackFull() => multiple TS are assigned."<<endl;
        }
      } else {
        if (priorityHitTS == 0) {
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::isAxialTrackFull() => There are no priority hit TS"<<endl;
        }
      }
    } // End superlayer loop
    return trackFull;
  }

  bool TRGCDCFitter3D::isStereoTrackFull( TRGCDCTrack & aTrack ){
    bool trackFull = 1;
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Check if all superlayers have one TS
      const vector<TCLink *> & links = aTrack.links(iSt*2+1);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if(nSegments != 1) {
        if (nSegments == 0){
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::isStereoTrackFull() => There are no TS."<<endl;
        } else {
          if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::isStereoTrackFull() => multiple TS are assigned."<<endl;
        }
      } else {
        if (priorityHitTS == 0) {
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::isStereoTrackFull() => There are no priority hit TS"<<endl;
        }
      }
    } // End superlayer loop
    return trackFull;
  }

  void TRGCDCFitter3D::findHitAxialSuperlayers( TRGCDCTrack & aTrack,  bool (&useAxSl)[5], bool printError ){
    std::fill_n( useAxSl, 5, 1 );
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      // Check if all superlayers have one TS
      const vector<TCLink *> & links = aTrack.links(iAx*2);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if(nSegments != 1) {
        if (nSegments == 0){
          useAxSl[iAx] = 0;
        } else {
          if (printError) cout<<"Fitter3D::findHitAxialSuperlayers() => multiple TS are assigned."<<endl;
        }
      } else {
        if (priorityHitTS == 0) {
          useAxSl[iAx] = 0;
          if (printError) cout<<"Fitter3D::findHitAxialSuperlayers() => There are no priority hit TS"<<endl;
        }
      }
    } // End superlayer loop
  }

  void TRGCDCFitter3D::findHitStereoSuperlayers( TRGCDCTrack & aTrack,  bool (&useStSl)[4] , bool printError){
    std::fill_n( useStSl, 4, 1 );
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Check if all superlayers have one TS
      const vector<TCLink *> & links = aTrack.links(iSt*2+1);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if(nSegments != 1) {
        if (nSegments == 0){
          useStSl[iSt] = 0;
        } else {
          if (printError) cout<<"Fitter3D::findHitStereoSuperlayers() => multiple TS are assigned."<<endl;
        }
      } else {
        if (priorityHitTS == 0) {
          useStSl[iSt] = 0;
          if (printError) cout<<"Fitter3D::findHitStereoSuperlayers() => There are no priority hit TS"<<endl;
        }
      }
    } // End superlayer loop
  }

  void TRGCDCFitter3D::removeImpossibleStereoSuperlayers( bool (&useStSl)[4] ){
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Check if rho is large enough for stereo super layer.
      if(2*m_mDouble["rho"] < m_mConstV["rr3D"][iSt] ) {
        useStSl[iSt] = 0;
        if (m_mBool["fIsPrintError"]) cout<<"Fitter3D::removeImpossibleStereoSuperlayers() => pt is too low for SL."<<endl;
      }
    } // End superlayer loop
  }

  int TRGCDCFitter3D::do2DFit( TRGCDCTrack & aTrack, std::map<std::string, bool> & m_mBool, std::map<std::string, double> & m_mConstD, std::map<std::string, std::vector<double> > & m_mConstV, std::map<std::string, double> & m_mDouble, std::map<std::string, std::vector<double> > & m_mVector) {
    bool useAxSl[5];
    findHitAxialSuperlayers(aTrack, useAxSl, m_mBool["fIsPrintError"]);
    // Check if number of axial super layer hits is smaller or equal to 1.
    int nHitAx = (int)useAxSl[0]+(int)useAxSl[1]+(int)useAxSl[2]+(int)useAxSl[3]+(int)useAxSl[4];
    if(nHitAx <= 1) {
      aTrack.setFitted(0);
      return 1;
    }

    // Fill information for axial layers
    m_mVector["wirePhi"] = vector<double> (9);
    m_mVector["lutLR"] = vector<double> (9);
    m_mVector["LR"] = vector<double> (9);
    m_mVector["driftLength"] = vector<double> (9);
    m_mVector["tdc"] = vector<double> (9);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if(useAxSl[iAx] == 1) {
        const vector<TCLink *> & links = aTrack.links(iAx*2);
        const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
        m_mVector["wirePhi"][iAx*2] = (double) t_segment->localId()/m_mConstV["nWires"][iAx*2]*4*m_mConstD["Trg_PI"];
        m_mVector["lutLR"][iAx*2] = t_segment->LUT()->getValue(t_segment->lutPattern());
        // mcLR should be removed.
        m_mVector["mcLR"][iAx*2] = t_segment->hit()->mcLR();
        m_mVector["driftLength"][iAx*2] = t_segment->hit()->drift();
        m_mVector["tdc"][iAx*2] = t_segment->priorityTime();
        if(m_mBool["fmcLR"]==1) m_mVector["LR"][iAx*2] = m_mVector["mcLR"][iAx*2];
        else if(m_mBool["fLRLUT"]==1) m_mVector["LR"][iAx*2] = m_mVector["lutLR"][iAx*2];
        else m_mVector["LR"][iAx*2] = 3;
      } else {
        m_mVector["wirePhi"][iAx*2] = 9999;
        m_mVector["lutLR"][iAx*2] = 9999;
        // mcLR should be removed.
        m_mVector["mcLR"][iAx*2] = 9999;
        m_mVector["driftLength"][iAx*2] = 9999;
        m_mVector["tdc"][iAx*2] = 9999;
        if(m_mBool["fmcLR"]==1) m_mVector["LR"][iAx*2] = 9999;
        else if(m_mBool["fLRLUT"]==1) m_mVector["LR"][iAx*2] = 9999;
        else m_mVector["LR"][iAx*2] = 9999;
      }
    } // End superlayer loop
    ////////////////////
    // Get 2D fit values
    // Get 2D fit values from IW 2D fitter
    m_mDouble["phi02D"] = aTrack.helix().phi0();
    m_mDouble["pt2D"] = aTrack.pt();
    if(aTrack.charge()<0) {
      m_mDouble["phi02D"] -= m_mConstD["Trg_PI"];
      if (m_mDouble["phi02D"] < 0) m_mDouble["phi02D"] += 2 * m_mConstD["Trg_PI"];
    }
    m_mDouble["dr2D"] = aTrack.helix().dr()*0.01;
    // Get 2D fit values from JB 2D fitter
    // Currently using JB fitter for 3D fitting
    m_mDouble["charge"] = double(aTrack.charge());
    // Set phi2DError for 2D fit
    m_mVector["phi2DError"] = vector<double> (5);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if(useAxSl[iAx] == 1) {
        if(m_mVector["LR"][2*iAx] != 3) m_mVector["phi2DError"][iAx] = m_mConstV["driftPhi2DError"][iAx];
        else m_mVector["phi2DError"][iAx] = m_mConstV["wirePhi2DError"][iAx];
      } else {
        m_mVector["phi2DError"][iAx] = 9999;
      }
    }
    // Set invPhi2DError for 2D fit
    m_mVector["phi2DInvError"] = vector<double> (5);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if(useAxSl[iAx] == 1) {
        m_mVector["phi2DInvError"][iAx] = 1/m_mVector["phi2DError"][iAx];
      } else {
        m_mVector["phi2DInvError"][iAx] = 0;
      }
    }
    // Calculate phi2D.
    m_mVector["phi2D"] = vector<double> (5);
    if (m_mBool["f2DFitDrift"] == 0) {
      for (unsigned iAx = 0; iAx < 5; iAx++) m_mVector["phi2D"][iAx] = m_mVector["wirePhi"][iAx*2];
    } else {
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        if(useAxSl[iAx] == 1) {
          m_mVector["phi2D"][iAx] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iAx*2], m_mVector["tdc"][iAx*2], m_mDouble["eventTime"], m_mConstV["rr"][iAx*2], m_mVector["LR"][iAx*2]);
        } else {
          m_mVector["phi2D"][iAx] = 9999;
        }
      }
    }
    // Fit2D
    if (m_mBool["f2DFit"] == 0) {
      m_mDouble["rho"] = m_mDouble["pt2D"]/0.01/1.5/0.299792458;
      m_mDouble["pt"] = 0.299792458*1.5*m_mDouble["rho"]/100;
      m_mDouble["phi0"] = m_mDouble["phi02D"];
      m_mDouble["fit2DChi2"] = 9999;
    } else {
      m_mDouble["rho"] = 0;
      m_mDouble["phi0"] = 0;
      m_mDouble["fit2DChi2"] = 0;
      Fitter3DUtility::rPhiFitter(&m_mConstV["rr2D"][0],&m_mVector["phi2D"][0],&m_mVector["phi2DInvError"][0],m_mDouble["rho"], m_mDouble["phi0"],m_mDouble["fit2DChi2"]); 
      m_mDouble["pt"] = 0.3*1.5*m_mDouble["rho"]/100;
    }
    return 0;
  }


  void TRGCDCFitter3D::terminate(){
    if(m_mBool["fRootFile"]) {
      HandleRoot::writeRoot(m_fileFitter3D);
      HandleRoot::terminateRoot(m_mTVectorD, m_mTClonesArray);
      delete m_fileFitter3D;
    }

    if(m_mBool["fVHDLFile"]) {
      //if(m_mSavedIoSignals.size()!=0) FpgaUtility::multipleWriteCoe(10, m_mSavedIoSignals, "./VHDL/LutData/Io/");
      //if(m_mSavedSignals.size()!=0) FpgaUtility::writeSignals("./VHDL/signals",m_mSavedSignals);
      if(m_mSavedIoSignals.size()!=0) FpgaUtility::multipleWriteCoe(10, m_mSavedIoSignals, "./");
      if(m_mSavedSignals.size()!=0) FpgaUtility::writeSignals("signalValues",m_mSavedSignals);
    }

    if(m_commonData) delete m_commonData;
  }

  string TRGCDCFitter3D::version(void) const {
    return string("TRGCDCFitter3D 6.0");
  }

  std::string TRGCDCFitter3D::name(void) const {
    return m_name;
  }


} // namespace Belle2
