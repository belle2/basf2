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

#include <map>
#include "TFile.h"
#include "TTree.h"
#include <framework/dataobjects/EventMetaData.h>
#include "framework/datastore/StoreArray.h"
#include "framework/datastore/RelationArray.h"
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
#include "trg/cdc/Fitter3D.h"
#include "trg/cdc/HandleRoot.h"

using namespace std;

namespace Belle2 {

  TRGCDCHough3DFinder::TRGCDCHough3DFinder(const TRGCDC& TRGCDC, bool makeRootFile, int finderMode)
    : _cdc(TRGCDC), m_makeRootFile(makeRootFile) , m_finderMode(finderMode)
  {

    m_fileFinder3D = 0;
    m_treeTrackFinder3D = 0;
    m_treeConstantsFinder3D = 0;

    m_mBool["fMc"] = 1;
    m_mBool["fVerbose"] = 0;
    m_mBool["fIsPrintError"] = 0;
    m_mBool["fIsIntegerEffect"] = 1;
    m_mBool["fmcLR"] = 0;
    m_mBool["fLRLUT"] = 1;
    m_mBool["f2DFitDrift"] = 1;
    m_mBool["f2DFit"] = 1;

    // For finder3D.
    m_mBool["debugEfficiency"] = 1;
    m_mBool["debugNTs"] = 1;

    m_mConstD["Trg_PI"] = 3.141592653589793;
    // Get rr,zToStraw,angleSt,nWire
    const CDC::CDCGeometryPar& cdcp = CDC::CDCGeometryPar::Instance();
    m_mConstV["rr"] = vector<double> (9);
    m_mConstV["nWires"] = vector<double> (9);
    m_mConstV["nTSs"] = vector<double> (9);
    for (unsigned iSL = 0; iSL < 9; iSL++) {
      unsigned t_layerId = _cdc.segment(iSL, 0).center().layerId();
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
      unsigned t_layerId = _cdc.stereoSegment(iSt, 0).center().layerId();
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
    //// KT study
    ////m_mConstV["wireZError"] = vector<double>  ({0.0581, 0.0785, 0.0728, 0.0767});
    ////m_mConstV["driftZError"] = vector<double>  ({0.00388, 0.00538, 0.00650, 0.00842});
    //// Orginal study
    //m_mConstV["wireZError"] = vector<double> (4);
    //m_mConstV["wireZError"][0] = 3.19263;
    //m_mConstV["wireZError"][1] = 2.8765;
    //m_mConstV["wireZError"][2] = 2.90057;
    //m_mConstV["wireZError"][3] = 3.96206;
    //m_mConstV["driftZError"] = vector<double> (4);
    //m_mConstV["driftZError"][0] = 3.19263;
    //m_mConstV["driftZError"][1] = 2.8765;
    //m_mConstV["driftZError"][2] = 2.90057;
    //m_mConstV["driftZError"][3] = 3.96206;

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
      unsigned t_layer = _cdc.segment(iSl, 0).center().layerId();
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

    // Save geometry to root file.
    TVectorD geometryHough3D(16);
    for (int i = 0; i < 4; i++) {
      geometryHough3D[i] = m_mConstV["rr"][2 * i + 1] / 100;
      geometryHough3D[i + 4] = m_mConstV["angleSt"][i];
      geometryHough3D[i + 8] = m_mConstV["zToStraw"][i] / 100;
      geometryHough3D[i + 12] = m_mConstV["nWires"][2 * i + 1];
    }

    m_Hough3DFinder = new Hough3DFinder();
    // 1: Hough3DFinder 2: GeoFinder 3: VHDL GeoFinder
    m_Hough3DFinder->setMode(m_finderMode);
    // Set input file name for VHDL GeoFinder.
    m_Hough3DFinder->setInputFileName(string(std::getenv("BELLE2_LOCAL_DIR")) + "/data/trg/cdc/GeoFinder.input");

    // For VHDL GEoFinder
    //m_Hough3DFinder->setInputFileName("GeoFinder.input");
    // cotStart, cotEnd, z0Start, z0End, cotSteps, z0Steps
    float tempInitVariables[] = { -3, 3, -2, 2, 1001, 1001};
    vector<float > initVariables(tempInitVariables, tempInitVariables + sizeof(tempInitVariables) / sizeof(tempInitVariables[0]));
    // Save the init variables
    m_Hough3DFinder->initialize(geometryHough3D, initVariables);

    m_mConstD["modeHough3D"] = m_Hough3DFinder->getMode();
    m_mConstV["initVariablesHough3D"] = vector<double> (6);
    m_mConstV["initVariablesHough3D"][0] = tempInitVariables[0];
    m_mConstV["initVariablesHough3D"][1] = tempInitVariables[1];
    m_mConstV["initVariablesHough3D"][2] = tempInitVariables[2];
    m_mConstV["initVariablesHough3D"][3] = tempInitVariables[3];
    m_mConstV["initVariablesHough3D"][4] = tempInitVariables[4];
    m_mConstV["initVariablesHough3D"][5] = tempInitVariables[5];

  }

  TRGCDCHough3DFinder::~TRGCDCHough3DFinder()
  {
  }

  void TRGCDCHough3DFinder::terminate(void)
  {
    if (m_makeRootFile) {
      HandleRoot::writeRoot(m_fileFinder3D);
      HandleRoot::terminateRoot(m_mRunTVectorD, m_mEventTVectorD, m_mTClonesArray);
      delete m_fileFinder3D;
    }
    m_Hough3DFinder->destruct();
  }

  void TRGCDCHough3DFinder::doit(vector<TCTrack*> const& trackList2D, vector<TCTrack*>& trackList3D)
  {
    // Loop over trackList2D and copy to make a new trackList3D. Will delete it at TRGCDC.cc.
    for (unsigned int iTrack = 0; iTrack < trackList2D.size(); iTrack++) {
      TCTrack& aTrack = * new TCTrack(* trackList2D[iTrack]);
      trackList3D.push_back(& aTrack);
    }
    doit(trackList3D);
  }

  void TRGCDCHough3DFinder::doit(vector<TCTrack*>& trackList)
  {

    // Assign track ID's.
    for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
      TCTrack& aTrack = *trackList[iTrack];
      aTrack.setTrackID(iTrack + 1);
    }

    if (m_finderMode == 0) doitPerfectly(trackList);
    if (m_finderMode != 0) doitFind(trackList);

  }

  void TRGCDCHough3DFinder::doitFind(vector<TCTrack*>& trackList)
  {
    TRGDebug::enterStage("3D finder");

    // For saving to root file.
    if (m_makeRootFile) {
      m_mDouble["iSave"] = 0;
      HandleRoot::initializeEvent(m_mEventTVectorD, m_mTClonesArray);
    }

    // Get event number.
    StoreObjPtr<EventMetaData> eventMetaDataPtr;
    // Event starts from 0.
    m_mDouble["eventNumber"] = eventMetaDataPtr->getEvent();;

    // Generate arrays for TS candidates.
    vector<vector<double> > stTSs(4);
    vector<vector<int> > stTSDrift(4);
    vector<vector<const TCSHit*> > p_stTSs(4);
    for (unsigned iSL = 0; iSL < 4; iSL++) {
      vector<const TCSHit*> hits = _cdc.stereoSegmentHits(iSL);
      // Initialize vectors.
      string slName = "st" + to_string(iSL);
      m_mEventV[slName + "_hit"] = vector<double> ();
      m_mEventV[slName + "_driftHit"] = vector<double> ();
      stTSs[iSL] = vector<double> ();
      stTSDrift[iSL] = vector<int> ();
      p_stTSs[iSL] = vector<const TCSHit*> ();
      // Fill vectors
      for (unsigned iHit = 0; iHit < hits.size(); iHit++) {
        if (hits[iHit] == 0) continue;
        //// Temporary solution. This should be done in the finder3D.
        //// Select only TSs that contain 1st priority.
        //if(hits[iHit]->segment().priorityPosition() != 3) {
        //  continue;
        //}
        double t_wirePhi = ((double)hits[iHit]->cell().localId()) / m_mConstV["nWires"][2 * iSL + 1] * 4 * m_mConstD["Trg_PI"];
        m_mEventV[slName + "_hit"].push_back(t_wirePhi);
        m_mEventV[slName + "_driftHit"].push_back(TRGCDCFitter3D::calPhi(hits[iHit], _cdc.getEventTime()));
        int t_tdc = hits[iHit]->segment().priorityTime();
        int t_lr = hits[iHit]->segment().LUT()->getValue(hits[iHit]->segment().lutPattern());;
        int t_priorityPosition = hits[iHit]->segment().priorityPosition();
        int t_driftInfo = (t_tdc << 4) + (t_lr << 2) + t_priorityPosition;
        stTSs[iSL].push_back(t_wirePhi);
        p_stTSs[iSL].push_back(hits[iHit]);
        stTSDrift[iSL].push_back(t_driftInfo);
        //cout<<"iSL:"<<iSL<<" iHit:"<<iHit<<" t_tdc:"<<t_tdc<<" t_lr:"<<t_lr<<" t_priorityPosition:"<<t_priorityPosition<<" t_driftInfo:"<<t_driftInfo<<endl;
      }
    }

    // Get MC values related with finding
    // numberTSsForParticle[mcId] = # superlayer hits.
    map<unsigned, unsigned> numberTSsForParticle;
    if (m_mBool["fMc"]) findNumberOfHitSuperlayersForMcParticles(p_stTSs, numberTSsForParticle);

    // Loop over all the tracks.
    m_mEventD["nTracks"] = trackList.size();
    for (unsigned iTrack = 0; iTrack < m_mEventD["nTracks"]; iTrack++) {

      TCTrack& aTrack = * trackList[iTrack];

      // Get MC values related with fitting
      if (m_mBool["fMc"]) TRGCDCFitter3D::getMCValues(_cdc, &aTrack, m_mConstD, m_mDouble, m_mVector);

      // Get track ID
      m_mDouble["trackId"] = aTrack.getTrackID();

      // 2D Fitter
      int fit2DResult = TRGCDCFitter3D::do2DFit(aTrack, m_mBool, m_mConstD, m_mConstV, m_mDouble, m_mVector);
      if (fit2DResult != 0) continue;

      // Set input of finder
      //vector<double > trackVariables = { m_mDouble["charge"], m_mDouble["rho"]/100, m_mDouble["phi0"] } ;
      vector<double > trackVariables = { m_mDouble["charge2D"], m_mDouble["rho"] / 100, m_mDouble["phi0"] } ;

      // Run finder
      m_Hough3DFinder->runFinder(trackVariables, stTSs, stTSDrift);

      // Get results of finder
      m_Hough3DFinder->getValues("bestTSIndex", m_mVector["bestTSIndex"]);
      const TCSHit* p_bestTS[4] = {0, 0, 0, 0};
      for (int iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["bestTSIndex"][iSt] == 999) p_bestTS[iSt] = 0;
        else p_bestTS[iSt] = p_stTSs[iSt][(int)m_mVector["bestTSIndex"][iSt]];
      }
      m_Hough3DFinder->getValues("bestTS", m_mVector["bestTS"]);
      // Find and append TS to track.
      for (unsigned iSt = 0; iSt < 4; iSt++) {
        if (m_mVector["bestTS"][iSt] != 999) aTrack.append(new TCLink(0, p_bestTS[iSt], p_bestTS[iSt]->cell().xyPosition()));
      }

      // For saving values from finder.
      if (m_Hough3DFinder->getMode() == 1) {
        m_Hough3DFinder->getValues("bestZ0", m_mVector["bestZ0"]);
        m_Hough3DFinder->getValues("bestCot", m_mVector["bestCot"]);
        m_Hough3DFinder->getValues("houghMax", m_mVector["houghMax"]);
        m_Hough3DFinder->getValues("minDiffHough", m_mVector["minDiffHough"]);
      }
      if (m_Hough3DFinder->getMode() == 2) {
        m_Hough3DFinder->getValues("st0GeoCandidatesPhi", m_mVector["st0GeoCandidatesPhi"]);
        m_Hough3DFinder->getValues("st1GeoCandidatesPhi", m_mVector["st1GeoCandidatesPhi"]);
        m_Hough3DFinder->getValues("st2GeoCandidatesPhi", m_mVector["st2GeoCandidatesPhi"]);
        m_Hough3DFinder->getValues("st3GeoCandidatesPhi", m_mVector["st3GeoCandidatesPhi"]);
        m_Hough3DFinder->getValues("st0GeoCandidatesDiffStWires", m_mVector["st0GeoCandidatesDiffStWires"]);
        m_Hough3DFinder->getValues("st1GeoCandidatesDiffStWires", m_mVector["st1GeoCandidatesDiffStWires"]);
        m_Hough3DFinder->getValues("st2GeoCandidatesDiffStWires", m_mVector["st2GeoCandidatesDiffStWires"]);
        m_Hough3DFinder->getValues("st3GeoCandidatesDiffStWires", m_mVector["st3GeoCandidatesDiffStWires"]);
        m_Hough3DFinder->getValues("stAxPhi", m_mVector["stAxPhi"]);
      }

      // Get MC values.
      if (m_mBool["fMc"]) {
        // Call storage array.
        StoreArray<CDCSimHit> SimHits("CDCSimHits");
        StoreArray<CDCHit> CDCHits("CDCHits");
        RelationArray relationCDCHits(SimHits, CDCHits);

        // save performance values (purity, efficiency)
        const TCRelation& trackRelation3D = aTrack.relation3D();
        m_mDouble["purity"] = trackRelation3D.purity3D(aTrack.relation2D().contributor(0));
        m_mDouble["efficiency"] = trackRelation3D.efficiency3D(aTrack.relation2D().contributor(0), numberTSsForParticle);
        // Find one mc stereo track segment per layer.
        m_mVector["mcTSs"] = vector<double> (4, 999);
        vector<const TCSHit*> mcTSList;
        perfectFinder(trackList, iTrack, mcTSList);
        for (unsigned iTS = 0; iTS < mcTSList.size(); iTS++) {
          int iSuperLayer = (int)(double(mcTSList[iTS]->cell().superLayerId()) - 1) / 2;
          m_mVector["mcTSs"][iSuperLayer] = (double)mcTSList[iTS]->cell().localId() / m_mConstV["nWires"][2 * iSuperLayer + 1] * 4 *
                                            m_mConstD["Trg_PI"];
        }
        // Save MC ture CDC's hit position
        m_mVector["mcTSsX"] = vector<double> (4);
        m_mVector["mcTSsY"] = vector<double> (4);
        for (unsigned iTS = 0; iTS < mcTSList.size(); iTS++) {
          unsigned iCDCSimHit = mcTSList[iTS]->iCDCSimHit();
          CDCSimHit* aCDCSimHit = SimHits[iCDCSimHit];
          TVector3 posWire = aCDCSimHit->getPosWire();
          m_mVector["mcTSsX"][iTS] = posWire.X();
          m_mVector["mcTSsY"][iTS] = posWire.Y();
        }
        // Calculate diff from perfect
        m_mVector["perfectWireDiff"] = vector<double> (4);
        m_mVector["perfectCalZ"] = vector<double> (4);
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          if (m_mVector["mcTSs"][iSt] == 999) {
            m_mVector["perfectWireDiff"][iSt] = 999;
            m_mVector["perfectCalZ"][iSt] = 999;
          } else {
            m_mVector["perfectWireDiff"][iSt] = Fitter3DUtility::calDeltaPhi(m_mDouble["mcCharge"], m_mConstV["angleSt"][iSt],
                                                m_mConstV["zToStraw"][iSt] / 100, m_mConstV["rr"][2 * iSt + 1] / 100,  m_mVector["mcTSs"][iSt], m_mDouble["rho"] / 100,
                                                m_mDouble["phi0"]) / 4 / m_mConstD["Trg_PI"] * m_mConstV["nWires"][2 * iSt + 1];
            m_mVector["perfectCalZ"][iSt] = Fitter3DUtility::calZ(m_mDouble["mcCharge"], m_mConstV["angleSt"][iSt],
                                                                  m_mConstV["zToStraw"][iSt] / 100, m_mConstV["rr"][2 * iSt + 1] / 100,  m_mVector["mcTSs"][iSt], m_mDouble["rho"] / 100,
                                                                  m_mDouble["phi0"]);
          }
        }
        // Find multiple mc stereo track segments per layer.
        unsigned int mcParticleId = aTrack.relation2D().contributor(0);
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          string mcTsName = "mcTsSt" + to_string(iSt);
          m_mVector[mcTsName] = vector<double> ();
          vector<const TCSHit*> hits = _cdc.stereoSegmentHits(iSt);
          // Find TS which match mc index
          for (unsigned iTS = 0; iTS < hits.size(); iTS++) {
            if (hits[iTS]->iMCParticle() == mcParticleId) m_mVector[mcTsName].push_back((double)hits[iTS]->cell().localId() /
                  m_mConstV["nWires"][2 * iSt + 1] * 4 * m_mConstD["Trg_PI"]);
          }
        }

        //cout<<"----newStart----"<<endl;
        //cout<<"mcCharge:"<<m_mDouble["mcCharge"]<<endl;
        //cout<<"rho:"<<m_mDouble["rho"]<<" phi0:"<<m_mDouble["phi0"]<<endl;
        //cout<<"purity:"<<m_mDouble["purity"]<<" efficiency:"<<m_mDouble["efficiency"]<<endl;
        //cout<<"mcTSs[0]:"<<m_mVector["mcTSs"][0]<<" mcTSs[1]:"<<m_mVector["mcTSs"][1]<<" mcTSs[2]:"<<m_mVector["mcTSs"][2]<<" mcTSs[3]:"<<m_mVector["mcTSs"][3]<<endl;
        //cout<<"mcTSsX[0]:"<<m_mVector["mcTSsX"][0]<<" mcTSsX[1]:"<<m_mVector["mcTSsX"][1]<<" mcTSsX[2]:"<<m_mVector["mcTSsX"][2]<<" mcTSsX[3]:"<<m_mVector["mcTSsX"][3]<<endl;
        //cout<<"mcTSsY[0]:"<<m_mVector["mcTSsY"][0]<<" mcTSsY[1]:"<<m_mVector["mcTSsY"][1]<<" mcTSsY[2]:"<<m_mVector["mcTSsY"][2]<<" mcTSsY[3]:"<<m_mVector["mcTSsY"][3]<<endl;
        //cout<<"perfectWireDiff[0]:"<<m_mVector["perfectWireDiff"][0]<<" perfectWireDiff[1]:"<<m_mVector["perfectWireDiff"][1]<<" perfectWireDiff[2]:"<<m_mVector["perfectWireDiff"][2]<<" perfectWireDiff[3]:"<<m_mVector["perfectWireDiff"][3]<<endl;
        //cout<<"perfectCalZ[0]:"<<m_mVector["perfectCalZ"][0]<<" perfectCalZ[1]:"<<m_mVector["perfectCalZ"][1]<<" perfectCalZ[2]:"<<m_mVector["perfectCalZ"][2]<<" perfectCalZ[3]:"<<m_mVector["perfectCalZ"][3]<<endl;
        //cout<<"----newEnd----"<<endl;
      }

      if (m_makeRootFile) {
        if (m_fileFinder3D == 0) {
          m_fileFinder3D = new TFile("Finder3D.root", "RECREATE");
          HandleRoot::initializeRoot("hough3D", &m_treeConstantsFinder3D, &m_treeTrackFinder3D,
                                     m_mRunTVectorD, m_mEventTVectorD, m_mTClonesArray,
                                     m_mConstD, m_mConstV,
                                     m_mEventD, m_mEventV,
                                     m_mDouble, m_mVector
                                    );
        }
        HandleRoot::saveTrackValues("hough3D",
                                    m_mTClonesArray, m_mDouble, m_mVector
                                   );
      }

    } // End of loop over all the tracks.

    // Will ignore events until event has a track.
    if (m_makeRootFile && m_fileFinder3D) {
      HandleRoot::saveEventValues("hough3D",
                                  m_mEventTVectorD, m_mEventD, m_mEventV
                                 );
      m_treeTrackFinder3D->Fill();
    }


    // Set debug values.
    if (m_mBool["debugEfficiency"]) {
      // Notify when efficiency is not 1.
      for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
        /* cppcheck-suppress variableScope */
        TCTrack& aTrack = * trackList[iTrack];
        // Find number of super layers that have priority layer hit.
        int nPriorityHitSL = 0;
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          int priorityHitSL = 0;
          for (unsigned iTS = 0; iTS < stTSDrift[iSt].size(); iTS++) {
            int t_priorityPosition = (stTSDrift[iSt][iTS] & 3);
            if (t_priorityPosition == 3) priorityHitSL = 1;
            //cout<<"iSt:"<<iSt<<" iTS:"<<iTS<<" priorityPosition:"<<t_priorityPosition<<" priorityHitSL:"<<priorityHitSL<<endl;
          }
          if (priorityHitSL) nPriorityHitSL++;
        }
        //cout<<"nPriorityHitSL:"<<nPriorityHitSL<<endl;
        if (m_mDouble["efficiency"] != 1) {
          // Remove case when all hits are secondary priority.
          if (m_mDouble["efficiency"] != nPriorityHitSL * 1. / 4) {
            aTrack.setDebugValue(TRGCDCTrack::EDebugValueType::find3D, 1);
          }
        }
      }
    }
    if (m_mBool["debugNTs"]) {
      // Notify when not enough TS are found
      for (unsigned iTrack = 0; iTrack < trackList.size(); iTrack++) {
        unsigned nHitStSl = 0;
        for (unsigned iSt = 0; iSt < 4; iSt++) {
          if (trackList[iTrack]->links(2 * iSt + 1).size() != 0) nHitStSl++;
        }
        if (nHitStSl < 2) trackList[iTrack]->setDebugValue(TRGCDCTrack::EDebugValueType::find3D, 1);
      }
    }

    TRGDebug::leaveStage("3D finder");

  }

  void TRGCDCHough3DFinder::perfectFinder(vector<TCTrack*>& trackList, unsigned j, vector<const TCSHit*>& mcTSList)
  {

    //Just a test
    StoreArray<CDCHit> CDCHits("CDCHits");
    StoreArray<CDCSimHit> SimHits("CDCSimHits");
    RelationArray rels(SimHits, CDCHits);

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


  void TRGCDCHough3DFinder::doitPerfectly(vector<TCTrack*>& trackList)
  {

    TRGDebug::enterStage("Perfect 3D Finder");
    if (TRGDebug::level())
      cout << TRGDebug::tab() << "givenTrk#=" << trackList.size() << endl;


    //...Track loop....
    for (unsigned j = 0; j < trackList.size(); j++) {
      //...G4 trackID...
      TCTrack* trk = trackList[j];

      vector<const TCSHit*> mcTSList;
      perfectFinder(trackList, j, mcTSList);
      for (unsigned iTS = 0; iTS < mcTSList.size(); iTS++) {
        trk->append(new TCLink(0, mcTSList[iTS], mcTSList[iTS]->cell().xyPosition()));
      }

      if (TRGDebug::level())
        trk->dump("", "> ");

    }

    TRGDebug::leaveStage("Perfect 3D Finder");

  }

  void TRGCDCHough3DFinder::findNumberOfHitSuperlayersForMcParticles(vector<vector<const TCSHit*> >& p_stTSs,
      map<unsigned, unsigned>& numberTSsForParticle)
  {
    vector<unsigned> mcParticleList;
    for (unsigned iLayer = 0; iLayer < 4; iLayer++) {
      // Find what mc particles there are in a layer
      mcParticleList.clear();
      for (unsigned iTS = 0; iTS < p_stTSs[iLayer].size(); iTS++) {
        unsigned iMCParticle = p_stTSs[iLayer][iTS]->iMCParticle();
        if (find(mcParticleList.begin(), mcParticleList.end(), iMCParticle) == mcParticleList.end()) {
          mcParticleList.push_back(iMCParticle);
        }
      }
      // Loop over mcParticleList and add to numberTSsForParticle
      for (unsigned iMCPart = 0; iMCPart < mcParticleList.size(); iMCPart++) {
        map<unsigned, unsigned>::iterator it = numberTSsForParticle.find(mcParticleList[iMCPart]);
        if (it != numberTSsForParticle.end()) ++it->second;
        else numberTSsForParticle[mcParticleList[iMCPart]] = 1;
      }
    }
  }


}

