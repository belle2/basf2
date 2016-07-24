/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Elisa Manoni, Benjamin Oberhof                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <list>
#include <iostream>
#include <ecl/modules/eclDataAnalysis/ECLDataAnalysisModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLCalDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>
#include <ecl/dataobjects/ECLPidLikelihood.h>
#include <ecl/dataobjects/ECLConnectedRegion.h>

//#include <ecl/dataobjects/ECLTrig.h>

using namespace std;
using namespace Belle2;
//using namespace ECL;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ECLDataAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDataAnalysisModule::ECLDataAnalysisModule()
  : Module(),
    m_rootFilePtr(0),
    m_writeToRoot(1),
    m_doTracking(0),
    m_tree(0),
    m_iExperiment(0),
    m_iRun(0),
    m_iEvent(0),
//Digit
    m_eclDigitMultip(0),
    m_eclDigitIdx(0),
    m_eclDigitToMc(0),
    m_eclDigitCellId(0),
    m_eclDigitAmp(0),
    m_eclDigitTimeFit(0),
    m_eclDigitFitQuality(0),
    m_eclDigitToShower(0),
    m_eclDigitToCalDigit(0),
//CalDigit
    m_eclCalDigitMultip(0),
    m_eclCalDigitIdx(0),
    m_eclCalDigitToMc1(0),
    m_eclCalDigitToMc1PDG(0),
    m_eclCalDigitToMcWeight1(0),
    m_eclCalDigitToMc2(0),
    m_eclCalDigitToMc2PDG(0),
    m_eclCalDigitToMcWeight2(0),
    m_eclCalDigitToMc3(0),
    m_eclCalDigitToMc3PDG(0),
    m_eclCalDigitToMcWeight3(0),
    m_eclCalDigitToMc4(0),
    m_eclCalDigitToMc4PDG(0),
    m_eclCalDigitToMcWeight4(0),
    m_eclCalDigitToMc5(0),
    m_eclCalDigitToMc5PDG(0),
    m_eclCalDigitToMcWeight5(0),
    m_eclCalDigitToBkgWeight(0),
    m_eclCalDigitSimHitSum(0),
    m_eclCalDigitToShower(0),
    m_eclCalDigitCellId(0),
    m_eclCalDigitAmp(0),
    m_eclCalDigitTimeFit(0),
    m_eclCalDigitFitQuality(0),
    m_eclCalDigitToCR(0),
//Connected Region
    m_eclCRIdx(0),
    m_eclCRIsTrack(0),
    m_eclCRLikelihoodMIPNGamma(0),
    m_eclCRLikelihoodChargedHadron(0),
    m_eclCRLikelihoodElectronNGamma(0),
    m_eclCRLikelihoodNGamma(0),
    m_eclCRLikelihoodNeutralHadron(0),
    m_eclCRLikelihoodMergedPi0(0),
//m_eclCRToCalDigit(0),
//SimHit
    m_eclSimHitMultip(0),
    m_eclSimHitIdx(0),
    m_eclSimHitToMc(0),
    m_eclSimHitCellId(0),
    m_eclSimHitPdg(0),
    m_eclSimHitEnergyDep(0),
    m_eclSimHitFlightTime(0),
    m_eclSimHitX(0),
    m_eclSimHitY(0),
    m_eclSimHitZ(0),
    m_eclSimHitPx(0),
    m_eclSimHitPy(0),
    m_eclSimHitPz(0),
//Hit
    m_eclHitMultip(0),
    m_eclHitIdx(0),
    m_eclHitToMc(0),
    m_eclHitToDigit(0),
    m_eclHitToDigitAmp(0),
    m_eclHitToPureDigit(0),
    m_eclHitToPureDigitAmp(0),
    m_eclHitCellId(0),
    m_eclHitEnergyDep(0),
    m_eclHitTimeAve(0),
//Cluster
    m_eclClusterMultip(0),
    m_eclClusterIdx(0),
    m_eclClusterToMc1(0),
    m_eclClusterToMcWeight1(0),
    m_eclClusterToMc1PDG(0),
    m_eclClusterToMc2(0),
    m_eclClusterToMcWeight2(0),
    m_eclClusterToMc3(0),
    m_eclClusterToMcWeight3(0),
    m_eclClusterToMc4(0),
    m_eclClusterToMcWeight4(0),
    m_eclClusterToMc5(0),
    m_eclClusterToMcWeight5(0),
    m_eclClusterToBkgWeight(0),
    m_eclClusterSimHitSum(0),
    m_eclClusterEnergy(0),
    m_eclClusterEnergyError(0),
    m_eclClusterTheta(0),
    m_eclClusterThetaError(0),
    m_eclClusterPhi(0),
    m_eclClusterPhiError(0),
    m_eclClusterR(0),
    m_eclClusterEnergyDepSum(0),
    m_eclClusterTiming(0),
    m_eclClusterTimingError(0),
    m_eclClusterE9oE25(0),
    m_eclClusterHighestE(0),
    m_eclClusterLat(0),
    m_eclClusterNofCrystals(0),
    m_eclClusterCrystalHealth(0),
    m_eclClusterMergedPi0(0),
    m_eclClusterPx(0),
    m_eclClusterPy(0),
    m_eclClusterPz(0),
    m_eclClusterIsTrack(0),
    m_eclClusterPi0Likel(0),
    m_eclClusterEtaLikel(0),
    m_eclClusterDeltaL(0),
    m_eclClusterBeta(0),
//PureDigit
    m_eclPureDigitMultip(0),
    m_eclPureDigitIdx(0),
    m_eclPureDigitToMc(0),
    m_eclPureDigitCellId(0),
    m_eclPureDigitAmp(0),
    m_eclPureDigitTimeFit(0),
    m_eclPureDigitFitQuality(0),
    m_eclPureDigitToCluster(0),
//PureCluster
    m_eclPureClusterMultip(0),
    m_eclPureClusterIdx(0),
    m_eclPureClusterToMc(0),
    m_eclPureClusterEnergy(0),
    m_eclPureClusterEnergyError(0),
    m_eclPureClusterTheta(0),
    m_eclPureClusterThetaError(0),
    m_eclPureClusterPhi(0),
    m_eclPureClusterPhiError(0),
    m_eclPureClusterR(0),
    m_eclPureClusterEnergyDepSum(0),
    m_eclPureClusterTiming(0),
    m_eclPureClusterTimingError(0),
    m_eclPureClusterE9oE25(0),
    m_eclPureClusterHighestE(0),
    m_eclPureClusterLat(0),
    m_eclPureClusterNofCrystals(0),
    m_eclPureClusterCrystalHealth(0),
    m_eclPureClusterMergedPi0(0),
    m_eclPureClusterPx(0),
    m_eclPureClusterPy(0),
    m_eclPureClusterPz(0),
    m_eclPureClusterIsTrack(0),
    m_eclPureClusterPi0Likel(0),
    m_eclPureClusterEtaLikel(0),
    m_eclPureClusterDeltaL(0),
    m_eclPureClusterBeta(0),
//Shower
    m_eclShowerMultip(0),
    m_eclShowerIdx(0),
    m_eclShowerToMc1(0),
    m_eclShowerToMcWeight1(0),
    m_eclShowerToMc1PDG(0),
    m_eclShowerToMc2(0),
    m_eclShowerToMcWeight2(0),
    m_eclShowerToMc3(0),
    m_eclShowerToMcWeight3(0),
    m_eclShowerToMc4(0),
    m_eclShowerToMcWeight4(0),
    m_eclShowerToMc5(0),
    m_eclShowerToMcWeight5(0),
    m_eclShowerSimHitSum(0),
    m_eclShowerUncEnergy(0),
    m_eclShowerEnergy(0),
    m_eclShowerTheta(0),
    m_eclShowerPhi(0),
    m_eclShowerR(0),
    m_eclShowerNHits(0),
    m_eclShowerE9oE25(0),
    m_eclShowerTime(0),
//new variables
    m_eclShowerConnectedRegionId(0),
    m_eclShowerHypothesisId(0),
    m_eclShowerCentralCellId(0),
    m_eclShowerEnergyError(0),
    m_eclShowerThetaError(0),
    m_eclShowerPhiError(0),
    m_eclShowerTimeResolution(0),
    m_eclShowerHighestEnergy(0),
    m_eclShowerLateralEnergy(0),
    m_eclShowerMinTrkDistance(0),
    m_eclShowerTrkDepth(0),
    m_eclShowerShowerDepth(0),
    m_eclShowerAbsZernike20(0),
    m_eclShowerAbsZernike40(0),
    m_eclShowerAbsZernike42(0),
    m_eclShowerAbsZernike51(0),
    m_eclShowerAbsZernike53(0),
    m_eclShowerSecondMoment(0),
    m_eclShowerE1oE9(0),
    m_eclShowerIsTrack(0),
    m_eclShowerIsCluster(0),
//MC
    m_mcMultip(0),
    m_mcIdx(0),
    m_mcPdg(0),
    m_mcMothPdg(0),
    m_mcGMothPdg(0),
    m_mcGGMothPdg(0),
    m_mcEnergy(0),
    m_mcPx(0),
    m_mcPy(0),
    m_mcPz(0),
    m_mcDecayVtxX(0),
    m_mcDecayVtxY(0),
    m_mcDecayVtxZ(0),
    m_mcProdVtxX(0),
    m_mcProdVtxY(0),
    m_mcProdVtxZ(0),
    m_mcSecondaryPhysProc(0),
//Tracks
    m_trkMultip(0),
    m_trkPdg(0),
    m_trkIdx(0),
    m_trkCharge(0),
    m_trkPx(0),
    m_trkPy(0),
    m_trkPz(0),
    m_trkP(0),
    m_trkTheta(0),
    m_trkPhi(0),
    m_trkX(0),
    m_trkY(0),
    m_trkZ(0),
//PID
    m_eclpidtrkIdx(0),
    m_eclpidEnergy(0),
    m_eclpidEop(0),
    m_eclpidE9E25(0),
    m_eclpidNCrystals(0),
    m_eclpidNClusters(0),
    m_eclLogLikeEl(0),
    m_eclLogLikeMu(0),
    m_eclLogLikePi(0)

{
  //Set module properties

  setDescription("This module produces an ntuple with ECL-related quantities starting from mdst");
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclDataAnalysis"));
  addParam("doTracking", m_doTracking, "set true if you want to save the informations from TrackFitResults'rootFileName'",
           bool(false));
  addParam("doPureCsIStudy", m_doPureCsIStudy, "set true if you want to save the informations for upgrade option'", bool(false));
  addParam("pure_clusters", m_pure_clusters, "name of input Pure CsI Clusters", string("ECLClustersPureCsI"));
  addParam("pure_digits", m_pure_digits, "name of input Pure CsI Digits", string("ECLDigitsPureCsI"));
  addParam("pure_cal_digits", m_pure_cal_digits, "name of input Pure CsI Cal Digits", string("ECLCalDigitsPureCsI"));
}

ECLDataAnalysisModule::~ECLDataAnalysisModule()
{
}


void ECLDataAnalysisModule::initialize()
{

  B2INFO("[ECLDataAnalysis Module]: Starting initialization of ECLDataAnalysis Module.");

  //StoreArray<ECLTrig>::required();
  StoreArray<ECLDigit>::required();
  StoreArray<ECLCalDigit>::required();
  StoreArray<ECLSimHit>::required();
  StoreArray<ECLHit>::required();
  StoreArray<ECLCluster>::required();
  StoreArray<ECLShower>::required();
  StoreArray<MCParticle>::required();
  StoreArray<ECLConnectedRegion>::required();
  //StoreArray<ECLHitAssignment>::required();

  if (m_doPureCsIStudy == 1) {
    StoreArray<ECLDigit>::required(m_pure_digits);
    StoreArray<ECLCluster>::required(m_pure_clusters);
    StoreArray<ECLCalDigit>::required(m_pure_cal_digits);
    //RelationArray ECLPureDigitsToMCParticle::isRequired();
    //RelationArray ECLPureDigitsToMCParticle.isRequired();
    //(<ECLDigit>(m_pure_digits),<MCParticle>)::isRequired;
    //StoreArray<ECLDigit>::required(m_pure_digits);
  }

  if (m_doTracking == true) {
    StoreArray<Track>::required();
    StoreArray<TrackFitResult>::required();
    StoreArray<ECLPidLikelihood>::required();
  }

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  // initialize tree
  m_tree     = new TTree("m_tree", "ECL Analysis tree");

  /* m_eclTriggerMultip=0;
  m_eclTriggerIdx = new std::vector<int>();
  m_eclTriggerCellId = new std::vector<int>();
  m_eclTriggerTime = new std::vector<double>();
  */
  /*
  m_eclDigitMultip = 0;
  m_eclDigitIdx = new std::vector<int>();
  m_eclDigitToMc = new std::vector<int>();
  m_eclDigitCellId = new std::vector<int>();
  m_eclDigitAmp = new std::vector<int>();
  m_eclDigitTimeFit = new std::vector<int>();
  m_eclDigitFitQuality = new std::vector<int>();
  m_eclDigitToShower = new std::vector<int>();
  m_eclDigitToCalDigit = new std::vector<int>();

  m_eclCalDigitMultip = 0;
  m_eclCalDigitIdx = new std::vector<int>();
  m_eclCalDigitToMc = new std::vector<int>();
  m_eclCalDigitToShower = new std::vector<int>();
  m_eclCalDigitCellId = new std::vector<int>();
  m_eclCalDigitAmp = new std::vector<int>();
  m_eclCalDigitTimeFit = new std::vector<int>();
  m_eclCalDigitFitQuality = new std::vector<int>();

  m_eclSimHitMultip = 0;
  m_eclSimHitIdx = new std::vector<int>();
  m_eclSimHitToMc = new std::vector<int>();
  m_eclSimHitCellId = new std::vector<int>();
  m_eclSimHitPdg =  new std::vector<int>();
  m_eclSimHitEnergyDep =  new std::vector<double>();
  m_eclSimHitFlightTime =  new std::vector<double>();
  m_eclSimHitX =  new std::vector<double>();
  m_eclSimHitY =  new std::vector<double>();
  m_eclSimHitZ =  new std::vector<double>();
  m_eclSimHitPx =  new std::vector<double>();
  m_eclSimHitPy  =  new std::vector<double>();
  m_eclSimHitPz = new std::vector<double>();

  m_eclHitMultip = 0;
  m_eclHitIdx = new std::vector<int>();
  m_eclHitToMc = new std::vector<int>();
  m_eclHitToDigit = new std::vector<int>();
  m_eclHitToDigitAmp = new std::vector<int>();
  m_eclHitToPureDigit = new std::vector<int>();
  m_eclHitToPureDigitAmp = new std::vector<int>();
  m_eclHitCellId = new std::vector<int>();
  m_eclHitEnergyDep = new std::vector<double>();
  m_eclHitTimeAve = new std::vector<double>();

  m_eclClusterMultip = 0;
  m_eclClusterIdx = new std::vector<int>();
  //m_eclClusterToMc = new std::vector<int>();
  //m_eclClusterToTrack = new std::vector<int>();
  m_eclClusterEnergy = new std::vector<double>();
  m_eclClusterEnergyError = new std::vector<double>();
  m_eclClusterTheta = new std::vector<double>();
  m_eclClusterThetaError = new std::vector<double>();
  m_eclClusterPhi = new std::vector<double>();
  m_eclClusterPhiError = new std::vector<double>();
  m_eclClusterR = new std::vector<double>();
  m_eclClusterEnergyDepSum = new std::vector<double>();
  m_eclClusterTiming = new std::vector<double>();
  m_eclClusterTimingError = new std::vector<double>();
  m_eclClusterE9oE25 = new std::vector<double>();
  m_eclClusterHighestE = new std::vector<double>();
  m_eclClusterLat = new std::vector<double>();
  m_eclClusterNofCrystals = new std::vector<int>();
  m_eclClusterCrystalHealth = new std::vector<int>();
  m_eclClusterMergedPi0 = new std::vector<double>();
  m_eclClusterPx = new std::vector<double>();
  m_eclClusterPy = new std::vector<double>();
  m_eclClusterPz = new std::vector<double>();
  m_eclClusterIsTrack = new std::vector<bool>();
  m_eclClusterPi0Likel = new std::vector<double>();
  m_eclClusterEtaLikel = new std::vector<double>();
  m_eclClusterDeltaL = new std::vector<double>();
  m_eclClusterBeta = new std::vector<double>();

  m_eclShowerMultip = 0;
  m_eclShowerIdx = new std::vector<int>();
  m_eclShowerToMc = new std::vector<int>();
  m_eclShowerUncEnergy = new std::vector<double>();
  m_eclShowerEnergy = new std::vector<double>();
  m_eclShowerTheta = new std::vector<double>();
  m_eclShowerPhi = new std::vector<double>();
  m_eclShowerR = new std::vector<double>();
  m_eclShowerNHits = new std::vector<double>();
  m_eclShowerE9oE25 = new std::vector<double>();

  m_mcMultip = 0;
  m_mcIdx = new std::vector<int>();
  m_mcPdg = new std::vector<int>();
  m_mcMothPdg = new std::vector<int>();
  m_mcGMothPdg = new std::vector<int>();
  m_mcGGMothPdg = new std::vector<int>();
  m_mcEnergy = new std::vector<double>();
  m_mcPx = new std::vector<double>();
  m_mcPy = new std::vector<double>();
  m_mcPz = new std::vector<double>();
  m_mcDecayVtxX = new std::vector<double>();
  m_mcDecayVtxY = new std::vector<double>();
  m_mcDecayVtxZ = new std::vector<double>();
  m_mcProdVtxX = new std::vector<double>();
  m_mcProdVtxY = new std::vector<double>();
  m_mcProdVtxZ = new std::vector<double>();
  m_mcSecondaryPhysProc = new std::vector<int>();

  m_trkMultip = 0;
  m_trkIdx = new std::vector<int>();
  m_trkPdg = new std::vector<int>();
  m_trkCharge = new std::vector<int>();
  m_trkPx = new std::vector<double>();
  m_trkPy = new std::vector<double>();
  m_trkPz = new std::vector<double>();
  m_trkP  = new std::vector<double>();
  m_trkTheta = new std::vector<double>();
  m_trkPhi = new std::vector<double>();
  m_trkX = new std::vector<double>();
  m_trkY = new std::vector<double>();
  m_trkZ = new std::vector<double>();

  m_eclpidtrkIdx = new std::vector<int>();
  m_eclpidEnergy = new std::vector<double>();
  m_eclpidEop = new std::vector<double>();
  m_eclpidE9E25 = new std::vector<double>();
  m_eclpidNCrystals = new std::vector<int>();
  m_eclpidNClusters = new std::vector<int>();
  m_eclLogLikeEl = new std::vector<double>();
  m_eclLogLikeMu = new std::vector<double>();
  m_eclLogLikePi = new std::vector<double>();
  */

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  /*m_tree->Branch("eclTriggerMultip",     &m_eclTriggerMultip,         "eclTriggerMultip/I");
  m_tree->Branch("eclTriggerIdx",     "std::vector<int>",       &m_eclTriggerIdx);
  m_tree->Branch("eclTriggerCellId",     "std::vector<int>",       &m_eclTriggerCellId);
  m_tree->Branch("eclTriggerTime",       "std::vector<double>",    &m_eclTriggerTime);*/

  m_tree->Branch("eclDigitMultip",     &m_eclDigitMultip,         "ecldigit_Multip/I");
  m_tree->Branch("eclDigitIdx",        "std::vector<int>",         &m_eclDigitIdx);
  m_tree->Branch("eclDigitToMC",      "std::vector<int>",          &m_eclDigitToMc);
  m_tree->Branch("eclDigitCellId",     "std::vector<int>",         &m_eclDigitCellId);
  m_tree->Branch("eclDigitAmp",        "std::vector<int>",         &m_eclDigitAmp);
  m_tree->Branch("eclDigitTimeFit",    "std::vector<int>",         &m_eclDigitTimeFit);
  m_tree->Branch("eclDigitFitQuality",    "std::vector<int>",         &m_eclDigitFitQuality);
  m_tree->Branch("eclDigitToShower",      "std::vector<int>",          &m_eclDigitToShower);
  m_tree->Branch("eclDigitToCalDigit",      "std::vector<int>",          &m_eclDigitToCalDigit);

  m_tree->Branch("eclCalDigitMultip",     &m_eclCalDigitMultip,         "eclCaldigit_Multip/I");
  m_tree->Branch("eclCalDigitIdx",        "std::vector<int>",         &m_eclCalDigitIdx);
  m_tree->Branch("eclCalDigitToMc1",      "std::vector<int>",       &m_eclCalDigitToMc1);
  m_tree->Branch("eclCalDigitToMc1PDG",      "std::vector<int>",       &m_eclCalDigitToMc1PDG);
  m_tree->Branch("eclCalDigitToMcWeight1",      "std::vector<double>",       &m_eclCalDigitToMcWeight1);
  m_tree->Branch("eclCalDigitToMc2",      "std::vector<int>",       &m_eclCalDigitToMc2);
  m_tree->Branch("eclCalDigitToMc2PDG",      "std::vector<int>",       &m_eclCalDigitToMc2PDG);
  m_tree->Branch("eclCalDigitToMcWeight2",      "std::vector<double>",       &m_eclCalDigitToMcWeight2);
  m_tree->Branch("eclCalDigitToMc3",      "std::vector<int>",       &m_eclCalDigitToMc3);
  m_tree->Branch("eclCalDigitToMc3PDG",      "std::vector<int>",       &m_eclCalDigitToMc3PDG);
  m_tree->Branch("eclCalDigitToMcWeight3",      "std::vector<double>",       &m_eclCalDigitToMcWeight3);
  m_tree->Branch("eclCalDigitToMc4",      "std::vector<int>",       &m_eclCalDigitToMc4);
  m_tree->Branch("eclCalDigitToMc4PDG",      "std::vector<int>",       &m_eclCalDigitToMc4PDG);
  m_tree->Branch("eclCalDigitToMcWeight4",      "std::vector<double>",       &m_eclCalDigitToMcWeight4);
  m_tree->Branch("eclCalDigitToMc5",      "std::vector<int>",       &m_eclCalDigitToMc5);
  m_tree->Branch("eclCalDigitToMc5PDG",      "std::vector<int>",       &m_eclCalDigitToMc5PDG);
  m_tree->Branch("eclCalDigitToMcWeight5",      "std::vector<double>",       &m_eclCalDigitToMcWeight5);
  m_tree->Branch("eclCalDigitToBkgWeight",      "std::vector<double>",       &m_eclCalDigitToBkgWeight);
  m_tree->Branch("eclCalDigitSimHitSum",      "std::vector<double>",       &m_eclCalDigitSimHitSum);
  m_tree->Branch("eclCalDigitToShower",      "std::vector<int>",          &m_eclCalDigitToShower);
  m_tree->Branch("eclCalDigitCellId",     "std::vector<int>",         &m_eclCalDigitCellId);
  m_tree->Branch("eclCalDigitAmp",        "std::vector<double>",         &m_eclCalDigitAmp);
  m_tree->Branch("eclCalDigitTimeFit",    "std::vector<double>",         &m_eclCalDigitTimeFit);
  m_tree->Branch("eclCalDigitFitQuality",    "std::vector<int>",         &m_eclCalDigitFitQuality);
  m_tree->Branch("eclCalDigitToCR",      "std::vector<int>",          &m_eclCalDigitToCR);

  m_tree->Branch("eclCRIdx", "std::vector<int>", &m_eclCRIdx);
  m_tree->Branch("eclCRIsTrack", "std::vector<int>", &m_eclCRIsTrack);
  m_tree->Branch("eclCRLikelihoodMIPNGamma", "std::vector<double>", &m_eclCRLikelihoodMIPNGamma);
  m_tree->Branch("eclCRLikelihoodChargedHadron", "std::vector<double>", &m_eclCRLikelihoodChargedHadron);
  m_tree->Branch("eclCRLikelihoodElectronNGamma", "std::vector<double>", &m_eclCRLikelihoodElectronNGamma);
  m_tree->Branch("eclCRLikelihoodNGamma", "std::vector<double>", &m_eclCRLikelihoodNGamma);
  m_tree->Branch("eclCRLikelihoodNeutralHadron", "std::vector<double>", &m_eclCRLikelihoodNeutralHadron);
  m_tree->Branch("eclCRLikelihoodMergedPi0", "std::vector<double>", &m_eclCRLikelihoodMergedPi0);

  m_tree->Branch("eclSimHitMultip",     &m_eclSimHitMultip,      "eclSimHitMultip/I");
  m_tree->Branch("eclSimHitIdx",     "std::vector<int>",       &m_eclSimHitIdx);
  m_tree->Branch("eclSimHitToMC",      "std::vector<int>",       &m_eclSimHitToMc);
  m_tree->Branch("eclSimHitCellId",     "std::vector<int>",       &m_eclSimHitCellId);
  m_tree->Branch("eclSimHitPdg",        "std::vector<int>",       &m_eclSimHitPdg);
  m_tree->Branch("eclSimHitEnergyDep",      "std::vector<double>",    &m_eclSimHitEnergyDep);
  m_tree->Branch("eclSimHitFlightTime",      "std::vector<double>",    &m_eclSimHitFlightTime);
  m_tree->Branch("eclSimHitX",          "std::vector<double>",    &m_eclSimHitX);
  m_tree->Branch("eclSimHitY",          "std::vector<double>",    &m_eclSimHitY);
  m_tree->Branch("eclSimHitZ",          "std::vector<double>",    &m_eclSimHitZ);
  m_tree->Branch("eclSimHitPx",         "std::vector<double>",    &m_eclSimHitPx);
  m_tree->Branch("eclSimHitPy",         "std::vector<double>",    &m_eclSimHitPy);
  m_tree->Branch("eclSimHitPz",         "std::vector<double>",    &m_eclSimHitPz);

  m_tree->Branch("eclHitMultip",     &m_eclHitMultip,      "eclHitMultip/I");
  m_tree->Branch("eclHitIdx",     "std::vector<int>",       &m_eclHitIdx);
  m_tree->Branch("eclHitToMC",      "std::vector<int>",       &m_eclHitToMc);
  m_tree->Branch("eclHitToDigit",      "std::vector<int>",       &m_eclHitToDigit);
  m_tree->Branch("eclHitToDigitAmp",      "std::vector<int>",       &m_eclHitToDigitAmp);
  m_tree->Branch("eclHitCellId",     "std::vector<int>",    &m_eclHitCellId);
  m_tree->Branch("eclHitEnergyDep",      "std::vector<double>", &m_eclHitEnergyDep);
  m_tree->Branch("eclHitTimeAve",       "std::vector<double>", &m_eclHitTimeAve);

  m_tree->Branch("eclClusterMultip",     &m_eclClusterMultip,     "eclClusterMultip/I");
  m_tree->Branch("eclClusterIdx",     "std::vector<int>",       &m_eclClusterIdx);
  m_tree->Branch("eclClusterToMc1",      "std::vector<int>",       &m_eclClusterToMc1);
  m_tree->Branch("eclClusterToMcWeight1",      "std::vector<double>",       &m_eclClusterToMcWeight1);
  m_tree->Branch("eclClusterToMc1PDG",      "std::vector<int>",       &m_eclClusterToMc1PDG);
  m_tree->Branch("eclClusterToMc2",      "std::vector<int>",       &m_eclClusterToMc2);
  m_tree->Branch("eclClusterToMcWeight2",      "std::vector<double>",       &m_eclClusterToMcWeight2);
  m_tree->Branch("eclClusterToMc2PDG",      "std::vector<int>",       &m_eclClusterToMc2PDG);
  m_tree->Branch("eclClusterToMc3",      "std::vector<int>",       &m_eclClusterToMc3);
  m_tree->Branch("eclClusterToMcWeight3",      "std::vector<double>",       &m_eclClusterToMcWeight3);
  m_tree->Branch("eclClusterToMc3PDG",      "std::vector<int>",       &m_eclClusterToMc3PDG);
  m_tree->Branch("eclClusterToMc4",      "std::vector<int>",       &m_eclClusterToMc4);
  m_tree->Branch("eclClusterToMcWeight4",      "std::vector<double>",       &m_eclClusterToMcWeight4);
  m_tree->Branch("eclClusterToMc4PDG",      "std::vector<int>",       &m_eclClusterToMc4PDG);
  m_tree->Branch("eclClusterToMc5",      "std::vector<int>",       &m_eclClusterToMc5);
  m_tree->Branch("eclClusterToMcWeight5",      "std::vector<double>",       &m_eclClusterToMcWeight5);
  m_tree->Branch("eclClusterToMc5PDG",      "std::vector<int>",       &m_eclClusterToMc5PDG);
  m_tree->Branch("eclClusterToBkgWeight",      "std::vector<double>",       &m_eclClusterToBkgWeight);
  m_tree->Branch("eclClusterSimHitSum",      "std::vector<double>",       &m_eclClusterSimHitSum);
  m_tree->Branch("eclClusterToShower",      "std::vector<int>",       &m_eclClusterToShower);
  //m_tree->Branch("eclClusterToTrack",      "std::vector<int>",       &m_eclClusterToTrack);
  m_tree->Branch("eclClusterEnergy",     "std::vector<double>",    &m_eclClusterEnergy);
  m_tree->Branch("eclClusterEnergyError",  "std::vector<double>",    &m_eclClusterEnergyError);
  m_tree->Branch("eclClusterTheta",      "std::vector<double>",    &m_eclClusterTheta);
  m_tree->Branch("eclClusterThetaError",   "std::vector<double>",    &m_eclClusterThetaError);
  m_tree->Branch("eclClusterPhi",        "std::vector<double>",    &m_eclClusterPhi);
  m_tree->Branch("eclClusterPhiError",     "std::vector<double>",    &m_eclClusterPhiError);
  m_tree->Branch("eclClusterR",          "std::vector<double>",    &m_eclClusterR);
  m_tree->Branch("eclClusterEnergyDepSum",   "std::vector<double>",    &m_eclClusterEnergyDepSum);
  m_tree->Branch("eclClusterTiming",     "std::vector<double>",    &m_eclClusterTiming);
  m_tree->Branch("eclClusterTimingError",  "std::vector<double>",    &m_eclClusterTimingError);
  m_tree->Branch("eclClusterE9oE25",     "std::vector<double>",    &m_eclClusterE9oE25);
  m_tree->Branch("eclClusterHighestE",   "std::vector<double>",    &m_eclClusterHighestE);
  m_tree->Branch("eclClusterLat",        "std::vector<double>",    &m_eclClusterLat);
  m_tree->Branch("eclClusterNofCrystals",   "std::vector<int>",       &m_eclClusterNofCrystals);
  m_tree->Branch("eclClusterCrystalHealth", "std::vector<int>",       &m_eclClusterCrystalHealth);
  m_tree->Branch("eclClusterMergedPi0",  "std::vector<double>",    &m_eclClusterMergedPi0);
  m_tree->Branch("eclClusterPx",         "std::vector<double>",    &m_eclClusterPx);
  m_tree->Branch("eclClusterPy",         "std::vector<double>",    &m_eclClusterPy);
  m_tree->Branch("eclClusterPz",         "std::vector<double>",    &m_eclClusterPz);
  m_tree->Branch("eclClusterIsTrack",    "std::vector<bool>",       &m_eclClusterIsTrack);
  m_tree->Branch("eclClusterClosestTrackDist",    "std::vector<bool>",       &m_eclClusterClosestTrackDist);
  m_tree->Branch("eclClusterPi0Likel",   "std::vector<double>",    &m_eclClusterPi0Likel);
  m_tree->Branch("eclClusterEtaLikel",   "std::vector<double>",    &m_eclClusterEtaLikel);
  m_tree->Branch("eclClusterDeltaL",     "std::vector<double>",    &m_eclClusterDeltaL);
  m_tree->Branch("eclClusterBeta",       "std::vector<double>",    &m_eclClusterBeta);


  if (m_doPureCsIStudy == true) {

    m_tree->Branch("eclHitToPureDigit",      "std::vector<int>",       &m_eclHitToPureDigit);
    m_tree->Branch("eclHitToPureDigitAmp",      "std::vector<int>",       &m_eclHitToPureDigitAmp);

    m_tree->Branch("eclPureDigitMultip",     &m_eclPureDigitMultip,         "ecdigit_Multip/I");
    m_tree->Branch("eclPureDigitIdx",        "std::vector<int>",         &m_eclPureDigitIdx);
    m_tree->Branch("eclPureDigitToMC",      "std::vector<int>",          &m_eclPureDigitToMc);
    m_tree->Branch("eclPureDigitCellId",     "std::vector<int>",         &m_eclPureDigitCellId);
    m_tree->Branch("eclPureDigitAmp",        "std::vector<int>",         &m_eclPureDigitAmp);
    m_tree->Branch("eclPureDigitTimeFit",    "std::vector<int>",         &m_eclPureDigitTimeFit);
    m_tree->Branch("eclPureDigitFitQuality",    "std::vector<int>",         &m_eclPureDigitFitQuality);
    m_tree->Branch("eclPureDigitToCluster",        "std::vector<int>",         &m_eclPureDigitToCluster);

    m_tree->Branch("eclPureClusterMultip",     &m_eclPureClusterMultip,     "eclPureClusterMultip/I");
    m_tree->Branch("eclPureClusterIdx",     "std::vector<int>",       &m_eclPureClusterIdx);
    m_tree->Branch("eclPureClusterToMC",      "std::vector<int>",          &m_eclPureClusterToMc);
    m_tree->Branch("eclPureClusterEnergy",     "std::vector<double>",    &m_eclPureClusterEnergy);
    m_tree->Branch("eclPureClusterEnergyError",  "std::vector<double>",    &m_eclPureClusterEnergyError);
    m_tree->Branch("eclPureClusterTheta",      "std::vector<double>",    &m_eclPureClusterTheta);
    m_tree->Branch("eclPureClusterThetaError",   "std::vector<double>",    &m_eclPureClusterThetaError);
    m_tree->Branch("eclPureClusterPhi",        "std::vector<double>",    &m_eclPureClusterPhi);
    m_tree->Branch("eclPureClusterPhiError",     "std::vector<double>",    &m_eclPureClusterPhiError);
    m_tree->Branch("eclPureClusterR",          "std::vector<double>",    &m_eclPureClusterR);
    m_tree->Branch("eclPureClusterEnergyDepSum",   "std::vector<double>",    &m_eclPureClusterEnergyDepSum);
    m_tree->Branch("eclPureClusterTiming",     "std::vector<double>",    &m_eclPureClusterTiming);
    m_tree->Branch("eclPureClusterTimingError",  "std::vector<double>",    &m_eclPureClusterTimingError);
    m_tree->Branch("eclPureClusterE9oE25",     "std::vector<double>",    &m_eclPureClusterE9oE25);
    m_tree->Branch("eclPureClusterHighestE",   "std::vector<double>",    &m_eclPureClusterHighestE);
    m_tree->Branch("eclPureClusterLat",        "std::vector<double>",    &m_eclPureClusterLat);
    m_tree->Branch("eclPureClusterNofCrystals",   "std::vector<int>",       &m_eclPureClusterNofCrystals);
    m_tree->Branch("eclPureClusterCrystalHealth", "std::vector<int>",       &m_eclPureClusterCrystalHealth);
    m_tree->Branch("eclPureClusterMergedPi0",  "std::vector<double>",    &m_eclPureClusterMergedPi0);
    m_tree->Branch("eclPureClusterPx",         "std::vector<double>",    &m_eclPureClusterPx);
    m_tree->Branch("eclPureClusterPy",         "std::vector<double>",    &m_eclPureClusterPy);
    m_tree->Branch("eclPureClusterPz",         "std::vector<double>",    &m_eclPureClusterPz);
    m_tree->Branch("eclPureClusterIsTrack",    "std::vector<bool>",       &m_eclPureClusterIsTrack);
    m_tree->Branch("eclPureClusterPi0Likel",   "std::vector<double>",    &m_eclPureClusterPi0Likel);
    m_tree->Branch("eclPureClusterEtaLikel",   "std::vector<double>",    &m_eclPureClusterEtaLikel);
    m_tree->Branch("eclPureClusterDeltaL",     "std::vector<double>",    &m_eclPureClusterDeltaL);
    m_tree->Branch("eclPureClusterBeta",       "std::vector<double>",    &m_eclPureClusterBeta);
  }

  ///SHOWERS

  m_tree->Branch("eclShowerMultip",     &m_eclShowerMultip,     "eclShowerMultip/I");
  m_tree->Branch("eclShowerIdx",     "std::vector<int>",       &m_eclShowerIdx);
  m_tree->Branch("eclShowerToMc1",      "std::vector<int>",       &m_eclShowerToMc1);
  m_tree->Branch("eclShowerToMcWeight1",      "std::vector<double>",       &m_eclShowerToMcWeight1);
  m_tree->Branch("eclShowerToMc1PDG",      "std::vector<int>",       &m_eclShowerToMc1PDG);
  m_tree->Branch("eclShowerToMc2",      "std::vector<int>",       &m_eclShowerToMc2);
  m_tree->Branch("eclShowerToMcWeight2",      "std::vector<double>",       &m_eclShowerToMcWeight2);
  m_tree->Branch("eclShowerToMc2PDG",      "std::vector<int>",       &m_eclShowerToMc2PDG);
  m_tree->Branch("eclShowerToMc3",      "std::vector<int>",       &m_eclShowerToMc3);
  m_tree->Branch("eclShowerToMcWeight3",      "std::vector<double>",       &m_eclShowerToMcWeight3);
  m_tree->Branch("eclShowerToMc3PDG",      "std::vector<int>",       &m_eclShowerToMc3PDG);
  m_tree->Branch("eclShowerToMc4",      "std::vector<int>",       &m_eclShowerToMc4);
  m_tree->Branch("eclShowerToMcWeight4",      "std::vector<double>",       &m_eclShowerToMcWeight4);
  m_tree->Branch("eclShowerToMc4PDG",      "std::vector<int>",       &m_eclShowerToMc4PDG);
  m_tree->Branch("eclShowerToMc5",      "std::vector<int>",       &m_eclShowerToMc5);
  m_tree->Branch("eclShowerToMcWeight5",      "std::vector<double>",       &m_eclShowerToMcWeight5);
  m_tree->Branch("eclShowerToMc5PDG",      "std::vector<int>",       &m_eclShowerToMc5PDG);
  m_tree->Branch("eclShowerToBkgWeight",      "std::vector<double>",       &m_eclShowerToBkgWeight);
  m_tree->Branch("eclShowerSimHitSum",      "std::vector<double>",       &m_eclShowerSimHitSum);
  m_tree->Branch("eclShowerEnergy",     "std::vector<double>",    &m_eclShowerEnergy);
  m_tree->Branch("eclShowerUncEnergy",  "std::vector<double>",    &m_eclShowerUncEnergy);
  m_tree->Branch("eclShowerTheta",      "std::vector<double>",    &m_eclShowerTheta);
  m_tree->Branch("eclShowerPhi",        "std::vector<double>",    &m_eclShowerPhi);
  m_tree->Branch("eclShowerR",          "std::vector<double>",    &m_eclShowerR);
  m_tree->Branch("eclShowerNHits",   "std::vector<double>",       &m_eclShowerNHits);
  m_tree->Branch("eclShowerE9oE25",     "std::vector<double>",    &m_eclShowerE9oE25);
  m_tree->Branch("eclShowerTime",     "std::vector<double>",    &m_eclShowerTime);
  //new
  m_tree->Branch("eclShowerConnectedRegionId",     "std::vector<int>",    &m_eclShowerConnectedRegionId);
  m_tree->Branch("eclShowerHypothesisId",     "std::vector<int>",    &m_eclShowerHypothesisId);
  m_tree->Branch("eclShowerCentralCellId",     "std::vector<int>",    &m_eclShowerCentralCellId);
  m_tree->Branch("eclShowerEnergyError",     "std::vector<double>",    &m_eclShowerEnergyError);
  m_tree->Branch("eclShowerThetaError",     "std::vector<double>",    &m_eclShowerThetaError);
  m_tree->Branch("eclShowerPhiError",     "std::vector<double>",    &m_eclShowerPhiError);
  m_tree->Branch("eclShowerTimeResolution",     "std::vector<double>",    &m_eclShowerTimeResolution);
  m_tree->Branch("eclShowerHighestEnergy",     "std::vector<double>",    &m_eclShowerHighestEnergy);
  m_tree->Branch("eclShowerLateralEnergy",     "std::vector<double>",    &m_eclShowerLateralEnergy);
  m_tree->Branch("eclShowerMinTrkDistance",     "std::vector<double>",    &m_eclShowerMinTrkDistance);
  m_tree->Branch("eclShowerTrkDepth",     "std::vector<double>",    &m_eclShowerTrkDepth);
  m_tree->Branch("eclShowerShowerDepth",     "std::vector<double>",    &m_eclShowerShowerDepth);
  m_tree->Branch("eclShowerAbsZernike20",     "std::vector<double>",    &m_eclShowerAbsZernike20);
  m_tree->Branch("eclShowerAbsZernike40",     "std::vector<double>",    &m_eclShowerAbsZernike40);
  m_tree->Branch("eclShowerAbsZernike42",     "std::vector<double>",    &m_eclShowerAbsZernike42);
  m_tree->Branch("eclShowerAbsZernike51",     "std::vector<double>",    &m_eclShowerAbsZernike51);
  m_tree->Branch("eclShowerAbsZernike53",     "std::vector<double>",    &m_eclShowerAbsZernike53);
  m_tree->Branch("eclShowerSecondMoment",     "std::vector<double>",    &m_eclShowerSecondMoment);
  m_tree->Branch("eclShowerE1oE9",     "std::vector<double>",    &m_eclShowerE1oE9);
  m_tree->Branch("eclShowerIsTrack",     "std::vector<int>",    &m_eclShowerIsTrack);
  m_tree->Branch("eclShowerIsCluster",     "std::vector<bool>",    &m_eclShowerIsCluster);


  m_tree->Branch("mcMultip",     &m_mcMultip,           "mcMultip/I");
  m_tree->Branch("mcIdx",        "std::vector<int>",    &m_mcIdx);
  m_tree->Branch("mcPdg",        "std::vector<int>",    &m_mcPdg);
  m_tree->Branch("mcMothPdg",    "std::vector<int>",    &m_mcMothPdg);
  m_tree->Branch("mcGMothPdg",   "std::vector<int>",    &m_mcGMothPdg);
  m_tree->Branch("mcGGMothPdg",  "std::vector<int>",    &m_mcGGMothPdg);
  m_tree->Branch("mcEnergy",     "std::vector<double>", &m_mcEnergy);
  m_tree->Branch("mcPx",         "std::vector<double>", &m_mcPx);
  m_tree->Branch("mcPy",         "std::vector<double>", &m_mcPy);
  m_tree->Branch("mcPz",         "std::vector<double>", &m_mcPz);
  m_tree->Branch("mcDecVtxx",    "std::vector<double>", &m_mcDecayVtxX);
  m_tree->Branch("mcDecVtxy",    "std::vector<double>", &m_mcDecayVtxY);
  m_tree->Branch("mcDecVtxz",    "std::vector<double>", &m_mcDecayVtxZ);
  m_tree->Branch("mcProdVtxx",   "std::vector<double>", &m_mcProdVtxX);
  m_tree->Branch("mcProdVtxy",   "std::vector<double>", &m_mcProdVtxY);
  m_tree->Branch("mcProdVtxz",   "std::vector<double>", &m_mcProdVtxZ);
  m_tree->Branch("mcSecProc",    "std::vector<int>",    &m_mcSecondaryPhysProc);

  if (m_doTracking == true) {
    m_tree->Branch("trkMultip",     &m_trkMultip,          "trkMulti/I");
    m_tree->Branch("trkIdx",     "std::vector<int>",       &m_trkIdx);
    m_tree->Branch("trkPdg",        "std::vector<int>",    &m_trkPdg);
    m_tree->Branch("trkCharge",        "std::vector<int>",    &m_trkCharge);
    m_tree->Branch("trkPx",         "std::vector<double>", &m_trkPx);
    m_tree->Branch("trkPy",         "std::vector<double>", &m_trkPy);
    m_tree->Branch("trkPz",         "std::vector<double>", &m_trkPz);
    m_tree->Branch("trkP",         "std::vector<double>", &m_trkP);
    m_tree->Branch("trkTheta",         "std::vector<double>", &m_trkTheta);
    m_tree->Branch("trkPhi",         "std::vector<double>", &m_trkPhi);
    m_tree->Branch("trkPosx",       "std::vector<double>", &m_trkX);
    m_tree->Branch("trkPosy",       "std::vector<double>", &m_trkY);
    m_tree->Branch("trkPosz",      "std::vector<double>",  &m_trkZ);

    m_tree->Branch("eclpidtrkIdx",     "std::vector<int>",  &m_eclpidtrkIdx);
    m_tree->Branch("eclpidEnergy",     "std::vector<double>",  &m_eclpidEnergy);
    m_tree->Branch("eclpidEop",        "std::vector<double>",  &m_eclpidEop);
    m_tree->Branch("eclpidE9E25",      "std::vector<double>",  &m_eclpidE9E25);
    m_tree->Branch("eclpidNCrystals",  "std::vector<int>",  &m_eclpidNCrystals);
    m_tree->Branch("eclpidNClusters",  "std::vector<int>",  &m_eclpidNClusters);
    m_tree->Branch("eclLogLikeEl",      "std::vector<double>",  &m_eclLogLikeEl);
    m_tree->Branch("eclLogLikeMu",      "std::vector<double>",  &m_eclLogLikeMu);
    m_tree->Branch("eclLogLikePi",      "std::vector<double>",  &m_eclLogLikePi);
  }

  B2INFO("[ECLDataAnalysis Module]: Initialization of ECLDataAnalysis Module completed.");

}

void ECLDataAnalysisModule::beginRun()
{
}


void ECLDataAnalysisModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ ECLDataAnalysisModule");

  //I HAD TO COMMENT ALL THE FOLLOWING, OTHERWISE IT CAUSES RUN-TIME CRASHES B.O. 20160406
  // re-initialize vars
  //m_eclTriggerMultip=0; m_eclTriggerCellId->clear();  m_eclTriggerTime->clear();   m_eclTriggerIdx->clear();

  ///Digits
  m_eclDigitMultip = 0; m_eclDigitIdx->clear(); m_eclDigitToMc->clear(); m_eclDigitCellId->clear(); m_eclDigitAmp->clear();
  m_eclDigitTimeFit->clear(); m_eclDigitFitQuality->clear(); m_eclDigitToShower->clear();
  m_eclDigitToCalDigit->clear(); //m_eclDigitToHit->clear();

  ///CalDigits
  m_eclCalDigitMultip = 0; m_eclCalDigitCellId->clear(); m_eclCalDigitAmp->clear(); m_eclCalDigitTimeFit->clear();
  m_eclCalDigitFitQuality->clear(); m_eclCalDigitIdx->clear();
  m_eclCalDigitToMc1->clear(); m_eclCalDigitToMcWeight1->clear(); m_eclCalDigitToMc2->clear();
  m_eclCalDigitToMcWeight2->clear(); m_eclCalDigitToMc3->clear(); m_eclCalDigitToMcWeight3->clear(); m_eclCalDigitToMc4->clear();
  m_eclCalDigitToMcWeight4->clear(); m_eclCalDigitToMc5->clear(); m_eclCalDigitToMcWeight5->clear();
  m_eclCalDigitToMc1PDG->clear();   m_eclCalDigitToMc2PDG->clear();   m_eclCalDigitToMc3PDG->clear();  m_eclCalDigitToMc4PDG->clear();
  m_eclCalDigitToMc5PDG->clear();
  m_eclCalDigitToBkgWeight->clear();  m_eclCalDigitSimHitSum->clear();
  m_eclCalDigitToShower->clear();//m_eclDigitToHit->clear();
  m_eclCalDigitToCR->clear();

  m_eclCRIdx->clear();
  m_eclCRIsTrack->clear();
  m_eclCRLikelihoodMIPNGamma->clear();
  m_eclCRLikelihoodChargedHadron->clear();
  m_eclCRLikelihoodElectronNGamma->clear();
  m_eclCRLikelihoodNGamma->clear();
  m_eclCRLikelihoodNeutralHadron->clear();
  m_eclCRLikelihoodMergedPi0->clear();

  //SimHit
  m_eclSimHitMultip = 0; m_eclSimHitCellId->clear(); m_eclSimHitPdg->clear(); m_eclSimHitEnergyDep->clear();
  m_eclSimHitFlightTime->clear(); m_eclSimHitIdx->clear(); m_eclSimHitToMc->clear(); m_eclSimHitX->clear(); m_eclSimHitY->clear();
  m_eclSimHitZ->clear(); m_eclSimHitPx->clear(); m_eclSimHitPy->clear(); m_eclSimHitPz->clear();

  ///Hit
  m_eclHitMultip = 0; m_eclHitCellId->clear(); m_eclHitEnergyDep->clear(); m_eclHitTimeAve->clear(); m_eclHitIdx->clear();
  m_eclHitToMc->clear(); m_eclHitToDigit->clear(); m_eclHitToDigitAmp->clear();

  ///Clusters
  m_eclClusterMultip = 0; m_eclClusterEnergy->clear(); m_eclClusterEnergyError->clear(); m_eclClusterTheta->clear();
  m_eclClusterThetaError->clear(); m_eclClusterPhi->clear(); m_eclClusterPhiError->clear(); m_eclClusterR->clear();
  m_eclClusterIdx->clear(); m_eclClusterToMc1->clear(); m_eclClusterToMcWeight1->clear(); m_eclClusterToMc1PDG->clear();
  m_eclClusterToMc2->clear();
  m_eclClusterToMcWeight2->clear(); m_eclClusterToMc2PDG->clear(); m_eclClusterToMc3->clear(); m_eclClusterToMcWeight3->clear();
  m_eclClusterToMc3PDG->clear(); m_eclClusterToMc4->clear(); m_eclClusterToMcWeight4->clear(); m_eclClusterToMc4PDG->clear();
  m_eclClusterToMc5->clear(); m_eclClusterToMcWeight5->clear(); m_eclClusterToMc5PDG->clear(); m_eclClusterToBkgWeight->clear();
  m_eclClusterSimHitSum->clear(); m_eclClusterEnergyDepSum->clear();  m_eclClusterTiming->clear();  m_eclClusterTimingError->clear();
  m_eclClusterE9oE25->clear();  m_eclClusterHighestE->clear();  m_eclClusterLat->clear(); m_eclClusterNofCrystals->clear();
  m_eclClusterCrystalHealth->clear();  m_eclClusterMergedPi0->clear(); m_eclClusterPx->clear();  m_eclClusterPy->clear();
  m_eclClusterPz->clear();  m_eclClusterIsTrack->clear(); m_eclClusterClosestTrackDist->clear(); m_eclClusterPi0Likel->clear();
  m_eclClusterEtaLikel->clear();
  m_eclClusterDeltaL->clear();  m_eclClusterBeta->clear(); m_eclClusterToShower->clear();

  ///Showers
  m_eclShowerMultip = 0; m_eclShowerIdx->clear(); m_eclShowerToMc1->clear(); m_eclShowerToMcWeight1->clear();
  m_eclShowerToMc1PDG->clear(); m_eclShowerToMc2->clear();
  m_eclShowerToMcWeight2->clear(); m_eclShowerToMc2PDG->clear(); m_eclShowerToMc3->clear(); m_eclShowerToMcWeight3->clear();
  m_eclShowerToMc3PDG->clear(); m_eclShowerToMc4->clear(); m_eclShowerToMcWeight4->clear(); m_eclShowerToMc4PDG->clear();
  m_eclShowerToMc5->clear(); m_eclShowerToMcWeight5->clear(); m_eclShowerToMc5PDG->clear(); m_eclShowerToBkgWeight->clear();
  m_eclShowerSimHitSum->clear();
  m_eclShowerUncEnergy->clear(); m_eclShowerEnergy->clear();
  m_eclShowerTheta->clear(); m_eclShowerPhi->clear(); m_eclShowerR->clear(); m_eclShowerNHits->clear(); m_eclShowerE9oE25->clear();
  m_eclShowerTime->clear();  m_eclShowerConnectedRegionId->clear();
  m_eclShowerHypothesisId->clear(); m_eclShowerCentralCellId->clear(); m_eclShowerEnergyError->clear();
  m_eclShowerThetaError->clear(); m_eclShowerPhiError->clear(); m_eclShowerTimeResolution->clear(); m_eclShowerHighestEnergy->clear();
  m_eclShowerLateralEnergy->clear(); m_eclShowerMinTrkDistance->clear(); m_eclShowerTrkDepth->clear();
  m_eclShowerShowerDepth->clear(); m_eclShowerAbsZernike20->clear(); m_eclShowerAbsZernike40->clear();
  m_eclShowerAbsZernike42->clear(); m_eclShowerAbsZernike51->clear(); m_eclShowerAbsZernike53->clear();
  m_eclShowerSecondMoment->clear(); m_eclShowerE1oE9->clear(); m_eclShowerIsTrack->clear(); m_eclShowerIsCluster->clear();

  ///Pure Digits
  if (m_doPureCsIStudy == true) {
    m_eclPureDigitMultip = 0;

    m_eclPureDigitIdx->clear();  m_eclPureDigitToMc->clear(); m_eclPureDigitCellId->clear(); m_eclPureDigitAmp->clear();
    m_eclPureDigitTimeFit->clear();  m_eclPureDigitFitQuality->clear(); m_eclPureDigitToCluster->clear();
    m_eclHitToPureDigit->clear();  m_eclHitToPureDigitAmp->clear();

    ///Pure Clusters
    m_eclPureClusterMultip = 0; m_eclPureClusterToMc->clear(); m_eclPureClusterEnergy->clear(); m_eclPureClusterEnergyError->clear();
    m_eclPureClusterTheta->clear();
    m_eclPureClusterThetaError->clear(); m_eclPureClusterPhi->clear(); m_eclPureClusterPhiError->clear(); m_eclPureClusterR->clear();
    m_eclPureClusterIdx->clear(); m_eclPureClusterTiming->clear(); m_eclPureClusterTimingError->clear();
    m_eclPureClusterE9oE25->clear(); m_eclPureClusterHighestE->clear(); m_eclPureClusterLat->clear();
    m_eclPureClusterNofCrystals->clear(); m_eclPureClusterCrystalHealth->clear(); m_eclPureClusterMergedPi0->clear();
    m_eclPureClusterPx->clear(); m_eclPureClusterPy->clear(); m_eclPureClusterPz->clear(); m_eclPureClusterIsTrack->clear();
    m_eclPureClusterPi0Likel->clear(); m_eclPureClusterEtaLikel->clear(); m_eclPureClusterDeltaL->clear();
    m_eclPureClusterBeta->clear();
  }

  ///MC
  m_mcMultip = 0; m_mcIdx->clear();  m_mcPdg->clear();  m_mcMothPdg->clear();  m_mcGMothPdg->clear();  m_mcGGMothPdg->clear();
  m_mcEnergy->clear();  m_mcPx->clear();  m_mcPy->clear();  m_mcPz->clear(); m_mcDecayVtxX->clear();  m_mcDecayVtxY->clear();
  m_mcDecayVtxZ->clear();  m_mcProdVtxX->clear();  m_mcProdVtxY->clear(); m_mcProdVtxZ->clear(); m_mcSecondaryPhysProc->clear();

  ///Tracks
  if (m_doTracking == true) {
    m_trkMultip = 0; m_trkIdx->clear(); m_trkPdg->clear(); m_trkCharge->clear(); m_trkPx->clear(); m_trkPy->clear(); m_trkPz->clear();
    m_trkP->clear();
    m_trkPhi->clear(); m_trkTheta->clear(); m_trkPhi->clear(); m_trkX->clear(); m_trkY->clear();  m_trkZ->clear();

    ///PID
    m_eclpidtrkIdx->clear(); m_eclpidEnergy->clear(); m_eclpidEop->clear(); m_eclpidE9E25->clear(); m_eclpidNCrystals->clear();
    m_eclpidNClusters->clear(); m_eclLogLikeEl->clear(); m_eclLogLikeMu->clear(); m_eclLogLikePi->clear();
  }

  StoreObjPtr<EventMetaData> eventmetadata;
  if (eventmetadata) {
    m_iExperiment = eventmetadata->getExperiment();
    m_iRun = eventmetadata->getRun();
    m_iEvent = eventmetadata->getEvent();
  } else {
    m_iExperiment = -1;
    m_iRun = -1;
    m_iEvent = -1;
  }

  //StoreArray<ECLTrig> trgs;
  StoreArray<ECLDigit> digits;
  StoreArray<ECLCalDigit> caldigits;
  StoreArray<ECLSimHit> simhits;
  StoreArray<ECLHit> hits;
  StoreArray<ECLShower> showers;
  StoreArray<ECLCluster> clusters;
  StoreArray<MCParticle> mcParticles;
  StoreArray<ECLConnectedRegion> cr;
  RelationArray ECLClusterToMC(clusters, mcParticles);
  RelationArray ECLShowerToMC(showers, mcParticles);
  RelationArray ECLHitsToDigit(hits, digits);
  RelationArray ECLHitsToMC(hits, mcParticles);
  RelationArray ECLCalDigitToMC(caldigits, mcParticles);
  RelationArray ECLCalDigitToCR(caldigits, cr);

  /* //IS THIS REALLY NEEDED?
    if (m_doPureCsIStudy == true) {
      StoreArray<ECLDigit> pure_digits(m_pure_digits);
      RelationArray ECLHitsToDigit(hits, pure_digits);
    }
  */

  /* TRIGGER, NOT YET IMPLEMENTED
  m_eclTriggerMultip=trgs.getEntries();
  for (int itrgs = 0; itrgs < trgs.getEntries() ; itrgs++){
    ECLTrig* aECLTrigs = trgs[itrgs];
    m_eclTriggerIdx->push_back(itrgs);
    m_eclTriggerCellId->push_back(aECLTrigs->getCellId());
    m_eclTriggerTime->push_back(aECLTrigs->getTimeTrig());
  }
  */

  //DIGITS
  m_eclDigitMultip = digits.getEntries();
  for (int idigits = 0; idigits < digits.getEntries() ; idigits++) {
    ECLDigit* aECLDigits = digits[idigits];
    m_eclDigitIdx->push_back(idigits);
    m_eclDigitCellId->push_back(aECLDigits->getCellId());
    m_eclDigitAmp->push_back(aECLDigits->getAmp());
    m_eclDigitTimeFit->push_back(aECLDigits->getTimeFit());
    m_eclDigitFitQuality->push_back(aECLDigits->getQuality());

    if (aECLDigits->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_digit = aECLDigits->getRelated<MCParticle>();
      m_eclDigitToMc->push_back(mc_digit->getArrayIndex());
    } else
      m_eclDigitToMc->push_back(-1);

    if (aECLDigits->getRelated<ECLShower>() != (nullptr)) {
      const ECLShower* shower_digit = aECLDigits->getRelated<ECLShower>();
      m_eclDigitToShower->push_back(shower_digit->getArrayIndex());
    } else
      m_eclDigitToShower->push_back(-1);

    if (aECLDigits->getRelated<ECLCalDigit>() != (nullptr)) {
      const ECLCalDigit* cal_digit = aECLDigits->getRelated<ECLCalDigit>();
      m_eclDigitToCalDigit->push_back(cal_digit->getArrayIndex());
    } else
      m_eclDigitToCalDigit->push_back(-1);
  }

  //CAL DIGITS
  m_eclCalDigitMultip = caldigits.getEntries();
  for (uint icaldigits = 0; icaldigits < (uint)caldigits.getEntries() ; icaldigits++) {
    ECLCalDigit* aECLCalDigits = caldigits[icaldigits];

    m_eclCalDigitIdx->push_back(icaldigits);
    m_eclCalDigitCellId->push_back(aECLCalDigits->getCellId());
    m_eclCalDigitAmp->push_back(aECLCalDigits->getEnergy());
    m_eclCalDigitTimeFit->push_back(aECLCalDigits->getTime());
    m_eclCalDigitFitQuality->push_back(aECLCalDigits->isFailedFit());

    double sumHit = 0;
    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    int ii = 0;
    sumHit = 0;
    int mc_idx = 0;

    for (int i = 0; i < ECLCalDigitToMC.getEntries(); i++) {
      mc_idx++;
      const RelationElement re = ECLCalDigitToMC[i];
      if (re.getFromIndex() == icaldigits) {
        idx[ii] = i;
        ii++;
        cout << "Digit: " << icaldigits << " Index: " << idx[i] << endl;
        sumHit = sumHit + re.getWeight();
        //  cout << "Energy: " << sumHit << endl;
      }
    }

    //Re-ordering based on contribution
    int y = 0;
    while (y < 10) {
      for (int i = 0; i < 10; i++) {
        if (((idx[i]) > -1) && ((idx[i + 1]) > -1)) {
          if (ECLCalDigitToMC[idx[i]].getWeight() < ECLCalDigitToMC[idx[i + 1]].getWeight()) {
            int temp = idx[i];
            idx[i] = idx[i + 1];
            idx[i + 1] = temp;
          }
        }
      }
      y++;
    }

    m_eclCalDigitToBkgWeight->push_back(aECLCalDigits->getEnergy() - sumHit);
    m_eclCalDigitSimHitSum->push_back(sumHit);
    if (idx[0] > -1) {
      m_eclCalDigitToMcWeight1->push_back(ECLCalDigitToMC[idx[0]].getWeight());
      m_eclCalDigitToMc1->push_back(ECLCalDigitToMC[idx[0]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLCalDigitToMC[idx[0]].getToIndex()];
      m_eclCalDigitToMc1PDG->push_back(amcParticle->getPDG());
      cout << "Index: " << ECLCalDigitToMC[idx[0]].getToIndex() << " PDG: " << amcParticle->getPDG() << " Energy: " <<
           ECLCalDigitToMC[idx[0]].getWeight() << endl;
    } else {
      m_eclCalDigitToMcWeight1->push_back(-1);
      m_eclCalDigitToMc1->push_back(-1);
      m_eclCalDigitToMc1PDG->push_back(-1);
    }
    if (idx[1] > -1) {
      m_eclCalDigitToMcWeight2->push_back(ECLCalDigitToMC[idx[1]].getWeight());
      m_eclCalDigitToMc2->push_back(ECLCalDigitToMC[idx[1]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLCalDigitToMC[idx[1]].getToIndex()];
      m_eclCalDigitToMc2PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMcWeight2->push_back(-1);
      m_eclCalDigitToMc2->push_back(-1);
      m_eclCalDigitToMc2PDG->push_back(-1);
    }
    if (idx[2] > -1) {
      m_eclCalDigitToMcWeight3->push_back(ECLCalDigitToMC[idx[2]].getWeight());
      m_eclCalDigitToMc3->push_back(ECLCalDigitToMC[idx[2]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLCalDigitToMC[idx[2]].getToIndex()];
      m_eclCalDigitToMc3PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMcWeight3->push_back(-1);
      m_eclCalDigitToMc3->push_back(-1);
      m_eclCalDigitToMc3PDG->push_back(-1);
    }
    if (idx[3] > -1) {
      m_eclCalDigitToMcWeight4->push_back(ECLCalDigitToMC[idx[3]].getWeight());
      m_eclCalDigitToMc4->push_back(ECLCalDigitToMC[idx[3]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLCalDigitToMC[idx[3]].getToIndex()];
      m_eclCalDigitToMc4PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMcWeight4->push_back(-1);
      m_eclCalDigitToMc4->push_back(-1);
      m_eclCalDigitToMc4PDG->push_back(-1);
    }
    if (idx[4] > -1) {
      m_eclCalDigitToMcWeight5->push_back(ECLCalDigitToMC[idx[4]].getWeight());
      m_eclCalDigitToMc5->push_back(ECLCalDigitToMC[idx[4]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLCalDigitToMC[idx[4]].getToIndex()];
      m_eclCalDigitToMc5PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclCalDigitToMcWeight5->push_back(-1);
      m_eclCalDigitToMc5->push_back(-1);
      m_eclCalDigitToMc5PDG->push_back(-1);
    }

    if (aECLCalDigits->getRelated<ECLShower>() != (nullptr)) {
      const ECLShower* shower_caldigit = aECLCalDigits->getRelated<ECLShower>();
      m_eclCalDigitToShower->push_back(shower_caldigit->getArrayIndex());
    } else
      m_eclCalDigitToShower->push_back(-1);

    if (aECLCalDigits->getRelated<ECLConnectedRegion>() != (nullptr)) {
      const ECLConnectedRegion* cr_caldigit = aECLCalDigits->getRelated<ECLConnectedRegion>();
      m_eclCalDigitToCR->push_back(cr_caldigit->getCRId());
    } else
      m_eclCalDigitToCR->push_back(-1);

  }


  //CR
  int CRmultip = cr.getEntries();
  for (int i = 0; i < CRmultip; i++) {
    ECLConnectedRegion* aECLCR = cr[i];
    m_eclCRIdx->push_back(aECLCR->getCRId());
    m_eclCRIsTrack->push_back(aECLCR->getIsTrack());
    m_eclCRLikelihoodMIPNGamma->push_back(aECLCR->getLikelihoodMIPNGamma());
    m_eclCRLikelihoodChargedHadron->push_back(aECLCR->getLikelihoodChargedHadron());
    m_eclCRLikelihoodElectronNGamma->push_back(aECLCR->getLikelihoodElectronNGamma());
    m_eclCRLikelihoodNGamma->push_back(aECLCR->getLikelihoodNGamma());
    m_eclCRLikelihoodNeutralHadron->push_back(aECLCR->getLikelihoodNeutralHadron());
    m_eclCRLikelihoodMergedPi0->push_back(aECLCR->getLikelihoodMergedPi0());
  }

  //SIM HITS
  m_eclSimHitMultip = simhits.getEntries();
  for (int isimhits = 0; isimhits < simhits.getEntries() ; isimhits++) {
    ECLSimHit* aECLSimHits = simhits[isimhits];

    m_eclSimHitIdx->push_back(isimhits);
    m_eclSimHitCellId->push_back(aECLSimHits->getCellId());
    m_eclSimHitPdg->push_back(aECLSimHits->getPDGCode());
    m_eclSimHitEnergyDep->push_back(aECLSimHits->getEnergyDep());
    m_eclSimHitFlightTime->push_back(aECLSimHits->getFlightTime());
    m_eclSimHitX->push_back(aECLSimHits->getPosition().X());
    m_eclSimHitY->push_back(aECLSimHits->getPosition().Y());
    m_eclSimHitZ->push_back(aECLSimHits->getPosition().Z());
    m_eclSimHitPx->push_back(aECLSimHits->getMomentum().X());
    m_eclSimHitPy->push_back(aECLSimHits->getMomentum().Y());
    m_eclSimHitPz->push_back(aECLSimHits->getMomentum().Z());

    if (aECLSimHits->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_simhit = aECLSimHits->getRelated<MCParticle>();
      m_eclSimHitToMc->push_back(mc_simhit->getArrayIndex());
    } else
      m_eclSimHitToMc->push_back(-1);
  }

  //HITS
  m_eclHitMultip = hits.getEntries();
  for (int ihits = 0; ihits < hits.getEntries() ; ihits++) {
    ECLHit* aECLHits = hits[ihits];
    m_eclHitIdx->push_back(ihits);
    m_eclHitCellId->push_back(aECLHits->getCellId());
    m_eclHitEnergyDep->push_back(aECLHits->getEnergyDep());
    m_eclHitTimeAve->push_back(aECLHits->getTimeAve());

    if (aECLHits->getRelated<ECLDigit>() != (nullptr)) {
      //const MCParticle* mc_digit = aECLHits->getRelated<MCParticle>();
      const ECLDigit* hit_digit = aECLHits->getRelated<ECLDigit>();
      m_eclHitToDigit->push_back(hit_digit->getArrayIndex());
      m_eclHitToDigitAmp->push_back(hit_digit->getAmp());
    } else {
      m_eclHitToDigit->push_back(-1);
      m_eclHitToDigitAmp->push_back(-1);
    }

    if (m_doPureCsIStudy == true) {
      StoreArray<ECLDigit> pure_digits(m_pure_digits);
      if (aECLHits->getRelated<ECLDigit>(m_pure_digits) != (nullptr)) {
        const ECLDigit* hit_pdigit = aECLHits->getRelated<ECLDigit>(m_pure_digits);
        m_eclHitToPureDigit->push_back(hit_pdigit->getArrayIndex());
        m_eclHitToPureDigitAmp->push_back(hit_pdigit->getAmp());
      } else {
        m_eclHitToPureDigit->push_back(-1);
        m_eclHitToPureDigitAmp->push_back(-1);
      }
    }

    if (aECLHits->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_hit = aECLHits->getRelated<MCParticle>();
      m_eclHitToMc->push_back(mc_hit->getArrayIndex());
    } else
      m_eclHitToMc->push_back(-1);
  }

  //CLUSTERS
  m_eclClusterMultip = clusters.getEntries();
  for (unsigned int iclusters = 0; iclusters < (unsigned int)clusters.getEntries() ; iclusters++) {
    ECLCluster* aECLClusters = clusters[iclusters];
    m_eclClusterIdx->push_back(iclusters);
    m_eclClusterEnergy->push_back(aECLClusters->getEnergy());
    m_eclClusterEnergyError->push_back(aECLClusters->getErrorEnergy());
    m_eclClusterTheta->push_back(aECLClusters->getTheta());
    m_eclClusterThetaError->push_back(aECLClusters->getErrorTheta());
    m_eclClusterPhi->push_back(aECLClusters->getPhi());
    m_eclClusterPhiError->push_back(aECLClusters->getErrorPhi());
    m_eclClusterR->push_back(aECLClusters->getR());
    m_eclClusterEnergyDepSum->push_back(aECLClusters->getEnedepSum());
    m_eclClusterTiming->push_back(aECLClusters->getTiming());
    m_eclClusterTimingError->push_back(aECLClusters->getErrorTiming());
    m_eclClusterE9oE25->push_back(aECLClusters->getE9oE25());
    m_eclClusterHighestE->push_back(aECLClusters->getHighestE());
    m_eclClusterLat->push_back(aECLClusters->getLAT());
    m_eclClusterNofCrystals->push_back(aECLClusters->getNofCrystals());
    m_eclClusterCrystalHealth->push_back(aECLClusters->getCrystHealth());
    m_eclClusterMergedPi0->push_back(aECLClusters->getMergedPi0());
    m_eclClusterPx->push_back(aECLClusters->getPx());
    m_eclClusterPy->push_back(aECLClusters->getPy());
    m_eclClusterPz->push_back(aECLClusters->getPz());
    m_eclClusterIsTrack->push_back(aECLClusters->getisTrack());
    m_eclClusterClosestTrackDist->push_back(aECLClusters->getTemporaryMinTrkDistance());
    m_eclClusterPi0Likel->push_back(aECLClusters->getpi0Likelihood());
    m_eclClusterEtaLikel->push_back(aECLClusters->getetaLikelihood());
    m_eclClusterDeltaL->push_back(aECLClusters->getdeltaL());
    m_eclClusterBeta->push_back(aECLClusters->getbeta());

    if (aECLClusters->getRelated<ECLShower>() != (nullptr)) {
      const ECLShower* shower_cluster = aECLClusters->getRelated<ECLShower>();
      m_eclClusterToShower->push_back(shower_cluster->getArrayIndex());
    } else
      m_eclClusterToShower->push_back(-1);


    //Dump MC Info - Multiple Matching
    double sumHit = 0;
    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    int ii = 0;
    sumHit = 0;
    int mc_idx = 0;
    for (int i = 0; i < ECLClusterToMC.getEntries(); i++) {
      mc_idx++;
      const RelationElement re = ECLClusterToMC[i];
      if (re.getFromIndex() == iclusters) {
        idx[ii] = i;
        ii++;
        sumHit = sumHit + re.getWeight();
      }
    }

    //Re-ordering based on contribution
    int y = 0;
    while (y < 10) {
      for (int i = 0; i < 10; i++) {
        if (((idx[i]) > -1) && ((idx[i + 1]) > -1)) {
          if (ECLClusterToMC[idx[i]].getWeight() < ECLClusterToMC[idx[i + 1]].getWeight()) {
            int temp = idx[i];
            idx[i] = idx[i + 1];
            idx[i + 1] = temp;
          }
        }
      }
      y++;
    }
    m_eclClusterToBkgWeight->push_back(aECLClusters->getEnergy() - sumHit);
    m_eclClusterSimHitSum->push_back(sumHit);
    if (idx[0] > -1) {
      m_eclClusterToMcWeight1->push_back(ECLClusterToMC[idx[0]].getWeight());
      m_eclClusterToMc1->push_back(ECLClusterToMC[idx[0]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLClusterToMC[idx[0]].getToIndex()];
      m_eclClusterToMc1PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMcWeight1->push_back(-1);
      m_eclClusterToMc1->push_back(-1);
      m_eclClusterToMc1PDG->push_back(-1);
    }
    if (idx[1] > -1) {
      m_eclClusterToMcWeight2->push_back(ECLClusterToMC[idx[1]].getWeight());
      m_eclClusterToMc2->push_back(ECLClusterToMC[idx[1]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLClusterToMC[idx[1]].getToIndex()];
      m_eclClusterToMc2PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMcWeight2->push_back(-1);
      m_eclClusterToMc2->push_back(-1);
      m_eclClusterToMc2PDG->push_back(-1);
    }
    if (idx[2] > -1) {
      m_eclClusterToMcWeight3->push_back(ECLClusterToMC[idx[2]].getWeight());
      m_eclClusterToMc3->push_back(ECLClusterToMC[idx[2]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLClusterToMC[idx[2]].getToIndex()];
      m_eclClusterToMc3PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMcWeight3->push_back(-1);
      m_eclClusterToMc3->push_back(-1);
      m_eclClusterToMc3PDG->push_back(-1);
    }
    if (idx[3] > -1) {
      m_eclClusterToMcWeight4->push_back(ECLClusterToMC[idx[3]].getWeight());
      m_eclClusterToMc4->push_back(ECLClusterToMC[idx[3]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLClusterToMC[idx[3]].getToIndex()];
      m_eclClusterToMc4PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMcWeight4->push_back(-1);
      m_eclClusterToMc4->push_back(-1);
      m_eclClusterToMc4PDG->push_back(-1);
    }
    if (idx[4] > -1) {
      m_eclClusterToMcWeight5->push_back(ECLClusterToMC[idx[4]].getWeight());
      m_eclClusterToMc5->push_back(ECLClusterToMC[idx[4]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLClusterToMC[idx[4]].getToIndex()];
      m_eclClusterToMc5PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclClusterToMcWeight5->push_back(-1);
      m_eclClusterToMc5->push_back(-1);
      m_eclClusterToMc5PDG->push_back(-1);
    }

  }

  if (m_doPureCsIStudy == true) {

    //StoreArray<ECLDigit> pure_digits(m_pure_digits);
    StoreArray<ECLDigit> pure_digits(m_pure_digits);
    StoreArray<ECLCalDigit> pure_cal_digits(m_pure_cal_digits);
    StoreArray<ECLCluster> pure_clusters(m_pure_clusters);
    pure_digits.requireRelationTo(mcParticles);
    pure_cal_digits.requireRelationTo(pure_clusters);
    //RelationArray ECLPureDigitToMc(pure_digits, mcParticles);
    //RelationArray ECLHitsToDigit(hits, pure_digits);
    //RelationArray ECLHitsToPureDigit(hits, pure_digits);
    //RelationArray ECLPureDigitToCluster(pure_digits, pure_clusters);

    m_eclPureDigitMultip = pure_digits.getEntries();
    for (int idigits = 0; idigits < pure_digits.getEntries() ; idigits++) {
      ECLDigit* aECLPureDigits = pure_digits[idigits];

      m_eclPureDigitIdx->push_back(idigits);
      m_eclPureDigitCellId->push_back(aECLPureDigits->getCellId());
      m_eclPureDigitAmp->push_back(aECLPureDigits->getAmp());
      m_eclPureDigitTimeFit->push_back(aECLPureDigits->getTimeFit());
      m_eclPureDigitFitQuality->push_back(aECLPureDigits->getQuality());

      ECLCalDigit* aECLPureCalDigits = pure_cal_digits[idigits];

      if (aECLPureCalDigits->getRelated<ECLCluster>(m_pure_clusters) != (nullptr)) {
        //StoreArray<ECLC> pure_digits(m_pure_digits);
        const ECLCluster* cluster_digit = aECLPureCalDigits->getRelated<ECLCluster>(m_pure_clusters);
        //const ECLShower* cluster_digit = aECLPureDigits->getRelated<ECLShower>;//(m_pure_clusters);
        m_eclPureDigitToCluster->push_back(cluster_digit->getArrayIndex());
      } else
        m_eclPureDigitToCluster->push_back(-1);

      if (aECLPureDigits->getRelated<MCParticle>() != (nullptr)) {
        //cout << "PureDigit has non-nullptr" << endl;
        const MCParticle* mc_digit = aECLPureDigits->getRelated<MCParticle>();
        m_eclPureDigitToMc->push_back(mc_digit->getArrayIndex());
      } else
        m_eclPureDigitToMc->push_back(-1);
    }

    m_eclPureClusterMultip = pure_clusters.getEntries();
    for (unsigned int iclusters = 0; iclusters < (unsigned int)pure_clusters.getEntries() ; iclusters++) {
      ECLCluster* aECLClusters = pure_clusters[iclusters];
      m_eclPureClusterIdx->push_back(iclusters);
      m_eclPureClusterEnergy->push_back(aECLClusters->getEnergy());
      m_eclPureClusterEnergyError->push_back(aECLClusters->getErrorEnergy());
      m_eclPureClusterTheta->push_back(aECLClusters->getTheta());
      m_eclPureClusterThetaError->push_back(aECLClusters->getErrorTheta());
      m_eclPureClusterPhi->push_back(aECLClusters->getPhi());
      m_eclPureClusterPhiError->push_back(aECLClusters->getErrorPhi());
      m_eclPureClusterR->push_back(aECLClusters->getR());
      m_eclPureClusterEnergyDepSum->push_back(aECLClusters->getEnedepSum());
      m_eclPureClusterTiming->push_back(aECLClusters->getTiming());
      m_eclPureClusterTimingError->push_back(aECLClusters->getErrorTiming());
      m_eclPureClusterE9oE25->push_back(aECLClusters->getE9oE25());
      m_eclPureClusterHighestE->push_back(aECLClusters->getHighestE());
      m_eclPureClusterLat->push_back(aECLClusters->getLAT());
      m_eclPureClusterNofCrystals->push_back(aECLClusters->getNofCrystals());
      m_eclPureClusterCrystalHealth->push_back(aECLClusters->getCrystHealth());
      m_eclPureClusterMergedPi0->push_back(aECLClusters->getMergedPi0());
      m_eclPureClusterPx->push_back(aECLClusters->getPx());
      m_eclPureClusterPy->push_back(aECLClusters->getPy());
      m_eclPureClusterPz->push_back(aECLClusters->getPz());
      m_eclPureClusterIsTrack->push_back(aECLClusters->getisTrack());
      m_eclPureClusterPi0Likel->push_back(aECLClusters->getpi0Likelihood());
      m_eclPureClusterEtaLikel->push_back(aECLClusters->getetaLikelihood());
      m_eclPureClusterDeltaL->push_back(aECLClusters->getdeltaL());
      m_eclPureClusterBeta->push_back(aECLClusters->getbeta());

      if (aECLClusters->getRelated<MCParticle>() != (nullptr)) {
        //cout << "PureDigit has non-nullptr" << endl;
        const MCParticle* mc_cluster = aECLClusters->getRelated<MCParticle>();
        m_eclPureClusterToMc->push_back(mc_cluster->getArrayIndex());
      } else
        m_eclPureClusterToMc->push_back(-1);
    }
  }

  m_eclShowerMultip = showers.getEntries();
  for (uint ishowers = 0; ishowers < (uint)showers.getEntries() ; ishowers++) {
    ECLShower* aECLShowers = showers[ishowers];
    m_eclShowerIdx->push_back(ishowers);
    m_eclShowerEnergy->push_back(aECLShowers->getEnergy());
    m_eclShowerTheta->push_back(aECLShowers->getTheta());
    m_eclShowerPhi->push_back(aECLShowers->getPhi());
    m_eclShowerR->push_back(aECLShowers->getR());
    m_eclShowerNHits->push_back(aECLShowers->getNofCrystals());
    m_eclShowerE9oE25->push_back(aECLShowers->getE9oE25());
    m_eclShowerUncEnergy->push_back(aECLShowers->getEnedepSum());
    m_eclShowerTime->push_back(aECLShowers->getTime());
    m_eclShowerConnectedRegionId->push_back(aECLShowers->getConnectedRegionId());
    m_eclShowerHypothesisId->push_back(aECLShowers->getHypothesisId());
    m_eclShowerCentralCellId->push_back(aECLShowers->getCentralCellId());
    m_eclShowerEnergyError->push_back(aECLShowers->getEnergyError());
    m_eclShowerThetaError->push_back(aECLShowers->getThetaError());
    m_eclShowerPhiError->push_back(aECLShowers->getPhiError());
    m_eclShowerTimeResolution->push_back(aECLShowers->getTimeResolution());
    m_eclShowerHighestEnergy->push_back(aECLShowers->getHighestEnergy());
    m_eclShowerLateralEnergy->push_back(aECLShowers->getLateralEnergy());
    m_eclShowerMinTrkDistance->push_back(aECLShowers->getMinTrkDistance());
    m_eclShowerTrkDepth->push_back(aECLShowers->getTrkDepth());
    m_eclShowerShowerDepth->push_back(aECLShowers->getShowerDepth());
    m_eclShowerAbsZernike20->push_back(aECLShowers->getAbsZernike20());
    m_eclShowerAbsZernike40->push_back(aECLShowers->getAbsZernike40());
    m_eclShowerAbsZernike42->push_back(aECLShowers->getAbsZernike42());
    m_eclShowerAbsZernike51->push_back(aECLShowers->getAbsZernike51());
    m_eclShowerAbsZernike53->push_back(aECLShowers->getAbsZernike53());
    m_eclShowerSecondMoment->push_back(aECLShowers->getSecondMoment());
    m_eclShowerE1oE9->push_back(aECLShowers->getE1oE9());

    double sumHit = 0;
    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    int ii = 0;
    sumHit = 0;
    int mc_idx = 0;
    for (int i = 0; i < ECLShowerToMC.getEntries(); i++) {
      mc_idx++;
      const RelationElement re = ECLShowerToMC[i];
      if (re.getFromIndex() == ishowers) {
        idx[ii] = i;
        ii++;
        sumHit = sumHit + re.getWeight();
      }
    }

    //Re-ordering based on contribution
    int y = 0;
    while (y < 10) {
      for (int i = 0; i < 10; i++) {
        if (((idx[i]) > -1) && ((idx[i + 1]) > -1)) {
          if (ECLShowerToMC[idx[i]].getWeight() < ECLShowerToMC[idx[i + 1]].getWeight()) {
            int temp = idx[i];
            idx[i] = idx[i + 1];
            idx[i + 1] = temp;
          }
        }
      }
      y++;
    }
    m_eclShowerToBkgWeight->push_back(aECLShowers->getEnergy() - sumHit);
    m_eclShowerSimHitSum->push_back(sumHit);
    if (idx[0] > -1) {
      m_eclShowerToMcWeight1->push_back(ECLShowerToMC[idx[0]].getWeight());
      m_eclShowerToMc1->push_back(ECLShowerToMC[idx[0]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLShowerToMC[idx[0]].getToIndex()];
      m_eclShowerToMc1PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMcWeight1->push_back(-1);
      m_eclShowerToMc1->push_back(-1);
      m_eclShowerToMc1PDG->push_back(-1);
    }
    if (idx[1] > -1) {
      m_eclShowerToMcWeight2->push_back(ECLShowerToMC[idx[1]].getWeight());
      m_eclShowerToMc2->push_back(ECLShowerToMC[idx[1]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLShowerToMC[idx[1]].getToIndex()];
      m_eclShowerToMc2PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMcWeight2->push_back(-1);
      m_eclShowerToMc2->push_back(-1);
      m_eclShowerToMc2PDG->push_back(-1);
    }
    if (idx[2] > -1) {
      m_eclShowerToMcWeight3->push_back(ECLShowerToMC[idx[2]].getWeight());
      m_eclShowerToMc3->push_back(ECLShowerToMC[idx[2]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLShowerToMC[idx[2]].getToIndex()];
      m_eclShowerToMc3PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMcWeight3->push_back(-1);
      m_eclShowerToMc3->push_back(-1);
      m_eclShowerToMc3PDG->push_back(-1);
    }
    if (idx[3] > -1) {
      m_eclShowerToMcWeight4->push_back(ECLShowerToMC[idx[3]].getWeight());
      m_eclShowerToMc4->push_back(ECLShowerToMC[idx[3]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLShowerToMC[idx[3]].getToIndex()];
      m_eclShowerToMc4PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMcWeight4->push_back(-1);
      m_eclShowerToMc4->push_back(-1);
      m_eclShowerToMc4PDG->push_back(-1);
    }
    if (idx[4] > -1) {
      m_eclShowerToMcWeight5->push_back(ECLShowerToMC[idx[4]].getWeight());
      m_eclShowerToMc5->push_back(ECLShowerToMC[idx[4]].getToIndex());
      MCParticle* amcParticle = mcParticles[ECLShowerToMC[idx[4]].getToIndex()];
      m_eclShowerToMc5PDG->push_back(amcParticle->getPDG());
    } else {
      m_eclShowerToMcWeight5->push_back(-1);
      m_eclShowerToMc5->push_back(-1);
      m_eclShowerToMc5PDG->push_back(-1);
    }

    if (aECLShowers->getRelated<ECLCluster>() != (nullptr)) {
      m_eclShowerIsCluster->push_back(1);
      const ECLCluster* cluster_shower = aECLShowers->getRelated<ECLCluster>();
      m_eclShowerIsTrack->push_back(cluster_shower->getisTrack());
    } else {
      m_eclShowerIsCluster->push_back(0);
      m_eclShowerIsTrack->push_back(-1);
    }

  }

  m_mcMultip = mcParticles.getEntries();
  for (int imcpart = 0; imcpart < mcParticles.getEntries(); imcpart++) {
    MCParticle* amcParticle = mcParticles[imcpart];
    m_mcIdx->push_back(amcParticle->getArrayIndex());
    m_mcPdg->push_back(amcParticle->getPDG());
    if (amcParticle->getMother() != NULL) m_mcMothPdg->push_back(amcParticle->getMother()->getPDG());
    else m_mcMothPdg->push_back(-999);
    if (amcParticle->getMother() != NULL
        && amcParticle->getMother()->getMother() != NULL) m_mcGMothPdg->push_back(amcParticle->getMother()->getMother()->getPDG());
    else m_mcGMothPdg->push_back(-999);
    if (amcParticle->getMother() != NULL && amcParticle->getMother()->getMother() != NULL
        && amcParticle->getMother()->getMother()->getMother() != NULL)
      m_mcGGMothPdg->push_back(amcParticle->getMother()->getMother()->getMother()->getPDG());
    else m_mcGGMothPdg->push_back(-999);
    m_mcEnergy->push_back(amcParticle->getEnergy());
    m_mcSecondaryPhysProc->push_back(amcParticle->getSecondaryPhysicsProcess());

    m_mcPx->push_back(amcParticle->getMomentum().X());
    m_mcPy->push_back(amcParticle->getMomentum().Y());
    m_mcPz->push_back(amcParticle->getMomentum().Z());

    m_mcDecayVtxX->push_back(amcParticle->getDecayVertex().X());
    m_mcDecayVtxY->push_back(amcParticle->getDecayVertex().Y());
    m_mcDecayVtxZ->push_back(amcParticle->getDecayVertex().Z());

    m_mcProdVtxX->push_back(amcParticle->getProductionVertex().X());
    m_mcProdVtxY->push_back(amcParticle->getProductionVertex().Y());
    m_mcProdVtxZ->push_back(amcParticle->getProductionVertex().Z());

  }

  if (m_doTracking == true) {
    StoreArray<TrackFitResult> trks;
    StoreArray<Track> tracks;
    m_trkMultip = 0;
    for (const Track& itrk : tracks) {
      const TrackFitResult* atrk = itrk.getTrackFitResult(Const::pion);
      if (atrk == nullptr) continue;

      m_trkIdx->push_back(m_trkMultip);
      m_trkPdg->push_back(atrk->getParticleType().getPDGCode());
      m_trkCharge->push_back(atrk->getChargeSign());
      m_trkPx->push_back(atrk->getMomentum().X());
      m_trkPy->push_back(atrk->getMomentum().Y());
      m_trkPz->push_back(atrk->getMomentum().Z());
      m_trkP->push_back(atrk->getMomentum().Mag());
      m_trkTheta->push_back(atrk->getMomentum().Theta());
      m_trkPhi->push_back(atrk->getMomentum().Phi());
      m_trkX->push_back(atrk->getPosition().X());
      m_trkY->push_back(atrk->getPosition().Y());
      m_trkZ->push_back(atrk->getPosition().Z());

      const ECLPidLikelihood* eclpid = itrk.getRelatedTo<ECLPidLikelihood>() ;

      if (eclpid != nullptr) {
        m_eclpidtrkIdx -> push_back(m_trkMultip);
        m_eclpidEnergy -> push_back(eclpid-> energy());
        m_eclpidEop    -> push_back(eclpid-> eop());
        m_eclpidE9E25  -> push_back(eclpid-> e9e25());
        m_eclpidNCrystals -> push_back(eclpid-> nCrystals());
        m_eclpidNClusters -> push_back(eclpid-> nClusters());
        m_eclLogLikeEl -> push_back(eclpid-> getLogLikelihood(Const::electron));
        m_eclLogLikeMu -> push_back(eclpid-> getLogLikelihood(Const::muon));
        m_eclLogLikePi -> push_back(eclpid-> getLogLikelihood(Const::pion));
      } else {
        m_eclpidtrkIdx -> push_back(m_trkMultip);
        m_eclpidEnergy -> push_back(0);
        m_eclpidEop    -> push_back(0);
        m_eclpidE9E25  -> push_back(0);
        m_eclpidNCrystals -> push_back(0);
        m_eclpidNClusters -> push_back(0);
        m_eclLogLikeEl -> push_back(0);
        m_eclLogLikeMu -> push_back(0);
        m_eclLogLikePi -> push_back(0);
      }
      m_trkMultip++;
    }
  }

  m_tree->Fill();
}


void ECLDataAnalysisModule::endRun()
{
}


void ECLDataAnalysisModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_tree->Write();
  }

}

int factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

