/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors:                                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/trackFitChecker/trackFitCheckerModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreArray.h>
#include <generators/dataobjects/MCParticle.h>
#include <pxd/dataobjects/PXDTrueHit.h>
#include <svd/dataobjects/SVDTrueHit.h>
#include <pxd/dataobjects/PXDRecoHit.h>
#include <svd/dataobjects/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <GFTrack.h>
#include <GFTools.h>
#include <RKTrackRep.h>

#include <TMatrixDEigen.h>


#include <limits>
//for root output

using namespace std;
using namespace Belle2;
using namespace boost::accumulators;
//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(trackFitChecker)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

trackFitCheckerModule::trackFitCheckerModule() : Module()
{
  //Set module properties
  setDescription("This module tacks GFTracks as input an calculates different statistical tests some of them using the simulated truth information");

  //Parameter definition
  addParam("outputFileName", m_dataOutFileName, "Output file name", string("forwardData.txt"));
  //addParam("outputFileName2", m_dataOutFileName2, "Output file name2", string("tfcpvalues.txt"));
  addParam("totalChi2Cut", m_totalChi2Cut, "only tracks with a total χ² lower than this value will be considered", 1E300);
  addParam("testSi", m_testSi, "execute the layer wise tests for PXD/SVD", false);
  addParam("testCdc", m_testCdc, "execute the layer wise tests for CDC", false);
  //addParam("useTruthInfo", m_useTruthInfo, "use the truth info from the geant4 simulation", false);
  addParam("testPrediction", m_testPrediction, "Additionally test the predicted state vecs from the Kalman filter. ONLY WOKRKS IF THEY ARE SAVED DURING FITTING WHICH IS NOT THE DEFAULT", false);
  //addParam("writeToB2info", m_writeToB2info, "Set to True if you want the results of the statistical tests written out with the B2INFO command", true);
  addParam("writeToRootFile", m_writeToRootFile, "Set to True if you want the data from the statistical tests written into a root file", false);
  addParam("writeToTextFile", m_writeToFile, "Set to True if you want the results of the statistical tests written out in a normal text file", false);

}


trackFitCheckerModule::~trackFitCheckerModule()
{

}

void trackFitCheckerModule::initialize()
{
  //set all user parameters

  //configure the output
  m_testOutputFileName = "statisticaltests.txt";
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
  const int vecSizeTruthTest = m_layerWiseTruthTestsVarNames.size();

  m_vertexTestsVarNames.push_back("x");
  m_vertexTestsVarNames.push_back("y");
  m_vertexTestsVarNames.push_back("z");
  m_vertexTestsVarNames.push_back("p_x");
  m_vertexTestsVarNames.push_back("p_y");
  m_vertexTestsVarNames.push_back("p_z");
  const int vecDataSize = m_vertexTestsVarNames.size();

  if (m_writeToRootFile == true) {
    m_rootFilePtr = new TFile("statisticaltestsData.root", "RECREATE");
    m_statDataTreePtr = new TTree("m_statDataTreePtr", "aTree");
    //init objects to store track wise data
    m_trackWiseDataForRoot["pValue_bu"] = new float(0);
    m_trackWiseDataForRoot["pValue_fu"] = new float(0);
    m_trackWiseDataForRoot["absMomVertex"] = new float(0);
    m_statDataTreePtr->Bronch("pValue_bu", "float", &(m_trackWiseDataForRoot["pValue_bu"]));
    m_statDataTreePtr->Bronch("pValue_fu", "float", &(m_trackWiseDataForRoot["pValue_fu"]));
    m_statDataTreePtr->Bronch("absMomVertex", "float", &(m_trackWiseDataForRoot["absMomVertex"]));
    //and objects for track wise vec data
//    m_trackWiseVecDataForRoot["zs_vertexPosMom"] = new Belle2::TrackWiseVecDataStruct(vecDataSize);
//    m_trackWiseVecDataForRoot["res_vertexPosMom"] = new Belle2::TrackWiseVecDataStruct(vecDataSize);
//    m_statDataTreePtr->Bronch("zs_vertexPosMom","Belle2::TrackWiseVecDataStruct",&(m_trackWiseDataForRoot["zs_vertexPosMom"]));
//    m_statDataTreePtr->Bronch("res_vertexPosMom","Belle2::TrackWiseVecDataStruct",&(m_trackWiseDataForRoot["res_vertexPosMom"]));
    m_trackWiseVecDataForRoot["zs_vertexPosMom"] = new std::vector<float>(vecDataSize);
    m_trackWiseVecDataForRoot["res_vertexPosMom"] = new std::vector<float>(vecDataSize);
    m_statDataTreePtr->Bronch("zs_vertexPosMom", "std::vector<float>", &(m_trackWiseVecDataForRoot["zs_vertexPosMom"]));
    m_statDataTreePtr->Bronch("res_vertexPosMom", "std::vector<float>", &(m_trackWiseVecDataForRoot["res_vertexPosMom"]));

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
    m_nPxdLayers = 2;//should come from the xml file
    m_nSvdLayers = 4;
  } else {
    m_nPxdLayers = 0;
    m_nSvdLayers = 0;
  }
  m_nSiLayers = m_nPxdLayers + m_nSvdLayers;
  m_nLayers = m_nPxdLayers + m_nSvdLayers + m_nCdcLayers;
  // m_dataOut.open(m_dataOutFileName.c_str());
  // m_dataOut.precision(14);
  // m_dataOut << "event#\tproc#\tfchi2tot\tabsMom\tq/p_t 1\tdu/dw_t\tdv/dw_t\tu_t\tv_t\tm_u\tm_v\tq/p\tdu/dw\tdv/dw\tu\tv\tsigma_u\tsigma_v\tcov(uv)\tfpChi2inc\tq/p_t 2\tdu/dw_t\tdv/dw_t\tu_t\tv_t\tm_u\tm_v\tq/p\tdu/dw\tdv/dw\tu\tv\tsigma_u\tsigma_v\tcov(uv)\tfpChi2inc\tq/p_t 3\tdu/dw_t\tdv/dw_t\tu_t\tv_t\tm_u\tm_v\tq/p\tdu/dw\tdv/dw\tu\tv\tsigma_u\tsigma_v\tcov(uv)\tfpChi2inc\tq/p_t 4\tdu/dw_t\tdv/dw_t\tu_t\tv_t\tm_u\tm_v\tq/p\tdu/dw\tdv/dw\tu\tv\tsigma_u\tsigma_v\tcov(uv)\tfpChi2inc\tq/p_t 5\tdu/dw_t\tdv/dw_t\tu_t\tv_t\tm_u\tm_v\tq/p\tdu/dw\tdv/dw\tu\tv\tsigma_u\tsigma_v\tcov(uv)\tfpChi2inc\tq/p_t 6\tdu/dw_t\tdv/dw_t\tu_t\tv_t\tm_u\tm_v\tq/p\tdu/dw\tdv/dw\tu\tv\tsigma_u\tsigma_v\tsigma_uv\tfpChi2inc\tfpChi2tot\n";

  //make all vector of vectors have the size of the number of current layers in use
  int vecSizeMeasTest = 3;
  if (m_testCdc == true and m_testSi == false) {
    vecSizeMeasTest = 2; //it is easier to just caluclate the chi^2 to although it does not have more info that the standart score in this calse
  }
  //int measDim = 2;



  // pulls (z) of cartesian coordinates of innermost hit and vertex
  m_trackWiseVecDataSamples["zs_vertexPosMom"].resize(6);
  // residuals of Cartesian coordinates of innermost hit and vertex
  m_trackWiseVecDataSamples["res_vertexPosMom"].resize(6);
  // pulls (z) and chi2s for the 5 track parameters in every layer using truth info
  if (m_testPrediction) resizeLayerWiseData("zs_and_chi2_fp_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_fu_t", vecSizeTruthTest);
  if (m_testPrediction) resizeLayerWiseData("zs_and_chi2_bp_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_bu_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_sm_t", vecSizeTruthTest);
  // pulls (z) and chi2s for the 5 track parameters in every layer using the projection onto the measurement m-Hx
  if (m_testPrediction) resizeLayerWiseData("zs_and_chi2_fp", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_fu", vecSizeMeasTest);
  if (m_testPrediction) resizeLayerWiseData("zs_and_chi2_bp", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_bu", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_sm", vecSizeMeasTest);

  // pulls and chi2 to test consistency of normal distribution model of measurements and the sigma of the digitizer with the sigma of the recoHits
  resizeLayerWiseData("zs_and_chi2_meas_t", vecSizeMeasTest);

  m_badR_fCounter = 0;
  m_badR_bCounter = 0;
  m_badR_smCounter = 0;
  m_processedTracks = 0;
  m_nCutawayTracks = 0;
  m_notPosDefCounter = 0;
  m_unSymmetricCounter = 0;
  m_failedSmootherCounter = 0;
}


void trackFitCheckerModule::beginRun()
{

}



void trackFitCheckerModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  int eventCounter = eventMetaDataPtr->getEvent();

  //simulated truth information
  StoreArray<MCParticle> aMcParticleArray("");
  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  //StoreArray<CDCTrueSimHit> aCdcTrueHitArray(""); //maybe one day this will be there :-)

  //genfit stuff
  StoreArray<GFTrackCand> trackCandidates(""); // to create a new track rep for extrapolation only
  StoreArray<GFTrack> fittedTracks(""); // the results of the track fit
  const int nFittedTracks = fittedTracks.getEntries();

  for (int i = 0; i not_eq nFittedTracks; ++i) {
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
    // first part: get variable disribing the hole track
    const double chi2tot_fu = aTrackPtr->getForwardChiSqu();
    // m_dataOut << eventCounter << "\t" << m_processedTracks << "\t" << chi2tot_fu;
    const int ndf = aTrackPtr->getNDF();
    const double pValue_bu = TMath::Prob(chi2tot_bu, ndf); // actually the p value would be 1-TMath::Prob(chi2tot, ndf) but particle physicists want to have it this way.
    const double pValue_fu = TMath::Prob(chi2tot_fu, ndf);
    fillTrackWiseData("pValue_bu", pValue_bu);
    fillTrackWiseData("pValue_fu", pValue_fu);
    TVector3 vertexPos;
    TVector3 vertexMom;
    TMatrixT<double> vertexCov(6, 6);
    vector<double> zVertexPosMom(6);
    vector<double> resVertexPosMom(6);
    TVector3 poca; //point of closest approach will be overwritten
    TVector3 dirInPoca; //direction of the track at the point of closest approach will be overwritten
    aTrackPtr->getCardinalRep()->extrapolateToPoint(trueVertexPos, poca, dirInPoca); //goto vertex position
    GFDetPlane planeThroughVertex(poca, dirInPoca); //get planeThroughVertex through fitted vertex position
    double vertexAbsMom = aTrackPtr->getMom(planeThroughVertex).Mag(); //get fitted momentum at fitted vertex
    fillTrackWiseData("absMomVertex", vertexAbsMom);
    // m_dataOut << "\t" << vertexAbsMom;
    aTrackPtr->getPosMomCov(planeThroughVertex, vertexPos, vertexMom, vertexCov);
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
    fillTrackWiseVecData("zs_vertexPosMom", zVertexPosMom);

    if (m_testSi == true or m_testCdc == true) {
      //now the layer wise tests
      TMatrixT<double> state;
      TMatrixT<double> cov;
      TMatrixT<double> propMat(5, 5);
      TMatrixT<double> trueState(5, 1);
      TMatrixT<double> onlyPropState(5, 1);
      TMatrixT<double> res;
      TMatrixT<double> R;
      TVector3 posInTrue;
      TVector3 posOutTrue;
      TVector3 pTrue;
      //vector<double> zs;
      vector<double> truthTests;
      int hitLayerId = -1;
      double uTrue = 0.0;
      double vTrue = 0.0;
      vector<double> testResutlsWithoutTruth;
      double fpChi2tot = 0.0;

      int nHits = aTrackPtr->getNumHits();
      for (int iGFHit = 0; iGFHit not_eq nHits; ++iGFHit) {
        //first determine the hit type then get the data from the hit
        GFAbsRecoHit* const aGFAbsRecoHitPtr = aTrackPtr->getHit(iGFHit);
        PXDRecoHit const* const aPxdRecoHitPtr = dynamic_cast<PXDRecoHit const * const>(aGFAbsRecoHitPtr);
        SVDRecoHit2D const* const aSvdRecoHitPtr =  dynamic_cast<SVDRecoHit2D const * const>(aGFAbsRecoHitPtr);
        CDCRecoHit* const aCdcRecoHitPtr = dynamic_cast<CDCRecoHit * const>(aGFAbsRecoHitPtr); // cannot use the additional const here because the getter fuctions inside the CDCRecoHit class are not decleared as const (although they could be const)
        int accuVecIndex; //this is an index to sort the info from one layer in the corresponding statistics container
        bool truthAvailable = true; //flag that a hit can set if there is no easy accessible truth info only a hack late there should also be truth info for the CDC
        if (aPxdRecoHitPtr not_eq NULL) {
          if (m_testSi == false) { // if the it is a pxd/svd hit but the user does not want to test pxd/svd hits skip this hit
            continue;
          }
          PXDTrueHit const* const aTrueHitPtr = aPxdRecoHitPtr->getTrueHit();
          hitLayerId = aTrueHitPtr->getSensorID().getLayer();
          pTrue = aTrueHitPtr->getMomentum();
          uTrue = aTrueHitPtr->getU();
          vTrue = aTrueHitPtr->getV();
          res.ResizeTo(2, 1);
          R.ResizeTo(2, 2);
          accuVecIndex = hitLayerId - 1;
        } else if (aSvdRecoHitPtr not_eq NULL) {
          if (m_testSi == false) { // if the it is a pxd/svd hit but the user does not want to test pxd/svd hits skip this hit
            continue;
          }
          SVDTrueHit const* const aTrueHitPtr = aSvdRecoHitPtr->getTrueHit();
          hitLayerId = aTrueHitPtr->getSensorID().getLayer();
          pTrue = aTrueHitPtr->getMomentum();
          uTrue = aTrueHitPtr->getU();
          vTrue = aTrueHitPtr->getV();
          res.ResizeTo(2, 1);
          R.ResizeTo(2, 2);
          accuVecIndex = hitLayerId - 1;
        } else if (aCdcRecoHitPtr not_eq NULL) {
          if (m_testCdc == false) { // if the it is a cdc hit but the user does not want to test cdc hits skip this hit
            continue;
          }
          hitLayerId = aCdcRecoHitPtr->getLayerId();
          accuVecIndex = hitLayerId + m_nPxdLayers + m_nSvdLayers;
          res.ResizeTo(1, 1);
          R.ResizeTo(1, 1);
          truthAvailable = false;
        } else {
          B2ERROR("An unknown type of recoHit was detected in trackFitCheckerModule::event(). This hit will not be included in the statistical tests");
          continue;
        }
        GFDetPlane detPlaneOfRecoHit = aGFAbsRecoHitPtr->getDetPlane(aTrackPtr->getTrackRep(0));
        TMatrixT<double> H = aGFAbsRecoHitPtr->getHMatrix(aTrackPtr->getTrackRep(0));
        TMatrixT<double> HT(TMatrixT<double>::kTransposed, H); // the transposed is needed later
        TMatrixT<double> m = aGFAbsRecoHitPtr->getHitCoord(detPlaneOfRecoHit); //measurement of hit
        TMatrixT<double> V = aGFAbsRecoHitPtr->getHitCov(detPlaneOfRecoHit); //covariance matrix of hit

        // build the true state vector x_t all in local coordinates
        if (truthAvailable == true) {
          trueState[0][0] = charge / pTrue.Mag(); // q/p
          trueState[1][0] = pTrue[0] / pTrue[2]; //dudw
          trueState[2][0] = pTrue[1] / pTrue[2];//dvdw
          trueState[3][0] = uTrue; // u
          trueState[4][0] = vTrue; // v
          res = m - H * trueState;
          truthTests = calcZs(res, V);
          truthTests.push_back(calcChi2(res, V));
          fillLayerWiseData("zs_and_chi2_meas_t", accuVecIndex, truthTests);
          // m_dataOut << "\t" << trueState[0][0] << "\t" << trueState[1][0] << "\t" << trueState[2][0] << "\t" << trueState[3][0] << "\t" << trueState[4][0] << "\t" << m[0][0] << "\t" << m[1][0]; // Achtung bei der CDC ist m nur 1D und m[1][0] wird dort eine seg fault erzeugen!!! Mach das besser!!!

        }
        //cout << "true state\n";trueState.Print();
        // now test if measurements am their theoretical variances that were feeded to genfit are consistent with the acutal distribution of measurments


        GFTools::getBiasedSmoothedData(aTrackPtr, 0, iGFHit, state, cov);
        res = m - H * state;
        R = V - H * cov * HT;
        if (hasMatrixNegDiagElement(R) == true) {
          ++m_badR_smCounter;
        } else {
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(calcChi2(res, R));
          fillLayerWiseData("zs_and_chi2_sm", accuVecIndex, testResutlsWithoutTruth);
        }
        // now calculate test qunatites with smoothed state vec and cov and the true state vec
        if (truthAvailable == true) {
          fillLayerWiseData("zs_and_chi2_sm_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));
        }

        //            // standard scores (pulls) calculated from residuals using the measurements and the predicted forward state
        if (m_testPrediction == true) {
          aTrackPtr->getBK(0)->getMatrix("fPreSt", iGFHit, state);
          aTrackPtr->getBK(0)->getMatrix("fPreCov", iGFHit, cov);//
          //            isMatrixCov(cov);// test mathematical properties of cov matrix
          res = m - H * state;
          R = V + H * cov * HT;
          testResutlsWithoutTruth = calcZs(res, R);
          double fpChi2increment = calcChi2(res, R);
          testResutlsWithoutTruth.push_back(fpChi2increment);
          fillLayerWiseData("zs_and_chi2_fp", accuVecIndex, testResutlsWithoutTruth);
          fpChi2tot += fpChi2increment;

          // m_dataOut << "\t" << state[0][0] << "\t" << state[1][0] << "\t" << state[2][0] << "\t" << state[3][0] << "\t" << state[4][0] << "\t" << sqrt(cov[3][3]) << "\t" << sqrt(cov[4][4]) << "\t" << cov[3][4] << "\t" << fpChi2increment;

          //get the propagation matrix
          //aTrackPtr->getBK(0)->getMatrix("fProp",iGFHit,propMat);
          //propMat.Print();

          // test using predicted forward state and truth information
          if (truthAvailable == true) {
            fillLayerWiseData("zs_and_chi2_fp_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));
          }
          //            //test the predicted state that I set myself
          //            /*TMatrixT<double> altTrueState(5,1);
          //            // get the true state
          //            aTrackPtr->getBK(0)->getMatrix("fPreStTrue",iGFHit,altTrueState);
          //            if (iGFHit == 0){
          //              truthTests = calcTestsWithTruthInfo(state,cov,altTrueState);
          //              m_z_pf_qOverPsLayer1(truthTests[0]);
          //              m_z_pf_dudwsLayer1(truthTests[1]);
          //              m_z_pf_dvdwsLayer1(truthTests[2]);
          //              m_z_pf_usLayer1(truthTests[3]);
          //              m_z_pf_vsLayer1(truthTests[4]);
          //              m_pf_chi2sLayer1(truthTests[5]);
          //            }*/
          //
          //            // test the difference of the geant4 and genfit propagation
          //            propOnlyTrRepPtr->extrapolate(aTrackPtr->getHit(iGFHit)->getDetPlane(aTrackPtr->getTrackRep(0)), onlyPropState);
          //            //aTrackPtr->getTrackRep(0)->extrapolate(aTrackPtr->getHit(iGFHit)->getDetPlane(aTrackPtr->getTrackRep(0)), onlyPropState);
          //            TMatrixT<double> resProps = onlyPropState - trueState;
          //            /*int nDigits = 14;
          //            cout << "showing the 2 different truth vectors first simHit then genfit propagation and the predicted and fitted forward state hitid " << iSiHit << iGFHit <<"\n";
          //            cout <<setprecision(nDigits)<< trueState[0][0] << " "<< trueState[1][0] << " "<< trueState[2][0] << " "<< trueState[3][0] << " "<< trueState[4][0] << "\n"; //true state
          //            cout <<setprecision(nDigits)<< onlyPropState[0][0] << " "<< onlyPropState[1][0] << " "<< onlyPropState[2][0] << " "<< onlyPropState[3][0] << " "<< onlyPropState[4][0] << "\n"; //predicted without any update
          //            cout <<setprecision(nDigits)<< state[0][0] << " "<< state[1][0] << " "<< state[2][0] << " "<< state[3][0] << " "<< state[4][0] << "\n"; //prdicted state
          //             // predicted with all previous updated
          //            /*cout <<setprecision(nDigits)<< fUpState[0][0] << " "<< fUpState[1][0] << " "<< fUpState[2][0] << " "<< fUpState[3][0] << " "<< fUpState[4][0] << "\n"; //updated
          //             */
          //
        }
        // standard scores (pulls) calculated from residuals using the measurements and the updated forward state
        //TMatrixT<double> fUpState(5,1);
        aTrackPtr->getBK(0)->getMatrix("fUpSt", iGFHit, state);
        aTrackPtr->getBK(0)->getMatrix("fUpCov", iGFHit, cov);
        isMatrixCov(cov);
        res = m - H * state;
        R = V - H * cov * HT;
        double fuChi2Inrement = 0;
        if (hasMatrixNegDiagElement(R) == true) {
          ++m_badR_fCounter;
        } else {
          fuChi2Inrement = calcChi2(res, R);
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(fuChi2Inrement);
          fillLayerWiseData("zs_and_chi2_fu", accuVecIndex, testResutlsWithoutTruth);
        }

        //// m_dataOut << "\t" << fuChi2Inrement;
        // test using updated forward state and truth information
        if (truthAvailable == true) {
          fillLayerWiseData("zs_and_chi2_fu_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));
        }
        // standard scores (pulls) calculated from residuals using the measurements and the predicted backward state
        if (m_testPrediction == true) {
          aTrackPtr->getBK(0)->getMatrix("bPreSt", iGFHit, state);
          aTrackPtr->getBK(0)->getMatrix("bPreCov", iGFHit, cov);
          isMatrixCov(cov);
          res = m - H * state;
          R = V + H * cov * HT;
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(calcChi2(res, R));
          fillLayerWiseData("zs_and_chi2_bp", accuVecIndex, testResutlsWithoutTruth);
          // test using updated backward state and truth information
          if (truthAvailable == true) {
            fillLayerWiseData("zs_and_chi2_bp_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));
          }
        }
        // standard scores (pulls) calculated from residuals using the measurements and the updated backward state
        aTrackPtr->getBK(0)->getMatrix("bUpSt", iGFHit, state);
        aTrackPtr->getBK(0)->getMatrix("bUpCov", iGFHit, cov);
        isMatrixCov(cov);
        res = m - H * state;
        R = V - H * cov * HT;
        if (hasMatrixNegDiagElement(R) == true) {
          ++m_badR_bCounter;
        } else {
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(calcChi2(res, R));
          fillLayerWiseData("zs_and_chi2_bu", accuVecIndex, testResutlsWithoutTruth);
        }

        // test using updated backward state and truth information
        if (truthAvailable == true) {
          fillLayerWiseData("zs_and_chi2_bu_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));
        }
      }
      // m_dataOut << "\t" << fpChi2tot;
      // m_dataOut << "\n";
    }
    m_statDataTreePtr->Fill();
    ++m_processedTracks;


  }
}

void trackFitCheckerModule::endRun()
{
  B2INFO("Now following the endRun Output from the trackFitChecker module");

  //  if (m_failedSmootherCounter not_eq 0){
  //    B2WARNING("Smoothed states could not be extracted " << m_failedSmootherCounter << " times";)
  //  }
  if (m_nCutawayTracks not_eq 0) {
    B2WARNING(m_nCutawayTracks << " tracks where cut out because of too large total χ²");
  }
  if (m_badR_fCounter not_eq 0 or m_badR_bCounter not_eq 0 or m_badR_smCounter not_eq 0) {
    B2WARNING("There were tracks hits with negative diagonal elements in the covariance matrix of the residuals. Occurrence forward: " << m_badR_fCounter << " backward: " << m_badR_bCounter << " smoother: " << m_badR_smCounter);
  }
  if (m_unSymmetricCounter not_eq 0) {
    B2WARNING(m_unSymmetricCounter << " covs where not symmetric ");
  }
  if (m_notPosDefCounter not_eq 0) {
    B2WARNING(m_notPosDefCounter << " covs had eigenvalues <= 0 ");
  }
  if (m_processedTracks <= 1) {
    B2WARNING("Only one or less events were processed. Statistics cannot be computed.");
  } else {
    printTrackWiseStatistics("pValue_fu");
    printTrackWiseStatistics("pValue_bu");
    printTrackWiseStatistics("absMomVertex");
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
    printTrackWiseVecStatistics("zs_vertexPosMom", m_vertexTestsVarNames);
    //looks a bit clumy with all the if (m_testSi == true) but the hope is there will easy accessable truth info for CDCHits so a better solution is not needed because the if (m_testSi == true) are temporary anyway
    if (m_nLayers > 0) {
      if (m_testSi == true) {
        printLayerWiseStatistics("zs_and_chi2_meas_t", measVarNames);
      }
      if (m_testPrediction == true) {
        if (m_testSi == true) {
          printLayerWiseStatistics("zs_and_chi2_fp_t", m_layerWiseTruthTestsVarNames);
        }
        printLayerWiseStatistics("zs_and_chi2_fp", measVarNames);
      }
      if (m_testSi == true) {
        printLayerWiseStatistics("zs_and_chi2_fu_t", m_layerWiseTruthTestsVarNames);
      }
      printLayerWiseStatistics("zs_and_chi2_fu", measVarNames);
      if (m_testPrediction == true) {
        if (m_testSi == true) {
          printLayerWiseStatistics("zs_and_chi2_bp_t", m_layerWiseTruthTestsVarNames);
        }
        printLayerWiseStatistics("zs_and_chi2_bp", measVarNames);
      }
      if (m_testSi == true) {
        printLayerWiseStatistics("zs_and_chi2_bu_t", m_layerWiseTruthTestsVarNames);
      }
      printLayerWiseStatistics("zs_and_chi2_bu", measVarNames);
      if (m_testSi == true) {
        printLayerWiseStatistics("zs_and_chi2_sm_t", m_layerWiseTruthTestsVarNames);
      }
      printLayerWiseStatistics("zs_and_chi2_sm", measVarNames);
    }
    //write out the test results
    B2INFO("\n" << m_textOutput.str());
    if (m_writeToFile == true) {
      ofstream testOutputToFile(m_testOutputFileName.c_str());
      testOutputToFile << m_textOutput.str();
      testOutputToFile.close();
    }
    //
    //
    //
    //      B2INFO("Now testing the predicted forward track parameters with the state I set")
    //      B2INFO("\t\tq/p,\tdudw,\tdvdw,\tu,\tv,\tχ²");
    //      {
    //        double mean_z_qOverP = mean(m_z_pf_qOverPsLayer1);
    //        double std_z_qOverP = sqrt(variance(m_z_pf_qOverPsLayer1));
    //        double mean_z_dudw = mean(m_z_pf_dudwsLayer1);
    //        double std_z_dudw = sqrt(variance(m_z_pf_dudwsLayer1));
    //        double mean_z_dvdw = mean(m_z_pf_dvdwsLayer1);
    //        double std_z_dvdw = sqrt(variance(m_z_pf_dvdwsLayer1));
    //        double mean_z_u = mean(m_z_pf_usLayer1);
    //        double std_z_u = sqrt(variance(m_z_pf_usLayer1));
    //        double mean_z_v = mean(m_z_pf_vsLayer1);
    //        double std_z_v = sqrt(variance(m_z_pf_vsLayer1));
    //        double mean_chi2 = mean(m_pf_chi2sLayer1);
    //        double std_chi2 = sqrt(variance(m_pf_chi2sLayer1));
    //        B2INFO("mean\t" << fixed<<setprecision(nDigits) << mean_z_qOverP << "\t" << mean_z_dudw << "\t"<< mean_z_dvdw << "\t"<< mean_z_u<<"\t" << mean_z_v<<"\t" <<  mean_chi2);
    //        B2INFO("std\t"<< fixed<<setprecision(nDigits) << std_z_qOverP<< "\t" << std_z_dudw << "\t"<< std_z_dvdw <<"\t" << std_z_u<<"\t" << std_z_v <<"\t" << std_chi2);
    //      }
    //    }
  }

}




void trackFitCheckerModule::terminate()
{
  // m_dataOut.close();
  if (m_statDataTreePtr not_eq NULL) {
    m_statDataTreePtr->Write();
    //m_statDataTreePtr->Scan();
//    m_statDataTreePtr->GetBranch("zs_vertexPosMom")->Dump();
//    //m_statDataTreePtr->Draw("zs_vertexPosMom[0]");
//    m_statDataTreePtr->Draw("pValue_bu.data");
//      std::cout << "Press ENTER to continue...";
//      std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );
    m_rootFilePtr->Close();
    //let there be delete for every new (ashes to ashes, dust to dust)
    delete m_statDataTreePtr;
    std::map<std::string, LayerWiseData* >::iterator iter = m_layerWiseDataForRoot.begin();
    std::map<std::string, LayerWiseData* >::const_iterator iterMax = m_layerWiseDataForRoot.end();
    while (iter not_eq iterMax) {
      delete(iter->second);
      ++iter;
    }

  }

}

// calculate a chi2 value from a residuum and it's covariance matrix R
double trackFitCheckerModule::calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R)
{
  TMatrixT<double> invR;
  GFTools::invertMatrix(R, invR);
  TMatrixT<double> resT(TMatrixT<double>::kTransposed, res);
  return (resT * invR * res)[0][0];
}
// calculate a chi2 value from a residuum and it's covariance matrix R
vector<double> trackFitCheckerModule::calcZs(const TMatrixT<double>& res, const TMatrixT<double>& R)
{
  const int numOfZ = R.GetNcols();
  vector<double> resultVec(numOfZ);
  for (int i = 0; i not_eq numOfZ; ++i) {
    resultVec[i] = res[i][0] / sqrt(R[i][i]);
  }
  return resultVec;
}

vector<double> trackFitCheckerModule::calcTestsWithTruthInfo(const TMatrixT<double>& state, const TMatrixT<double>& cov, const TMatrixT<double>& trueState)
{
  TMatrixT<double> res = state - trueState;
  vector<double> resultVec = calcZs(res, cov);

  resultVec.push_back(calcChi2(res, cov));

  TMatrixT<double> resXY(2, 1);
  TMatrixT<double> covXY(2, 2);

  resXY[0][0] = res[1][0];
  resXY[1][0] = res[2][0];
  covXY = cov.GetSub(1, 2, 1, 2);
  resultVec.push_back(calcChi2(resXY, covXY));

  resXY[0][0] = res[3][0];
  resXY[1][0] = res[4][0];
  covXY = cov.GetSub(3, 4, 3, 4);

  resultVec.push_back(calcChi2(resXY, covXY));

  return resultVec;
}

bool trackFitCheckerModule::hasMatrixNegDiagElement(const TMatrixT<double>& aMatrix)
{
  int n = aMatrix.GetNrows(); //matrix must be quadratic
  for (int i = 0; i not_eq n; ++i) {
    if (aMatrix[i][i] < 0.0) {
      return true;
    }
  }
  return false;
}

void trackFitCheckerModule::isMatrixCov(const TMatrixT<double>& cov)
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

bool trackFitCheckerModule::isSymmetric(const TMatrixT<double>& aMatrix)
{
  int n = aMatrix.GetNrows();
  int m = aMatrix.GetNcols();
  if (n not_eq m) {
    return false;
  }
  const double epsilon = 1E-15;
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

void trackFitCheckerModule::printLayerWiseStatistics(const string& nameOfDataSample, const vector<string>& layerWiseVarNames)
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
    aStrStr << "mean\tstd\t";
  }
  aStrStr << "\n"; //delete the last tab from the stream and add a newline
  m_textOutput << "Information on " << nameOfDataSample << " for all layers\npara\\l\t" << aStrStr.str();
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << layerWiseVarNames[i];
    for (int l = 0; l not_eq nOfLayers; ++l) {
      double tempMean = mean(dataSample[l][i]);
      double tempStd = sqrt(variance(dataSample[l][i]));
      //cout << "layer: " << l+1 << " para: " << i+1 << " count: " << boost::accumulators::count(dataSample[l][i]) << "\n";
      m_textOutput << fixed << "\t" << tempMean << "\t" << tempStd;
    }
    m_textOutput << "\n";
  }
}

void trackFitCheckerModule::printTrackWiseStatistics(const string& nameOfDataSample)
{
  StatisticsContainer&  dataSample = m_trackWiseDataSamples[nameOfDataSample];

  m_textOutput << "Information on " << nameOfDataSample << "\nmean\tstd\n";
  m_textOutput << fixed << mean(dataSample) << "\t" << sqrt(variance(dataSample)) << "\n";
}

void trackFitCheckerModule::printTrackWiseVecStatistics(const string& nameOfDataSample, const vector<string>& varNames)
{
  vector<StatisticsContainer>& dataSample = m_trackWiseVecDataSamples[nameOfDataSample];

  const int nOfVars = dataSample.size();
  m_textOutput << "Information on " << nameOfDataSample << "\n\tmean\tstd\n";
  for (int i = 0; i not_eq nOfVars; ++i) {
    m_textOutput << fixed << varNames[i] << "\t" << mean(dataSample[i]) << "\t" << sqrt(variance(dataSample[i])) << "\n";
  }

}

void trackFitCheckerModule::resizeLayerWiseData(const string& nameOfDataSample, const int nVarsToTest)
{
  m_layerWiseDataSamples[nameOfDataSample].resize(m_nLayers);
  for (int l = 0; l not_eq m_nLayers; ++l) {
    m_layerWiseDataSamples[nameOfDataSample][l].resize(nVarsToTest);
  }
  if (m_writeToRootFile == true) {
    m_layerWiseDataForRoot[nameOfDataSample] = new Belle2::LayerWiseData(m_nLayers, nVarsToTest);
    m_statDataTreePtr->Bronch(nameOfDataSample.c_str(), "Belle2::LayerWiseData", &(m_layerWiseDataForRoot[nameOfDataSample]));
  }
}

void trackFitCheckerModule::fillLayerWiseData(const string& nameOfDataSample, const int accuVecIndex, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_layerWiseDataSamples[nameOfDataSample][accuVecIndex][i](newData[i]);
  }
  if (m_writeToRootFile == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      m_layerWiseDataForRoot[nameOfDataSample]->layerVecData[accuVecIndex][i] = float(newData[i]);
    }
  }
}

void trackFitCheckerModule::fillTrackWiseVecData(const string& nameOfDataSample, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_trackWiseVecDataSamples[nameOfDataSample][i](newData[i]);
  }
  if (m_writeToRootFile == true) {
    for (int i = 0; i not_eq nNewData; ++i) {
      m_trackWiseVecDataForRoot[nameOfDataSample]->at(i) = float(newData[i]);
    }
  }
}

void trackFitCheckerModule::fillTrackWiseData(const string& nameOfDataSample, const double newData)
{
  m_trackWiseDataSamples[nameOfDataSample](newData);
  if (m_writeToRootFile == true) {
    *(m_trackWiseDataForRoot[nameOfDataSample]) = float(newData);
  }
}
