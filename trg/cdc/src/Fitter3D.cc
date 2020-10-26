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
#include <cmath>

#include <framework/dataobjects/EventMetaData.h>
#include "cdc/dataobjects/CDCSimHit.h"
#include "cdc/geometry/CDCGeometryPar.h"
#include "trg/trg/Time.h"
#include "trg/trg/Signal.h"
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

  TRGCDCFitter3D::TRGCDCFitter3D(const std::string& name,
                                 const std::string& rootFitter3DFile,
                                 const TRGCDC& TRGCDC,
                                 const std::map<std::string, bool>& flags)
    : m_name(name), m_cdc(TRGCDC),
      m_mBool(flags), m_rootFitter3DFileName(rootFitter3DFile),
      m_treeTrackFitter3D(), m_treeConstantsFitter3D() // 2019/07/31 by ytlai
  {
    m_fileFitter3D = 0;
    m_commonData = 0;
  }

  TRGCDCFitter3D::~TRGCDCFitter3D()
  {
  }

  void TRGCDCFitter3D::initialize()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.isRequired();

    // If we are using Monte Carlo information.
    m_mBool["fMc"] = 1;
    m_mBool["fVerbose"] = 0;
    m_mBool["fIsPrintError"] = 0;
    m_mBool["fIsIntegerEffect"] = 1;
    m_mBool["debugFitted"] = 1;
    m_mBool["debugLargeZ0"] = 0;

    // Init values
    m_mConstD["Trg_PI"] = 3.141592653589793;

    // Get rr,zToStraw,angleSt,nWire
    const CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    m_mConstV["rr"] = vector<double> (9);
    m_mConstV["nWires"] = vector<double> (9);
    m_mConstV["nTSs"] = vector<double> (9);
    for (unsigned iSL = 0; iSL < 9; iSL++) {
      unsigned t_layerId = m_cdc.segment(iSL, 0).center().layerId();
      m_mConstV["rr"][iSL] = cdcp.senseWireR(t_layerId);
      m_mConstV["nWires"][iSL] = cdcp.nWiresInLayer(t_layerId) * 2;
      m_mConstV["nTSs"][iSL] = cdcp.nWiresInLayer(t_layerId);
    }
    m_mConstV["nTSs2D"] = vector<double> (5);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      m_mConstV["nTSs2D"][iAx] = m_mConstV["nTSs"][2 * iAx];
    }

    m_mConstV["zToStraw"] = vector<double> (4);
    m_mConstV["zToOppositeStraw"] = vector<double> (4);
    m_mConstV["angleSt"] = vector<double> (4);
    m_mConstV["nShift"] = vector<double> (4);
    for (int iSt = 0; iSt < 4; iSt++) {
      unsigned t_layerId = m_cdc.stereoSegment(iSt, 0).center().layerId();
      m_mConstV["zToStraw"][iSt] = cdcp.senseWireBZ(t_layerId);
      m_mConstV["zToOppositeStraw"][iSt] = cdcp.senseWireFZ(t_layerId);
      m_mConstV["angleSt"][iSt] = 2 * m_mConstV["rr"][2 * iSt + 1] * sin(m_mConstD["Trg_PI"] * cdcp.nShifts(t_layerId) /
                                  (2 * cdcp.nWiresInLayer(t_layerId))) / (cdcp.senseWireFZ(t_layerId) - cdcp.senseWireBZ(t_layerId));
      m_mConstV["nShift"][iSt] = cdcp.nShifts(t_layerId);
    }

    m_mConstV["rr2D"] = vector<double> (5);
    m_mConstV["rr3D"] = vector<double> (4);
    for (int iAx = 0; iAx < 5; iAx++) m_mConstV["rr2D"][iAx] = m_mConstV["rr"][iAx * 2];
    for (int iSt = 0; iSt < 4; iSt++) m_mConstV["rr3D"][iSt] = m_mConstV["rr"][iSt * 2 + 1];

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

    // (2016.06.07) study
    //m_mConstV["wireZError"] = vector<double> ({4.752, 6.393, 6.578, 6.418});
    //m_mConstV["driftZError"] = vector<double> ({0.4701, 0.7203, 0.8058, 0.9382});
    m_mConstV["driftZError"] = vector<double> ({0.7676, 0.9753, 1.029, 1.372});
    m_mConstV["wireZError"] = vector<double> ({0.7676, 0.9753, 1.029, 1.372});

    // Make driftLength table for each superlayer. Up to 511 clock ticks.
    // driftLengthTableSLX[ tdcCount (~2ns unit) ] = drift length (cm)
    for (unsigned iSl = 0; iSl < 9; iSl++) {
      string tableName = "driftLengthTableSL" + to_string(iSl);
      unsigned tableSize = 512;
      m_mConstV[tableName] = vector<double> (tableSize);
      unsigned t_layer = m_cdc.segment(iSl, 0).center().layerId();
      for (unsigned iTick = 0; iTick < tableSize; iTick++) {
        double t_driftTime = iTick * 2 * cdcp.getTdcBinWidth();
        double avgDriftLength = 0;
        if (m_mBool["fXtSimple"] == 1) {
          avgDriftLength = cdcp.getNominalDriftV() * t_driftTime;
        } else {
          double driftLength_0 = cdcp.getDriftLength(t_driftTime, t_layer, 0);
          double driftLength_1 = cdcp.getDriftLength(t_driftTime, t_layer, 1);
          avgDriftLength = (driftLength_0 + driftLength_1) / 2;
        }
        m_mConstV[tableName][iTick] = avgDriftLength;
      }
    }

    if (m_mBool["fVerbose"]) {
      cout << "fLRLUT:       " << m_mBool["fLRLUT"] << endl;
      cout << "fMc:          " << m_mBool["fMc"] << endl;
      cout << "fVerbose:     " << m_mBool["fVerbose"] << endl;
      if (m_mBool["fMc"]) cout << "fmcLR:        " << m_mBool["fmcLR"] << endl;
      cout << "fRoot:        " << m_mBool["fRootFile"] << endl;
      cout << "PI:           " << m_mConstD["Trg_PI"] << endl;
      cout << "rr:           " << m_mConstV["rr"][0] << " " << m_mConstV["rr"][1] << " " << m_mConstV["rr"][2] << " " <<
           m_mConstV["rr"][3] << " " << m_mConstV["rr"][4] << " " << m_mConstV["rr"][5] << " " << m_mConstV["rr"][6] << " " <<
           m_mConstV["rr"][7] << " " << m_mConstV["rr"][8] << endl;
      cout << "nWires:       " << int(m_mConstV["nWires"][0]) << " " << int(m_mConstV["nWires"][1]) << " " << int(
             m_mConstV["nWires"][2]) << " " << int(m_mConstV["nWires"][3]) << " " << int(m_mConstV["nWires"][4]) << " " << int(
             m_mConstV["nWires"][5]) << " " << int(m_mConstV["nWires"][6]) << " " << int(m_mConstV["nWires"][7]) << " " << int(
             m_mConstV["nWires"][8]) << endl;
      cout << "zToStraw:     " << m_mConstV["zToStraw"][0] << " " << m_mConstV["zToStraw"][1] << " " << m_mConstV["zToStraw"][2] << " " <<
           m_mConstV["zToStraw"][3] << endl;
      cout << "angleSt:      " << m_mConstV["angleSt"][0] << " " << m_mConstV["angleSt"][1] << " " << m_mConstV["angleSt"][2] << " " <<
           m_mConstV["angleSt"][3] << endl;
      cout << "wireZError:   " << m_mConstV["wireZError"][0] << " " << m_mConstV["wireZError"][1] << " " << m_mConstV["wireZError"][2] <<
           " " << m_mConstV["wireZError"][3] << endl;
      cout << "driftZError:  " << m_mConstV["driftZError"][0] << " " << m_mConstV["driftZError"][1] << " " << m_mConstV["driftZError"][2]
           << " " << m_mConstV["driftZError"][3] << endl;
      cout << "wirePhiError: " << m_mConstV["wirePhi2DError"][0] << " " << m_mConstV["wirePhi2DError"][1] << " " <<
           m_mConstV["wirePhi2DError"][2] << " " << m_mConstV["wirePhi2DError"][3] << " " << m_mConstV["wirePhi2DError"][4] << endl;
      cout << "driftPhiError: " << m_mConstV["driftPhi2DError"][0] << " " << m_mConstV["driftPhi2DError"][1] << " " <<
           m_mConstV["driftPhi2DError"][2] << " " << m_mConstV["driftPhi2DError"][3] << " " << m_mConstV["driftPhi2DError"][4] << endl;
    }


  }

  int TRGCDCFitter3D::doit(std::vector<TRGCDCTrack*>& trackList)
  {

    TRGDebug::enterStage("Fitter 3D");

    // Set values for saving data.
    if (m_mBool["fRootFile"]) {
      m_mDouble["iSave"] = 0;
      HandleRoot::initializeEvent(m_mTClonesArray);
    }

    // Get common values for event.
    m_mDouble["eventTime"] = m_cdc.getEventTime();
    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    // Event starts from 0.
    m_mDouble["eventNumber"] = eventMetaDataPtr->getEvent();

    // Fitter3D
    // Loop over all tracks
    for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {

      TCTrack& aTrack = * trackList[iTrack];

      ///////////////////////////////////////
      // Get MC values for fitter.
      if (m_mBool["fMc"]) getMCValues(m_cdc, &aTrack, m_mConstD, m_mDouble, m_mVector);
      // Get track ID
      m_mDouble["trackId"] = aTrack.getTrackID();

      ///////////////////////////////////
      //// 2D Fitter
      int fit2DResult = do2DFit(aTrack, m_mBool, m_mConstD, m_mConstV, m_mDouble, m_mVector);
      if (fit2DResult != 0) {
        aTrack.setFitted(0);
        if (m_mBool["fVerbose"]) cout << "Exit due to 2D fit result:" << fit2DResult << endl;
        continue;
      }

      /////////////////////////////////
      // 3D Fitter
      // Print input TSs
      if (m_mBool["fVerbose"]) {
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          const vector<TCLink*>& links = aTrack.links(iSt * 2 + 1);
          const unsigned nSegments = links.size();
          cout << "iSt:" << iSt << " nSegments:" << nSegments << endl;
          for (unsigned iTS = 0; iTS < nSegments; iTS++) {
            const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
            cout << "  tsId:" << t_segment->localId()
                 << " tdc:" << t_segment->priorityTime() << " lr:" << t_segment->LUT()->getValue(t_segment->lutPattern())
                 << " priorityPosition:" << t_segment->priorityPosition() << endl;
          }
        }
      }
      // Check which stereo super layers should be used.
      m_mVector["useStSl"] = vector<double> (4);
      findHitStereoSuperlayers(aTrack, m_mVector["useStSl"], m_mBool["fIsPrintError"]);
      removeImpossibleStereoSuperlayers(m_mVector["useStSl"]);
      m_mDouble["nHitStSl"] = m_mVector["useStSl"][0] + m_mVector["useStSl"][1] + m_mVector["useStSl"][2] + m_mVector["useStSl"][3];

      m_mVector["useSl"] = vector<double> (9);
      for (unsigned iAx = 0; iAx < 5; iAx++) m_mVector["useSl"][2 * iAx] = m_mVector["useAxSl"][iAx];
      for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["useSl"][2 * iSt + 1] = m_mVector["useAxSl"][iSt];

      // Fill information for stereo layers
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          const vector<TCLink*>& links = aTrack.links(iSt * 2 + 1);
          const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[0]->hit()->cell());
          m_mVector["tsId"][iSt * 2 + 1] = t_segment->localId();
          m_mVector["wirePhi"][iSt * 2 + 1] = (double) t_segment->localId() / m_mConstV["nWires"][iSt * 2 + 1] * 4 * m_mConstD["Trg_PI"];
          m_mVector["lutLR"][iSt * 2 + 1] = t_segment->LUT()->getValue(t_segment->lutPattern());
          if (m_mBool["fMc"]) m_mVector["mcLR"][iSt * 2 + 1] = t_segment->hit()->mcLR() + 1;
          m_mVector["driftLength"][iSt * 2 + 1] = t_segment->hit()->drift();
          m_mVector["tdc"][iSt * 2 + 1] = t_segment->priorityTime();
          if (m_mBool["fmcLR"] == 1) m_mVector["LR"][iSt * 2 + 1] = m_mVector["mcLR"][iSt * 2 + 1];
          else if (m_mBool["fLRLUT"] == 1) m_mVector["LR"][iSt * 2 + 1] = m_mVector["lutLR"][iSt * 2 + 1];
          else m_mVector["LR"][iSt * 2 + 1] = 3;
        } else {
          m_mVector["tsId"][iSt * 2 + 1] = 9999;
          m_mVector["wirePhi"][iSt * 2 + 1] = 9999;
          m_mVector["lutLR"][iSt * 2 + 1] = 0;
          if (m_mBool["fMc"]) m_mVector["mcLR"][iSt * 2 + 1] = 9999;
          m_mVector["driftLength"][iSt * 2 + 1] = 9999;
          m_mVector["tdc"][iSt * 2 + 1] = 9999;
          if (m_mBool["fmcLR"] == 1) m_mVector["LR"][iSt * 2 + 1] = 9999;
          else if (m_mBool["fLRLUT"] == 1) m_mVector["LR"][iSt * 2 + 1] = 9999;
          else m_mVector["LR"][iSt * 2 + 1] = 9999;
        }
      } // End superlayer loop

      // Calculate phi3D.
      m_mVector["phi3D"] = vector<double> (4);
      if (m_mDouble["eventTime"] == 9999) {
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          m_mVector["phi3D"][iSt] = m_mVector["wirePhi"][iSt * 2 + 1];
        }
      } else {
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          if (m_mVector["useStSl"][iSt] == 1) {
            // Get drift length from table.
            string tableName = "driftLengthTableSL" + to_string(iSt * 2 + 1);
            double t_driftTime = m_mVector["tdc"][iSt * 2 + 1] - m_mDouble["eventTime"];
            if (t_driftTime < 0) t_driftTime = 0;
            double t_driftLength = m_mConstV[tableName][(unsigned)t_driftTime];
            m_mVector["phi3D"][iSt] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iSt * 2 + 1], t_driftLength, m_mConstV["rr3D"][iSt],
                                                              m_mVector["LR"][iSt * 2 + 1]);
          } else {
            m_mVector["phi3D"][iSt] = 9999;
          }
        }
      }
      // Get zerror for 3D fit
      m_mVector["zError"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          // Check LR.
          if (m_mVector["LR"][2 * iSt + 1] != 3) m_mVector["zError"][iSt] = m_mConstV["driftZError"][iSt];
          else m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
          // Check eventTime
          if (m_mDouble["eventTime"] == 9999) m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
        } else {
          m_mVector["zError"][iSt] = 9999;
        }
      }
      // Get inverse zerror ^ 2
      m_mVector["iZError2"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          m_mVector["iZError2"][iSt] = 1 / pow(m_mVector["zError"][iSt], 2);
        } else {
          m_mVector["iZError2"][iSt] = 0;
        }
      }

      // Calculate zz
      m_mVector["zz"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          //m_mVector["zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge"], m_mConstV["angleSt"][iSt], m_mConstV["zToStraw"][iSt], m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
          m_mVector["zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge2D"], m_mConstV["angleSt"][iSt], m_mConstV["zToStraw"][iSt],
                                                       m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
        } else {
          m_mVector["zz"][iSt] = 0;
        }
      }
      // Calculate arcS
      m_mVector["arcS"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          m_mVector["arcS"][iSt] = Fitter3DUtility::calS(m_mDouble["rho"], m_mConstV["rr3D"][iSt]);
        } else {
          m_mVector["arcS"][iSt] = 0;
        }
      }
      // Fit3D
      m_mDouble["z0"] = 0;
      m_mDouble["cot"] = 0;
      m_mDouble["zChi2"] = 0;
      Fitter3DUtility::rSFit(&m_mVector["iZError2"][0], &m_mVector["arcS"][0], &m_mVector["zz"][0], m_mDouble["z0"], m_mDouble["cot"],
                             m_mDouble["zChi2"]);
      // Change to deg
      m_mDouble["theta"] = m_mConstD["Trg_PI"] / 2 - atan(m_mDouble["cot"]);
      m_mDouble["theta"] = 180 / m_mConstD["Trg_PI"];

      if (m_mBool["fVerbose"]) print3DInformation(iTrack);

      // For failed fits. When cot is 0 or nan.
      if (m_mDouble["cot"] == 0 || std::isnan(m_mDouble["cot"])) {
        aTrack.setFitted(0);
        aTrack.setDebugValue(TRGCDCTrack::EDebugValueType::fit3D, 1);
        if (m_mBool["fVerbose"]) cout << "Exit due to 3D fit cot result:" << m_mDouble["cot"] << endl;
        continue;
      }

      // Set track in trackList
      if (m_mBool["fVerbose"]) cout << "Fit was done successfully." << endl;
      // Set Helix parameters
      TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0));
      CLHEP::HepVector a(5);
      a = aTrack.helix().a();
      aTrack.setFitted(1);
      //if(m_mDouble["charge"]<0)
      if (m_mDouble["charge2D"] < 0) {
        a[1] = fmod(m_mDouble["phi0"] + m_mConstD["Trg_PI"], 2 * m_mConstD["Trg_PI"]);
      } else {
        a[1] = m_mDouble["phi0"];
      }
      //a[2] = 1/m_mDouble["pt"]*m_mDouble["charge"];
      a[2] = 1 / m_mDouble["pt"] * m_mDouble["charge2D"];
      a[3] = m_mDouble["z0"];
      a[4] = m_mDouble["cot"];
      helix.a(a);
      aTrack.set2DFitChi2(m_mDouble["fit2DChi2"]);
      aTrack.set3DFitChi2(m_mDouble["zChi2"]);
      aTrack.setHelix(helix);

      ///////////////
      // Save values
      if (m_mBool["fRootFile"]) {
        if (m_fileFitter3D == 0) {
          m_fileFitter3D = new TFile(m_rootFitter3DFileName.c_str(), "RECREATE");
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

    } // End track loop

    // Save values to file
    // To prevent crash when there are no tracks in first event.
    // If there is no track in first case then the ROOT saving functions might fail.
    // This is due to bad software design.
    // The first event that have tracks will be event 0 in ROOT file.
    if (m_mBool["fRootFile"]) {
      if (m_fileFitter3D != 0) m_treeTrackFitter3D->Fill();
    }

    if (m_mBool["debugFitted"]) {
      for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
        TCTrack& aTrack = * trackList[iTrack];
        if (aTrack.fitted() == 0) aTrack.setDebugValue(TRGCDCTrack::EDebugValueType::fit3D, 1);
      }
    }
    if (m_mBool["debugLargeZ0"]) {
      // If 3D fit z0 is larger or smaller than expected.
      for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
        TCTrack& aTrack = *trackList[iTrack];
        if (aTrack.fitted()) {
          double fitZ0 = aTrack.helix().dz();
          if (fitZ0 > 20 || fitZ0 < -20) aTrack.setDebugValue(TRGCDCTrack::EDebugValueType::fit3D, 1);
        }
      }
    }


    TRGDebug::leaveStage("Fitter 3D");
    return 1;

  }

  int TRGCDCFitter3D::doitComplex(std::vector<TRGCDCTrack*>& trackList)
  {

    TRGDebug::enterStage("Fitter 3D");

    // Set values for saving data.
    if (m_mBool["fRootFile"]) {
      m_mDouble["iSave"] = 0;
      HandleRoot::initializeEvent(m_mTClonesArray);
    }
    if (m_commonData == 0) {
      m_commonData = new Belle2::TRGCDCJSignalData();
    }

    // Get common values for event.
    m_mDouble["eventTime"] = m_cdc.getEventTime();
    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    // Event starts from 0.
    m_mDouble["eventNumber"] = eventMetaDataPtr->getEvent();

    // Fitter3D
    // Loop over all tracks
    for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {

      TCTrack& aTrack = * trackList[iTrack];

      /////////////////////////////////
      // Get MC values for 3D fitter
      if (m_mBool["fMc"]) getMCValues(m_cdc, &aTrack, m_mConstD,  m_mDouble, m_mVector);
      // Get input values for 3D fitter
      // Get event and track ID
      m_mDouble["trackId"] = aTrack.getTrackID();

      ///////////////////////////////////
      //// 2D Fitter
      int fit2DResult = do2DFit(aTrack, m_mBool, m_mConstD, m_mConstV, m_mDouble, m_mVector);
      if (fit2DResult != 0) {
        aTrack.setFitted(0);
        continue;
      }

      //////////////////////
      // Start of 3D fitter
      // Print input TSs
      if (m_mBool["fVerbose"]) {
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          const vector<TCLink*>& links = aTrack.links(iSt * 2 + 1);
          const unsigned nSegments = links.size();
          cout << "iSt:" << iSt << " nSegments:" << nSegments << endl;
          for (unsigned iTS = 0; iTS < nSegments; iTS++) {
            const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
            cout << "  tsId:" << t_segment->localId()
                 << " tdc:" << t_segment->priorityTime() << " lr:" << t_segment->LUT()->getValue(t_segment->lutPattern())
                 << " priorityPosition:" << t_segment->priorityPosition() << endl;
          }
        }
      }
      m_mVector["useStSl"] = vector<double> (4);
      findHitStereoSuperlayers(aTrack, m_mVector["useStSl"], m_mBool["fIsPrintError"]);
      removeImpossibleStereoSuperlayers(m_mVector["useStSl"]);
      m_mDouble["nHitStSl"] = m_mVector["useStSl"][0] + m_mVector["useStSl"][1] + m_mVector["useStSl"][2] + m_mVector["useStSl"][3];

      // Fill information for stereo layers
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          const vector<TCLink*>& links = aTrack.links(iSt * 2 + 1);
          const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[0]->hit()->cell());
          m_mVector["tsId"][iSt * 2 + 1] = t_segment->localId();
          m_mVector["wirePhi"][iSt * 2 + 1] = (double) t_segment->localId() / m_mConstV["nWires"][iSt * 2 + 1] * 4 * m_mConstD["Trg_PI"];
          m_mVector["lutLR"][iSt * 2 + 1] = t_segment->LUT()->getValue(t_segment->lutPattern());
          if (m_mBool["fMc"]) m_mVector["mcLR"][iSt * 2 + 1] = t_segment->hit()->mcLR() + 1;
          m_mVector["driftLength"][iSt * 2 + 1] = t_segment->hit()->drift();
          m_mVector["tdc"][iSt * 2 + 1] = t_segment->priorityTime();
          if (m_mBool["fmcLR"] == 1) m_mVector["LR"][iSt * 2 + 1] = m_mVector["mcLR"][iSt * 2 + 1];
          else if (m_mBool["fLRLUT"] == 1) m_mVector["LR"][iSt * 2 + 1] = m_mVector["lutLR"][iSt * 2 + 1];
          else m_mVector["LR"][iSt * 2 + 1] = 3;
        } else {
          m_mVector["tsId"][iSt * 2 + 1] = 0;
          m_mVector["wirePhi"][iSt * 2 + 1] = 9999;
          m_mVector["lutLR"][iSt * 2 + 1] = 0;
          if (m_mBool["fMc"]) m_mVector["mcLR"][iSt * 2 + 1] = 9999;
          m_mVector["driftLength"][iSt * 2 + 1] = 9999;
          m_mVector["tdc"][iSt * 2 + 1] = 0;
          if (m_mBool["fmcLR"] == 1) m_mVector["LR"][iSt * 2 + 1] = 9999;
          else if (m_mBool["fLRLUT"] == 1) m_mVector["LR"][iSt * 2 + 1] = 9999;
          else m_mVector["LR"][iSt * 2 + 1] = 9999;
        }
      } // End superlayer loop
      //// Test method to find event time using hit TS's tdc.
      //int minTSTdc = 9999;
      //for(unsigned iSl=0; iSl<9; iSl++){
      //  if (minTSTdc > m_mVector["tdc"][iSl]) minTSTdc = m_mVector["tdc"][iSl];
      //}
      //m_mDouble["eventTime"] = minTSTdc;

      // Calculate phi3D.
      m_mVector["phi3D"] = vector<double> (4);
      if (m_mDouble["eventTime"] == 9999) {
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          m_mVector["phi3D"][iSt] = m_mVector["wirePhi"][iSt * 2 + 1];
        }
      } else {
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          // Get drift length from table.
          string tableName = "driftLengthTableSL" + to_string(iSt * 2 + 1);
          double t_driftTime = m_mVector["tdc"][iSt * 2 + 1] - m_mDouble["eventTime"];
          if (t_driftTime < 0) t_driftTime = 0;
          double t_driftLength = m_mConstV[tableName][(unsigned)t_driftTime];
          m_mVector["phi3D"][iSt] = Fitter3DUtility::calPhi(m_mVector["wirePhi"][iSt * 2 + 1], t_driftLength, m_mConstV["rr3D"][iSt],
                                                            m_mVector["LR"][iSt * 2 + 1]);
        }
      }

      // Get zerror for 3D fit
      m_mVector["zError"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        // Check LR.
        if (m_mVector["LR"][2 * iSt + 1] != 3) m_mVector["zError"][iSt] = m_mConstV["driftZError"][iSt];
        else m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
        // Check eventTime
        if (m_mDouble["eventTime"] == 9999) m_mVector["zError"][iSt] = m_mConstV["wireZError"][iSt];
      }
      // Get inverse zerror ^ 2
      m_mVector["iZError2"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["useStSl"][iSt] == 1) {
          m_mVector["iZError2"][iSt] = 1 / pow(m_mVector["zError"][iSt], 2);
        } else {
          m_mVector["iZError2"][iSt] = 0;
        }
      }

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
      int phiBitSize = 13;
      // pt = 0.3*1.5*rho*0.01;
      //double rhoMin = 48;
      /* cppcheck-suppress variableScope */
      double rhoMin = 20;
      double rhoMax = 2500;
      //double rhoMax = 1600;
      // 5bit (clock counter) + 4 bit (2ns resolution)
      m_mConstD["tdcBitSize"] = 9;
      m_mConstD["rhoBitSize"] = 11;
      m_mConstD["iError2BitSize"] = 8;
      m_mConstD["iError2Max"] = 1 / pow(m_mConstV["wireZError"][0], 2);
      // LUT values
      m_mConstD["JB"] = 0;
      m_mConstD["driftPhiLUTOutBitSize"] = phiBitSize - 1;
      m_mConstD["driftPhiLUTInBitSize"] = m_mConstD["tdcBitSize"];
      m_mConstD["acosLUTOutBitSize"] = phiBitSize - 1;
      m_mConstD["acosLUTInBitSize"] = m_mConstD["rhoBitSize"];
      m_mConstD["zLUTInBitSize"] = phiBitSize;
      m_mConstD["zLUTOutBitSize"] = 9;
      m_mConstD["iDenLUTInBitSize"] = 13;
      m_mConstD["iDenLUTOutBitSize"] = 11; // To increase wireZError = 2.5*driftZError
      // Rotate by quadrants depending on charge and cc(circle center)
      m_mDouble["relRefPhi"] = 0;
      int t_quadrant = Fitter3DUtility::findQuadrant(m_mDouble["phi0"]);
      //if (m_mDouble["charge"] == -1)
      if (m_mDouble["charge2D"] == -1) {
        if (t_quadrant == 1) m_mDouble["relRefPhi"] = 0;
        else if (t_quadrant == 2) m_mDouble["relRefPhi"] = -m_mConstD["Trg_PI"] / 2;
        else if (t_quadrant == 3) m_mDouble["relRefPhi"] = -m_mConstD["Trg_PI"];
        else if (t_quadrant == 4) m_mDouble["relRefPhi"] = m_mConstD["Trg_PI"] / 2;
      } else {
        if (t_quadrant == 1) m_mDouble["relRefPhi"] = m_mConstD["Trg_PI"] / 2;
        else if (t_quadrant == 2) m_mDouble["relRefPhi"] = 0;
        else if (t_quadrant == 3) m_mDouble["relRefPhi"] = -m_mConstD["Trg_PI"] / 2;
        else if (t_quadrant == 4) m_mDouble["relRefPhi"] = -m_mConstD["Trg_PI"];
      }
      // Rotate phi
      m_mDouble["relPhi0"] = Fitter3DUtility::rotatePhi(m_mDouble["phi0"], m_mDouble["relRefPhi"]);
      m_mVector["relPhi3D"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4;
           iSt++) m_mVector["relPhi3D"][iSt] = Fitter3DUtility::rotatePhi(m_mVector["phi3D"][iSt], m_mDouble["relRefPhi"]);

      // Rotate wirePhi
      m_mVector["relWirePhi3D"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        double t_relWirePhi = Fitter3DUtility::rotatePhi(m_mVector["wirePhi"][2 * iSt + 1], m_mDouble["relRefPhi"]);
        bool rangeOk = 0;
        while (rangeOk == 0) {
          if (t_relWirePhi < 0) t_relWirePhi += 2 * m_mConstD["Trg_PI"];
          else if (t_relWirePhi > 2 * m_mConstD["Trg_PI"]) t_relWirePhi -= 2 * m_mConstD["Trg_PI"];
          else rangeOk = 1;
        }
        m_mVector["relWirePhi3D"][iSt] = t_relWirePhi;
      }

      // Rotate tsId
      m_mVector["relTsId3D"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4;
           iSt++) m_mVector["relTsId3D"][iSt] = Fitter3DUtility::rotateTsId(m_mVector["tsId"][2 * iSt + 1],
                                                  m_mDouble["relRefPhi"] / m_mConstD["Trg_PI"] / 2 * m_mConstV["nTSs"][2 * iSt + 1], m_mConstV["nTSs"][2 * iSt + 1]);

      // Constrain rho to rhoMax. For removing warnings when changing to signals.
      if (m_mDouble["rho"] > rhoMax) {
        m_mDouble["rho"] = rhoMax;
        m_mDouble["pt"] = rhoMax * 0.3 * 1.5 * 0.01;
      }

      // Constrain event time
      m_mDouble["eventTimeValid"] = 1;
      if (m_mDouble["eventTime"] == 9999) m_mDouble["eventTimeValid"] = 0;
      // Change tdc and eventTime to 9 bit unsigned value. (Ex: -1 => 511, -2 => 510)
      m_mVector["unsignedTdc"] = vector<double> (9);
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        //cout<<"iSt:"<<iSt<<" tdc:"<<m_mVector["tdc"][2*iSt+1]<<" unsignedTdc:"<<Fitter3DUtility::toUnsignedTdc(m_mVector["tdc"][2*iSt+1], 9)<<endl;
        m_mVector["unsignedTdc"][2 * iSt + 1] = Fitter3DUtility::toUnsignedTdc(m_mVector["tdc"][2 * iSt + 1], m_mConstD["tdcBitSize"]);
      }
      m_mDouble["unsignedEventTime"] = Fitter3DUtility::toUnsignedTdc(m_mDouble["eventTime"], m_mConstD["tdcBitSize"]);
      //cout<<"eventTime:"<<m_mDouble["eventTime"]<<" unsignedEventTime:"<<Fitter3DUtility::toUnsignedTdc(m_mDouble["eventTime"], 9)<<endl;

      // Change to Signals.
      {
        vector<tuple<string, double, int, double, double, int> > t_values = {
          make_tuple("phi0", m_mDouble["relPhi0"], phiBitSize, phiMin, phiMax, 0),
          make_tuple("rho", m_mDouble["rho"], m_mConstD["rhoBitSize"], rhoMin, rhoMax, 0),
          //make_tuple("charge",(int) (m_mDouble["charge"]==1 ? 1 : 0), 1, 0, 1.5, 0),
          make_tuple("charge", (int)(m_mDouble["charge2D"] == 1 ? 1 : 0), 1, 0, 1.5, 0),
          make_tuple("lr_0", m_mVector["lutLR"][1], 2, 0, 3.5, 0),
          make_tuple("lr_1", m_mVector["lutLR"][3], 2, 0, 3.5, 0),
          make_tuple("lr_2", m_mVector["lutLR"][5], 2, 0, 3.5, 0),
          make_tuple("lr_3", m_mVector["lutLR"][7], 2, 0, 3.5, 0),
          make_tuple("tsId_0", m_mVector["relTsId3D"][0], ceil(log(m_mConstV["nTSs"][1]) / log(2)), 0, pow(2, ceil(log(m_mConstV["nTSs"][1]) / log(2))) - 0.5, 0),
          make_tuple("tsId_1", m_mVector["relTsId3D"][1], ceil(log(m_mConstV["nTSs"][3]) / log(2)), 0, pow(2, ceil(log(m_mConstV["nTSs"][3]) / log(2))) - 0.5, 0),
          make_tuple("tsId_2", m_mVector["relTsId3D"][2], ceil(log(m_mConstV["nTSs"][5]) / log(2)), 0, pow(2, ceil(log(m_mConstV["nTSs"][5]) / log(2))) - 0.5, 0),
          make_tuple("tsId_3", m_mVector["relTsId3D"][3], ceil(log(m_mConstV["nTSs"][7]) / log(2)), 0, pow(2, ceil(log(m_mConstV["nTSs"][7]) / log(2))) - 0.5, 0),
          make_tuple("tdc_0", m_mVector["unsignedTdc"][1], m_mConstD["tdcBitSize"], 0, pow(2, m_mConstD["tdcBitSize"]) - 0.5, 0),
          make_tuple("tdc_1", m_mVector["unsignedTdc"][3], m_mConstD["tdcBitSize"], 0, pow(2, m_mConstD["tdcBitSize"]) - 0.5, 0),
          make_tuple("tdc_2", m_mVector["unsignedTdc"][5], m_mConstD["tdcBitSize"], 0, pow(2, m_mConstD["tdcBitSize"]) - 0.5, 0),
          make_tuple("tdc_3", m_mVector["unsignedTdc"][7], m_mConstD["tdcBitSize"], 0, pow(2, m_mConstD["tdcBitSize"]) - 0.5, 0),
          make_tuple("eventTime", m_mDouble["unsignedEventTime"], m_mConstD["tdcBitSize"], 0, pow(2, m_mConstD["tdcBitSize"]) - 0.5, 0),
          make_tuple("eventTimeValid", (int) m_mDouble["eventTimeValid"], 1, 0, 1.5, 0),
        };
        TRGCDCJSignal::valuesToMapSignals(t_values, m_commonData, m_mSignalStorage);
      }

      Fitter3DUtility::setError(m_mConstD, m_mConstV, m_mSignalStorage);
      Fitter3DUtility::calPhi(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
      Fitter3DUtility::constrainRPerStSl(m_mConstV, m_mSignalStorage);
      Fitter3DUtility::calZ(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
      Fitter3DUtility::calS(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
      Fitter3DUtility::rSFit(m_mConstD, m_mConstV, m_mSignalStorage, m_mLutStorage);
      // Change to values.
      vector<tuple<string, double, int, double, double, int> > resultValues;
      {
        vector<pair<string, int> > t_chooseSignals = {
          make_pair("z0", 0), make_pair("cot", 0), make_pair("chi2Sum", 0)
        };
        TRGCDCJSignal::mapSignalsToValues(m_mSignalStorage, t_chooseSignals, resultValues);
      }

      // Post handling of signals.
      // Name all signals.
      if ((*m_mSignalStorage.begin()).second.getName() == "") {
        for (auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it) {
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
      if (m_mBool["fRootFile"]) {
        // Check if there is a name.
        if ((*m_mSignalStorage.begin()).second.getName() != "") {

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
            // Changes names with "_" to m_mVector.
            HandleRoot::convertSignalValuesToMaps(outValues, m_mDouble, m_mVector);
          }
        }
      }

      m_mBool["fVHDLFile"] = 0;
      if (m_mBool["fVHDLFile"]) {
        // Check if there is a name.
        if ((*m_mSignalStorage.begin()).second.getName() != "") {
          // Saves to file only one time.
          saveVhdlAndCoe();
          // Saves values to memory. Wrote to file at terminate().
          saveAllSignals();
          saveIoSignals();
        }
      }

      //// Simple version of Fitter3D for comparison with complex mode.
      // Calculate zz
      m_mVector["float_zz"] = vector<double> (4);
      //for (unsigned iSt = 0; iSt < 4; iSt++) m_mVector["float_zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge"], m_mConstV["angleSt"][iSt], m_mConstV["zToStraw"][iSt], m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
      for (unsigned iSt = 0; iSt < 4;
           iSt++) m_mVector["float_zz"][iSt] = Fitter3DUtility::calZ(m_mDouble["charge2D"], m_mConstV["angleSt"][iSt],
                                                 m_mConstV["zToStraw"][iSt], m_mConstV["rr3D"][iSt], m_mVector["phi3D"][iSt], m_mDouble["rho"], m_mDouble["phi0"]);
      // Calculate arcS
      m_mVector["float_arcS"] = vector<double> (4);
      for (unsigned iSt = 0; iSt < 4;
           iSt++) m_mVector["float_arcS"][iSt] = Fitter3DUtility::calS(m_mDouble["rho"], m_mConstV["rr3D"][iSt]);
      // Fit3D
      m_mDouble["float_z0"] = 0;
      m_mDouble["float_cot"] = 0;
      m_mDouble["float_zChi2"] = 0;
      Fitter3DUtility::rSFit(&m_mVector["iZError2"][0], &m_mVector["float_arcS"][0], &m_mVector["float_zz"][0], m_mDouble["float_z0"],
                             m_mDouble["float_cot"], m_mDouble["float_zChi2"]);

      if (m_mBool["fVerbose"]) {
        for (unsigned iSt = 0; iSt < 4; iSt++) cout << "float_zz[" << iSt << "] : " << m_mVector["float_zz"][iSt] << " ";
        cout << endl;
        for (unsigned iSt = 0; iSt < 4; iSt++) cout << "float_arcS[" << iSt << "] : " << m_mVector["float_arcS"][iSt] << " ";
        cout << endl;
        cout << "float_z0: " << m_mDouble["float_z0"] << endl;
        cout << "float_zChi2: " << m_mDouble["float_zChi2"] << endl;

        print3DInformation(iTrack);
      }

      // For failed fits.
      if (m_mDouble["cot"] == 0) {
        aTrack.setFitted(0);
        continue;
      }

      // Set track in trackListOut.
      // Set Helix parameters
      TRGCDCHelix helix(ORIGIN, CLHEP::HepVector(5, 0), CLHEP::HepSymMatrix(5, 0));
      CLHEP::HepVector a(5);
      a = aTrack.helix().a();
      aTrack.setFitted(1);
      //if(m_mDouble["charge"]<0)
      if (m_mDouble["charge2D"] < 0) {
        a[1] = fmod(m_mDouble["phi0"] + m_mConstD["Trg_PI"], 2 * m_mConstD["Trg_PI"]);
      } else {
        a[1] = m_mDouble["phi0"];
      }
      //a[2] = 1/m_mDouble["pt"]*m_mDouble["charge"];
      a[2] = 1 / m_mDouble["pt"] * m_mDouble["charge2D"];
      a[3] = m_mDouble["z0"];
      a[4] = m_mDouble["cot"];
      helix.a(a);
      aTrack.setHelix(helix);
      aTrack.set2DFitChi2(m_mDouble["fit2DChi2"]);
      aTrack.set3DFitChi2(m_mDouble["zChi2"]);

      ///////////////
      // Save values
      if (m_mBool["fRootFile"]) {
        if (m_fileFitter3D == 0) {
          m_fileFitter3D = new TFile(m_rootFitter3DFileName.c_str(), "RECREATE");
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

    } // End track loop

    // Save values to file
    // To prevent crash when there are no tracks in first event.
    // If there is no track in first evnet then the HandleRoot saving functions will fail.
    // This is due to bad HandleRoot software design.
    // The first event that have tracks will be event 0 in ROOT file.
    if (m_mBool["fRootFile"]) {
      if (m_fileFitter3D != 0) m_treeTrackFitter3D->Fill();
    }

    if (m_mBool["debugFitted"]) {
      for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
        TCTrack& aTrack = * trackList[iTrack];
        if (aTrack.fitted() == 0) aTrack.setDebugValue(TRGCDCTrack::EDebugValueType::fit3D, 1);
      }
    }
    if (m_mBool["debugLargeZ0"]) {
      // If 3D fit z0 is larger or smaller than expected.
      for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
        TCTrack& aTrack = *trackList[iTrack];
        if (aTrack.fitted()) {
          double fitZ0 = aTrack.helix().dz();
          if (fitZ0 > 20 || fitZ0 < -20) aTrack.setDebugValue(TRGCDCTrack::EDebugValueType::fit3D, 1);
        }
      }
    }

    TRGDebug::leaveStage("Fitter 3D");

    return 1;

  }

  double TRGCDCFitter3D::calPhi(TRGCDCSegmentHit const* segmentHit, double eventTime)
  {
    const CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    unsigned localId = segmentHit->segment().center().localId();
    unsigned layerId = segmentHit->segment().center().layerId();
    int nWires = cdcp.nWiresInLayer(layerId) * 2;
    double rr = cdcp.senseWireR(layerId);
    double tdc = segmentHit->segment().priorityTime();
    int lr = segmentHit->segment().LUT()->getValue(segmentHit->segment().lutPattern());
    return Fitter3DUtility::calPhi(localId, nWires, tdc, eventTime, rr, lr);
  }


  /////////////////////////////////////////////////////////////////////////////////////////////
  ////// Function for saving
  /////////////////////////////////////////////////////////////////////////////////////////////
  // Functions for saving.
  void TRGCDCFitter3D::saveVhdlAndCoe()
  {
    // Print Vhdl
    if (m_commonData->getPrintedToFile() == 0) {
      if (m_commonData->getPrintVhdl() == 0) {
        m_commonData->setVhdlOutputFile("Fitter3D.vhd");
        m_commonData->setPrintVhdl(1);
      } else {
        m_commonData->setPrintVhdl(0);
        m_commonData->entryVhdlCode();
        m_commonData->signalsVhdlCode();
        m_commonData->buffersVhdlCode();
        m_commonData->printToFile();
        // Print LUTs.
        for (map<string, TRGCDCJLUT*>::iterator it = m_mLutStorage.begin(); it != m_mLutStorage.end(); ++it) {
          //it->second->makeCOE("./VHDL/LutData/"+it->first+".coe");
          it->second->makeCOE(it->first + ".coe");
        }
      }
    }
  }

  void TRGCDCFitter3D::saveAllSignals()
  {
    // Save all signals to m_mSavedSignals. Limit to 10 times.
    if ((*m_mSavedSignals.begin()).second.size() < 10 || m_mSavedSignals.empty()) {
      for (auto it = m_mSignalStorage.begin(); it != m_mSignalStorage.end(); ++it) {
        m_mSavedSignals[(*it).first].push_back((*it).second.getInt());
      }
    }
  }

  void TRGCDCFitter3D::saveIoSignals()
  {
    // Save input output signals. Limit to 1024.
    if ((*m_mSavedIoSignals.begin()).second.size() < 1025 || m_mSavedIoSignals.empty()) {
      m_mSavedIoSignals["phi0"].push_back(m_mSignalStorage["phi0"].getInt());
      for (unsigned iSt = 0; iSt < 4;
           iSt++) m_mSavedIoSignals["phi_" + to_string(iSt)].push_back(m_mSignalStorage["phi_" + to_string(iSt)].getInt());
      m_mSavedIoSignals["rho"].push_back(m_mSignalStorage["rho"].getInt());
      for (unsigned iSt = 0; iSt < 4;
           iSt++) m_mSavedIoSignals["iZError2_" + to_string(iSt)].push_back(m_mSignalStorage["iZError2_" + to_string(iSt)].getInt());
      //m_mSavedIoSignals["charge"].push_back(m_mSignalStorage["charge"].getInt());
      m_mSavedIoSignals["charge"].push_back(m_mSignalStorage["charge"].getInt());
      m_mSavedIoSignals["z0"].push_back(m_mSignalStorage["z0"].getInt());
      m_mSavedIoSignals["cot"].push_back(m_mSignalStorage["cot"].getInt());
      m_mSavedIoSignals["chi2Sum"].push_back(m_mSignalStorage["chi2Sum"].getInt());
    }
  }


  void TRGCDCFitter3D::getMCValues(const TRGCDC& m_cdc_in, TRGCDCTrack* aTrack, const std::map<std::string, double>& m_mConstD_in,
                                   std::map<std::string, double>& m_mDouble_in, std::map<std::string, std::vector<double> >& m_mVector_in)
  {
    // Access to track's MC particle.
    const TCRelation& trackRelation = aTrack->relation();
    // Biggest contibutor is 0. Next is 1 and so on.
    const MCParticle& trackMCParticle = trackRelation.mcParticle(0);

    // Calculated impact position
    TVector3 vertex = trackMCParticle.getVertex();
    TLorentzVector vector4 = trackMCParticle.get4Vector();
    TVector2 helixCenter;
    TVector3 impactPosition;
    Fitter3DUtility::findImpactPosition(&vertex, &vector4, int(m_mDouble_in["mcCharge"]), helixCenter, impactPosition);
    m_mVector_in["mcVertex"] = vector<double> ({vertex.X(), vertex.Y(), vertex.Z()});
    m_mVector_in["mcMomentum"] = vector<double> ({vector4.Px(), vector4.Py(), vector4.Pz()});
    m_mVector_in["helixCenter"] = vector<double> ({helixCenter.X(), helixCenter.Y()});
    m_mVector_in["impactPosition"] = vector<double> ({impactPosition.X(), impactPosition.Y(), impactPosition.Z()});

    // Access track's particle parameters
    m_mDouble_in["mcPt"] = trackMCParticle.getMomentum().Pt();
    m_mDouble_in["mcPhi0"] = 0;
    if (trackMCParticle.getCharge() > 0) m_mDouble_in["mcPhi0"] = trackMCParticle.getMomentum().Phi() - m_mConstD_in.at("Trg_PI") / 2;
    if (trackMCParticle.getCharge() < 0) m_mDouble_in["mcPhi0"] = trackMCParticle.getMomentum().Phi() + m_mConstD_in.at("Trg_PI") / 2;
    // Change range to [0,2pi]
    if (m_mDouble_in["mcPhi0"] < 0) m_mDouble_in["mcPhi0"] += 2 * m_mConstD_in.at("Trg_PI");
    //m_mDouble["mcZ0"] = trackMCParticle.getVertex().Z();
    m_mDouble_in["mcZ0"] = impactPosition.Z();
    m_mDouble_in["mcCot"] = trackMCParticle.getMomentum().Pz() / trackMCParticle.getMomentum().Pt();
    m_mDouble_in["mcCharge"] = trackMCParticle.getCharge();

    // mcStatus[0]: statusbit, mcStatus[1]: pdg, mcStatus[2]: charge
    TVectorD mcStatus(3);
    m_mDouble_in["mcStatus"] = trackMCParticle.getStatus();
    m_mDouble_in["pdgId"] = trackMCParticle.getPDG();

    // Find position of track for each super layer
    //...G4 trackID...
    unsigned id = trackRelation.contributor(0);
    vector<const TCSHit*> mcAllTSList[9];
    vector<const TCSHit*> mcTSList(9);
    //...Segment loop...
    const vector<const TCSHit*> hits = m_cdc_in.segmentHits();
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
    m_mVector_in["mcPosX"] = vector<double> ({9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999});
    m_mVector_in["mcPosY"] = vector<double> ({9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999});
    m_mVector_in["mcPosZ"] = vector<double> ({9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999, 9999});
    for (unsigned iSL = 0; iSL < 9; iSL++) {
      if (mcTSList[iSL] != 0) {
        TVector3 posTrack = mcTSList[iSL]->simHit()->getPosTrack();
        m_mVector_in["mcPosX"][iSL] = posTrack.X();
        m_mVector_in["mcPosY"][iSL] = posTrack.Y();
        m_mVector_in["mcPosZ"][iSL] = posTrack.Z();
      }
    }
    // Get mc LR
    m_mVector_in["simMcLR"] = vector<double> (9);
    for (unsigned iSL = 0; iSL < 9; iSL++) {
      if (mcTSList[iSL] != 0) {
        m_mVector_in["simMcLR"][iSL] = mcTSList[iSL]->simHit()->getPosFlag();
      }
    }

    //// Find helix parameters and mc position at R
    //for(unsigned iSL=0; iSL<9; iSL++){
    //  if(mcTSList[iSL]!=0) {
    //    TVectorD t_helixParameters;
    //    TVector3 t_positionAtR;
    //    TVector3 t_momentumAtR;
    //    Fitter3DUtility::calHelixParameters(mcTSList[iSL]->simHit()->getPosIn(), mcTSList[iSL]->simHit()->getMomentum(),trackMCParticle.getCharge(),t_helixParameters);
    //    //cout<<"dr: "<<helixParameters[0]<<" phi0: "<<helixParameters[1]<<" R: "<<1/helixParameters[2]/0.3/1.5*100<<" dz: "<<helixParameters[3]<<" tanLambda: "<<helixParameters[4]<<endl;
    //    //calVectorsAtR(t_helixParameters, trackMCParticle.getCharge(), m_rr[iSL]*100, t_positionAtR, t_momentumAtR);
    //    //cout<<" x: "<<t_positionAtR.X()<<"  y: "<<t_positionAtR.Y()<<"  z: "<<t_positionAtR.Z()<<endl;
    //    //cout<<"Px: "<<t_momentumAtR.X()<<" Py: "<<t_momentumAtR.Y()<<" Pz: "<<t_momentumAtR.Z()<<endl;
    //  }
    //}
  }

  bool TRGCDCFitter3D::isAxialTrackFull(const TRGCDCTrack& aTrack)
  {
    bool trackFull = 1;
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      // Check if all superlayers have one TS
      const vector<TCLink*>& links = aTrack.links(iAx * 2);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if (nSegments != 1) {
        if (nSegments == 0) {
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout << "Fitter3D::isAxialTrackFull() => There are no TS." << endl;
        } else {
          if (m_mBool["fIsPrintError"]) cout << "Fitter3D::isAxialTrackFull() => multiple TS are assigned." << endl;
        }
      } else {
        if (priorityHitTS == 0) {
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout << "Fitter3D::isAxialTrackFull() => There are no priority hit TS" << endl;
        }
      }
    } // End superlayer loop
    return trackFull;
  }

  bool TRGCDCFitter3D::isStereoTrackFull(const TRGCDCTrack& aTrack)
  {
    bool trackFull = 1;
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Check if all superlayers have one TS
      const vector<TCLink*>& links = aTrack.links(iSt * 2 + 1);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if (nSegments != 1) {
        if (nSegments == 0) {
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout << "Fitter3D::isStereoTrackFull() => There are no TS." << endl;
        } else {
          if (m_mBool["fIsPrintError"]) cout << "Fitter3D::isStereoTrackFull() => multiple TS are assigned." << endl;
        }
      } else {
        if (priorityHitTS == 0) {
          trackFull = 0;
          if (m_mBool["fIsPrintError"]) cout << "Fitter3D::isStereoTrackFull() => There are no priority hit TS" << endl;
        }
      }
    } // End superlayer loop
    return trackFull;
  }

  void TRGCDCFitter3D::findHitAxialSuperlayers(const TRGCDCTrack& aTrack,  vector<double>& useAxSl, bool printError)
  {
    useAxSl.assign(5, 1);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      // Check if all superlayers have one TS
      const vector<TCLink*>& links = aTrack.links(iAx * 2);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      //cout<<"iAx:"<<iAx<<" nSegments:"<<nSegments<<endl;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
        //cout<<" iTS:"<<iTS<<" priority:"<<t_segment->center().hit()<<" tsId:"<<t_segment->localId()<<endl;
      }
      if (nSegments != 1) {
        if (nSegments == 0) {
          useAxSl[iAx] = 0;
        } else {
          if (printError) cout << "Fitter3D::findHitAxialSuperlayers() => multiple TS are assigned." << endl;
        }
      } else {
        if (priorityHitTS == 0) {
          useAxSl[iAx] = 0;
          if (printError) cout << "Fitter3D::findHitAxialSuperlayers() => There are no priority hit TS" << endl;
        }
      }
    } // End superlayer loop
  }

  void TRGCDCFitter3D::findHitStereoSuperlayers(const TRGCDCTrack& aTrack,  vector<double>& useStSl, bool printError)
  {
    useStSl.assign(4, 1);
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Check if all superlayers have one TS
      const vector<TCLink*>& links = aTrack.links(iSt * 2 + 1);
      const unsigned nSegments = links.size();
      // Find if there is a TS with a priority hit.
      // Loop over all TS in same superlayer.
      bool priorityHitTS = 0;
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
        if (t_segment->center().hit() != 0)  priorityHitTS = 1;
      }
      if (nSegments != 1) {
        if (nSegments == 0) {
          useStSl[iSt] = 0;
        } else {
          if (printError) cout << "Fitter3D::findHitStereoSuperlayers() => multiple TS are assigned." << endl;
        }
      } else {
        if (priorityHitTS == 0) {
          useStSl[iSt] = 0;
          if (printError) cout << "Fitter3D::findHitStereoSuperlayers() => There are no priority hit TS" << endl;
        }
      }
    } // End superlayer loop
  }

  void TRGCDCFitter3D::removeImpossibleStereoSuperlayers(vector<double>& useStSl)
  {
    for (unsigned iSt = 0; iSt < 4; iSt++) {
      // Check if rho is large enough for stereo super layer.
      if (2 * m_mDouble["rho"] < m_mConstV["rr3D"][iSt]) {
        useStSl[iSt] = 0;
        if (m_mBool["fIsPrintError"]) cout << "Fitter3D::removeImpossibleStereoSuperlayers() => pt is too low for SL." << endl;
      }
    } // End superlayer loop
  }

  void TRGCDCFitter3D::selectAxialTSs(const TRGCDCTrack& aTrack, vector<int>& bestTSIndex)
  {
    bestTSIndex.resize(5);
    std::fill_n(bestTSIndex.begin(), 5, -1);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      // Check if all superlayers have one TS
      const vector<TCLink*>& links = aTrack.links(iAx * 2);
      const unsigned nSegments = links.size();
      //cout<<"iAx:"<<iAx<<" nSegments:"<<nSegments<<endl;
      // tsCandiateInfo[iTS] = 1st priority, tdc
      vector<tuple<int, double> > tsCandiateInfo(nSegments);
      // Get information from candidates.
      for (unsigned iTS = 0; iTS < nSegments; iTS++) {
        const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[iTS]->hit()->cell());
        int firstPriority = 0;
        if (t_segment->center().hit() != 0)  firstPriority = 1;
        double tdc = t_segment->priorityTime();
        std::get<0>(tsCandiateInfo[iTS]) = firstPriority;
        std::get<1>(tsCandiateInfo[iTS]) = tdc;
        //cout<<"Ax:"<<iAx<<" iTS:"<<iTS<<" firstPriority:"<<firstPriority<<" tdc:"<<tdc<<endl;
      }
      // Select best candidate.
      // bestTS => tsIndex, { 1st priority, tdc }
      pair<int, tuple<int, double> > bestTS = make_pair(-1, make_tuple(-1, 9999));
      // If there is a candidate.
      if (tsCandiateInfo.size() != 0) {
        //// Selection type first TS.
        //if (std::get<0>(tsCandiateInfo[0]) == 1) {
        //  bestTS.first = 0;
        //  bestTS.second = tsCandiateInfo[0];
        //}
        //// Selection type random.
        //int randomIndex = gRandom->Integer(nSegments);
        //if (std::get<0>(tsCandiateInfo[randomIndex]) == 1) {
        //  bestTS.first = randomIndex;
        //  bestTS.second = tsCandiateInfo[randomIndex];
        //}
        // Selection type 1st priority.
        for (unsigned iTS = 0; iTS < nSegments; iTS++) {
          if (std::get<0>(tsCandiateInfo[iTS]) == 1) {
            bool select = 0;
            if (bestTS.first == -1) select = 1;
            else if (std::get<1>(bestTS.second) > std::get<1>(tsCandiateInfo[iTS])) select = 1;
            if (select) {
              bestTS.first = iTS;
              bestTS.second = tsCandiateInfo[iTS];
            }
          }
        }
      }
      //cout<<"Ax:"<<iAx<<" best: iTS:"<<bestTS.first<<" firstPriority:"<<std::get<0>(bestTS.second)<<" tdc:"<<std::get<1>(bestTS.second)<<endl;
      bestTSIndex[iAx] = bestTS.first;
    } // End superlayer loop
  }

  int TRGCDCFitter3D::do2DFit(TRGCDCTrack& aTrack, const std::map<std::string, bool>& m_mBool_in,
                              const std::map<std::string, double>& m_mConstD_in,
                              std::map<std::string, std::vector<double> >& m_mConstV_in, std::map<std::string, double>& m_mDouble_in,
                              std::map<std::string, std::vector<double> >& m_mVector_in)
  {
    m_mVector_in["useAxSl"] = vector<double> (5);
    //findHitAxialSuperlayers(aTrack, useAxSl, m_mBool_in["fIsPrintError"]);

    // Find best TS between links for each SL.
    vector<int> bestTSIndex(5);
    selectAxialTSs(aTrack, bestTSIndex);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if (bestTSIndex[iAx] != -1) m_mVector_in["useAxSl"][iAx] = 1;
      //cout<<"useAxSl["<<iAx<<"]:"<<useAxSl[iAx]<<endl;
    }

    // Check if number of axial super layer hits is smaller or equal to 1.
    m_mDouble_in["nHitAx"] = m_mVector_in["useAxSl"][0] + m_mVector_in["useAxSl"][1] + m_mVector_in["useAxSl"][2] +
                             m_mVector_in["useAxSl"][3] +
                             m_mVector_in["useAxSl"][4];
    if (m_mDouble_in["nHitAx"] <= 1) {
      if (m_mBool_in.at("fVerbose") == 1) cout << "[2DFit] Exiting because nHitAx is " << m_mDouble_in["nHitAx"] << endl;
      aTrack.setFitted(0);
      return 1;
    }

    // Fill information for axial layers
    m_mVector_in["tsId"] = vector<double> (9);
    m_mVector_in["tsId2D"] = vector<double> (5);
    m_mVector_in["wirePhi"] = vector<double> (9);
    m_mVector_in["lutLR"] = vector<double> (9);
    m_mVector_in["LR"] = vector<double> (9);
    m_mVector_in["driftLength"] = vector<double> (9);
    m_mVector_in["tdc"] = vector<double> (9);
    if (!m_mVector_in.count("mcLR")) m_mVector_in["mcLR"] = vector<double> (9);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if (m_mVector_in["useAxSl"][iAx] == 1) {
        const vector<TCLink*>& links = aTrack.links(iAx * 2);
        //const TCSegment * t_segment = dynamic_cast<const TCSegment *>(& links[0]->hit()->cell());
        const TCSegment* t_segment = dynamic_cast<const TCSegment*>(& links[bestTSIndex[iAx]]->hit()->cell());
        m_mVector_in["tsId"][iAx * 2] = t_segment->localId();
        m_mVector_in["tsId2D"][iAx] = m_mVector_in["tsId"][iAx * 2];
        m_mVector_in["wirePhi"][iAx * 2] = (double) t_segment->localId() / m_mConstV_in["nWires"][iAx * 2] * 4 * m_mConstD_in.at("Trg_PI");
        m_mVector_in["lutLR"][iAx * 2] = t_segment->LUT()->getValue(t_segment->lutPattern());
        // mcLR should be removed.
        if (m_mBool_in.at("fMc")) m_mVector_in["mcLR"][iAx * 2] = t_segment->hit()->mcLR() + 1;
        m_mVector_in["driftLength"][iAx * 2] = t_segment->hit()->drift();
        m_mVector_in["tdc"][iAx * 2] = t_segment->priorityTime();
        if (m_mBool_in.at("fmcLR") == 1) m_mVector_in["LR"][iAx * 2] = m_mVector_in["mcLR"][iAx * 2];
        else if (m_mBool_in.at("fLRLUT") == 1) m_mVector_in["LR"][iAx * 2] = m_mVector_in["lutLR"][iAx * 2];
        else m_mVector_in["LR"][iAx * 2] = 3;
      } else {
        m_mVector_in["tsId"][iAx * 2] = 9999;
        m_mVector_in["wirePhi"][iAx * 2] = 9999;
        m_mVector_in["lutLR"][iAx * 2] = 0;
        // mcLR should be removed.
        if (m_mBool_in.at("fMc")) m_mVector_in["mcLR"][iAx * 2] = 9999;
        m_mVector_in["driftLength"][iAx * 2] = 9999;
        m_mVector_in["tdc"][iAx * 2] = 9999;
        if (m_mBool_in.at("fmcLR") == 1) m_mVector_in["LR"][iAx * 2] = 9999;
        else if (m_mBool_in.at("fLRLUT") == 1) m_mVector_in["LR"][iAx * 2] = 9999;
        else m_mVector_in["LR"][iAx * 2] = 9999;
      }
    } // End superlayer loop
    //// Test method to find event time using hit TS's tdc.
    //int minTSTdc = 9999;
    //for(unsigned iAx=0; iAx<9; iAx++){
    //  if (minTSTdc > m_mVector_in["tdc"][2*iAx]) minTSTdc = m_mVector_in["tdc"][2*iAx];
    //}
    //m_mDouble_in["eventTime"] = minTSTdc;

    ////////////////////
    // Get 2D fit values
    // Get 2D fit values from IW 2D fitter
    m_mDouble_in["phi02D"] = aTrack.helix().phi0();
    m_mDouble_in["pt2D"] = aTrack.pt();
    if (aTrack.charge() < 0) {
      m_mDouble_in["phi02D"] -= m_mConstD_in.at("Trg_PI");
      if (m_mDouble_in["phi02D"] < 0) m_mDouble_in["phi02D"] += 2 * m_mConstD_in.at("Trg_PI");
    }
    m_mDouble_in["dr2D"] = aTrack.helix().dr() * 0.01;
    // Get 2D fit values from JB 2D fitter
    // Currently using JB fitter for 3D fitting
    m_mDouble_in["charge"] = double(aTrack.charge());
    // Set phi2DError for 2D fit
    m_mVector_in["phi2DError"] = vector<double> (5);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if (m_mVector_in["useAxSl"][iAx] == 1) {
        // Check LR.
        if (m_mVector_in["LR"][2 * iAx] != 3) m_mVector_in["phi2DError"][iAx] = m_mConstV_in["driftPhi2DError"][iAx];
        else m_mVector_in["phi2DError"][iAx] = m_mConstV_in["wirePhi2DError"][iAx];
        // Check event time.
        if (m_mDouble_in["eventTime"] == 9999) m_mVector_in["phi2DError"][iAx] = m_mConstV_in["wirePhi2DError"][iAx];
      } else {
        m_mVector_in["phi2DError"][iAx] = 9999;
      }
    }
    // Set invPhi2DError for 2D fit
    m_mVector_in["phi2DInvError"] = vector<double> (5);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      if (m_mVector_in["useAxSl"][iAx] == 1) {
        m_mVector_in["phi2DInvError"][iAx] = 1 / m_mVector_in["phi2DError"][iAx];
      } else {
        m_mVector_in["phi2DInvError"][iAx] = 0;
      }
    }
    // Calculate phi2D.
    m_mVector_in["phi2D"] = vector<double> (5);
    if (m_mBool_in.at("f2DFitDrift") == 0 || m_mDouble_in["eventTime"] == 9999) {
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        m_mVector_in["phi2D"][iAx] = m_mVector_in["wirePhi"][iAx * 2];
      }
    } else {
      for (unsigned iAx = 0; iAx < 5; iAx++) {
        if (m_mVector_in["useAxSl"][iAx] == 1) {
          // Get drift length from table.
          string tableName = "driftLengthTableSL" + to_string(iAx * 2);
          double t_driftTime = m_mVector_in["tdc"][iAx * 2] - m_mDouble_in["eventTime"];
          if (t_driftTime < 0) t_driftTime = 0;
          if (t_driftTime > 511) t_driftTime = 511;
          double t_driftLength = m_mConstV_in[tableName][(unsigned)t_driftTime];
          m_mVector_in["phi2D"][iAx] = Fitter3DUtility::calPhi(m_mVector_in["wirePhi"][iAx * 2], t_driftLength, m_mConstV_in["rr"][iAx * 2],
                                                               m_mVector_in["LR"][iAx * 2]);
        } else {
          m_mVector_in["phi2D"][iAx] = 9999;
        }
      }
    }
    // Fit2D
    if (m_mBool_in.at("f2DFit") == 0) {
      m_mDouble_in["rho"] = m_mDouble_in["pt2D"] / 0.01 / 1.5 / 0.299792458;
      m_mDouble_in["pt"] = 0.299792458 * 1.5 * m_mDouble_in["rho"] / 100;
      m_mDouble_in["phi0"] = m_mDouble_in["phi02D"];
      m_mDouble_in["fit2DChi2"] = 9999;
    } else {
      m_mDouble_in["rho"] = 0;
      m_mDouble_in["phi0"] = 0;
      m_mDouble_in["fit2DChi2"] = 0;
      Fitter3DUtility::rPhiFitter(&m_mConstV_in["rr2D"][0], &m_mVector_in["phi2D"][0], &m_mVector_in["phi2DInvError"][0],
                                  m_mDouble_in["rho"],
                                  m_mDouble_in["phi0"], m_mDouble_in["fit2DChi2"]);
      m_mDouble_in["pt"] = 0.3 * 1.5 * m_mDouble_in["rho"] / 100;
    }

    // Find charge of particle.
    Fitter3DUtility::chargeFinder(&m_mConstV_in["nTSs2D"][0], &m_mVector_in["tsId2D"][0], &m_mVector_in["useAxSl"][0],
                                  m_mDouble_in["phi0"],
                                  m_mDouble_in["charge"], m_mDouble_in["charge2D"]);

    if (m_mBool_in.at("fVerbose")) {
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]f2DFit:        " <<
           m_mBool_in.at("f2DFit") <<
           endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]evtTime:       " <<
           m_mDouble_in["eventTime"]
           << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]wirePhi:       " <<
           m_mVector_in["wirePhi"][0]
           << " " << m_mVector_in["wirePhi"][1] << " " << m_mVector_in["wirePhi"][2] << " " << m_mVector_in["wirePhi"][3] << " " <<
           m_mVector_in["wirePhi"][4] << " " << m_mVector_in["wirePhi"][5] << " " << m_mVector_in["wirePhi"][6] << " " <<
           m_mVector_in["wirePhi"][7] << " "
           << m_mVector_in["wirePhi"][8] << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]LR:            " << int(
             m_mVector_in["LR"][0]) << " " << int(m_mVector_in["LR"][1]) << " " << int(m_mVector_in["LR"][2]) << " " << int(
             m_mVector_in["LR"][3]) << " " << int(m_mVector_in["LR"][4]) << " " << int(m_mVector_in["LR"][5]) << " " << int(
             m_mVector_in["LR"][6]) << " " << int(m_mVector_in["LR"][7]) << " " << int(m_mVector_in["LR"][8]) << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]drift:         " <<
           m_mVector_in["driftLength"][0] << " " << m_mVector_in["driftLength"][1] << " " << m_mVector_in["driftLength"][2] << " " <<
           m_mVector_in["driftLength"][3] << " " << m_mVector_in["driftLength"][4] << " " << m_mVector_in["driftLength"][5] << " " <<
           m_mVector_in["driftLength"][6] << " " << m_mVector_in["driftLength"][7] << " " << m_mVector_in["driftLength"][8] << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]tdc:           " <<
           m_mVector_in["tdc"][0] <<
           " " << m_mVector_in["tdc"][1] << " " << m_mVector_in["tdc"][2] << " " << m_mVector_in["tdc"][3] << " " << m_mVector_in["tdc"][4] <<
           " " <<
           m_mVector_in["tdc"][5] << " " << m_mVector_in["tdc"][6] << " " << m_mVector_in["tdc"][7] << " " << m_mVector_in["tdc"][8] << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]rr2D:          " <<
           m_mConstV_in["rr2D"][0] <<
           " " << m_mConstV_in["rr2D"][1] << " " << m_mConstV_in["rr2D"][2] << " " << m_mConstV_in["rr2D"][3] << " " << m_mConstV_in["rr2D"][4]
           << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]Phi2D:         " <<
           m_mVector_in["phi2D"][0]
           << " " << m_mVector_in["phi2D"][1] << " " << m_mVector_in["phi2D"][2] << " " << m_mVector_in["phi2D"][3] << " " <<
           m_mVector_in["phi2D"][4] <<
           endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]Phi2DInvError: " <<
           m_mVector_in["phi2DInvError"][0] << " " << m_mVector_in["phi2DInvError"][1] << " " << m_mVector_in["phi2DInvError"][2] << " " <<
           m_mVector_in["phi2DInvError"][3] << " " << m_mVector_in["phi2DInvError"][4] << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]charge:        " << int(
             m_mDouble_in["charge"]) << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]charge2D:        " << int(
             m_mDouble_in["charge2D"]) << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]pt:            " <<
           m_mDouble_in["pt"] <<
           endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]rho:           " <<
           m_mDouble_in["rho"] <<
           endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]phi0:          " <<
           m_mDouble_in["phi0"] <<
           " " << m_mDouble_in["phi0"] / m_mConstD_in.at("Trg_PI") * 180 << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]fit2DChi2:     " <<
           m_mDouble_in["fit2DChi2"]
           << endl;
      cout << "[E" << int(m_mDouble_in["eventNumber"]) << "][T" << int(m_mDouble_in["trackId"]) << "]useAxSl:      " << int(
             m_mVector_in["useAxSl"][0]) << " " << int(m_mVector_in["useAxSl"][1]) << " " << int(m_mVector_in["useAxSl"][2]) << " " << int(
             m_mVector_in["useAxSl"][3]) << endl;
    }

    if (std::isnan(m_mDouble_in["rho"]) || std::isnan(m_mDouble_in["phi0"])) {
      if (m_mBool_in.at("fVerbose") == 1) cout << "[2Dfit] Exiting because rho or phi0 is nan." << endl;
      return 2;
    }
    return 0;
  }

  void TRGCDCFitter3D::print3DInformation(int iTrack)
  {
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]evtTime: " << m_mDouble["eventTime"] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]wirePhi: " << m_mVector["wirePhi"][0] << " " <<
         m_mVector["wirePhi"][1] << " " << m_mVector["wirePhi"][2] << " " << m_mVector["wirePhi"][3] << " " << m_mVector["wirePhi"][4] << " "
         << m_mVector["wirePhi"][5] << " " << m_mVector["wirePhi"][6] << " " << m_mVector["wirePhi"][7] << " " << m_mVector["wirePhi"][8] <<
         endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]LR:      " << int(m_mVector["LR"][0]) << " " << int(
           m_mVector["LR"][1]) << " " << int(m_mVector["LR"][2]) << " " << int(m_mVector["LR"][3]) << " " << int(
           m_mVector["LR"][4]) << " " << int(m_mVector["LR"][5]) << " " << int(m_mVector["LR"][6]) << " " << int(
           m_mVector["LR"][7]) << " " << int(m_mVector["LR"][8]) << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]lutLR:      " << int(m_mVector["lutLR"][0]) << " " << int(
           m_mVector["lutLR"][1]) << " " << int(m_mVector["lutLR"][2]) << " " << int(m_mVector["lutLR"][3]) << " " << int(
           m_mVector["lutLR"][4]) << " " << int(m_mVector["lutLR"][5]) << " " << int(m_mVector["lutLR"][6]) << " " << int(
           m_mVector["lutLR"][7]) << " " << int(m_mVector["lutLR"][8]) << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]useStSl:      " << int(m_mVector["useStSl"][0]) << " " << int(
           m_mVector["useStSl"][1]) << " " << int(m_mVector["useStSl"][2]) << " " << int(m_mVector["useStSl"][3]) << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]drift:   " << m_mVector["driftLength"][0] << " " <<
         m_mVector["driftLength"][1] << " " << m_mVector["driftLength"][2] << " " << m_mVector["driftLength"][3] << " " <<
         m_mVector["driftLength"][4] << " " << m_mVector["driftLength"][5] << " " << m_mVector["driftLength"][6] << " " <<
         m_mVector["driftLength"][7] << " " << m_mVector["driftLength"][8] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]tdc:     " << m_mVector["tdc"][0] << " " <<
         m_mVector["tdc"][1] << " " << m_mVector["tdc"][2] << " " << m_mVector["tdc"][3] << " " << m_mVector["tdc"][4] << " " <<
         m_mVector["tdc"][5] << " " << m_mVector["tdc"][6] << " " << m_mVector["tdc"][7] << " " << m_mVector["tdc"][8] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]Phi2D:   " << m_mVector["phi2D"][0] << " " <<
         m_mVector["phi2D"][1] << " " << m_mVector["phi2D"][2] << " " << m_mVector["phi2D"][3] << " " << m_mVector["phi2D"][4] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]Phi3D:   " << m_mVector["phi3D"][0] << " " <<
         m_mVector["phi3D"][1] << " " << m_mVector["phi3D"][2] << " " << m_mVector["phi3D"][3] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]zz:      " << m_mVector["zz"][0] << " " << m_mVector["zz"][1]
         << " " << m_mVector["zz"][2] << " " << m_mVector["zz"][3] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]arcS:    " << m_mVector["arcS"][0] << " " <<
         m_mVector["arcS"][1] << " " << m_mVector["arcS"][2] << " " << m_mVector["arcS"][3] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]zerror:  " << m_mVector["zError"][0] << " " <<
         m_mVector["zError"][1] << " " << m_mVector["zError"][2] << " " << m_mVector["zError"][3] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]izerror:  " << m_mVector["iZError2"][0] << " " <<
         m_mVector["iZError2"][1] << " " << m_mVector["iZError2"][2] << " " << m_mVector["iZError2"][3] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]charge:  " << int(m_mDouble["charge"]) << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]pt:      " << m_mDouble["pt"] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]phi0:    " << m_mDouble["phi0"] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]z0:      " << m_mDouble["z0"] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]cot:     " << m_mDouble["cot"] << endl;
    cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]chi2:    " << m_mDouble["zChi2"] << endl;
    if (m_mBool["fMc"]) {
      cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]mcCharge:  " << int(m_mDouble["mcCharge"]) << endl;
      cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]mcPosZ:    " << m_mVector["mcPosZ"][1] << " " <<
           m_mVector["mcPosZ"][3] << " " << m_mVector["mcPosZ"][5] << " " << m_mVector["mcPosZ"][7] << endl;
      cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]mcPosZ:    " << m_mVector["mcPosZ"][1] << " " <<
           m_mVector["mcPosZ"][3] << " " << m_mVector["mcPosZ"][5] << " " << m_mVector["mcPosZ"][7] << endl;
      cout << "[E" << int(m_mDouble["eventNumber"]) << "][T" << iTrack << "]mcLR:      " << int(m_mVector["mcLR"][0]) << " " << int(
             m_mVector["mcLR"][1]) << " " << int(m_mVector["mcLR"][2]) << " " << int(m_mVector["mcLR"][3]) << " " << int(
             m_mVector["mcLR"][4]) << " " << int(m_mVector["mcLR"][5]) << " " << int(m_mVector["mcLR"][6]) << " " << int(
             m_mVector["mcLR"][7]) << " " << int(m_mVector["mcLR"][8]) << endl;
    }
  }


  void TRGCDCFitter3D::terminate()
  {
    if (m_mBool["fRootFile"]) {
      HandleRoot::writeRoot(m_fileFitter3D);
      HandleRoot::terminateRoot(m_mTVectorD, m_mTClonesArray);
      delete m_fileFitter3D;
    }

    if (m_mBool["fVHDLFile"]) {
      //if(m_mSavedIoSignals.size()!=0) FpgaUtility::multipleWriteCoe(10, m_mSavedIoSignals, "./VHDL/LutData/Io/");
      //if(m_mSavedSignals.size()!=0) FpgaUtility::writeSignals("./VHDL/signals",m_mSavedSignals);
      if (m_mSavedIoSignals.size() != 0) FpgaUtility::multipleWriteCoe(10, m_mSavedIoSignals, "./");
      if (m_mSavedSignals.size() != 0) FpgaUtility::writeSignals("signalValues", m_mSavedSignals);
    }

    if (m_commonData) delete m_commonData;
  }

  string TRGCDCFitter3D::version(void) const
  {
    return string("TRGCDCFitter3D 6.0");
  }

  std::string TRGCDCFitter3D::name(void) const
  {
    return m_name;
  }

  void TRGCDCFitter3D::getStereoGeometry(map<string, vector<double> >& stGeometry)
  {
    stGeometry["priorityLayer"] = {10, 22, 34, 46};
    stGeometry["nWires"] = vector<double> (4);
    stGeometry["cdcRadius"] = vector<double> (4);
    stGeometry["zToStraw"] = vector<double> (4);
    stGeometry["nShift"] = vector<double> (4);
    stGeometry["angleSt"] = vector<double> (4);
    const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    for (int iSt = 0; iSt < 4; ++iSt) {
      stGeometry["nWires"][iSt] = cdc.nWiresInLayer(stGeometry["priorityLayer"][iSt]) * 2;
      stGeometry["cdcRadius"][iSt] = cdc.senseWireR(stGeometry["priorityLayer"][iSt]);
      stGeometry["zToStraw"][iSt] = cdc.senseWireBZ(stGeometry["priorityLayer"][iSt]);
      stGeometry["nShift"][iSt] = cdc.nShifts(stGeometry["priorityLayer"][iSt]);
      stGeometry["angleSt"][iSt] = 2 * stGeometry["cdcRadius"][iSt] * sin(M_PI * stGeometry["nShift"][iSt] /
                                   (stGeometry["nWires"][iSt])) /
                                   (cdc.senseWireFZ(stGeometry["priorityLayer"][iSt]) - stGeometry["zToStraw"][iSt]);
    }
  }

  void TRGCDCFitter3D::getStereoXt(vector<double> const& stPriorityLayer, vector<vector<double> >& stXts, bool isSimple)
  {
    stXts.resize(stPriorityLayer.size(), vector<double> (512));
    const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    for (unsigned iSt = 0; iSt < stPriorityLayer.size(); ++iSt) {
      for (unsigned iTick = 0; iTick < stXts[iSt].size(); ++iTick) {
        double t = iTick * 2 * cdc.getTdcBinWidth();
        if (isSimple) {
          stXts[iSt][iTick] = cdc.getNominalDriftV() * t;
        } else {
          double driftLength_0 = cdc.getDriftLength(t, stPriorityLayer[iSt], 0);
          double driftLength_1 = cdc.getDriftLength(t, stPriorityLayer[iSt], 1);
          stXts[iSt][iTick] = (driftLength_0 + driftLength_1) / 2;
        }
      }
    }
  }

  void TRGCDCFitter3D::getConstants(std::map<std::string, double>& mConstD, std::map<std::string, std::vector<double> >& mConstV,
                                    bool isXtSimple)
  {
    const CDC::CDCGeometryPar& cdc = CDC::CDCGeometryPar::Instance();
    mConstD["Trg_PI"] = 3.141592653589793;
    mConstV["priorityLayer"] = {3, 10, 16, 22, 28, 34, 40, 46, 52};
    mConstV["rr"] = vector<double> (9);
    mConstV["nWires"] = vector<double> (9);
    mConstV["nTSs"] = vector<double> (9);
    for (unsigned iSL = 0; iSL < 9; iSL++) {
      unsigned t_layerId = mConstV["priorityLayer"][iSL];
      mConstV["rr"][iSL] = cdc.senseWireR(t_layerId);
      mConstV["nWires"][iSL] = cdc.nWiresInLayer(t_layerId) * 2;
      mConstV["nTSs"][iSL] = cdc.nWiresInLayer(t_layerId);
    }
    mConstV["nTSs2D"] = vector<double> (5);
    for (unsigned iAx = 0; iAx < 5; iAx++) {
      mConstV["nTSs2D"][iAx] = mConstV["nTSs"][2 * iAx];
    }
    mConstV["zToStraw"] = vector<double> (4);
    mConstV["zToOppositeStraw"] = vector<double> (4);
    mConstV["angleSt"] = vector<double> (4);
    mConstV["nShift"] = vector<double> (4);
    for (int iSt = 0; iSt < 4; iSt++) {
      unsigned t_layerId = mConstV["priorityLayer"][iSt * 2 + 1];
      mConstV["zToStraw"][iSt] = cdc.senseWireBZ(t_layerId);
      mConstV["zToOppositeStraw"][iSt] = cdc.senseWireFZ(t_layerId);
      mConstV["angleSt"][iSt] = 2 * mConstV["rr"][2 * iSt + 1] * sin(mConstD["Trg_PI"] * cdc.nShifts(t_layerId) /
                                (2 * cdc.nWiresInLayer(t_layerId))) / (cdc.senseWireFZ(t_layerId) - cdc.senseWireBZ(t_layerId));
      mConstV["nShift"][iSt] = cdc.nShifts(t_layerId);
    }

    mConstV["rr2D"] = vector<double> (5);
    mConstV["rr3D"] = vector<double> (4);
    for (int iAx = 0; iAx < 5; iAx++) mConstV["rr2D"][iAx] = mConstV["rr"][iAx * 2];
    for (int iSt = 0; iSt < 4; iSt++) mConstV["rr3D"][iSt] = mConstV["rr"][iSt * 2 + 1];

    mConstV["wirePhi2DError"] = vector<double> (5);
    mConstV["driftPhi2DError"] = vector<double> (5);
    mConstV["wirePhi2DError"][0] = 0.00085106;
    mConstV["wirePhi2DError"][1] = 0.00039841;
    mConstV["wirePhi2DError"][2] = 0.00025806;
    mConstV["wirePhi2DError"][3] = 0.00019084;
    mConstV["wirePhi2DError"][4] = 0.0001514;
    mConstV["driftPhi2DError"][0] = 0.00085106;
    mConstV["driftPhi2DError"][1] = 0.00039841;
    mConstV["driftPhi2DError"][2] = 0.00025806;
    mConstV["driftPhi2DError"][3] = 0.00019084;
    mConstV["driftPhi2DError"][4] = 0.0001514;
    mConstV["driftZError"] = vector<double> ({0.7676, 0.9753, 1.029, 1.372});
    mConstV["wireZError"] = vector<double> ({0.7676, 0.9753, 1.029, 1.372});

    // Make driftLength table for each superlayer. Up to 511 clock ticks.
    // driftLengthTableSLX[ tdcCount (~2ns unit) ] = drift length (cm)
    for (unsigned iSl = 0; iSl < 9; iSl++) {
      string tableName = "driftLengthTableSL" + to_string(iSl);
      unsigned tableSize = 512;
      mConstV[tableName] = vector<double> (tableSize);
      unsigned t_layer = mConstV["priorityLayer"][iSl];
      for (unsigned iTick = 0; iTick < tableSize; iTick++) {
        double t_driftTime = iTick * 2 * cdc.getTdcBinWidth();
        double avgDriftLength = 0;
        if (isXtSimple == 1) {
          avgDriftLength = cdc.getNominalDriftV() * t_driftTime;
        } else {
          double driftLength_0 = cdc.getDriftLength(t_driftTime, t_layer, 0);
          double driftLength_1 = cdc.getDriftLength(t_driftTime, t_layer, 1);
          avgDriftLength = (driftLength_0 + driftLength_1) / 2;
        }
        mConstV[tableName][iTick] = avgDriftLength;
      }
    }

    mConstD["tdcBitSize"] = 9;
    mConstD["rhoBitSize"] = 11;
    mConstD["iError2BitSize"] = 8;
    mConstD["iError2Max"] = 1 / pow(mConstV["wireZError"][0], 2);
    // LUT values
    mConstD["JB"] = 0;
    mConstD["phiMax"] = mConstD["Trg_PI"];
    mConstD["phiMin"] = -mConstD["Trg_PI"];
    mConstD["rhoMin"] = 20;
    mConstD["rhoMax"] = 2500;
    mConstD["phiBitSize"] = 13;
    mConstD["driftPhiLUTOutBitSize"] = mConstD["phiBitSize"] - 1;
    mConstD["driftPhiLUTInBitSize"] = mConstD["tdcBitSize"];
    mConstD["acosLUTOutBitSize"] = mConstD["phiBitSize"] - 1;
    mConstD["acosLUTInBitSize"] = mConstD["rhoBitSize"];
    mConstD["zLUTInBitSize"] = mConstD["phiBitSize"];
    mConstD["zLUTOutBitSize"] = 9;
    mConstD["iDenLUTInBitSize"] = 13;
    mConstD["iDenLUTOutBitSize"] = 11; // To increase wireZError = 2.5*driftZError
  }

} // namespace Belle2
