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

#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>

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

#include <geometry/GeometryManager.h>
#include <geometry/bfieldmap/BFieldMap.h>

#include <tracking/gfbfield/GFGeant4Field.h>

#include <GFTrack.h>
#include <GFTools.h>
#include <RKTrackRep.h>

#include <GFFieldManager.h>
#include <GFConstField.h>

#include <GFException.h>

#include <TMatrixDEigen.h>

//C++ st libs
#include <cmath>
#include <limits>
#include <numeric>
#include <set>
//for root output

using namespace std;
using namespace Belle2;
using namespace boost::accumulators;
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
  setDescription("This module tacks GFTracks as input an calculates different statistical tests some of them using the simulated truth information");

  //Parameter definition
  addParam("outputFileName", m_dataOutFileName, "A common name for all output files of this module. Suffixes to distinguish them will be added automatically", string("trackFitChecker"));
  addParam("totalChi2Cut", m_totalChi2Cut, "only tracks with a total χ² lower than this value will be considered", 1E300);
  addParam("testSi", m_testSi, "execute the layer wise tests for PXD/SVD", false);
  addParam("robustTests", m_robust, "activate additional robust statistical tests (median and MAD)", false);

  addParam("testCdc", m_testCdc, "execute the layer wise tests for CDC", false);
  addParam("truthAvailable", m_truthAvailable, "is truth info available or every hit?", false);
  addParam("inspectTracks", m_inspectTracks, "write track parameters into a text file for further inspection. When 0 this function is switched off. 1 or 2 will enable this function but have different arrangments of data in text file. EXPERIMENTAL", 0);
  addParam("writeToRootFile", m_writeToRootFile, "Set to True if you want the data from the statistical tests written into a root file", false);
  addParam("writeToTextFile", m_writeToFile, "Set to True if you want the results of the statistical tests written out in a normal text file", false);
}


TrackFitCheckerModule::~TrackFitCheckerModule()
{

}

void TrackFitCheckerModule::initialize()
{

  //cerr << "m_inspectTracks " << m_inspectTracks << endl;


  //setup genfit geometry and magneic field in case you what to used data saved on disc because then the genifitter modul was not run
  // convert the geant4 geometry to a TGeo geometry
  geometry::GeometryManager& geoManager = geometry::GeometryManager::getInstance();
  geoManager.createTGeoRepresentation();
  //pass the magnetic field to genfit
  GFFieldManager::getInstance()->init(new GFGeant4Field());
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

  } else {
    m_rootFilePtr = NULL;
    m_statDataTreePtr = NULL;
  }





  if (m_testCdc == true) {
    m_nCdcLayers = 56; //should come from the xml file
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
  m_nSiLayers = m_nPxdLayers + m_nSvdLayers;
  m_nLayers = m_nPxdLayers + m_nSvdLayers + m_nCdcLayers;

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

  if (m_robust == true) { //set the scaling factors for the MAD. No MAD will be caclulated when
    m_madScalingFactors["pulls_vertexPosMom"] = 1.4826; //scaling factor for normal distribute variables
    m_madScalingFactors["absMomVertex"] = 1.4826;
    m_madScalingFactors["res_vertexPosMom"] = 1.4826;
    m_madScalingFactors["pulls_vertexPosMom"] = 1.4826;
    m_madScalingFactors["res_curvVertex"] = 1.4826;
    m_madScalingFactors["relRes_curvVertex"] = 1.4826;
    m_madScalingFactors["relRes_p_T"] = 1.4826;
  }


  m_badR_fCounter = 0;
  m_badR_bCounter = 0;
  m_badR_smCounter = 0;
  m_processedTracks = 0;
  m_nCutawayTracks = 0;
  m_notPosDefCounter = 0;
  m_unSymmetricCounter = 0;

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
}


void TrackFitCheckerModule::beginRun()
{

}



void TrackFitCheckerModule::event()
{

  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();

  B2DEBUG(100, "**********   TrackFitCheckerModule  processing event number: " << eventCounter << " ************");

  //simulated truth information
  StoreArray<MCParticle> aMcParticleArray("");
  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  //StoreArray<CDCTrueSimHit> aCdcTrueHitArray(""); //maybe one day this will be there :-)




  //genfit stuff
  StoreArray<GFTrackCand> trackCandidates(""); // to create a new track rep for extrapolation only
  StoreArray<GFTrack> fittedTracks(""); // the results of the track fit
  const int nFittedTracks = fittedTracks.getEntries();
  //stringstreams for text file output of all forward tracking parameters

  //test one time for prediction and and weights
  if (nFittedTracks not_eq 0 and m_wAndPredPresentsTested == false) {
    try {
      double dafWeight = -1;
      fittedTracks[0]->getBK(0)->getNumber("dafWeight", 0,  dafWeight);
    } catch (GFException& e) {
      m_testDaf = false;
    }
    try {
      TMatrixD state;
      fittedTracks[0]->getBK(0)->getMatrix("fPreSt", 0, state);
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
    const int mcParticleIndex = aTrackPtr->getCand().getMcTrackId();
    const double charge = aMcParticleArray[mcParticleIndex]->getCharge();
    const TVector3 trueVertexMom = aMcParticleArray[mcParticleIndex]->getMomentum();
    const TVector3 trueVertexPos = aMcParticleArray[mcParticleIndex]->getVertex();
    //GFAbsTrackRep* propOnlyTrRepPtr = new RKTrackRep(trackCandidates[i]);

    const double chi2tot_bu = aTrackPtr->getChiSqu(); // returns the total chi2 from the backward filter
    if (chi2tot_bu > m_totalChi2Cut) {//consider this track to be an outlier and discard it; jump to next iteration of loop
      ++m_nCutawayTracks;
      continue;
    }
    // first part: get variables describing the hole track
    const double chi2tot_fu = aTrackPtr->getForwardChiSqu();
    const int ndf = aTrackPtr->getNDF();
    const double pValue_bu = TMath::Prob(chi2tot_bu, ndf);
    const double pValue_fu = TMath::Prob(chi2tot_fu, ndf);
    fillTrackWiseData("pValue_bu", pValue_bu);
    fillTrackWiseData("pValue_fu", pValue_fu);
    fillTrackWiseData("chi2tot_bu", chi2tot_bu);
    fillTrackWiseData("chi2tot_fu", chi2tot_fu);
    TVector3 vertexPos;
    TVector3 vertexMom;
    TMatrixT<double> vertexCov(6, 6);
    vector<double> zVertexPosMom(6);
    vector<double> resVertexPosMom(6);
    TVector3 poca; //point of closest approach will be overwritten
    TVector3 dirInPoca; //direction of the track at the point of closest approach will be overwritten
    RKTrackRep* aRKTrackRepPtr = static_cast<RKTrackRep*>(aTrackPtr->getCardinalRep());

    aRKTrackRepPtr->setPropDir(-1);
    try {
      aRKTrackRepPtr->extrapolateToPoint(trueVertexPos, poca, dirInPoca);
    } catch (GFException& e) {
      B2WARNING("Extrapolation of a track in Event " << eventCounter <<  " to his true vertex position failed. Track will be ignored in statistical tests");
      ++m_extrapFailed;
      continue;
    }


    GFDetPlane planeThroughVertex(poca, dirInPoca); //get planeThroughVertex through fitted vertex position

    /*
    TMatrixD state7D;
    TMatrixD cov7D;
    aRKTrackRepPtr->getStateCov7D(planeThroughVertex,state7D,cov7D);
    //state7D.Print();
    //cov7D.Print();
    TMatrixD trueState7D(7,1);
    double absTrueVertexMom = trueVertexMom.Mag();
    trueState7D[0][0] = trueVertexPos[0];
    trueState7D[1][0] = trueVertexPos[1];
    trueState7D[2][0] = trueVertexPos[2];
    trueState7D[3][0] = trueVertexMom[0]/absTrueVertexMom;
    trueState7D[4][0] = trueVertexMom[1]/absTrueVertexMom;
    trueState7D[5][0] = trueVertexMom[2]/absTrueVertexMom;
    trueState7D[6][0] = charge/absTrueVertexMom;

    vector<double> resVertex7D(7);
    resVertex7D[0] = (state7D[0][0] - trueState7D[0][0]);
    resVertex7D[1] = (state7D[1][0] - trueState7D[1][0]);
    resVertex7D[2] = (state7D[2][0] - trueState7D[2][0]);
    resVertex7D[3] = (state7D[3][0] - trueState7D[3][0]);
    resVertex7D[4] = (state7D[4][0] - trueState7D[4][0]);
    resVertex7D[5] = (state7D[5][0] - trueState7D[5][0]);
    resVertex7D[6] = (state7D[6][0] - trueState7D[6][0]);

    fillTrackWiseVecData("res_vertexState", resVertex7D);
    vector<double> zVertex7D(7);
    zVertex7D[0] = resVertex7D[0] / sqrt(cov7D[0][0]);
    zVertex7D[1] = resVertex7D[1] / sqrt(cov7D[1][1]);
    zVertex7D[2] = resVertex7D[2] / sqrt(cov7D[2][2]);
    zVertex7D[3] = resVertex7D[3] / sqrt(cov7D[3][3]);
    zVertex7D[4] = resVertex7D[4] / sqrt(cov7D[4][4]);
    zVertex7D[5] = resVertex7D[5] / sqrt(cov7D[5][5]);
    zVertex7D[6] = resVertex7D[6] / sqrt(cov7D[6][6]);
    fillTrackWiseVecData("pulls_vertexState", zVertex7D);*/
    //vertexMom = aTrackPtr->getMom(planeThroughVertex);
    //get fitted momentum at fitted vertex

    aTrackPtr->getPosMomCov(planeThroughVertex, vertexPos, vertexMom, vertexCov);
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
    B2DEBUG(100, "filled all track wise tests");
    if (m_nLayers not_eq 0) { // now the layer wise tests

      extractTrackData(aTrackPtr, charge); // read all the data for the layer wise tests from GFTracks
      B2DEBUG(100, "extractTrackData finished successfully");
      // do the layer wise test uses only data from GFTrack object
      if (m_testDaf == true) {
        //testDaf(aTrackPtr);
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
      printTrackWiseStatistics(trackWiseDataIter->first);
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
    printTrackWiseVecStatistics("res_vertexPosMom", m_vertexTestsVarNames);
    printTrackWiseVecStatistics("pulls_vertexPosMom", m_vertexTestsVarNames);
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
    if (m_testDaf == true) {
      vector<string> dafVarNames;
      dafVarNames.push_back("w_all");
      dafVarNames.push_back("w_right");
      dafVarNames.push_back("w_wrong");
      dafVarNames.push_back("w_bg1");
      dafVarNames.push_back("w_bg2");
      printLayerWiseStatistics("DAF_weights", dafVarNames);
      dafVarNames[0] = "c_all";
      dafVarNames[1] = "c_right";
      dafVarNames[2] = "c_wrong";
      dafVarNames[3] = "c_bg1";
      dafVarNames[4] = "c_bg2";
      printLayerWiseStatistics("DAF_chi2s", dafVarNames);
    } else {
      if (m_nLayers > 0) {
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_meas_t", measVarNames);
          printLayerWiseStatistics("res_meas_t", measVarNames);
        }
        if (m_testPrediction == true) {
          if (m_truthAvailable == true) {
            printLayerWiseStatistics("pulls_and_chi2_fp_t", m_layerWiseTruthTestsVarNames);
          }
          printLayerWiseStatistics("pulls_and_chi2_fp", measVarNames);
        }
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_fu_t", m_layerWiseTruthTestsVarNames);
        }
        printLayerWiseStatistics("pulls_and_chi2_fu", measVarNames);
        if (m_testPrediction == true) {
          if (m_truthAvailable == true) {
            printLayerWiseStatistics("pulls_and_chi2_bp_t", m_layerWiseTruthTestsVarNames);
          }
          printLayerWiseStatistics("pulls_and_chi2_bp", measVarNames);
        }
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_bu_t", m_layerWiseTruthTestsVarNames);
        }
        printLayerWiseStatistics("pulls_and_chi2_bu", measVarNames);
        if (m_truthAvailable == true) {
          printLayerWiseStatistics("pulls_and_chi2_sm_t", m_layerWiseTruthTestsVarNames);
        }
        printLayerWiseStatistics("pulls_and_chi2_sm", measVarNames);
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

}

// calculate a chi2 value from a residuum and it's covariance matrix R
double TrackFitCheckerModule::calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R) const
{
  TMatrixT<double> invR;
  GFTools::invertMatrix(R, invR);
  TMatrixT<double> resT(TMatrixT<double>::kTransposed, res);
  return (resT * invR * res)[0][0];
}
// calculate a chi2 value from a residuum and it's covariance matrix R
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

void TrackFitCheckerModule::printLayerWiseStatistics(const string& nameOfDataSample, const vector<string>& layerWiseVarNames, const bool count)
{
  vector<vector<StatisticsContainer> >&  dataSample = m_layerWiseDataSamples[nameOfDataSample];

  int nOfLayers = dataSample.size();
  int nOfVars = dataSample[0].size();
  //construct the string for the text output to include the correct number of layers
  stringstream aStrStr;
  const int nOfLayersPlus1 = nOfLayers + 1;
  for (int l = 1; l not_eq nOfLayersPlus1; ++l) { //start at 1 to for the textoutput
    aStrStr << l << "\t\t";
//    if( m_robust == true){
//      aStrStr << "\t\t\t";
//    }
    if (count == true) {
      aStrStr << "\t";
    }
  }
  aStrStr << "\n\t";
  for (int l = 0; l not_eq nOfLayers; ++l) {
    aStrStr << "mean\tstd\t";
//    if( m_robust == true){
//      aStrStr << "median\tMAD std\toutlier\t";
//    }
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

      m_textOutput << fixed << "\t" << tempMean << "\t" << tempStd;
//      if( m_robust == true){
//        double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
//        if (madScalingFactor > 0.001) {
//          double aMedian = median(dataSample[l][i]);
//          double scaledMad = madScalingFactor*calcMad(m_layerWiseData[nameOfDataSample][l][i],aMedian);
//          int nOutliers = countOutliers(m_layerWiseData[nameOfDataSample][l][i],aMedian, scaledMad, 4);
//          m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers;// << "\t" << calcMedian(m_trackWiseVecData[nameOfDataSample][i]);
//        } else {
//          m_textOutput << "\tno scaling => no MAD";
//        }
//      }

      if (count == true) {
        m_textOutput << "\t" << boost::accumulators::count(dataSample[l][i]);
      }
    }
    m_textOutput << "\n";
  }
}

void TrackFitCheckerModule::printTrackWiseStatistics(const string& nameOfDataSample, const bool count)
{
  StatisticsContainer&  dataSample = m_trackWiseDataSamples[nameOfDataSample];

  m_textOutput << "Information on " << nameOfDataSample << "\nmean\tstd";
  if (m_robust == true) {
    m_textOutput << "\tmedian\tMAD std\toutlier";
  }
  if (count == true) {
    m_textOutput << "\tcount";
  }
  m_textOutput << "\n";
  m_textOutput << fixed << mean(dataSample) << "\t" << sqrt(variance(dataSample));

  if (m_robust == true) {
    double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
    if (madScalingFactor > 0.001) {
      double aMedian = median(dataSample);
      double scaledMad = madScalingFactor * calcMad(m_trackWiseData[nameOfDataSample], aMedian);
      int nOutliers = countOutliers(m_trackWiseData[nameOfDataSample], aMedian, scaledMad, 4);
      m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers; // << "\t" << calcMedian(m_trackWiseData[nameOfDataSample]);
    } else {
      m_textOutput << "\tcannot compute MAD std";
    }
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
    m_textOutput << "\tmedian\tMAD std\toutlier";
  }
  if (count == true) {
    m_textOutput << "\tcount";
  }
  m_textOutput << "\n";
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << fixed << varNames[i] << "\t" << mean(dataSample[i]) << "\t" << sqrt(variance(dataSample[i]));
    if (m_robust == true) {
      double madScalingFactor =  m_madScalingFactors[nameOfDataSample];
      if (madScalingFactor > 0.001) {
        double aMedian = median(dataSample[i]);
        double scaledMad = madScalingFactor * calcMad(m_trackWiseVecData[nameOfDataSample][i], aMedian);
        int nOutliers = countOutliers(m_trackWiseVecData[nameOfDataSample][i], aMedian, scaledMad, 4);
        m_textOutput << "\t" << aMedian << "\t" << scaledMad << "\t" << nOutliers; //<< "\t" << calcMedian(m_trackWiseVecData[nameOfDataSample][i]);
      } else {
        m_textOutput << "\tno scaling/MAD";
      }
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

  if (m_writeToRootFile == true) {
    m_layerWiseDataForRoot[nameOfDataSample] = new std::vector<vector<float> >(m_nLayers, vector<float>(nVarsToTest));
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), "std::vector<std::vector<float> >", &(m_layerWiseDataForRoot[nameOfDataSample]));
  }
  if (m_robust == true) {
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

// this function does not check whether m_writeToRootFile it true or not so it should only be called when it is true
void TrackFitCheckerModule::registerTrackWiseData(const string& nameOfDataSample)
{
  if (m_writeToRootFile == true) {
    m_trackWiseDataForRoot[nameOfDataSample] = float(-999);
    m_statDataTreePtr->Branch(nameOfDataSample.c_str(), &(m_trackWiseDataForRoot[nameOfDataSample]));
  }
}

void TrackFitCheckerModule::fillLayerWiseData(const string& nameOfDataSample, const int accuVecIndex, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_layerWiseDataSamples[nameOfDataSample][accuVecIndex][i](newData[i]);
  }
  if (m_writeToRootFile == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      (*m_layerWiseDataForRoot[nameOfDataSample])[accuVecIndex][i] = float(newData[i]);
    }
  }
  if (m_robust == true) {
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


void TrackFitCheckerModule::extractTrackData(GFTrack* const aTrackPtr, const double charge)
{


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
  TMatrixT<double> state;
  TMatrixT<double> cov;
  for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
    GFAbsRecoHit*  aGFAbsRecoHitPtr = aTrackPtr->getHit(iGFHit);

    VXDTrueHit const* aVxdTrueHitPtr = NULL;
    PXDRecoHit const*  aPxdRecoHitPtr = dynamic_cast<PXDRecoHit const* >(aGFAbsRecoHitPtr);
    SVDRecoHit2D const*  aSvdRecoHit2DPtr =  dynamic_cast<SVDRecoHit2D  const* >(aGFAbsRecoHitPtr);
    SVDRecoHit const*  aSvdRecoHitPtr =  dynamic_cast<SVDRecoHit  const* >(aGFAbsRecoHitPtr);
    CDCRecoHit const*  aCdcRecoHitPtr = dynamic_cast<CDCRecoHit const* >(aGFAbsRecoHitPtr); // cannot use the additional const here because the getter fuctions inside the CDCRecoHit class are not decleared as const (although they could be const)
    if (aPxdRecoHitPtr not_eq NULL) {
      m_trackData.accuVecIndices.push_back(aPxdRecoHitPtr->getSensorID().getLayerNumber() - 1);
      m_trackData.detIds.push_back(0);
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
          // RelationIndex<PXDCluster, PXDTrueHit>::range_from iterPair = relPxdClusterTrueHit.getFrom(aPxdCluster);

          //aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(iterPair.first->to);
          if (relPxdClusterTrueHit.getFirstTo(aPxdCluster) not_eq NULL) {
            aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(relPxdClusterTrueHit.getFirstTo(aPxdCluster)->to);
          }


          B2DEBUG(100, "aVxdTrueHitPtr is " << aVxdTrueHitPtr << " after the relation from clusters to trueHits was searched");
          //assert(aVxdTrueHitPtr not_eq NULL);
          //cout << "aVxdTrueHitPtr: " << aVxdTrueHitPtr << endl;
        }
      }
    } else if (aSvdRecoHit2DPtr not_eq NULL) {
      m_trackData.accuVecIndices.push_back(aSvdRecoHit2DPtr->getSensorID().getLayerNumber() - 1);
      m_trackData.detIds.push_back(1);
      if (m_truthAvailable == true) {
        aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(aSvdRecoHit2DPtr->getTrueHit());
        if (aVxdTrueHitPtr == NULL) {
          if (aSvdRecoHit2DPtr->getSimpleDigiHit() not_eq NULL) {
            aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(aSvdRecoHit2DPtr->getSimpleDigiHit()->getTrueHit());
          }
        }
      }
    } else if (aSvdRecoHitPtr not_eq NULL) {
      m_trackData.accuVecIndices.push_back(aSvdRecoHitPtr->getSensorID().getLayerNumber() - 1);
      m_trackData.detIds.push_back(1);
      if (m_truthAvailable == true) {
        RelationIndex<SVDCluster, SVDTrueHit> relSvdClusterTrueHit;
        const SVDCluster* aSvdCluster = aSvdRecoHitPtr->getCluster();
        //RelationIndex<SVDCluster, SVDTrueHit>::range_from iterPair = relSvdClusterTrueHit.getFrom(aSvdCluster);

        //aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(iterPair.first->to);
        if (relSvdClusterTrueHit.getFirstTo(aSvdCluster) not_eq NULL) {
          aVxdTrueHitPtr = static_cast<VXDTrueHit const*>(relSvdClusterTrueHit.getFirstTo(aSvdCluster)->to);
        }

        //assert(aVxdTrueHitPtr not_eq NULL);
        // cout << "aVxdTrueHitPtr3: " << aVxdTrueHitPtr << endl;
      }




    } else if (aCdcRecoHitPtr not_eq NULL) {
      m_trackData.accuVecIndices.push_back(aCdcRecoHitPtr->getLayerId() + m_nPxdLayers + m_nSvdLayers);
      m_trackData.detIds.push_back(2);
    } else {
      B2ERROR("An unknown type of recoHit was detected in TrackFitCheckerModule::event(). This hit will not be included in the statistical tests");
    }

    if (m_testDaf == true or(aCdcRecoHitPtr not_eq NULL and m_testCdc == false) or(aPxdRecoHitPtr not_eq NULL and m_testSi == false)or(aSvdRecoHit2DPtr not_eq NULL and m_testSi == false)) {    // skip all other stuff when daf is tested because it is not used
      continue;
    }
    GFDetPlane detPlaneOfRecoHit = aGFAbsRecoHitPtr->getDetPlane(aTrackPtr->getTrackRep(trackRepId));



    TMatrixD m;
    TMatrixD V;
    //cerr << "bin hier vor get measurement\n";
    aGFAbsRecoHitPtr->getMeasurement(rep, detPlaneOfRecoHit, rep->getState(), rep->getCov(), m, V);
    //cerr << "und jetzt danach\n";
    m_trackData.Hs.push_back(aGFAbsRecoHitPtr->getHMatrix(aTrackPtr->getTrackRep(0)));
    m_trackData.ms.push_back(m);
    m_trackData.Vs.push_back(V);


    aTrackPtr->getBK(trackRepId)->getMatrix("fUpSt", iGFHit, state);
    aTrackPtr->getBK(trackRepId)->getMatrix("fUpCov", iGFHit, cov);
    m_trackData.states_fu.push_back(state);
    m_trackData.covs_fu.push_back(cov);
    aTrackPtr->getBK(trackRepId)->getMatrix("bUpSt", iGFHit, state);
    aTrackPtr->getBK(trackRepId)->getMatrix("bUpCov", iGFHit, cov);
    m_trackData.states_bu.push_back(state);
    m_trackData.covs_bu.push_back(cov);
    //cerr << "gebias\n";
    GFTools::getBiasedSmoothedData(aTrackPtr, trackRepId, iGFHit, state, cov);
    //cerr << "und jetzt danach\n";
    m_trackData.states_sm.push_back(state);
    m_trackData.covs_sm.push_back(cov);

    if (m_testPrediction == true) {
      aTrackPtr->getBK(trackRepId)->getMatrix("fPreSt", iGFHit, state);
      aTrackPtr->getBK(trackRepId)->getMatrix("fPreCov", iGFHit, cov);
      m_trackData.states_fp.push_back(state);
      m_trackData.covs_fp.push_back(cov);
      aTrackPtr->getBK(trackRepId)->getMatrix("bPreSt", iGFHit, state);
      aTrackPtr->getBK(trackRepId)->getMatrix("bPreCov", iGFHit, cov);
      m_trackData.states_bp.push_back(state);
      m_trackData.covs_bp.push_back(cov);
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

  for (int iGFHit = 0; iGFHit not_eq m_trackData.nHits; ++iGFHit) {
    int detId = m_trackData.detIds[iGFHit];
    if (detId == 0 or detId == 1) {  //at the moment there is only truth info for PXD and SVD hits
      int accuVecIndex = m_trackData.accuVecIndices[iGFHit];

      TMatrixT<double> trueState = m_trackData.states_t[iGFHit];
      if (trueState.GetNrows() == 0) { // this should not happen!!! If it does something went wrong. As a workaround just skip the hit (better the crashing I guess)
        continue;
      }
      //      cout << "true, m, H, V" << endl;
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
      fillLayerWiseData("res_meas_t", accuVecIndex, rootMatrixToStdVec(res));
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
    int detId = m_trackData.detIds[iGFHit];
    int accuVecIndex = m_trackData.accuVecIndices[iGFHit];
    if (detId == 0 or detId == 1) {
      if (m_testSi == false) {
        continue;
      }
    } else if (detId == 2) {
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
  int nHits = aTrackPtr->getNumHits();
  double dafWeight = -2.0;
  double dafChi2 = -2.0;

  vector<vector<float> > allWeights(m_nLayers, vector<float>(5, -1.0)); //it is importent that the content is initalized to -1
  vector<vector<float> > allChi2s(m_nLayers, vector<float>(5, -1.0));
  for (int iGFHit = 0; iGFHit not_eq nHits; ++iGFHit) {
    int accuVecIndex = m_trackData.accuVecIndices[iGFHit];

    aTrackPtr->getBK(0)->getNumber("dafWeight", iGFHit,  dafWeight); // double check if this still works with background => more then one hit... probarbly it does work...
    //aTrackPtr->getBK(0)->getNumber("dafChi2s", iGFHit,  dafChi2);

    GFAbsRecoHit* aGFAbsRecoHitPtr = aTrackPtr->getHit(iGFHit);
    PXDRecoHit const*  aPxdRecoHitPtr = dynamic_cast<PXDRecoHit const* >(aGFAbsRecoHitPtr);
    SVDRecoHit2D const*  aSvdRecoHit2DPtr =  dynamic_cast<SVDRecoHit2D  const* >(aGFAbsRecoHitPtr);
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
    } else { //Probably a CDC hit -- not supported yet

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



//void TrackFitCheckerModule::trunctatedMeanAndStd(std::vector<double> data, bool symmetric, double& mean, double& std){
//  const int n = data.size();
//  double ratio = 0.001; // relative amount of data that gets truncated away
//
//  if (symmetric == true){ // cut will be applied to both sides of the sorted smaple
//    int cut = int(ratio*n/2.0 + 0.5);
//    int truncN = n - cut -cut;
//    sort(data.begin(), data.end());
//    double sum = std::accumulate(data.begin()+cut, data.end()-cut, 0.0);
//    mean = sum / truncN;
//    std = 0;
//    for ( int i = cut; i not_eq n-cut; ++i){
//      double diff = data[i] - mean;
//      std += diff*diff;
//    }
//    std = 1.0/(truncN-1) * std;
//
//  } else { // cut will only be applied to the side with the larger values of the sorted sample
//    int cut = int(ratio*n + 0.5);
//    int truncN = n -cut;
//    sort(data.begin(), data.end());
//    double sum = std::accumulate(data.begin(), data.end()-cut, 0.0);
//    mean = sum / truncN;
//    std = 0;
//    for ( int i = 0; i not_eq n-cut; ++i){
//      double diff = data[i] - mean;
//      std += diff*diff;
//    }
//    std = 1.0/(truncN-1) * std;
//
//  }
//}

int TrackFitCheckerModule::countOutliers(const vector<double>& dataSample, const double mean, const double sigma, const double widthScaling)
{

  int n = dataSample.size();
  int nOutliers = 0;
  double halfInterval = widthScaling * sigma;
  assert(halfInterval > 0.0); //both widthScaling and sigma must be positive therefore halfInterval, too!
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


vector<double> TrackFitCheckerModule::rootMatrixToStdVec(const TMatrixT<double>&  rootMatrix) const
{
  int n = rootMatrix.GetNrows();
  vector<double> stdVec(n);
  for (int i = 0; i not_eq n; ++i) {
    stdVec[i] = rootMatrix[i][0];
  }
  return stdVec;
}
