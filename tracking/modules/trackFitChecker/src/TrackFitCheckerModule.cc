/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Moritz Nadler                                            *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFitChecker/TrackFitCheckerModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/gearbox/Const.h>
#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

#include <vxd/geometry/GeoCache.h>
#include <svd/geometry/SensorInfo.h>
#include <pxd/geometry/SensorInfo.h>
#include <vxd/dataobjects/VXDTrueHit.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <cdc/dataobjects/CDCRecoHit.h>
#include <cdc/dataobjects/CDCSimHit.h>
#include <cdc/geometry/CDCGeometryPar.h>

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/gfbfield/GFGeant4Field.h>
#include <tracking/trackCandidateHits/CDCTrackCandHit.h>

#include <GFTrack.h>
#include <GFTools.h>
#include <RKTrackRep.h>

#include <GFFieldManager.h>
#include <GFConstField.h>
#include <GFMaterialEffects.h>
#include <GFTGeoMaterialInterface.h>
#include <GFException.h>
//root stuff
#include <TMatrixDEigen.h>
#include <TGeoManager.h>
#include <TDecompSVD.h>
#include <Math/ProbFunc.h>

#include <boost/math/special_functions/sign.hpp>

//C++ st libs
#include <cmath>
#include <limits>
#include <numeric>
#include <set>

using namespace std;
using namespace Belle2;
using namespace boost::accumulators;
using namespace Tracking;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(TrackFitChecker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

TrackFitCheckerModule::TrackFitCheckerModule() : Module()
{
  //Set module properties
  setDescription("This module takes GFTracks as input an calculates different statistical tests some of them using the simulated truth information");

  //Parameter definition
  addParam("outputFileName", m_dataOutFileName, "A common name for all output files of this module. Suffixes to distinguish them will be added automatically", string("trackFitChecker"));
  addParam("totalChi2Cut", m_totalChi2Cut, "only tracks with a total χ² lower than this value will be considered", 1E300);
  addParam("testSi", m_testSi, "execute the layer wise tests for PXD/SVD", false);
  addParam("robustTests", m_robust, "activate additional robust statistical tests (median and MAD)", false);
  addParam("testLRRes", m_testLRRes, "test if DAF correctly resolved the left right ambiguity in the CDC", false);
  addParam("testCdc", m_testCdc, "execute the layer wise tests for CDC", false);
  addParam("truthAvailable", m_truthAvailable, "is truth info available for every hit?", false);
  addParam("inspectTracks", m_inspectTracks, "write track parameters into a text file for further inspection. When 0 this function is switched off. 1 or 2 will enable this function but have different arrangments of data in text file. EXPERIMENTAL", 0);
  addParam("writeToRootFile", m_writeToRootFile, "Set to True if you want the data from the statistical tests written into a root file", false);
  addParam("writeToTextFile", m_writeToFile, "Set to True if you want the results of the statistical tests written out in a normal text file", false);
  addParam("exportTracksForRaveDeveloper", m_exportTracksForRaveDeveloper, "Writes tracks into text file in a format used by rave developers", false);
}


TrackFitCheckerModule::~TrackFitCheckerModule()
{

}

void TrackFitCheckerModule::initialize()
{
  StoreArray<GFTrack>::required();
  StoreArray<MCParticle>::required();
  if (gGeoManager == NULL) { //setup geometry and B-field for Genfit if not already there
    geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
    geoManager.createTGeoRepresentation();
    //pass the magnetic field to genfit
    GFFieldManager::getInstance()->init(new GFGeant4Field());
    GFMaterialEffects::getInstance()->init(new GFTGeoMaterialInterface());
  }

  //set all user parameters
  if (m_inspectTracks > 0 and m_truthAvailable == false) {
    m_inspectTracks = 0;
    B2WARNING("The option inspectTracks was disabled because it can only work when truthAvailable is true, which is not the case");
  }
  //cerr << "m_inspectTracks " << m_inspectTracks << endl;

  //configure the output
  m_textOutput.precision(4);
  //set the default names of variables stored in the statistics container if they are mulitdimensional (like the 5 track parameters)
  m_layerWiseTruthTestsVarNames.push_back("q/p");
  m_layerWiseTruthTestsVarNames.push_back("du/dw");
  m_layerWiseTruthTestsVarNames.push_back("dv/dw");
  m_layerWiseTruthTestsVarNames.push_back("u");
  m_layerWiseTruthTestsVarNames.push_back("v");
  m_layerWiseTruthTestsVarNames.push_back("χ²");
  m_layerWiseTruthTestsVarNames.push_back("χ²direc");
  m_layerWiseTruthTestsVarNames.push_back("χ²uv");
  m_layerWiseTruthTestsVarNames.push_back("χ²diuv");
  m_layerWiseTruthTestsVarNames.push_back("χ²qp_di");
  m_layerWiseTruthTestsVarNames.push_back("χ²qp_uv");

  const int vecSizeTruthTest = m_layerWiseTruthTestsVarNames.size();

  m_vertexTestsVarNames.push_back("x");
  m_vertexTestsVarNames.push_back("y");
  m_vertexTestsVarNames.push_back("z");
  m_vertexTestsVarNames.push_back("p_x");
  m_vertexTestsVarNames.push_back("p_y");
  m_vertexTestsVarNames.push_back("p_z");
  const int vecDataSize = m_vertexTestsVarNames.size();

  if (m_writeToRootFile == true) {
    string testDataFileName = m_dataOutFileName + "StatData.root";
    m_rootFilePtr = new TFile(testDataFileName.c_str(), "RECREATE");
    m_statDataTreePtr = new TTree("m_statDataTreePtr", "aTree");
    //init objects to store track wise data
    registerTrackWiseData("pValue_bu");
    registerTrackWiseData("pValue_fu");
    registerTrackWiseData("chi2tot_bu");
    registerTrackWiseData("chi2tot_fu");
    registerTrackWiseData("absMomVertex");
    registerTrackWiseData("res_curvVertex");
    registerTrackWiseData("relRes_curvVertex");
    registerTrackWiseData("relRes_p_T");
    registerTVector3("trueVertexPos");
    registerTVector3("trueVertexMom");
    registerInt("genfitStatusFlag");
  } else {
    m_rootFilePtr = NULL;
    m_statDataTreePtr = NULL;
  }

  if (m_testCdc == true) {
    CDC::CDCGeometryPar& cdcg = CDC::CDCGeometryPar::Instance();
    m_nCdcLayers =  cdcg.nWireLayers();
  } else {
    m_nCdcLayers = 0;
  }
  if (m_testSi == true) {
    VXD::GeoCache& geo = VXD::GeoCache::getInstance();
    m_nPxdLayers = geo.getLayers(PXD::SensorInfo::PXD).size();
    m_nSvdLayers = geo.getLayers(SVD::SensorInfo::SVD).size();
  } else {
    m_nPxdLayers = 0;
    m_nSvdLayers = 0;
  }
  if (m_testLRRes == true) {
    m_testCdc = false; // do not make any other tests on he CDC data. Only test the left right resolution
    CDC::CDCGeometryPar& cdcg = CDC::CDCGeometryPar::Instance();
    m_nCdcLayers =  cdcg.nWireLayers();
  }
  m_nSiLayers = m_nPxdLayers + m_nSvdLayers;
  m_nLayers = m_nPxdLayers + m_nSvdLayers + m_nCdcLayers;
  B2DEBUG(100, "nLayers " << m_nLayers);

  //make all vector of vectors have the size of the number of current layers in use
  int vecSizeMeasTest = 3;
  if (m_testCdc == true and m_testSi == false) {
    vecSizeMeasTest = 2; //it is easier to just caluclate the chi^2 to although it does not have more info than the standard score in this case
  }
  //int measDim = 2;

  // pulls (z) of cartesian coordinates at vertex
  registerTrackWiseVecData("pulls_vertexPosMom", vecDataSize);
  // residuals of Cartesian coordinates at vertex
  registerTrackWiseVecData("res_vertexPosMom", vecDataSize);
  // direclty the vertex coordinates
  registerTrackWiseVecData("vertexPosMom", vecDataSize);
  // pulls (z) of cartesian 7D coordinates at vertex
  //registerTrackWiseVecData("pulls_vertexState", 7);
  // residuals of cartesian 7D coordinates at vertex
  //  registerTrackWiseVecData("res_vertexState", 7);

  // pulls (z) and chi2s for the 5 track parameters in every layer using truth info
  registerLayerWiseData("pulls_and_chi2_fp_t", vecSizeTruthTest);
  registerLayerWiseData("pulls_and_chi2_fu_t", vecSizeTruthTest);
  registerLayerWiseData("pulls_and_chi2_bp_t", vecSizeTruthTest);
  registerLayerWiseData("pulls_and_chi2_bu_t", vecSizeTruthTest);
  registerLayerWiseData("pulls_and_chi2_sm_t", vecSizeTruthTest);
  // pulls (z) and chi2s for the 5 track parameters in every layer using the projection onto the measurement m-Hx
  registerLayerWiseData("pulls_and_chi2_fp", vecSizeMeasTest);
  registerLayerWiseData("pulls_and_chi2_fu", vecSizeMeasTest);
  registerLayerWiseData("pulls_and_chi2_bp", vecSizeMeasTest);
  registerLayerWiseData("pulls_and_chi2_bu", vecSizeMeasTest);
  registerLayerWiseData("pulls_and_chi2_sm", vecSizeMeasTest);

  // pulls and chi2 to test consistency of normal distribution model of measurements and the sigma of the digitizer with the sigma of the recoHits
  registerLayerWiseData("pulls_and_chi2_meas_t", vecSizeMeasTest);
  registerLayerWiseData("res_meas_t", 2);

  registerLayerWiseData("DAF_weights", 5); // at the moment only tracks 1 or 2 BG hits
  registerLayerWiseData("DAF_chi2s", 5);
  //registerLayerWiseData("DAF_weights_BG", 3);
  if (m_testLRRes) {
    registerLayerWiseData("LRAmbiRes", 2);
  }


  if (m_robust == true) { //set the scaling factors for the MAD. No MAD will be calculated when this value is not there or 0
    m_madScalingFactors["pulls_vertexPosMom"] = 1.4826; //scaling factor for normal distributed variables
    m_madScalingFactors["absMomVertex"] = 1.4826;
    m_madScalingFactors["res_vertexPosMom"] = 1.4826;
    m_madScalingFactors["vertexPosMom"] = 1.4826;
    m_madScalingFactors["res_curvVertex"] = 1.4826;
    m_madScalingFactors["relRes_curvVertex"] = 1.4826;
    m_madScalingFactors["relRes_p_T"] = 1.4826;
    m_madScalingFactors["pValue_bu"] = 4.0 / 3.0;  //scaling factor for uniform distributed variables
    m_madScalingFactors["pValue_fu"] = 4.0 / 3.0;

    m_trunctationRatios["pulls_vertexPosMom"] = 0.02; //scaling factor for normal distributed variables
    m_trunctationRatios["absMomVertex"] = 0.02;
    m_trunctationRatios["res_vertexPosMom"] = 0.02;
    m_trunctationRatios["vertexPosMom"] = 0.02;
    m_trunctationRatios["res_curvVertex"] = 0.02;
    m_trunctationRatios["relRes_curvVertex"] = 0.02;
    m_trunctationRatios["relRes_p_T"] = 0.02;
    m_trunctationRatios["pValue_bu"] = 0.02;
    m_trunctationRatios["pValue_fu"] = 0.02;

    m_trunctationRatios["chi2tot_bu"] = 0.02;
    m_trunctationRatios["chi2tot_fu"] = 0.02;

  }


  m_badR_fCounter = 0;
  m_badR_bCounter = 0;
  m_badR_smCounter = 0;
  m_processedTracks = 0;
  m_nCutawayTracks = 0;
  m_notPosDefCounter = 0;
  m_unSymmetricCounter = 0;
  m_extrapFailed = 0;
  //these member variables will be set only once when the event loop starts
  m_wAndPredPresentsTested = false;
  m_testDaf = true;
  m_testPrediction = true;


  if (m_inspectTracks > 0) {
    string forwardDataFileName = m_dataOutFileName + "ForwardData.txt";
    m_dataOut.open(forwardDataFileName.c_str());
    m_dataOut << "#event#\tproc#\tlayer#\tχ²_inc,p\tχ²_inc,u\tm_u\tm_v\tΔu\tΔv\n";
    m_dataOut << "#\t|p|\tq/p_p\tdu/dw_p\tdv/dw_p\tu_p\tv_p\tσ_u,p\tσ_v,p\n";
    m_dataOut << "#\tχ²_tot,u\tq/p_u\tdu/dw_u\tdv/dw_u\tu_u\tv_u\tσ_u,u\tσ_v,u\n";
    m_dataOut << "#\t\tq/p\tdu/dw\tdv/dw\tu\tv\tΔΦ\tΔθ\n";
    // m_dataOut.precision(14);
  }

  if (m_exportTracksForRaveDeveloper == true) {
    m_forRaveOut.open((m_dataOutFileName + "ForRaveDev.txt").c_str());
  }
}


void TrackFitCheckerModule::beginRun()
{

}



void TrackFitCheckerModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();
  if (m_exportTracksForRaveDeveloper == true) {
    m_forRaveOut << "event: id=" << eventCounter << ";\n";
  }

  B2DEBUG(100, "**********   TrackFitCheckerModule  processing event number: " << eventCounter << " ************");
  //simulated truth information
  StoreArray<MCParticle> aMcParticleArray;
  StoreArray<PXDTrueHit> aPxdTrueHitArray;
  StoreArray<SVDTrueHit> aSvdTrueHitArray;
  //StoreArray<CDCTrueSimHit> aCdcTrueHitArray(""); //maybe one day this will be there :-)




  //genfit stuff
  StoreArray<GFTrack> fittedTracks(""); // the results of the track fit
  const int nFittedTracks = fittedTracks.getEntries();
  //stringstreams for text file output of all forward tracking parameters

  //test one time for prediction and and weights
  if (nFittedTracks not_eq 0 and m_wAndPredPresentsTested == false) {
    if (m_testSi == true) {
      try {
        fittedTracks[0]->getBK(0)->getNumber(GFBKKey_dafWeight, 0);
      } catch (GFException& e) {
        m_testDaf = false;
      }
    } else {
      m_testDaf = false;
    }

    try {
      fittedTracks[0]->getBK(0)->getVector(GFBKKey_fSt, 0);
    } catch (GFException& e) {
      //m_layerWiseTests.havePredicitons = false;
      m_testPrediction = false;
      if (m_inspectTracks > 0) {
        m_dataOut << "#inspect tracks was disabled because no predictions are available in the Genfit bookkeeping\n";
      }
    }

    m_wAndPredPresentsTested = true;
  }


  for (int i = 0; i not_eq nFittedTracks; ++i) { // loop over all tracks in one event
    GFTrack* const aTrackPtr = fittedTracks[i];
    GFTrackCand aTrackCand = aTrackPtr->getCand();
    const int mcParticleIndex = aTrackCand.getMcTrackId();
    const double charge = aMcParticleArray[mcParticleIndex]->getCharge();
    const TVector3 trueVertexMom = aMcParticleArray[mcParticleIndex]->getMomentum();
    const TVector3 trueVertexPos = aMcParticleArray[mcParticleIndex]->getVertex();

    //write the mcparticle info to the root output
    fillTVector3("trueVertexPos", trueVertexPos);
    fillTVector3("trueVertexMom", trueVertexMom);
    RKTrackRep* aRKTrackRepPtr = static_cast<RKTrackRep*>(aTrackPtr->getCardinalRep());

    const int genfitStatusFlag = aRKTrackRepPtr->getStatusFlag();
    fillInt("genfitStatusFlag", genfitStatusFlag);

    if (genfitStatusFlag not_eq 0) {
      // we have a track that was not fitted successfully no tests can be done. Goto next track
      if (m_writeToRootFile == true) {
        m_statDataTreePtr->Fill(); // attention! this fill here means that all branches in the root tree besides trueVertexPos, trueVertexMom and genfitStatusFlag will have the value of the previous track. Keep this in mind when analyzing the tree afterwards
      }
      continue;
    }
    TVector3 poca; //point of closest approach will be overwritten
    TVector3 dirInPoca; //direction of the track at the point of closest approach will be overwritten
    aRKTrackRepPtr->setPropDir(-1);
    try {
      B2DEBUG(100, "before propagation");
      aRKTrackRepPtr->extrapolateToPoint(trueVertexPos, poca, dirInPoca);
    } catch (GFException& e) {
      B2WARNING("Extrapolation of a track in Event " << eventCounter <<  " to his true vertex position failed. Track will be ignored in statistical tests");
      ++m_extrapFailed;
      continue;
    }
    B2DEBUG(100, "after propagation");
    //    poca.Print();
    //    dirInPoca.Print();

    const double chi2tot_bu = aTrackPtr->getChiSqu(); // returns the total chi2 from the backward filter
    if (chi2tot_bu > m_totalChi2Cut) {//consider this track to be an outlier and discard it. Goto next track
      ++m_nCutawayTracks;
      continue;
    }
    // first part: get variables describing the hole track
    const double chi2tot_fu = aTrackPtr->getForwardChiSqu();
    const double ndf = aTrackPtr->getNDF();
    const double pValue_bu = ROOT::Math::chisquared_cdf_c(chi2tot_bu, ndf);
    B2DEBUG(100, "p value of fitted track " << i << " is " << pValue_bu);
    const double pValue_fu = ROOT::Math::chisquared_cdf_c(chi2tot_fu, ndf);
    fillTrackWiseData("pValue_bu", pValue_bu);
    fillTrackWiseData("pValue_fu", pValue_fu);
    fillTrackWiseData("chi2tot_bu", chi2tot_bu);
    fillTrackWiseData("chi2tot_fu", chi2tot_fu);
    TVector3 vertexPos;
    TVector3 vertexMom;
    TMatrixDSym vertexCov(6);
    vector<double> zVertexPosMom(6);
    vector<double> resVertexPosMom(6);
    vector<double> vertexPosMom(6);

    GFDetPlane planeThroughVertex(poca, dirInPoca); //get planeThroughVertex through fitted vertex position
    B2DEBUG(100, "plane through vertex constructed");
    //get fitted momentum at fitted vertex
    try {
      aTrackPtr->getPosMomCov(planeThroughVertex, vertexPos, vertexMom, vertexCov);
    } catch (GFException& e) {
      B2WARNING("Extrapolation of a track in Event " << eventCounter <<  " to its true vertex position failed. Track will be ignored in statistical tests");
      ++m_extrapFailed;
      continue;
    }

    B2DEBUG(100, "pos mom cov extracted");
    if (m_exportTracksForRaveDeveloper == true) {
      if (i == 0) {
        m_forRaveOut << "event:simvtx: x=" << trueVertexPos[0] << "; y=" << trueVertexPos[1] << "; z=" << trueVertexPos[2] << ".\n"; //assuming all tracks in one event comming from the same vertex
      }
      m_forRaveOut << "event:track: dpxpx=" << vertexCov[3][3] << "; dpxpy=" << vertexCov[3][4] << "; dpxpz=" << vertexCov[3][5] << "; dpypy=" << vertexCov[4][4] << "; dpypz=" << vertexCov[4][5] << "; dpzpz=" << vertexCov[5][5] << "; dxpx=" << vertexCov[0][3] << "; dxpy=" << vertexCov[0][4] << "; dxpz=" << vertexCov[0][5] << "; dxx=" << vertexCov[0][0] << "; dxy=" << vertexCov[0][1] << "; dxz=" << vertexCov[0][2] << "; dypx=" << vertexCov[1][3] << "; dypy=" << vertexCov[1][4] << "; dypz=" << vertexCov[1][5] << "; dyy=" << vertexCov[1][1] << "; dyz=" << vertexCov[1][2] << "; dzpx=" << vertexCov[2][3] << "; dzpy=" << vertexCov[2][4] << "; dzpz=" << vertexCov[2][5] << "; dzz=" << vertexCov[2][2] << "; px=" << vertexMom[0] << "; py=" << vertexMom[1] << "; pz=" << vertexMom[2] << "; q=" << charge << "; x=" << vertexPos[0] << "; y=" << vertexPos[1] << "; z=" << vertexPos[2] << ".\n";
    }
    double vertexAbsMom = vertexMom.Mag();
    fillTrackWiseData("absMomVertex", vertexAbsMom);
    double res_curvatureAtVertex =  1.0 / vertexMom.Pt() - 1.0 / trueVertexMom.Pt();
    fillTrackWiseData("res_curvVertex", res_curvatureAtVertex);
    double relRes_curvatureAtVertex = res_curvatureAtVertex * trueVertexMom.Pt();
    fillTrackWiseData("relRes_curvVertex", relRes_curvatureAtVertex);
    double relRes_p_T = (vertexMom.Pt() - trueVertexMom.Pt()) / trueVertexMom.Pt();

    fillTrackWiseData("relRes_p_T", relRes_p_T);
    resVertexPosMom[0] = (vertexPos[0] - trueVertexPos[0]);
    resVertexPosMom[1] = (vertexPos[1] - trueVertexPos[1]);
    resVertexPosMom[2] = (vertexPos[2] - trueVertexPos[2]);
    resVertexPosMom[3] = (vertexMom[0] - trueVertexMom[0]);
    resVertexPosMom[4] = (vertexMom[1] - trueVertexMom[1]);
    resVertexPosMom[5] = (vertexMom[2] - trueVertexMom[2]);
    fillTrackWiseVecData("res_vertexPosMom", resVertexPosMom);
    zVertexPosMom[0] = resVertexPosMom[0] / sqrt(vertexCov[0][0]);
    zVertexPosMom[1] = resVertexPosMom[1] / sqrt(vertexCov[1][1]);
    zVertexPosMom[2] = resVertexPosMom[2] / sqrt(vertexCov[2][2]);
    zVertexPosMom[3] = resVertexPosMom[3] / sqrt(vertexCov[3][3]);
    zVertexPosMom[4] = resVertexPosMom[4] / sqrt(vertexCov[4][4]);
    zVertexPosMom[5] = resVertexPosMom[5] / sqrt(vertexCov[5][5]);
    fillTrackWiseVecData("pulls_vertexPosMom", zVertexPosMom);
    vertexPosMom[0] = vertexPos[0];
    vertexPosMom[1] = vertexPos[1];
    vertexPosMom[2] = vertexPos[2];
    vertexPosMom[3] = vertexMom[0];
    vertexPosMom[4] = vertexMom[1];
    vertexPosMom[5] = vertexMom[2];
    fillTrackWiseVecData("vertexPosMom", vertexPosMom);
    B2DEBUG(100, "filled all track wise tests");

    if (m_nLayers not_eq 0) { // now the layer wise tests
      //cerr << "m_nLayers: " << m_nLayers << endl;
      if (m_testLRRes not_eq true) {
        //cout << "aTrackPtr->getNumHits()" << aTrackPtr->getNumHits() << endl;
        extractTrackData(aTrackPtr, charge);// read all the data for the layer wise tests from GFTracks
        B2DEBUG(100, "extractTrackData finished successfully");
      }
      // do the layer wise test uses only data from GFTrack object
      if (m_testLRRes == true) {
        testLRAmbiResolution(aTrackPtr);
        B2DEBUG(100, "executed left right resolution tests");
      } else if (m_testDaf == true) {
        testDaf(aTrackPtr);
        B2DEBUG(100, "executed DAF tests");
      } else {
        normalTests();
        B2DEBUG(100, "executed normal tests");
        if (m_truthAvailable == true) {
          truthTests(); //uses data from trueHits
          B2DEBUG(100, "executed truth tests");
        }
        if (m_inspectTracks > 0 and m_testPrediction == true) {
          inspectTracks(chi2tot_fu, vertexAbsMom);
          B2DEBUG(100, "executed inspect tracks");
        }
      }

    }
    if (m_writeToRootFile == true) {
      m_statDataTreePtr->Fill();
    }


    ++m_processedTracks;
  }
  if (m_exportTracksForRaveDeveloper == true) {
    m_forRaveOut << "event:fill\n";
  }
}

void TrackFitCheckerModule::endRun()
{

  B2INFO("Now following the endRun Output from the trackFitChecker module");

  if (m_nCutawayTracks not_eq 0) {
    B2WARNING(m_nCutawayTracks << " tracks where cut out because of too large total χ²");
  }
  if (m_extrapFailed not_eq 0) {
    B2WARNING(m_extrapFailed << " tracks could not be extrapolated to their true vertex position.");
  }
  if (m_badR_fCounter not_eq 0 or m_badR_bCounter not_eq 0 or m_badR_smCounter not_eq 0) {
    B2WARNING("There were tracks that produce negative diagonal elements in the covariance matrix R of the residuals r = m-H*state. Occurrence forward: " << m_badR_fCounter << " backward: " << m_badR_bCounter << " smoother: " << m_badR_smCounter);
  }
  if (m_unSymmetricCounter not_eq 0) {
    B2WARNING(m_unSymmetricCounter << " covs where not symmetric ");
  }
  if (m_notPosDefCounter not_eq 0) {
    B2WARNING(m_notPosDefCounter << " covs had eigenvalues <= 0 ");
  }
  if (m_processedTracks <= 1) {
    B2WARNING("Only " << m_processedTracks << " track(s) were processed. Statistics cannot be computed.");
  } else {
    m_textOutput << "Number of processed tracks: " << m_processedTracks << "\n";

    // loop over all track wise data and print it to terminal and/or to text file
    std::map<std::string, StatisticsContainer >::iterator  trackWiseDataIter =  m_trackWiseDataSamples.begin();
    std::map<std::string, StatisticsContainer >::const_iterator trackWiseDataIterMax = m_trackWiseDataSamples.end();
    while (trackWiseDataIter not_eq trackWiseDataIterMax) {
      printTrackWiseStatistics(trackWiseDataIter->first, true);
      ++trackWiseDataIter;
    }

    vector<string> measVarNames;
    measVarNames.push_back("u");
    measVarNames.push_back("v");
    measVarNames.push_back("χ²");
    if (m_testSi == false and m_testCdc == true) {
      measVarNames[0] = "d.l.";
    }
    if (m_testSi == true and m_testCdc == true) {
      measVarNames[0] = "u/d.l.";
      measVarNames[1] = "v/χ²";
    }
    printTrackWiseVecStatistics("vertexPosMom", m_vertexTestsVarNames, true);
    printTrackWiseVecStatistics("res_vertexPosMom", m_vertexTestsVarNames, true);
    printTrackWiseVecStatistics("pulls_vertexPosMom", m_vertexTestsVarNames, true);

    /*vector<string> vertexTests7DVarNames;
    vertexTests7DVarNames.push_back("x");
    vertexTests7DVarNames.push_back("y");
    vertexTests7DVarNames.push_back("z");
    vertexTests7DVarNames.push_back("a_x");
    vertexTests7DVarNames.push_back("a_y");
    vertexTests7DVarNames.push_back("a_z");
    vertexTests7DVarNames.push_back("q/p");
        printTrackWiseVecStatistics("res_vertexState", vertexTests7DVarNames);
        printTrackWiseVecStatistics("pulls_vertexState", vertexTests7DVarNames);*/
    //looks a bit clumsy with all the if (m_testSi == true) but the hope is there will easy accessible truth info for CDCHits so a better solution is not needed because the if (m_testSi == true) are temporary anyway

    if (m_testLRRes == true) {
      measVarNames[0] = "true";
      measVarNames[1] = "cand";
      printLRResData("LRAmbiRes", measVarNames);
    } else if (m_testDaf == true) {
      vector<string> dafVarNames;
      dafVarNames.push_back("w_all");
      dafVarNames.push_back("w_right");
      dafVarNames.push_back("w_wrong");
      dafVarNames.push_back("w_bg1");
      dafVarNames.push_back("w_bg2");
      printLayerWiseStatistics("DAF_weights", dafVarNames, 0);
      dafVarNames[0] = "c_all";
      dafVarNames[1] = "c_right";
      dafVarNames[2] = "c_wrong";
      dafVarNames[3] = "c_bg1";
      dafVarNames[4] = "c_bg2";
      printLayerWiseStatistics("DAF_chi2s", dafVarNames, 0);
    } else {
      if (m_nLayers > 0) {
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_meas_t", measVarNames, 2);
          printLayerWiseStatistics("res_meas_t", measVarNames, 0);
        }
        if (m_testPrediction == true) {
          if (m_truthAvailable == true) {
            printLayerWiseStatistics("pulls_and_chi2_fp_t", m_layerWiseTruthTestsVarNames, 5);
          }
          printLayerWiseStatistics("pulls_and_chi2_fp", measVarNames, 2);
        }
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_fu_t", m_layerWiseTruthTestsVarNames, 5);
        }
        printLayerWiseStatistics("pulls_and_chi2_fu", measVarNames, 2);
        if (m_testPrediction == true) {
          if (m_truthAvailable == true) {
            printLayerWiseStatistics("pulls_and_chi2_bp_t", m_layerWiseTruthTestsVarNames, 5);
          }
          printLayerWiseStatistics("pulls_and_chi2_bp", measVarNames, 2);
        }
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_bu_t", m_layerWiseTruthTestsVarNames, 5);
        }
        printLayerWiseStatistics("pulls_and_chi2_bu", measVarNames, 2);
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_sm_t", m_layerWiseTruthTestsVarNames, 5);
        }
        printLayerWiseStatistics("pulls_and_chi2_sm", measVarNames, 2);
      }
    }
    //write out the test results
    B2INFO("\n" << m_textOutput.str());
    if (m_writeToFile == true) {
      ofstream testOutputToFile((m_dataOutFileName + "StatTests.txt").c_str());
      testOutputToFile << m_textOutput.str();
      testOutputToFile.close();
    }

  }

}




void TrackFitCheckerModule::terminate()
{

  if (m_statDataTreePtr not_eq NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_statDataTreePtr->Write();
    m_rootFilePtr->Close();

    // delete all the objects associated with branches
    std::map<std::string, std::vector<float>* >::iterator iter2 = m_trackWiseVecDataForRoot.begin();
    std::map<std::string, std::vector<float>* >::const_iterator iterMax2 = m_trackWiseVecDataForRoot.end();
    while (iter2 not_eq iterMax2) {
      delete(iter2->second);
      ++iter2;
    }
    std::map<std::string, std::vector<vector<float> >* >::iterator iter3 = m_layerWiseDataForRoot.begin();
    std::map<std::string, std::vector<vector<float> >* >::const_iterator iterMax3 = m_layerWiseDataForRoot.end();
    while (iter3 not_eq iterMax3) {
      delete(iter3->second);
      ++iter3;
    }
  }
  if (m_inspectTracks  > 0) {
    m_dataOut.close();
  }
  if (m_exportTracksForRaveDeveloper == true) {
    m_forRaveOut.close();
  }

}

// calculate a chi2 value from a residuum and it's covariance matrix R
double TrackFitCheckerModule::calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R) const
{
  TMatrixT<double> invR = invertMatrix(R);

  TMatrixT<double> resT(TMatrixT<double>::kTransposed, res);
  return (resT * invR * res)[0][0];
}
// calculate pulls from a residuum and it's covariance matrix R
vector<double> TrackFitCheckerModule::calcZs(const TMatrixT<double>& res, const TMatrixT<double>& R) const
{
  const int numOfZ = R.GetNcols();
  vector<double> resultVec(numOfZ);
  for (int i = 0; i not_eq numOfZ; ++i) {
    resultVec[i] = res[i][0] / sqrt(R[i][i]);
  }
  return resultVec;
}

vector<double> TrackFitCheckerModule::calcTestsWithTruthInfo(const TMatrixT<double>& state, const TMatrixT<double>& cov, const TMatrixT<double>& trueState) const
{
  //first the all 5 pulls and the chi2 from the hole vector/matrix
  TMatrixT<double> res = state - trueState;
  vector<double> resultVec = calcZs(res, cov);

  resultVec.push_back(calcChi2(res, cov));

  TMatrixT<double> subRes(2, 1);
  TMatrixT<double> subCov(2, 2);
  //chi2 only of directions
  subRes[0][0] = res[1][0];
  subRes[1][0] = res[2][0];
  subCov = cov.GetSub(1, 2, 1, 2);
  resultVec.push_back(calcChi2(subRes, subCov));
  //chi2 only of u and v
  subRes[0][0] = res[3][0];
  subRes[1][0] = res[4][0];
  subCov = cov.GetSub(3, 4, 3, 4);

  resultVec.push_back(calcChi2(subRes, subCov));
  // chi2 only of directions and u and v
  subRes.ResizeTo(4, 1);
  subCov.ResizeTo(4, 4);
  subRes[0][0] = res[1][0];
  subRes[1][0] = res[2][0];
  subRes[2][0] = res[3][0];
  subRes[3][0] = res[4][0];
  subCov = cov.GetSub(1, 4, 1, 4);
  resultVec.push_back(calcChi2(subRes, subCov));

  //chi2 of q/p and directions
  subRes.ResizeTo(3, 1);
  subCov.ResizeTo(3, 3);
  subRes[0][0] = res[0][0];
  subRes[1][0] = res[1][0];
  subRes[2][0] = res[2][0];

  subCov[0][0] = cov[0][0];
  subCov[1][0] = cov[1][0];
  subCov[2][0] = cov[2][0];

  subCov[0][1] = cov[0][1];
  subCov[1][1] = cov[1][1];
  subCov[2][1] = cov[2][1];

  subCov[0][2] = cov[0][2];
  subCov[1][2] = cov[1][2];
  subCov[2][2] = cov[2][2];
  resultVec.push_back(calcChi2(subRes, subCov));

  // chi2 of q/p and u and v
  subRes[0][0] = res[0][0];
  subRes[1][0] = res[3][0];
  subRes[2][0] = res[4][0];

  subCov[0][0] = cov[0][0];
  subCov[1][0] = cov[3][0];
  subCov[2][0] = cov[4][0];

  subCov[0][1] = cov[0][3];
  subCov[1][1] = cov[3][3];
  subCov[2][1] = cov[4][3];

  subCov[0][2] = cov[0][4];
  subCov[1][2] = cov[3][4];
  subCov[2][2] = cov[4][4];
  resultVec.push_back(calcChi2(subRes, subCov));

  return resultVec;
}

bool TrackFitCheckerModule::hasMatrixNegDiagElement(const TMatrixT<double>& aMatrix) const
{
  int n = aMatrix.GetNrows(); //matrix must be quadratic
  for (int i = 0; i not_eq n; ++i) {
    if (aMatrix[i][i] < 0.0) {
      return true;
    }
  }
  return false;
}

void TrackFitCheckerModule::isMatrixCov(const TMatrixT<double>& cov)
{
  if (isSymmetric(cov) == false) {
    ++m_unSymmetricCounter;
  }
  TMatrixDEigen eigenProblem(cov);
  TMatrixT<double> eigenValues = eigenProblem.GetEigenValues();
  //cov.Print();
  //eigenValues.Print();
  if (eigenValues[0][0] <= 0.0 or eigenValues[1][1] <= 0.0 or eigenValues[2][2] <= 0.0 or eigenValues[3][3] <= 0.0 or eigenValues[4][4] <= 0.0) {
    ++m_notPosDefCounter;
  }
  //return
}

bool TrackFitCheckerModule::isSymmetric(const TMatrixT<double>& aMatrix) const
{
  int n = aMatrix.GetNrows();
  int m = aMatrix.GetNcols();
  if (n not_eq m) {
    return false;
  }
  const static double epsilon = 1E-15;
  TMatrixT<double> aMatrixT(TMatrixT<double>::kTransposed, aMatrix);
  TMatrixT<double> diffMat = aMatrix - aMatrixT;

  for (int i = 0; i not_eq n; ++i) {
    for (int j = 0; j not_eq n; ++j) {
      if (abs(diffMat[i][j]) > epsilon) {
        return false;
      }
    }
  }
  return true;
}

void TrackFitCheckerModule::printLayerWiseStatistics(const string& nameOfDataSample, const vector<string>& layerWiseVarNames, int madVars, const bool count)
{
  vector<vector<StatisticsContainer> >&  dataSample = m_layerWiseDataSamples[nameOfDataSample];

  int nOfLayers = dataSample.size();
  int nOfVars = dataSample[0].size();
  //construct the string for the text output to include the correct number of layers
  stringstream aStrStr;
  const int nOfLayersPlus1 = nOfLayers + 1;
  for (int l = 1; l not_eq nOfLayersPlus1; ++l) { //start at 1 to for the textoutput
    aStrStr << l << "\t\t";
    if (m_robust == true) {
      aStrStr << "\t\t\t";
    }
    if (count == true) {
      aStrStr << "\t";
    }
  }
  aStrStr << "\n\t";
  for (int l = 0; l not_eq nOfLayers; ++l) {
    aStrStr << "mean\tstd\t";
    if (m_robust == true) {
      aStrStr << "trm/med\ttrσ/MAD\t#outCAT\t";
    }
    if (count == true) {
      aStrStr << "count\t";
    }
  }
  aStrStr << "\n";
  m_textOutput << "Information on " << nameOfDataSample << " for all layers\npara\\l\t" << aStrStr.str();
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << layerWiseVarNames[i];
    for (int l = 0; l not_eq nOfLayers; ++l) {
      double tempMean = mean(dataSample[l][i]);
      double tempStd = sqrt(variance(dataSample[l][i]));
      vector<double>& data = m_layerWiseData[nameOfDataSample][l][i];
      m_textOutput << fixed << "\t" << tempMean << "\t" << tempStd;
      if (m_robust == true) {
        if (data.empty() == false) {
          if (i < madVars) { //
            double aMedian = median(dataSample[l][i]);
            double scaledMad = 1.4826 * calcMad(data, aMedian);
            int nOutliers = countOutliers(data, aMedian, scaledMad, 4);
            m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers;
          } else {
            double mean = 0;
            double std = 0;
            int nCutAwayTracks = trunctatedMeanAndStd(data, 0.02, false, mean, std);
            m_textOutput << "\t" << mean << "\t" << std << "\t" << nCutAwayTracks;

          }
        } else {
          m_textOutput << "\t" << "no" << "\t" << "data" << "\t" << " ";
        }
        //double madScalingFactor =  m_madScalingFactors[nameOfDataSample];

      }

      if (count == true) {
        m_textOutput << "\t" << boost::accumulators::count(dataSample[l][i]);
      }
    }
    m_textOutput << "\n";
  }
}

void TrackFitCheckerModule::printLRResData(const string& nameOfDataSample, const vector<string>& layerWiseVarNames)
{
  vector<vector<StatisticsContainer> >&  dataSample = m_layerWiseDataSamples[nameOfDataSample];

  int nOfLayers = dataSample.size();
  int nOfVars = dataSample[0].size();
  //construct the string for the text output to include the correct number of layers
  stringstream aStrStr;
  const int nOfLayersPlus1 = nOfLayers + 1;
  for (int l = 1; l not_eq nOfLayersPlus1; ++l) { //start at 1 to for the textoutput
    aStrStr << l << "\t\t";
  }
  aStrStr << "\n\t";
  for (int l = 0; l not_eq nOfLayers; ++l) {
    aStrStr << "mean\t";

    aStrStr << "count\t";
  }
  aStrStr << "\n";
  m_textOutput << "Information on " << nameOfDataSample << " for all layers\npara\\l\t" << aStrStr.str();
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << layerWiseVarNames[i];
    for (int l = 0; l not_eq nOfLayers; ++l) {
      double tempMean = mean(dataSample[l][i]);

      m_textOutput << fixed << "\t" << tempMean;
      m_textOutput << "\t" << boost::accumulators::count(dataSample[l][i]);

    }
    m_textOutput << "\n";
  }
}


void TrackFitCheckerModule::printTrackWiseStatistics(const string& nameOfDataSample, const bool count)
{
  StatisticsContainer&  dataSample = m_trackWiseDataSamples[nameOfDataSample];

  m_textOutput << "Information on " << nameOfDataSample << "\nmean\tstd";
  if (m_robust == true) {
    m_textOutput << "\tmedian\tMAD std\toutlier\ttr mean\ttr std\tignored";
  }
  if (count == true) {
    m_textOutput << "\tcount";
  }
  m_textOutput << "\n";
  m_textOutput << fixed << mean(dataSample) << "\t" << sqrt(variance(dataSample));

  if (m_robust == true) {
    double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
    vector<double>& data = m_trackWiseData[nameOfDataSample];
    if (madScalingFactor > 1E-100) {
      double aMedian = median(dataSample);
      double scaledMad = madScalingFactor * calcMad(data, aMedian);
      int nOutliers = countOutliers(data, aMedian, scaledMad, 4);
      m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers; // << "\t" << calcMedian(m_trackWiseData[nameOfDataSample]);
    } else {
      m_textOutput << "\tno scaling for MAD";
    }
    double trunctationRatio = m_trunctationRatios[nameOfDataSample];
    if (trunctationRatio > 1E-100) {
      double mean = 0;
      double std = 0;
      int nCutAwayTracks = trunctatedMeanAndStd(data, trunctationRatio, true, mean, std);
      m_textOutput << "\t" << mean << "\t" << std << "\t" << nCutAwayTracks;

    } else {
      m_textOutput << "\tno cut away ratio given";
    }
  } else if (count == true) {
    m_textOutput << "\t\t\t\t\t\t";
  }
  if (count == true) {
    m_textOutput << "\t" << boost::accumulators::count(dataSample);
  }
  m_textOutput << "\n";
}

void TrackFitCheckerModule::printTrackWiseVecStatistics(const string& nameOfDataSample, const vector<string>& varNames, const bool count)
{
  vector<StatisticsContainer>& dataSample = m_trackWiseVecDataSamples[nameOfDataSample];

  const int nOfVars = dataSample.size();
  m_textOutput << "Information on " << nameOfDataSample << "\n\tmean\tstd";
  if (m_robust == true) {
    m_textOutput << "\tmedian\tMAD std\toutlier\ttr mean\ttr std\tignored";
  }
  if (count == true) {
    m_textOutput << "\tcount";
  }
  m_textOutput << "\n";
  double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
  double trunctationRatio = m_trunctationRatios[nameOfDataSample];
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << fixed << varNames[i] << "\t" << mean(dataSample[i]) << "\t" << sqrt(variance(dataSample[i]));
    if (m_robust == true) {

      vector<double>& data = m_trackWiseVecData[nameOfDataSample][i];
      if (madScalingFactor > 1E-100) {
        double aMedian = median(dataSample[i]);
        double scaledMad = madScalingFactor * calcMad(data, aMedian);
        int nOutliers = countOutliers(data, aMedian, scaledMad, 4);
        m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers; //<< "\t" << calcMedian(data);
      } else {
        m_textOutput << "\tno scaling for MAD";
      }
      if (trunctationRatio > 1E-100) {
        double mean = 0;
        double std = 0;
        int nCutAwayTracks = trunctatedMeanAndStd(data, trunctationRatio, true, mean, std);
        m_textOutput << "\t" << mean << "\t" << std << "\t" << nCutAwayTracks;

      } else {
        m_textOutput << "\tno cut away ratio given";
      }
    } else if (count == true) {
      m_textOutput << "\t\t\t\t\t\t";
    }
    if (count == true) {
      m_textOutput << "\t" << boost::accumulators::count(dataSample[i]);
    }
    m_textOutput << "\n";
  }

}



void TrackFitCheckerModule::registerLayerWiseData(const string& nameOfDataSample, const int nVarsToTest)
{
  m_layerWiseDataSamples[nameOfDataSample].resize(m_nLayers, vector<StatisticsContainer>(nVarsToTest));

  if (m_writeToRootFile == true and m_nLayers > 0) {
    m_layerWiseDataForRoot[nameOfDataSample] = new std::vector<vector<float> >(m_nLayers, vector<float>(nVarsToTest));
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), "std::vector<std::vector<float> >", &(m_layerWiseDataForRoot[nameOfDataSample]));
  }
  if (m_robust == true and m_nLayers > 0) {
    m_layerWiseData[nameOfDataSample].resize(m_nLayers, vector<vector<double> >(nVarsToTest));
  }
}

void TrackFitCheckerModule::registerTrackWiseVecData(const string& nameOfDataSample, const int nVarsToTest)
{
  m_trackWiseVecDataSamples[nameOfDataSample].resize(nVarsToTest);
  if (m_writeToRootFile == true) {
    m_trackWiseVecDataForRoot[nameOfDataSample] = new std::vector<float>(nVarsToTest);
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), "std::vector<float>", &(m_trackWiseVecDataForRoot[nameOfDataSample]));
  }
  if (m_robust == true) {
    m_trackWiseVecData[nameOfDataSample].resize(nVarsToTest);
  }
}


void TrackFitCheckerModule::registerTrackWiseData(const string& nameOfDataSample)
{
  if (m_writeToRootFile == true) {
    m_trackWiseDataForRoot[nameOfDataSample] = float(-999);
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), &(m_trackWiseDataForRoot[nameOfDataSample]));
  }
}
void TrackFitCheckerModule::registerTVector3(const std::string& nameOfDataSample)
{
  m_TVector3ForRoot[nameOfDataSample] = new TVector3();
  m_statDataTreePtr->Branch(nameOfDataSample.c_str(), "TVector3", &(m_TVector3ForRoot[nameOfDataSample]));
}

void TrackFitCheckerModule::registerInt(const std::string& nameOfDataSample)
{
  m_intForRoot[nameOfDataSample] = int(-999);
  m_statDataTreePtr->Branch(nameOfDataSample.c_str(), &(m_intForRoot[nameOfDataSample]));
}

void TrackFitCheckerModule::fillLayerWiseData(const string& nameOfDataSample, const int accuVecIndex, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_layerWiseDataSamples[nameOfDataSample][accuVecIndex][i](newData[i]);
    //cout << "filled " << nameOfDataSample << " at index " << accuVecIndex << " and variable " << i << " with data bit " << newData[i] << endl;
  }
  if (m_writeToRootFile == true and m_nLayers > 0) {
    for (int i = 0; i not_eq nNewData; ++i) {
      (*m_layerWiseDataForRoot[nameOfDataSample])[accuVecIndex][i] = float(newData[i]);
    }
  }
  if (m_robust == true and m_nLayers > 0) {
    for (int i = 0; i not_eq nNewData; ++i) {
      m_layerWiseData[nameOfDataSample][accuVecIndex][i].push_back(newData[i]);
    }
  }
}

void TrackFitCheckerModule::fillTrackWiseVecData(const string& nameOfDataSample, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_trackWiseVecDataSamples[nameOfDataSample][i](newData[i]);
  }
  if (m_writeToRootFile == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      (*m_trackWiseVecDataForRoot[nameOfDataSample])[i] = float(newData[i]);
    }
  }
  if (m_robust == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      m_trackWiseVecData[nameOfDataSample][i].push_back(newData[i]);
    }
  }
}

void TrackFitCheckerModule::fillTrackWiseData(const string& nameOfDataSample, const double newData)
{
  m_trackWiseDataSamples[nameOfDataSample](newData);
  if (m_writeToRootFile == true) {
    m_trackWiseDataForRoot[nameOfDataSample] = float(newData);
  }
  if (m_robust == true) {
    m_trackWiseData[nameOfDataSample].push_back(newData);
  }
}

void TrackFitCheckerModule::fillTVector3(const std::string& nameOfDataSample, const TVector3& newData)
{
  if (m_writeToRootFile == true) {
    (*m_TVector3ForRoot[nameOfDataSample]) = newData;
  }
}

void TrackFitCheckerModule::fillInt(const std::string& nameOfDataSample, const int newData)
{
  if (m_writeToRootFile == true) {
    m_intForRoot[nameOfDataSample] = newData;
  }
}


void TrackFitCheckerModule::extractTrackData(GFTrack* const aTrackPtr, const double charge)
{

  //cerr << "1";
  //make sure anything from the last track is cleared;
  m_trackData.accuVecIndices.clear();
  m_trackData.detIds.clear();
  m_trackData.ms.clear();
  m_trackData.Hs.clear();
  m_trackData.Vs.clear();
  m_trackData.states_fu.clear();
  m_trackData.covs_fu.clear();
  m_trackData.states_bu.clear();
  m_trackData.covs_bu.clear();
  m_trackData.states_sm.clear();
  m_trackData.covs_sm.clear();
  m_trackData.states_t.clear();
  m_trackData.states_fp.clear();
  m_trackData.covs_fp.clear();
  m_trackData.states_bp.clear();
  m_trackData.covs_bp.clear();

  const int trackRepId = 0;
  GFAbsTrackRep* rep = aTrackPtr->getTrackRep(trackRepId);
  m_trackData.nHits = aTrackPtr->getNumHits();
  //cout << "nHits: " << aTrackPtr->getNumHits() << endl;
  for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
    GFAbsRecoHit*  aGFAbsRecoHitPtr = aTrackPtr->getHit(iGFHit);
    //cerr << "2 iGFHit " << iGFHit << "\n";
    VXDTrueHit const* aVxdTrueHitPtr = NULL;
    PXDRecoHit const*  aPxdRecoHitPtr = dynamic_cast<PXDRecoHit const* >(aGFAbsRecoHitPtr);
    SVDRecoHit2D const*  aSvdRecoHit2DPtr =  dynamic_cast<SVDRecoHit2D  const* >(aGFAbsRecoHitPtr);
    SVDRecoHit const*  aSvdRecoHitPtr =  dynamic_cast<SVDRecoHit  const* >(aGFAbsRecoHitPtr);
    CDCRecoHit const*  aCdcRecoHitPtr = dynamic_cast<CDCRecoHit const* >(aGFAbsRecoHitPtr); // cannot use the additional const here because the getter fuctions inside the CDCRecoHit class are not decleared as const (although they could be const)

    if (aPxdRecoHitPtr not_eq NULL) {
      //cerr << "aPxdRecoHitPtr";
      m_trackData.accuVecIndices.push_back(aPxdRecoHitPtr->getSensorID().getLayerNumber() - 1);
      m_trackData.detIds.push_back(Const::PXD);
      if (m_truthAvailable == true) {
        aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(aPxdRecoHitPtr->getTrueHit()); // first check if there is a pointer directly to the true hit
        if (aVxdTrueHitPtr == NULL) {
          if (aPxdRecoHitPtr->getSimpleDigiHit() not_eq NULL) { //then check if there is a simple digiHit with a pointer to a trueHit
            aVxdTrueHitPtr = aPxdRecoHitPtr->getSimpleDigiHit()->getTrueHit();
          }
        }
        if (aVxdTrueHitPtr == NULL) { //then check if there is a cluster which has a relation to a trueHit
          RelationIndex<PXDCluster, PXDTrueHit> relPxdClusterTrueHit;

          const PXDCluster* aPxdCluster = aPxdRecoHitPtr->getCluster();
          //now use the relations to get the trueHit
          // RelationIndex<PXDCluster, PXDTrueHit>::range_from iterPair = relPxdClusterTrueHit.getElementsFrom(aPxdCluster);

          //aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(iterPair.first->to);
          if (relPxdClusterTrueHit.getFirstElementFrom(aPxdCluster) not_eq NULL) {
            aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(relPxdClusterTrueHit.getFirstElementFrom(aPxdCluster)->to);
          }


          B2DEBUG(100, "aVxdTrueHitPtr is " << aVxdTrueHitPtr << " after the relation from clusters to trueHits was searched");
          //assert(aVxdTrueHitPtr not_eq NULL);
          //cout << "aVxdTrueHitPtr: " << aVxdTrueHitPtr << endl;
        }
      }
    } else if (aSvdRecoHit2DPtr not_eq NULL) {
      //cerr << "aSvdRecoHit2DPtr";
      int accuVecIndex = aSvdRecoHit2DPtr->getSensorID().getLayerNumber() - 1;
      if (m_nPxdLayers == 0) {
        accuVecIndex -= 2; // if the PXD is not simulated the first SVD layer will use the 0 element in all the layer wise statistics container
      }
      m_trackData.accuVecIndices.push_back(accuVecIndex);
      m_trackData.detIds.push_back(Const::SVD);
      if (m_truthAvailable == true) {
        aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(aSvdRecoHit2DPtr->getTrueHit());
        if (aVxdTrueHitPtr == NULL) {
          if (aSvdRecoHit2DPtr->getSimpleDigiHit() not_eq NULL) {
            aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(aSvdRecoHit2DPtr->getSimpleDigiHit()->getTrueHit());
          }
        }
      }
    } else if (aSvdRecoHitPtr not_eq NULL) {
      //cerr << "aSvdRecoHitPtr";
      int accuVecIndex = aSvdRecoHitPtr->getSensorID().getLayerNumber() - 1;
      if (m_nPxdLayers == 0) {
        accuVecIndex -= 2; // if the PXD is not simulated the first SVD layer will use the 0 element in all the layer wise statistics container
      }
      m_trackData.accuVecIndices.push_back(accuVecIndex);
      m_trackData.detIds.push_back(Const::SVD);
      if (m_truthAvailable == true) {
        RelationIndex<SVDCluster, SVDTrueHit> relSvdClusterTrueHit;
        const SVDCluster* aSvdCluster = aSvdRecoHitPtr->getCluster();
        //RelationIndex<SVDCluster, SVDTrueHit>::range_from iterPair = relSvdClusterTrueHit.getElementsFrom(aSvdCluster);

        //aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(iterPair.first->to);
        if (relSvdClusterTrueHit.getFirstElementFrom(aSvdCluster) not_eq NULL) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(relSvdClusterTrueHit.getFirstElementFrom(aSvdCluster)->to);
        }

        //assert(aVxdTrueHitPtr not_eq NULL);
        // cout << "aVxdTrueHitPtr3: " << aVxdTrueHitPtr << endl;
      }




    } else if (aCdcRecoHitPtr not_eq NULL) {
      //cerr << "aCdcRecoHitPtr";
      m_trackData.accuVecIndices.push_back(aCdcRecoHitPtr->getWireID().getICLayer() + m_nPxdLayers + m_nSvdLayers);
      m_trackData.detIds.push_back(Const::CDC);
    } else {
      B2ERROR("An unknown type of recoHit was detected in TrackFitCheckerModule::event(). This hit will not be included in the statistical tests");
    }

    //    if (m_testDaf == true or(aCdcRecoHitPtr not_eq NULL and m_testCdc == false) or(aPxdRecoHitPtr not_eq NULL and m_testSi == false)or(aSvdRecoHit2DPtr not_eq NULL and m_testSi == false)) {    // skip all other stuff when daf is tested because it is not used // something is wrong with the skipping better leave it out
    //      continue;
    //    }
    //cerr << "4";
    GFDetPlane detPlaneOfRecoHit = aGFAbsRecoHitPtr->getDetPlane(rep);

    //cerr << "5";
    TVectorD state(5);
    TMatrixDSym cov(5);
    TVectorD m;
    TMatrixDSym V;
    //cerr << "bin hier vor get measurement\n";
    aGFAbsRecoHitPtr->getMeasurement(rep, detPlaneOfRecoHit, rep->getState(), rep->getCov(), m, V);
    //cerr << "und jetzt danach\n";
    m_trackData.Hs.push_back(aGFAbsRecoHitPtr->getHMatrix(rep));

    m_trackData.ms.push_back(TMatrixD(m.GetNrows(), 1, m.GetMatrixArray()));
    m_trackData.Vs.push_back(TMatrixD(V));

    state = aTrackPtr->getBK(trackRepId)->getVector(GFBKKey_fUpSt, iGFHit);
    cov = aTrackPtr->getBK(trackRepId)->getSymMatrix(GFBKKey_fUpCov, iGFHit);
    m_trackData.states_fu.push_back(TMatrixD(state.GetNrows(), 1, state.GetMatrixArray()));
    m_trackData.covs_fu.push_back(TMatrixD(cov));
    state = aTrackPtr->getBK(trackRepId)->getVector(GFBKKey_bUpSt, iGFHit);
    cov = aTrackPtr->getBK(trackRepId)->getSymMatrix(GFBKKey_bUpCov, iGFHit);
    m_trackData.states_bu.push_back(TMatrixD(state.GetNrows(), 1, state.GetMatrixArray()));
    m_trackData.covs_bu.push_back(TMatrixD(cov));
    //cerr << "getbias\n";
    GFTools::getBiasedSmoothedData(aTrackPtr, trackRepId, iGFHit, state, cov);
    //cerr << "und jetzt danach\n";
    m_trackData.states_sm.push_back(TMatrixD(state.GetNrows(), 1, state.GetMatrixArray()));
    m_trackData.covs_sm.push_back(TMatrixD(cov));

    if (m_testPrediction == true) {
      state = aTrackPtr->getBK(trackRepId)->getVector(GFBKKey_fSt, iGFHit);
      cov = aTrackPtr->getBK(trackRepId)->getSymMatrix(GFBKKey_fCov, iGFHit);
      m_trackData.states_fp.push_back(TMatrixD(state.GetNrows(), 1, state.GetMatrixArray()));
      m_trackData.covs_fp.push_back(TMatrixD(cov));
      state = aTrackPtr->getBK(trackRepId)->getVector(GFBKKey_bSt, iGFHit);
      cov = aTrackPtr->getBK(trackRepId)->getSymMatrix(GFBKKey_bCov, iGFHit);
      m_trackData.states_bp.push_back(TMatrixD(state.GetNrows(), 1, state.GetMatrixArray()));
      m_trackData.covs_bp.push_back(TMatrixD(cov));
    } else if (m_inspectTracks > 0) {

    }

    if (aVxdTrueHitPtr not_eq NULL) {
      TMatrixT<double> trueState(5, 1);
      TVector3 pTrue = aVxdTrueHitPtr->getMomentum();
      trueState[0][0] = charge / pTrue.Mag(); // q/p
      trueState[1][0] = pTrue[0] / pTrue[2]; //dudw
      trueState[2][0] = pTrue[1] / pTrue[2];//dvdw
      trueState[3][0] = aVxdTrueHitPtr->getU(); // u
      trueState[4][0] = aVxdTrueHitPtr->getV(); // v
      m_trackData.states_t.push_back(trueState);
    } else if (m_truthAvailable == true) {
      B2WARNING("Although truthAvailable is set to true the simulated truth for the current hit could not be found. This should not happen. Something is wrong");
      TMatrixT<double> trueState;
      m_trackData.states_t.push_back(trueState);
    }
  }
}

void TrackFitCheckerModule::truthTests()  //
{
//  cout << "TrackFitCheckerModule::truthTests" << endl;
//  cout << "m_trackData.nHits" << m_trackData.nHits <<endl;
  for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
    //cout << "for loop " << iGFHit << endl;
    int detId = m_trackData.detIds[iGFHit];
    if (detId == Const::PXD or detId == Const::SVD) {  //at the moment there is only truth info for PXD and SVD hits
      int accuVecIndex = m_trackData.accuVecIndices[iGFHit];

      TMatrixT<double> trueState = m_trackData.states_t[iGFHit];
      if (trueState.GetNrows() == 0) { // this should not happen!!! If it does something went wrong. As a workaround just skip the hit (better than crashing I guess)
        continue;
      }
      //cout << "true, m, H, V" << endl;
      //      trueState.Print();
      //      m_trackData.ms[iGFHit].Print();
      //      m_trackData.Hs[iGFHit].Print();
      //cerr << "hier?";
      //cerr <<  "i:" << iGFHit << " detID: " << detId << "\n";
      //m_trackData.ms[iGFHit].Print();
      //m_trackData.Hs[iGFHit].Print();
      //trueState.Print();

      //vector<double> measTrueTests(2);
      TMatrixT<double> res = m_trackData.ms[iGFHit] - m_trackData.Hs[iGFHit] * trueState;
      //      if (res.GetNrows() == 1 ){ cannot work as long as every hit is filled seperatly
      //        if ( m_trackData.Hs[iGFHit][3][0] > 0.1 ){ // u coordinate
      //          measTrueTests[0] = res[0][0];
      //        } else { // v coordinate
      //          measTrueTests[1] = res[0][0];
      //        }
      //      }
      vector<double> resStdVec(res.GetMatrixArray(), res.GetMatrixArray() + res.GetNrows()); //convert TMatrixD to std::vector
      fillLayerWiseData("res_meas_t", accuVecIndex, resStdVec);
      //res.Print();
      TMatrixT<double> V = m_trackData.Vs[iGFHit];
      //V.Print();
      //V.Sqrt().Print();
      vector<double> measTrueTests = calcZs(res, V);
      //cerr << "blub";
      measTrueTests.push_back(calcChi2(res, V));
      //cerr << "bla\n";
      fillLayerWiseData("pulls_and_chi2_meas_t", accuVecIndex, measTrueTests);

      fillLayerWiseData("pulls_and_chi2_sm_t", accuVecIndex, calcTestsWithTruthInfo(m_trackData.states_sm[iGFHit], m_trackData.covs_sm[iGFHit], trueState));
      fillLayerWiseData("pulls_and_chi2_fu_t", accuVecIndex, calcTestsWithTruthInfo(m_trackData.states_fu[iGFHit], m_trackData.covs_fu[iGFHit], trueState));
      fillLayerWiseData("pulls_and_chi2_bu_t", accuVecIndex, calcTestsWithTruthInfo(m_trackData.states_bu[iGFHit], m_trackData.covs_bu[iGFHit], trueState));
      if (m_testPrediction == true) {
        fillLayerWiseData("pulls_and_chi2_fp_t", accuVecIndex, calcTestsWithTruthInfo(m_trackData.states_fp[iGFHit], m_trackData.covs_fp[iGFHit], trueState));
        fillLayerWiseData("pulls_and_chi2_bp_t", accuVecIndex, calcTestsWithTruthInfo(m_trackData.states_bp[iGFHit], m_trackData.covs_bp[iGFHit], trueState));
      }
    }
  }
}

void TrackFitCheckerModule::normalTests()
{
  TMatrixT<double> state(5, 1);
  TMatrixT<double> res;
  TMatrixT<double> R;
  for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
    B2DEBUG(100, "function normalTests() processing hit " << iGFHit);
    int detId = m_trackData.detIds[iGFHit];
    int accuVecIndex = m_trackData.accuVecIndices[iGFHit];
    if (detId == Const::PXD or detId == Const::SVD) {
      if (m_testSi == false) {
        continue;
      }
    } else if (detId == Const::CDC) {
      if (m_testCdc == false) {
        continue;
      }
    }

    const TMatrixT<double> H = m_trackData.Hs[iGFHit];
    const TMatrixT<double> HT(TMatrixT<double>::kTransposed, H); // the transposed is needed later
    const TMatrixT<double> m = m_trackData.ms[iGFHit]; //measurement of hit
    const TMatrixT<double> V = m_trackData.Vs[iGFHit]; //covariance matrix of hit
    if (V.GetNcols() == 1) {
      res.ResizeTo(1, 1);
      R.ResizeTo(1, 1);
    } else if (V.GetNcols() == 2) {
      res.ResizeTo(2, 1);
      R.ResizeTo(2, 2);
    }
    res = m - H * m_trackData.states_sm[iGFHit];
    R = V - H * m_trackData.covs_sm[iGFHit] * HT;
    if (hasMatrixNegDiagElement(R) == true) {
      ++m_badR_smCounter;
    } else {
      vector<double> testResutlsWithoutTruth = calcZs(res, R);
      testResutlsWithoutTruth.push_back(calcChi2(res, R));
      fillLayerWiseData("pulls_and_chi2_sm", accuVecIndex, testResutlsWithoutTruth);
    }

    res = m - H * m_trackData.states_fu[iGFHit];
    R = V - H * m_trackData.covs_fu[iGFHit] * HT;
    if (hasMatrixNegDiagElement(R) == true) {
      ++m_badR_fCounter;
    } else {
      vector<double> testResutlsWithoutTruth = calcZs(res, R);
      testResutlsWithoutTruth.push_back(calcChi2(res, R));
      fillLayerWiseData("pulls_and_chi2_fu", accuVecIndex, testResutlsWithoutTruth);
    }

    res = m - H * m_trackData.states_bu[iGFHit];
    R = V - H * m_trackData.covs_bu[iGFHit] * HT;
    if (hasMatrixNegDiagElement(R) == true) {
      ++m_badR_bCounter;
    } else {
      vector<double> testResutlsWithoutTruth = calcZs(res, R);
      testResutlsWithoutTruth.push_back(calcChi2(res, R));
      fillLayerWiseData("pulls_and_chi2_bu", accuVecIndex, testResutlsWithoutTruth);
    }

    if (m_testPrediction == true) {
      res = m - H * m_trackData.states_fp[iGFHit];
      R = V + H * m_trackData.covs_fp[iGFHit] * HT;
      vector<double> testResutlsWithoutTruth = calcZs(res, R);
      testResutlsWithoutTruth.push_back(calcChi2(res, R));
      fillLayerWiseData("pulls_and_chi2_fp", accuVecIndex, testResutlsWithoutTruth);

      res = m - H * m_trackData.states_bp[iGFHit];
      R = V + H * m_trackData.covs_bp[iGFHit] * HT;
      testResutlsWithoutTruth = calcZs(res, R);
      testResutlsWithoutTruth.push_back(calcChi2(res, R));
      fillLayerWiseData("pulls_and_chi2_bp", accuVecIndex, testResutlsWithoutTruth);

    }

  }
}


void TrackFitCheckerModule::inspectTracks(double chi2tot_fu, double vertexAbsMom)
{
  //#event# proc# layer#  χ²_inc,p  χ²_inc,u  m_u m_v Δu Δv
  //# |p| q/p_p du/dw_p dv/dw_p u_p v_p σ_u,p  σ_v,p
  //# χ²_tot,u  q/p_u du/dw_u dv/dw_u u_u v_u σ_u,u  σ_v,u
  //#   q/p du/dw dv/dw u v ΔΦ  Δθ
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);

  //cerr << "iam in inspectTracks and m_inspectTracks is " << m_inspectTracks  << "\n";
  int eventCounter = eventMetaDataPtr->getEvent();
  if (m_inspectTracks == 1) { // old style output
    stringstream secondLine;
    stringstream thirdLine;
    stringstream forthLine;

    m_dataOut << eventCounter << "\t" << m_processedTracks ;
    if (chi2tot_fu > 50.0) {
      thirdLine << "!!!!\t" << chi2tot_fu; //mark as outlier track
    } else {
      thirdLine << "\t" << chi2tot_fu;
    }
    forthLine << "\t";
    secondLine << "\t" << vertexAbsMom;

    for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
      int hitLayerId = m_trackData.accuVecIndices[iGFHit] + 1;
      forthLine << "\t" << m_trackData.states_t[iGFHit][0][0] << "\t" << m_trackData.states_t[iGFHit][1][0] << "\t" << m_trackData.states_t[iGFHit][2][0] << "\t" << m_trackData.states_t[iGFHit][3][0] << "\t" << m_trackData.states_t[iGFHit][4][0] /*<< "\t" << pOutTrue.DeltaPhi(pInTrue) << "\t" <<  pOutTrue.Theta() - pInTrue.Theta()*/;

      secondLine << "\t" << m_trackData.states_fp[iGFHit][0][0] << "\t" << m_trackData.states_fp[iGFHit][1][0] << "\t" << m_trackData.states_fp[iGFHit][2][0] << "\t" << m_trackData.states_fp[iGFHit][3][0] << "\t" << m_trackData.states_fp[iGFHit][4][0] << "\t" << sqrt(m_trackData.covs_fp[iGFHit][3][3]) << "\t" << sqrt(m_trackData.covs_fp[iGFHit][4][4]);

      thirdLine  << "\t" << m_trackData.states_fu[iGFHit][0][0] << "\t" << m_trackData.states_fu[iGFHit][1][0] << "\t" << m_trackData.states_fu[iGFHit][2][0] << "\t" << m_trackData.states_fu[iGFHit][3][0] << "\t" << m_trackData.states_fu[iGFHit][4][0] << "\t" << sqrt(m_trackData.covs_fu[iGFHit][3][3]) << "\t" << sqrt(m_trackData.covs_fu[iGFHit][4][4]);
      if (m_trackData.ms[iGFHit].GetNrows() == 2) {
        m_dataOut << "\t" << "layer " << hitLayerId /*<< "\t" <<  fpChi2increment << "\t" << fuChi2Inrement*/ << "\t" << m_trackData.ms[iGFHit][0][0] << "\t" << m_trackData.ms[iGFHit][1][0] /*<< "\t" << uOutTrue - uInTrue << "\t" << vOutTrue - vInTrue*/;
      } else if (m_trackData.ms[iGFHit].GetNrows() == 2) {
        m_dataOut << "\t" << "layer " << hitLayerId /*<< "\t" <<  fpChi2increment << "\t" << fuChi2Inrement*/ << "\t" << m_trackData.ms[iGFHit][0][0] << "\t" << m_trackData.ms[iGFHit][1][0] /*<< "\t" << uOutTrue - uInTrue << "\t" << vOutTrue - vInTrue*/;

      }

      m_dataOut << "\t" << "layer " << hitLayerId /*<< "\t" <<  fpChi2increment << "\t" << fuChi2Inrement*/ << "\t" << m_trackData.ms[iGFHit][0][0] << "\t" << "1DHit" /*<< "\t" << uOutTrue - uInTrue << "\t" << vOutTrue - vInTrue*/;


    }
    m_dataOut << "\n" << secondLine.rdbuf() << "\n" << thirdLine.rdbuf() << "\n" << forthLine.rdbuf() << "\n";

  } else if (m_inspectTracks == 2) {  // mainly input data to the track
    stringstream secondLine;
    m_dataOut << eventCounter << "\t" << m_processedTracks ;
    secondLine << chi2tot_fu << "\t" << vertexAbsMom;
    for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
      int hitLayerId = m_trackData.accuVecIndices[iGFHit] + 1;
      if (m_trackData.ms[iGFHit].GetNrows() == 2) {
        m_dataOut << "\t" << "layer " << hitLayerId << "\t" << sqrt(m_trackData.Vs[iGFHit][0][0]) << "\t" << sqrt(m_trackData.Vs[iGFHit][1][1]) << "\t" << m_trackData.ms[iGFHit][0][0] << "\t" << m_trackData.ms[iGFHit][1][0];
      }
      if (m_trackData.ms[iGFHit].GetNrows() == 1 and m_trackData.Hs[iGFHit][0][3] > 0.1) { // 1D hit with u coordinate
        m_dataOut << "\t" << "layer " << hitLayerId << "\t" << sqrt(m_trackData.Vs[iGFHit][0][0]) << "\t" << "\t" << m_trackData.ms[iGFHit][0][0] << "\t";
      }
      if (m_trackData.ms[iGFHit].GetNrows() == 1 and m_trackData.Hs[iGFHit][0][4] > 0.1) { // 1D hit with u coordinate
        m_dataOut << "\t" << "layer " << hitLayerId << "\t" << "\t" << sqrt(m_trackData.Vs[iGFHit][0][0]) << "\t"  << "\t" << m_trackData.ms[iGFHit][0][0];
      }
      secondLine << "\t" << m_trackData.states_t[iGFHit][0][0] << "\t" << m_trackData.states_t[iGFHit][1][0] << "\t" << m_trackData.states_t[iGFHit][2][0] << "\t" << m_trackData.states_t[iGFHit][3][0] << "\t" << m_trackData.states_t[iGFHit][4][0];
    }
    m_dataOut << "\n" << secondLine.rdbuf() << "\n";
  }

}




void TrackFitCheckerModule::testDaf(GFTrack* const aTrackPtr)
{
  //first test the weights
  const int nHits = aTrackPtr->getNumHits();
  double dafWeight = -2.0;
  double dafChi2 = -2.0;

  vector<vector<float> > allWeights(m_nLayers, vector<float>(5, -1.0)); //it is importent that the content is initalized to -1
  vector<vector<float> > allChi2s(m_nLayers, vector<float>(5, -1.0));
  for (int iGFHit = 0; iGFHit not_eq nHits; ++iGFHit) {
    int accuVecIndex = m_trackData.accuVecIndices[iGFHit];
    //aTrackPtr->getBK(0)->getNumber("dafChi2s", iGFHit,  dafChi2);
    dafWeight = aTrackPtr->getBK(0)->getNumber(GFBKKey_dafWeight, iGFHit);// double check if this still works with background => more then one hit... probarbly it does work...
    GFAbsRecoHit* aGFAbsRecoHitPtr = aTrackPtr->getHit(iGFHit);
    const PXDRecoHit*   aPxdRecoHitPtr = dynamic_cast<const PXDRecoHit* >(aGFAbsRecoHitPtr);
    const SVDRecoHit2D*   aSvdRecoHit2DPtr =  dynamic_cast<const SVDRecoHit2D* >(aGFAbsRecoHitPtr);

    const VXDSimpleDigiHit* aVXDSimpleDigiHit = NULL;
    const VXDTrueHit* aVXDTrueHit = NULL;

    if (aPxdRecoHitPtr not_eq NULL) {
      aVXDSimpleDigiHit = aPxdRecoHitPtr->getSimpleDigiHit();
      if (aVXDSimpleDigiHit == NULL) {
        aVXDTrueHit = static_cast<const VXDTrueHit*>(aPxdRecoHitPtr->getTrueHit());
      }
    } else if (aSvdRecoHit2DPtr not_eq NULL) {
      aVXDSimpleDigiHit = aSvdRecoHit2DPtr->getSimpleDigiHit();
      if (aVXDSimpleDigiHit == NULL) {
        aVXDTrueHit = static_cast<const VXDTrueHit*>(aSvdRecoHit2DPtr->getTrueHit());
      }
    } else { //

    }
    //XXX if (aVXDSimpleDigiHit->getTrueHit() not_eq NULL) cout << "iGFHit=" << iGFHit << " u,v="  << aVXDSimpleDigiHit->getTrueHit()->getU() << " , "<< aVXDSimpleDigiHit->getTrueHit()->getV() << endl;

    //allWeights[accuVecIndex][0] = dafWeight; // first all weights in go into one histogram
    //allChi2s[accuVecIndex][0] = dafChi2;
    if (aVXDSimpleDigiHit not_eq NULL) { // at the moment only VXDSimpleDigiHit are support so skip if there is no VXDSimpleDigiHit

      if (aVXDSimpleDigiHit->getTrueHit() not_eq NULL and aVXDSimpleDigiHit->isVarianceCorrect() == true) { //no background hit, no outlier hit
        allWeights[accuVecIndex][1] = dafWeight;
        allChi2s[accuVecIndex][1] = dafChi2;
      }
      if (aVXDSimpleDigiHit->getTrueHit() not_eq NULL and aVXDSimpleDigiHit->isVarianceCorrect() == false) { //no background hit, but measurement outlier hit
        allWeights[accuVecIndex][2] = dafWeight;
        allChi2s[accuVecIndex][2] = dafChi2;
      }
      if (aVXDSimpleDigiHit->getTrueHit() == NULL) {
        if (allWeights[accuVecIndex][3] < 0.0) {
          allWeights[accuVecIndex][3] = dafWeight;// first background hit
          allChi2s[accuVecIndex][3] = dafChi2;
        } else {
          allWeights[accuVecIndex][4] = dafWeight;
          allChi2s[accuVecIndex][4] = dafChi2;
        }
      }
    } else if (aVXDTrueHit not_eq NULL) { // not really useful at the moment... just to make sure nothing crashes when somebody fits trueHits with the daf
      //no background hit, no outlier hit
      allWeights[accuVecIndex][1] = dafWeight;
      allChi2s[accuVecIndex][1] = dafChi2;

    }

  }

  for (int l = 0; l not_eq m_nLayers; ++l) {
    for (int i = 1; i not_eq 5; ++i) { // index starts at 1
      if (allWeights[l][i] >= 0.0) {
        m_layerWiseDataSamples["DAF_weights"][l][i](allWeights[l][i]);
        m_layerWiseDataSamples["DAF_weights"][l][0](allWeights[l][i]);
        m_layerWiseDataSamples["DAF_chi2s"][l][i](allChi2s[l][i]);
        m_layerWiseDataSamples["DAF_chi2s"][l][0](allChi2s[l][i]);
      }
    }
  }

  if (m_writeToRootFile == true) {
    for (int l = 0; l not_eq m_nLayers; ++l) {
      for (int i = 1; i not_eq 5; ++i) {// index starts at 1
        (*m_layerWiseDataForRoot["DAF_weights"])[l][i] = allWeights[l][i];
        (*m_layerWiseDataForRoot["DAF_chi2s"])[l][i] = allChi2s[l][i];
      }
    }
  }
  if (m_robust == true) {
    for (int l = 0; l not_eq m_nLayers; ++l) {
      for (int i = 1; i not_eq 5; ++i) {// index starts at 1
        m_layerWiseData["DAF_weights"][l][i].push_back(allWeights[l][i]);
      }
    }
  }
}

void TrackFitCheckerModule::testLRAmbiResolution(GFTrack* const aTrackPtr)
{
  const int nHits = aTrackPtr->getNumHits();
  const GFTrackCand& aTrackCand = aTrackPtr->getCand();
  CDC::CDCGeometryPar& cdcGeometry = CDC::CDCGeometryPar::Instance();
  for (int iGFHit = 0; iGFHit not_eq nHits; ++iGFHit) {
    B2DEBUG(100, "iGFHit " << iGFHit);
    GFAbsRecoHit* aGFAbsRecoHitPtr = aTrackPtr->getHit(iGFHit);
    const CDCRecoHit* aCdcRecoHit =  dynamic_cast<const CDCRecoHit* >(aGFAbsRecoHitPtr);
    if (aCdcRecoHit not_eq NULL) {
      const CDCHit* aCdcHit = aCdcRecoHit->getCDCHit();
      int lrSignFromDaf = aCdcRecoHit->getLeftRightResolution();
      RelationIndex<CDCSimHit, CDCHit> cdcSimHitToHitRel;
      //by just tacking the first instead of inspecting all simHit I might sometimes get the wrong sign but I hope it is not too bad
      const CDCSimHit* correspondingCdcSimHit = cdcSimHitToHitRel.getFirstElementTo(aCdcHit)->from;
      //now determine the correct sign for the cdc hit drift length
      TVector3 simHitPos = correspondingCdcSimHit->getPosTrack();
      TVector3 simMom = correspondingCdcSimHit->getMomentum();
      TVector3 simHitPosOnWire = correspondingCdcSimHit->getPosWire();
      TVector3 wireStartPos = cdcGeometry.wireBackwardPosition(correspondingCdcSimHit->getWireID());
      TVector3 wireDir = simHitPosOnWire - wireStartPos;
      TVector3 wireToSimHit = simHitPos - simHitPosOnWire;
      double scalarProduct = wireToSimHit * (wireDir.Cross(simMom));
      int lrAmbiSign = boost::math::sign(scalarProduct);
      vector<double> lrAmbiSigns(2, 0.0);
      if (lrAmbiSign == lrSignFromDaf) {
        lrAmbiSigns[0] = 1.0;
      }

      CDCTrackCandHit* aCdcTrackCandHit = static_cast<CDCTrackCandHit*>(aTrackCand.getHit(iGFHit));
      int lrAmbiSignFromCand = aCdcTrackCandHit->getLeftRightResolution();
      B2DEBUG(100, "l/r ambi signs simulation, GFTrack, GFTrackCand " <<  lrAmbiSign << " " << lrSignFromDaf << " " << lrAmbiSignFromCand);
      if (lrAmbiSign == lrAmbiSignFromCand) {
        lrAmbiSigns[1] = 1.0;
      }
      fillLayerWiseData("LRAmbiRes", correspondingCdcSimHit->getWireID().getICLayer(), lrAmbiSigns);
    }
  }

}

double TrackFitCheckerModule::calcMad(const std::vector<double>& data, const double& median)
{
  const int n = data.size();
  vector<double> absRes(n);

  for (int i = 0; i not_eq n; ++i) {
    absRes[i] = abs(data[i] - median);
  }

  const int mid = n / 2;
  sort(absRes.begin(), absRes.end());
  double mad = n % 2 == 0 ? (absRes[mid] + absRes[mid - 1]) / 2.0 : absRes[mid];

  return mad;
}

double TrackFitCheckerModule::calcMedian(std::vector<double> data)
{
  const int n = data.size();
  const int mid = n / 2;
  sort(data.begin(), data.end());
  return  n % 2 == 0 ? (data[mid] + data[mid - 1]) / 2.0 : data[mid];
}



int TrackFitCheckerModule::trunctatedMeanAndStd(std::vector<double> data, const double cutRatio, const bool symmetric, double& mean, double& std)
{

  const int n = data.size();
  double truncN = -1;
  if ((n* cutRatio < 2 and symmetric == true) or(n* cutRatio < 1 and symmetric == false)) {
    return 0;
  }
  sort(data.begin(), data.end());
  std = 0;
  int i = 0;
  int iMax = 0;
  double sum = 0;

  if (symmetric == true) { // cut will be applied to both sides of the sorted sample
    int cut = int(cutRatio * n / 2.0 + 0.5);
    truncN = n - cut - cut;
    i = cut;
    iMax = n - cut;
    sum = std::accumulate(data.begin() + cut, data.end() - cut, 0.0);
  } else { // cut will only be applied to the side with the larger values of the sorted sample
    int cut = int(cutRatio * n + 0.5);
    truncN = n - cut;
    i = 0;
    iMax = n - cut;
    sum = std::accumulate(data.begin(), data.end() - cut, 0.0);
  }
  mean = sum / truncN;
  while (i not_eq iMax) {
    double diff = data[i] - mean;
    std += diff * diff;
    ++i;
  }
  std = sqrt(1.0 / (truncN - 1) * std);
  return n - truncN;
}

int TrackFitCheckerModule::countOutliers(const vector<double>& dataSample, const double mean, const double sigma, const double widthScaling)
{

  int n = dataSample.size();
  int nOutliers = 0;
  double halfInterval = widthScaling * sigma;
  assert(not(halfInterval < 0.0)); //both widthScaling and sigma must be positive therefore halfInterval, too!
  double lowerCut = mean - halfInterval;
  double upperCut = mean + halfInterval;
  B2DEBUG(100, "n=" << n << ",mean=" << mean << ",sigma=" << sigma << ",lowerCut=" << lowerCut << ",upperCut=" << upperCut);
  for (int i = 0; i not_eq n; ++i) {
    if (dataSample[i] < lowerCut or dataSample[i] > upperCut) {
      ++nOutliers;
    }
  }
  return nOutliers;
}

TMatrixD TrackFitCheckerModule::invertMatrix(const TMatrixD& aMatrix)
{
  bool status = 0;
  TDecompSVD invertAlgo(aMatrix);
  TMatrixD inv = invertAlgo.Invert(status);
  if (status == false) {
    B2ERROR("Matrix inversion in TrackFitCheckerModule::invertMatrix failed");
  }
  return inv;
}

