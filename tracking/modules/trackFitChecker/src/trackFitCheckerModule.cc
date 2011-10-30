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
  addParam("outputFileName2", m_dataOutFileName2, "Output file name2", string("tfcpvalues.txt"));
  addParam("totalChi2Cut", m_totalChi2Cut, "only tracks with a total χ2 lower than this value will be considered", 1E300);
  addParam("testSi", m_testSi, "execute additionlly the Si only tests", true);
  addParam("testCdc", m_testCdc, "execute additionlly the CDC only tests", false);
  addParam("useTruthInfo", m_useTruthInfo, "use the truth info from the geant4 simulation", true);
  addParam("testPrediction", m_testPrediction, "additionlly test the predicted state vecs from the Kalman filter. ONLY WOKRKS IF THEY ARE SAVED DURING FITTING WHICH IS NOT THE DEFAULT", false);
}


trackFitCheckerModule::~trackFitCheckerModule()
{

}

void trackFitCheckerModule::initialize()
{
  m_nDigits = 4;
  m_nSiLayers = 6;
  m_nPxdLayers = 2;
  //m_nCdcLayers = 0;
  m_nSvdLayers = 4;
  m_dataOut.open(m_dataOutFileName.c_str());
  m_dataOut2.open(m_dataOutFileName2.c_str());
  //m_absMomOut.open("absMomOut.txt");
  m_dataOut.precision(14);
  m_dataOut2.precision(14);
  m_dataOut2 << "event#\tproc#\tndf\tchi2tot\tfChi2tot\tpValue\tfpValue\tsmChi2l1\tsmChi2l2\tsmChi2l3\tsmChi2l4\tsmChi2l5\tsmChi2l6\tz_qOverPl1\tresPropV4\tabs(p)\n";
  //m_dataOut << "event#\tproc#\tfchi2tot\tbchi2tot\tabsMom\tu_t 1\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi2\tu_t 2\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi2\tu_t 3\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi2\tu_t 4\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi2\tu_t 5\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi2\tu_t 6\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi2\n";
  m_dataOut << "event#\tproc#\tfchi2tot\tbchi2tot\tabsMom\tu_t 1\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi\tfchi2\tu_t 2\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi\tfchi2\tu_t 3\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi\tfchi2\tu_t 4\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi\tfchi2\tu_t 5\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi\tfchi2\tu_t 6\tv_t\tu_m\tv_m\tu\tv\tvar_u\tvar_v\tcov_uv\tcov_vu\tpchi\tfchi2\tpfChi2tot\n";

  //m_absMomOut.precision(14);
  m_failedSmootherCounter = 0;


  //make all vector of vectors have the size of the number of current layers in use

  //map<string, vector<vector<double> > >::const_iterator iter = m_layerWiseDataSamples.begin();
  //map<string, vector<vector<double> > >::const_iterator iterMax = m_layerWiseDataSamples.end();

  int vecSizeTruthTest = 6 + 1; //the +1 is for the uv subcov
  int vecSizeMeasTest = 3;
  int measDim = 2;

  //set the default names of varialbes stored in the statistics container if they are mulitdimensional (like the 5 track parameters)
  m_layerWiseTruthTestsVarNames.push_back("q/p");
  m_layerWiseTruthTestsVarNames.push_back("du/dw");
  m_layerWiseTruthTestsVarNames.push_back("dv/dw");
  m_layerWiseTruthTestsVarNames.push_back("u");
  m_layerWiseTruthTestsVarNames.push_back("v");
  m_layerWiseTruthTestsVarNames.push_back("χ²");
  m_layerWiseTruthTestsVarNames.push_back("χ²uv");

  m_vertexTestsVarNames.push_back("x");
  m_vertexTestsVarNames.push_back("y");
  m_vertexTestsVarNames.push_back("z");
  m_vertexTestsVarNames.push_back("p_x");
  m_vertexTestsVarNames.push_back("p_y");
  m_vertexTestsVarNames.push_back("p_z");


  // pulls (z) of cartesian coordinates of innermost hit and vertex
  m_trackWiseDataVecSamples["zs_vertexPosMom"].resize(6);

  // pulls (z) and chi2s for the 5 track parameters in every layer using truth info
  resizeLayerWiseData("zs_and_chi2_fp_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_fu_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_bp_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_bu_t", vecSizeTruthTest);
  resizeLayerWiseData("zs_and_chi2_sm_t", vecSizeTruthTest);
  // pulls (z) and chi2s for the 5 track parameters in every layer using the projection onto the measurement m-Hx
  resizeLayerWiseData("zs_and_chi2_fp", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_fu", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_bp", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_bu", vecSizeMeasTest);
  resizeLayerWiseData("zs_and_chi2_sm", vecSizeMeasTest);

  // pulls and chi2 to test constistency of normal distribution model of measurements and the sigma of the digitzer with the sigma of the recoHits
  resizeLayerWiseData("zs_and_chi2_meas_t", vecSizeMeasTest);


  m_badR_fCounter = 0;
  m_badR_bCounter = 0;
  m_badR_smCounter = 0;
  m_processedTracks = 0;
  m_eventCounter = 0;
  m_nCutawayTracks = 0;
  m_notPosDefCounter = 0;
  m_unSymmetricCounter = 0;

}


void trackFitCheckerModule::beginRun()
{

}



void trackFitCheckerModule::event()
{

  //simulated particles and hits
  StoreArray<MCParticle> aMcParticleArray("");
  //m_dataOut << "mcParticles " << aMcParticleArray->GetEntries() << "\n";
  StoreArray<PXDTrueHit> aPxdTrueHitArray("");
  int nPxdSimHits = aPxdTrueHitArray.getEntries();
  //m_dataOut << "pxdSimHits " << aPxdTrueHitArray->GetEntries() << "\n";
  StoreArray<SVDTrueHit> aSvdTrueHitArray("");
  int nSvdSimHits = aSvdTrueHitArray.getEntries();
  int nSiSimHits = nPxdSimHits + nSvdSimHits;
  //m_dataOut << "svdSimHits " << aSvdTrueHitArray->GetEntries() << "\n";
  //StoreArray<CDCTrueSimHit> aCdcTrueHitArray("");


  //all relations

  //genfit stuff
  StoreArray<GFTrackCand> trackCandidates(""); // to create a new track rep for extrapolation only
  StoreArray<GFTrack> fittedTracks(""); // the results of the track fit
  int nFittedTracks = fittedTracks.getEntries();
  // testoutput
  StoreArray<TrackFitCheckerTempHelperClass> qualityIndicators("QualityIndicators");

  //cerr << "===== event2\n";

  //m_dataOut << "chi2totWerte: ";
  //cerr << "===== event3\n";
  //cout << m_eventCounter << " " << flush;
  for (int i = 0; i not_eq nFittedTracks; ++i) {

    GFTrack* aTrackPtr = fittedTracks[i];
    int mcParticleIndex = aTrackPtr->getCand().getMcTrackId();

    double charge = aMcParticleArray[mcParticleIndex]->getCharge();
    TVector3 trueVertexMom = aMcParticleArray[mcParticleIndex]->getMomentum();
    TVector3 trueVertexPos = aMcParticleArray[mcParticleIndex]->getVertex();
    GFAbsTrackRep* propOnlyTrRepPtr = new RKTrackRep(trackCandidates[i]);


    double chi2tot_bu = aTrackPtr->getChiSqu(); //at the moment genfit returns the total chi2 from the backward filter
    if (chi2tot_bu > m_totalChi2Cut) {
      //consider this track to be an outlier and discard it
      ++m_nCutawayTracks;
    } else { // not and outlier contine with tests
      // first part: get variable disribing the hole track
      double chi2tot_fu = aTrackPtr->getForwardChiSqu();
      int ndf = aTrackPtr->getNDF();
      double pValue_bu = TMath::Prob(chi2tot_bu, ndf); // actually the p value would be 1-TMath::Prob(chi2tot, ndf) but particle physicists want to have it this way.
      double pValue_fu = TMath::Prob(chi2tot_fu, ndf);
      m_trackWiseDataSamples["pValue_bu"](pValue_bu);
      m_trackWiseDataSamples["pValue_fu"](pValue_fu);

      TVector3 vertexPos;
      TVector3 vertexMom;
      TMatrixT<double> vertexCov(6, 6);
      TVector3 poca; //point of closest approach will be overwritten
      TVector3 dirInPoca; //direction of the track at the point of closest approach will be overwritten
      aTrackPtr->getCardinalRep()->extrapolateToPoint(trueVertexPos, poca, dirInPoca); //goto vertex position
      GFDetPlane plane(poca, dirInPoca); //get plane through fitted vertex position
      double vertexAbsMom = aTrackPtr->getMom(plane).Mag(); //get fitted momentum at fitted vertex
      m_trackWiseDataSamples["absMomVertex"](vertexAbsMom);
      aTrackPtr->getPosMomCov(plane, vertexPos, vertexMom, vertexCov);
      TVector3 resVertexMom = vertexMom - trueVertexMom;
      TVector3 resVertexPos = vertexPos - trueVertexPos;
      m_trackWiseDataVecSamples["zs_vertexPosMom"][0](resVertexPos[0] / sqrt(vertexCov[0][0]));
      m_trackWiseDataVecSamples["zs_vertexPosMom"][1](resVertexPos[1] / sqrt(vertexCov[1][1]));
      m_trackWiseDataVecSamples["zs_vertexPosMom"][2](resVertexPos[2] / sqrt(vertexCov[2][2]));
      m_trackWiseDataVecSamples["zs_vertexPosMom"][3](resVertexPos[0] / sqrt(vertexCov[3][3]));
      m_trackWiseDataVecSamples["zs_vertexPosMom"][4](resVertexPos[1] / sqrt(vertexCov[4][4]));
      m_trackWiseDataVecSamples["zs_vertexPosMom"][5](resVertexPos[2] / sqrt(vertexCov[5][5]));



      //write stuff in helper class... to be able to make plots with the TBrowser... something better should be used in the future
      new(qualityIndicators->AddrAt(i)) TrackFitCheckerTempHelperClass();
      qualityIndicators[i]->chi2tot_fu = chi2tot_fu;
      qualityIndicators[i]->pValue_fu = pValue_fu;
      qualityIndicators[i]->chi2tot_bu = chi2tot_bu;
      qualityIndicators[i]->pValue_bu = pValue_bu;
      //now the layer wise tests
      TVector3 pLocalTrue;
      TMatrixT<double> state;
      TMatrixT<double> cov;
      TMatrixT<double> propMat(5, 5);
      TMatrixT<double> trueState(5, 1);
      TMatrixT<double> onlyPropState(5, 1);
      TMatrixT<double> res(2, 1);
      TMatrixT<double> R(2, 2);
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
        //first get truth info or every GF recoHit
        PXDRecoHit const*const aPxdRecoHitPtr = dynamic_cast<PXDRecoHit*>(aTrackPtr->getHit(iGFHit));
        if (aPxdRecoHitPtr not_eq NULL) {
          PXDTrueHit const*const aTrueHitPtr = aPxdRecoHitPtr->getTrueHit();
          hitLayerId = aTrueHitPtr->getSensorID().getLayer();
          pTrue = aTrueHitPtr->getMomentum();
          uTrue = aTrueHitPtr->getU();
          vTrue = aTrueHitPtr->getV();
        } else {
          SVDRecoHit2D const*const aSvdRecoHitPtr = static_cast<SVDRecoHit2D*>(aTrackPtr->getHit(iGFHit));
          SVDTrueHit const*const aTrueHitPtr = aSvdRecoHitPtr->getTrueHit();
          hitLayerId = aTrueHitPtr->getSensorID().getLayer();
          pTrue = aTrueHitPtr->getMomentum();
          uTrue = aTrueHitPtr->getU();
          vTrue = aTrueHitPtr->getV();
        }

        int accuVecIndex = hitLayerId - 1;
        //second get the data from the hit
        TMatrixT<double> H = aTrackPtr->getHit(iGFHit)->getHMatrix(aTrackPtr->getTrackRep(0)); // this should not work because in the module genfitter releaseHits() is called and getHit performs a boundary check. Why it works only Yoda knows
        TMatrixT<double> HT(TMatrixT<double>::kTransposed, H); // the transposed is needed later
        TMatrixT<double> m = aTrackPtr->getHit(iGFHit)->getRawHitCoord(); //measurement of hit
        TMatrixT<double> V = aTrackPtr->getHit(iGFHit)->getRawHitCov(); //covariance matrix of hit
        //cout << "m"; m.Print();cout << "V"; V.Print();
        // build the true state vector x_t all in local coordinates
        trueState[0][0] = charge / pTrue.Mag(); // q/p
        trueState[1][0] = pTrue[0] / pTrue[2]; //dudw
        trueState[2][0] = pTrue[1] / pTrue[2];//dvdw
        trueState[3][0] = uTrue; // u
        trueState[4][0] = vTrue; // v
        m_dataOut << "\t" << trueState[3][0] << "\t" << trueState[4][0] << "\t" << m[0][0] << "\t" << m[1][0];
        //cout << "true state\n";trueState.Print();
        // now test if measurements am their theoretical variances that were feeded to genfit are consistent with the acutal distribution of measurments
        res = m - H * trueState;
        truthTests = calcZs(res, V);
        truthTests.push_back(calcChi2(res, V));
        fillLayerWiseData("zs_and_chi2_meas_t", accuVecIndex, truthTests);
//
        GFTools::getBiasedSmoothedData(aTrackPtr, 0, iGFHit, state, cov);
//            cout << "smoother";
//            state.Print();
//            cov.Print();
        res = m - H * state;
        R = V - H * cov * HT;
        if (R[0][0] <= 0.0 or R[1][1] <= 0.0) {
          ++m_badR_smCounter;
        } else {
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(calcChi2(res, R));
          fillLayerWiseData("zs_and_chi2_sm", accuVecIndex, testResutlsWithoutTruth);
        }
        // now calculate test qunatites with smoothed state vec and cov and the true state vec
        fillLayerWiseData("zs_and_chi2_sm_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));

//
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
          //cout << "chi2inrement = " << calcChi2(res,R) << "\n";
          m_dataOut << "\t" << state[3][0] << "\t" << state[4][0] << "\t" << cov[3][3] << "\t" << cov[4][4] << "\t" << cov[3][4] << "\t" <<  cov[4][3] << "\t" << fpChi2increment;

          //get the propagation matrix
          //aTrackPtr->getBK(0)->getMatrix("fProp",iGFHit,propMat);
          //propMat.Print();

          // test using predicted forward state and truth information
          fillLayerWiseData("zs_and_chi2_fp_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));

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
        double fChi2Inrement = 0;
        if (R[0][0] <= 0.0 or R[1][1] <= 0.0) {
          ++m_badR_fCounter;
        } else {
          fChi2Inrement = calcChi2(res, R);
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(fChi2Inrement);
          fillLayerWiseData("zs_and_chi2_fu", accuVecIndex, testResutlsWithoutTruth);
        }

        m_dataOut << "\t" << fChi2Inrement;
        // test using updated forward state and truth information
        fillLayerWiseData("zs_and_chi2_fu_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));

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
          fillLayerWiseData("zs_and_chi2_bp_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));
        }

        // standard scores (pulls) calculated from residuals using the measurements and the updated backward state
        aTrackPtr->getBK(0)->getMatrix("bUpSt", iGFHit, state);
        aTrackPtr->getBK(0)->getMatrix("bUpCov", iGFHit, cov);
        isMatrixCov(cov);
        res = m - H * state;
        R = V - H * cov * HT;
        if (R[0][0] <= 0.0 or R[1][1] <= 0.0) {
          ++m_badR_bCounter;
        } else {
          testResutlsWithoutTruth = calcZs(res, R);
          testResutlsWithoutTruth.push_back(calcChi2(res, R));
          fillLayerWiseData("zs_and_chi2_bu", accuVecIndex, testResutlsWithoutTruth);
        }

        // test using updated backward state and truth information
        fillLayerWiseData("zs_and_chi2_bu_t", accuVecIndex, calcTestsWithTruthInfo(state, cov, trueState));



//        }
//        m_dataOut << "\t" << pfChi2tot;
//        /*if (layerIdSum not_eq 21){
//          B2ERROR("layerIdSum is not 21 " << m_eventCounter << "\t" << m_processedTracks << "\t" << layerIdSum);
//          m_dataOut << m_eventCounter << "\t" << m_processedTracks << "\t" << layerIdSum << "\n";
//        }*/
//        //m_dataOut2 << "\t" <<z_qOverPDataOut<< "\t" <<z_vDataOut <<"\t"<< vertexAbsMom;
//      }
//      m_dataOut2 << "\n";
//      m_dataOut << "\n";
        ++m_processedTracks;
      }

    }
  }
  //m_dataOut << "\n";
  ++m_eventCounter;
}

void trackFitCheckerModule::endRun()
{
  B2INFO("This is the endRun Output from the trackFitChecker module");

//  if (m_failedSmootherCounter not_eq 0){
//    B2WARNING("Smoothed states could not be extracted " << m_failedSmootherCounter << " times";)
//  }
  if (m_nCutawayTracks not_eq 0) {
    B2WARNING(m_nCutawayTracks << " tracks where cut out because of too large total χ²";)
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
    printLayerWiseStatistics("zs_and_chi2_meas_t", measVarNames);
    printTrackWiseVecStatistics("zs_vertexPosMom", m_vertexTestsVarNames);

    printLayerWiseStatistics("zs_and_chi2_fp_t", m_layerWiseTruthTestsVarNames);
    printLayerWiseStatistics("zs_and_chi2_fp", measVarNames);
    printLayerWiseStatistics("zs_and_chi2_fu_t", m_layerWiseTruthTestsVarNames);
    printLayerWiseStatistics("zs_and_chi2_fu", measVarNames);

    printLayerWiseStatistics("zs_and_chi2_bp_t", m_layerWiseTruthTestsVarNames);
    printLayerWiseStatistics("zs_and_chi2_bp", measVarNames);
    printLayerWiseStatistics("zs_and_chi2_bu_t", m_layerWiseTruthTestsVarNames);
    printLayerWiseStatistics("zs_and_chi2_bu", measVarNames);

    printLayerWiseStatistics("zs_and_chi2_sm_t", m_layerWiseTruthTestsVarNames);
    printLayerWiseStatistics("zs_and_chi2_sm", measVarNames);

//    if (m_testSi == true){
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
  //m_dataOut << "trackFitCheckerModule::terminate()\n";
  m_dataOut2.close();
  m_dataOut.close();
  //m_absMomOut.close();
}

// calculate a chi2 value from a residuum and it's covariance matrix R
double trackFitCheckerModule::calcChi2(const TMatrixT<double>& res, const TMatrixT<double>& R)
{
  TMatrixT<double> invR;
  GFTools::invertMatrix(R, invR);
  TMatrixT<double> resT(TMatrixT<double>::kTransposed, res);
  return (resT*invR*res)[0][0];
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

  TMatrixT<double> resUV(2, 1);
  TMatrixT<double> covUV(2, 2);
  resUV[0][0] = res[3][0];
  resUV[1][0] = res[4][0];
  covUV = cov.GetSub(3, 4, 3, 4);

  resultVec.push_back(calcChi2(resUV, covUV));

  return resultVec;
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

  cout.precision(m_nDigits);

  int nOfLayers = dataSample.size();
  int nOfVars = dataSample[0].size();
  cout << "Information on " << nameOfDataSample << " for all layers\npara\\l\t1\t\t2\t\t3\t\t4\t\t5\t\t6\n\tmean\tstd\tmean\tstd\tmean\tstd\tmean\tstd\tmean\tstd\tmean\tstd\n";
  for (int i = 0; i not_eq nOfVars; ++i) {
    cout << layerWiseVarNames[i];
    for (int l = 0; l not_eq nOfLayers; ++l) {
      double tempMean = mean(dataSample[l][i]);
      double tempStd = sqrt(variance(dataSample[l][i]));
      cout << fixed << "\t" << tempMean << "\t" << tempStd;
    }
    cout << "\n";
  }
}

void trackFitCheckerModule::printTrackWiseStatistics(const string& nameOfDataSample)
{

  StatisticsContainer&  dataSample = m_trackWiseDataSamples[nameOfDataSample];

  cout.precision(m_nDigits);

  cout << "Information on " << nameOfDataSample << "\nmean\tstd\n";
  cout << fixed << mean(dataSample) << "\t" << sqrt(variance(dataSample)) << "\n";
}

void trackFitCheckerModule::printTrackWiseVecStatistics(const string& nameOfDataSample, const vector<string>& trackWiseVarNames)
{

  vector<StatisticsContainer>&  dataSample = m_trackWiseDataVecSamples[nameOfDataSample];

  cout.precision(m_nDigits);
  const int nOfVars = dataSample.size();
  cout << "Information on " << nameOfDataSample << "\n\tmean\tstd\n";
  for (int i = 0; i not_eq nOfVars; ++i) {
    cout << fixed << trackWiseVarNames[i] << "\t" << mean(dataSample[i]) << "\t" << sqrt(variance(dataSample[i])) << "\n";
  }

}

void trackFitCheckerModule::resizeLayerWiseData(const string& nameOfDataSample, const int nVarsToTest)
{
  m_layerWiseDataSamples[nameOfDataSample].resize(m_nSiLayers);
  for (int l = 0; l not_eq m_nSiLayers; ++l) {
    m_layerWiseDataSamples[nameOfDataSample][l].resize(nVarsToTest);
  }
}

void trackFitCheckerModule::fillLayerWiseData(const string& nameOfDataSample, const int accuVecIndex, const vector<double>& newData)
{
  const int nNewData = newData.size();
  for (int i = 0; i not_eq nNewData; ++i) {
    m_layerWiseDataSamples[nameOfDataSample][accuVecIndex][i](newData[i]);
  }
}