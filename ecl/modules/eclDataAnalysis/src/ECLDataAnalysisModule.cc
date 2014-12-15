/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Elisa Manoni, Benjamin Oberhof                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclDataAnalysis/ECLDataAnalysisModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>
#include <framework/logging/Logger.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <ecl/dataobjects/ECLDigit.h>
#include <ecl/dataobjects/ECLDsp.h>
#include <ecl/dataobjects/ECLGamma.h>
#include <ecl/dataobjects/ECLHit.h>
#include <ecl/dataobjects/ECLPi0.h>
#include <ecl/dataobjects/ECLShower.h>
#include <ecl/dataobjects/ECLSimHit.h>
//#include <ecl/dataobjects/ECLTrig.h>
#include <mdst/dataobjects/Track.h>
#include <list>
#include <iostream>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLDataAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLDataAnalysisModule::ECLDataAnalysisModule()
  : Module()
{
  //Set module properties
  setDescription("This module produces an ntuple with ECL-related quantities starting from mdst");

  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));

  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclDataAnalysis"));

  addParam("doTracking", m_doTracking,
           "set true if you want to save the informations from TrackFitResults'rootFileName'", bool(true));

}

ECLDataAnalysisModule::~ECLDataAnalysisModule()
{
}


void ECLDataAnalysisModule::initialize()
{

  B2INFO("[ECLDataAnalysis Module]: Starting initialization of ECLDataAnalysis Module.");

  //StoreArray<ECLTrig>::required();
  StoreArray<ECLDigit>::required();
  StoreArray<ECLSimHit>::required();
  StoreArray<ECLHit>::required();
  StoreArray<ECLShower>::required();
  StoreArray<ECLCluster>::required();
  StoreArray<ECLGamma>::required();
  StoreArray<ECLPi0>::required();
  StoreArray<MCParticle>::required();
  if (m_doTracking == true) StoreArray<TrackFitResult>::required();
  //  StoreArray<ECLDsp>::required();

  if (m_writeToRoot == true) {
    m_rootFileName += ".root";
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

  m_eclDigitMultip = 0;
  m_eclDigitIdx = new std::vector<int>();
  m_eclDigitToMc = new std::vector<int>();
  m_eclDigitCellId = new std::vector<int>();
  m_eclDigitAmp = new std::vector<int>();
  m_eclDigitTimeFit = new std::vector<int>();
  m_eclDigitFitQuality = new std::vector<int>();

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
  m_eclHitCellId = new std::vector<int>();
  m_eclHitEnergyDep = new std::vector<double>();
  m_eclHitTimeAve = new std::vector<double>();

  m_eclShowerMultip = 0;
  m_eclShowerIdx = new std::vector<int>();
  m_eclShowerToMc = new std::vector<int>();
  m_eclShowerToGamma = new std::vector<int>();
  m_eclShowerEnergy = new std::vector<double>();
  m_eclShowerTheta = new std::vector<double>();
  m_eclShowerPhi = new std::vector<double>();
  m_eclShowerR = new std::vector<double>();
  m_eclShowerNHits = new std::vector<int>();
  m_eclShowerE9oE25 = new std::vector<double>();
  m_eclShowerUncEnergy = new std::vector<double>();

  m_eclClusterMultip = 0;
  m_eclClusterIdx = new std::vector<int>();
  m_eclClusterToMc = new std::vector<int>();
  m_eclClusterToShower = new std::vector<int>();
  m_eclClusterToTrack = new std::vector<int>();
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


  m_eclGammaMultip = 0;
  m_eclGammaIdx = new std::vector<int>();
  m_eclGammaEnergy = new std::vector<double>();
  m_eclGammaTheta = new std::vector<double>();
  m_eclGammaPhi = new std::vector<double>();
  m_eclGammaR = new std::vector<double>();
  m_eclGammaPx = new std::vector<double>();
  m_eclGammaPy = new std::vector<double>();
  m_eclGammaPz = new std::vector<double>();

  m_eclPi0Multip = 0;
  m_eclPi0Idx = new std::vector<int>();
  m_eclPi0ToGamma = new std::vector<int>();
  m_eclPi0ShowerId1 = new std::vector<int>();
  m_eclPi0ShowerId2 = new std::vector<int>();
  m_eclPi0Energy = new std::vector<double>();
  m_eclPi0Px = new std::vector<double>();
  m_eclPi0Py = new std::vector<double>();
  m_eclPi0Pz = new std::vector<double>();
  m_eclPi0Mass = new std::vector<double>();
  m_eclPi0MassFit = new std::vector<double>();
  m_eclPi0Chi2 = new std::vector<double>();
  m_eclPi0PValue = new std::vector<double>();

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
  m_trkPx = new std::vector<double>();
  m_trkPy = new std::vector<double>();
  m_trkPz = new std::vector<double>();
  m_trkX = new std::vector<double>();
  m_trkY = new std::vector<double>();
  m_trkZ = new std::vector<double>();


  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  /*m_tree->Branch("eclTriggerMultip",     &m_eclTriggerMultip,         "eclTriggerMultip/I");
  m_tree->Branch("eclTriggerIdx",     "std::vector<int>",       &m_eclTriggerIdx);
  m_tree->Branch("eclTriggerCellId",     "std::vector<int>",       &m_eclTriggerCellId);
  m_tree->Branch("eclTriggerTime",       "std::vector<double>",    &m_eclTriggerTime);*/

  m_tree->Branch("eclDigitMultip",     &m_eclDigitMultip,         "ecdigit_Multip/I");
  m_tree->Branch("eclDigitIdx",        "std::vector<int>",         &m_eclDigitIdx);
  m_tree->Branch("eclDigitToMC",      "std::vector<int>",          &m_eclDigitToMc);
  m_tree->Branch("eclDigitCellId",     "std::vector<int>",         &m_eclDigitCellId);
  m_tree->Branch("eclDigitAmp",        "std::vector<int>",         &m_eclDigitAmp);
  m_tree->Branch("eclDigitTimeFit",    "std::vector<int>",         &m_eclDigitTimeFit);
  m_tree->Branch("eclDigitFitQuality",    "std::vector<int>",         &m_eclDigitFitQuality);

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
  m_tree->Branch("eclHitCellId",     "std::vector<int>",    &m_eclHitCellId);
  m_tree->Branch("eclHitEnergyDep",      "std::vector<double>", &m_eclHitEnergyDep);
  m_tree->Branch("eclHitTimeAve",       "std::vector<double>", &m_eclHitTimeAve);

  m_tree->Branch("eclShowerMultip",     &m_eclShowerMultip,      "eclShowerMultip/I");
  m_tree->Branch("eclShowerToGamma",      "std::vector<int>",       &m_eclShowerToGamma);
  m_tree->Branch("eclShowerIdx",     "std::vector<int>",       &m_eclShowerIdx);
  m_tree->Branch("eclShowerToMc",      "std::vector<int>",       &m_eclShowerToMc);
  m_tree->Branch("eclShowerEnergy",     "std::vector<double>",    &m_eclShowerEnergy);
  m_tree->Branch("eclShowerTheta",      "std::vector<double>",    &m_eclShowerTheta);
  m_tree->Branch("eclShowerPhi",        "std::vector<double>",    &m_eclShowerPhi);
  m_tree->Branch("eclShowerR",          "std::vector<double>",    &m_eclShowerR);
  m_tree->Branch("eclShowerNhits",      "std::vector<int>",       &m_eclShowerNHits);
  m_tree->Branch("eclShowerE9oE25",     "std::vector<double>",    &m_eclShowerE9oE25);
  m_tree->Branch("eclShowerUncEnergy",  "std::vector<double>",    &m_eclShowerUncEnergy);

  m_tree->Branch("eclClusterMultip",     &m_eclClusterMultip,     "eclClusterMultip/I");
  m_tree->Branch("eclClusterIdx",     "std::vector<int>",       &m_eclClusterIdx);
  m_tree->Branch("eclClusterToMc",      "std::vector<int>",       &m_eclClusterToMc);
  m_tree->Branch("eclClusterToShower",      "std::vector<int>",       &m_eclClusterToShower);
  m_tree->Branch("eclClusterToTrack",      "std::vector<int>",       &m_eclClusterToTrack);
  m_tree->Branch("eclClusterEnergy",     "std::vector<double>",    &m_eclClusterEnergy);
  m_tree->Branch("eclClusterEnergyError",  "std::vector<double>",    &m_eclClusterEnergyError);
  m_tree->Branch("eclClusterTheta",      "std::vector<double>",    &m_eclClusterTheta);
  m_tree->Branch("eclClusterThetaError",   "std::vector<double>",    &m_eclClusterThetaError);
  m_tree->Branch("eclClusterPhi",        "std::vector<double>",    &m_eclClusterPhi);
  m_tree->Branch("eclClusterPhiError",     "std::vector<double>",    &m_eclClusterPhiError);
  m_tree->Branch("eclClusterr",          "std::vector<double>",    &m_eclClusterR);
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
  m_tree->Branch("eclClusterPi0Likel",   "std::vector<double>",    &m_eclClusterPi0Likel);
  m_tree->Branch("eclClusterEtaLikel",   "std::vector<double>",    &m_eclClusterEtaLikel);
  m_tree->Branch("eclClusterDeltaL",     "std::vector<double>",    &m_eclClusterDeltaL);
  m_tree->Branch("eclClusterBeta",       "std::vector<double>",    &m_eclClusterBeta);

  m_tree->Branch("eclGammaMultip",     &m_eclGammaMultip,      "eclGammaMultip/I");
  m_tree->Branch("eclGammaIdx",     "std::vector<int>",       &m_eclGammaIdx);

  m_tree->Branch("eclGammaEnergy",     "std::vector<double>",    &m_eclGammaEnergy);
  m_tree->Branch("eclGammaTheta",      "std::vector<double>",    &m_eclGammaTheta);
  m_tree->Branch("eclGammaPhi",        "std::vector<double>",    &m_eclGammaPhi);
  m_tree->Branch("eclGammar",          "std::vector<double>",    &m_eclGammaR);
  m_tree->Branch("eclGammaPx",         "std::vector<double>",    &m_eclGammaPx);
  m_tree->Branch("eclGammaPy",         "std::vector<double>",    &m_eclGammaPy);
  m_tree->Branch("eclGammaPz",         "std::vector<double>",    &m_eclGammaPz);

  m_tree->Branch("eclPi0Multip",     &m_eclPi0Multip,      "eclPi0Multip/I");
  m_tree->Branch("eclPi0Idx",     "std::vector<int>",       &m_eclPi0Idx);
  m_tree->Branch("eclPi0ToGamma",      "std::vector<int>",       &m_eclPi0ToGamma);
  m_tree->Branch("eclPi0ShowerId1",  "std::vector<int>",    &m_eclPi0ShowerId1);
  m_tree->Branch("eclPi0ShowerId2",  "std::vector<int>",    &m_eclPi0ShowerId2);
  m_tree->Branch("eclPi0Energy",     "std::vector<double>", &m_eclPi0Energy);
  m_tree->Branch("eclPi0Px",         "std::vector<double>", &m_eclPi0Px);
  m_tree->Branch("eclPi0Py",         "std::vector<double>", &m_eclPi0Py);
  m_tree->Branch("eclPi0Pz",         "std::vector<double>", &m_eclPi0Pz);
  m_tree->Branch("eclPi0Mass",       "std::vector<double>", &m_eclPi0Mass);
  m_tree->Branch("eclPi0MassFit",    "std::vector<double>", &m_eclPi0MassFit);
  m_tree->Branch("eclPi0Chi2",       "std::vector<double>", &m_eclPi0Chi2);
  m_tree->Branch("eclPi0Pvalue",       "std::vector<double>", &m_eclPi0PValue);

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
    m_tree->Branch("trk_Idx",     "std::vector<int>",       &m_trkIdx);
    m_tree->Branch("trkPdg",        "std::vector<int>",    &m_trkPdg);
    m_tree->Branch("trkPx",         "std::vector<double>", &m_trkPx);
    m_tree->Branch("trkPy",         "std::vector<double>", &m_trkPy);
    m_tree->Branch("trkPz",         "std::vector<double>", &m_trkPz);
    m_tree->Branch("trkPosx",       "std::vector<double>", &m_trkX);
    m_tree->Branch("trkPosy",       "std::vector<double>", &m_trkY);
    m_tree->Branch("trkPosz",      "std::vector<double>",  &m_trkZ);
  }

  B2INFO("[ECLDataAnalysis Module]: Initialization of ECLDataAnalysis Module completed.");


}

void ECLDataAnalysisModule::beginRun()
{
}


void ECLDataAnalysisModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ ECLDataAnalysisModule");

  // re-initialize vars
  /*m_eclTriggerMultip=0;*/  m_eclDigitMultip = 0;  m_eclSimHitMultip = 0;  m_eclHitMultip = 0;
  m_eclShowerMultip = 0;  m_eclClusterMultip = 0;  m_eclGammaMultip = 0;  m_eclPi0Multip = 0;
  m_mcMultip = 0;  m_trkMultip = 0;

  //m_eclTriggerCellId->clear();  m_eclTriggerTime->clear();   m_eclTriggerIdx->clear();

  m_eclDigitCellId->clear();  m_eclDigitAmp->clear();  m_eclDigitTimeFit->clear();  m_eclDigitFitQuality->clear();  m_eclDigitIdx->clear();   m_eclDigitToMc->clear();

  m_eclSimHitCellId->clear(); m_eclSimHitPdg->clear(); m_eclSimHitEnergyDep->clear(); m_eclSimHitFlightTime->clear(); m_eclSimHitIdx->clear();   m_eclSimHitToMc->clear();
  m_eclSimHitX->clear(); m_eclSimHitY->clear(); m_eclSimHitZ->clear();
  m_eclSimHitPx->clear(); m_eclSimHitPy->clear(); m_eclSimHitPz->clear();

  m_eclHitCellId->clear(); m_eclHitEnergyDep->clear(); m_eclHitTimeAve->clear(); m_eclHitIdx->clear();   m_eclHitToMc->clear();

  m_eclShowerToMc->clear();   m_eclShowerToGamma->clear();
  m_eclShowerEnergy->clear(); m_eclShowerTheta->clear();  m_eclShowerPhi->clear(); m_eclShowerIdx->clear();
  m_eclShowerR->clear();  m_eclShowerNHits->clear();  m_eclShowerE9oE25->clear();  m_eclShowerUncEnergy->clear();

  m_eclGammaEnergy->clear();  m_eclGammaTheta->clear();  m_eclGammaPhi->clear();  m_eclGammaR->clear();
  m_eclGammaPx->clear();  m_eclGammaPy->clear();  m_eclGammaPz->clear(); m_eclGammaIdx->clear();

  m_eclPi0ToGamma->clear();
  m_eclPi0ShowerId1->clear(); m_eclPi0ShowerId2->clear(); m_eclPi0Idx->clear();
  m_eclPi0Energy->clear(); m_eclPi0Px->clear(); m_eclPi0Py->clear(); m_eclPi0Pz->clear();
  m_eclPi0Mass->clear(); m_eclPi0MassFit->clear(); m_eclPi0Chi2->clear(); m_eclPi0PValue->clear();

  m_eclClusterEnergy->clear();  m_eclClusterEnergyError->clear();  m_eclClusterTheta->clear();  m_eclClusterThetaError->clear();
  m_eclClusterPhi->clear();  m_eclClusterPhiError->clear();  m_eclClusterR->clear();
  m_eclClusterIdx->clear();  m_eclClusterToMc->clear(); m_eclClusterToShower->clear(); m_eclClusterToTrack->clear();
  m_eclClusterEnergyDepSum->clear();  m_eclClusterTiming->clear();  m_eclClusterTimingError->clear();
  m_eclClusterE9oE25->clear();  m_eclClusterHighestE->clear();  m_eclClusterLat->clear();
  m_eclClusterNofCrystals->clear();  m_eclClusterCrystalHealth->clear();  m_eclClusterMergedPi0->clear();
  m_eclClusterPx->clear();  m_eclClusterPy->clear();  m_eclClusterPz->clear();  m_eclClusterIsTrack->clear();
  m_eclClusterPi0Likel->clear();  m_eclClusterEtaLikel->clear();  m_eclClusterDeltaL->clear();  m_eclClusterBeta->clear();

  m_mcIdx->clear();  m_mcPdg->clear();  m_mcMothPdg->clear();  m_mcGMothPdg->clear();  m_mcGGMothPdg->clear();
  m_mcEnergy->clear();  m_mcPx->clear();  m_mcPy->clear();  m_mcPz->clear();
  m_mcDecayVtxX->clear();  m_mcDecayVtxY->clear();  m_mcDecayVtxZ->clear();  m_mcProdVtxX->clear();  m_mcProdVtxY->clear();  m_mcProdVtxZ->clear();
  m_mcSecondaryPhysProc->clear();

  m_trkIdx->clear();
  m_trkPdg->clear();   m_trkPx->clear();  m_trkPy->clear();  m_trkPz->clear();
  m_trkX->clear();  m_trkY->clear();  m_trkZ->clear();

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
  StoreArray<ECLSimHit> simhits;
  StoreArray<ECLHit> hits;
  StoreArray<ECLShower> showers;
  StoreArray<ECLCluster> clusters;
  StoreArray<ECLGamma> gammas;
  StoreArray<ECLPi0> pi0s;
  StoreArray<MCParticle> mcParticles;

  /*
  m_eclTriggerMultip=trgs.getEntries();
  for (int itrgs = 0; itrgs < trgs.getEntries() ; itrgs++){
    ECLTrig* aECLTrigs = trgs[itrgs];

    m_eclTriggerIdx->push_back(itrgs);
    m_eclTriggerCellId->push_back(aECLTrigs->getCellId());
    m_eclTriggerTime->push_back(aECLTrigs->getTimeTrig());
  }
  */

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
  }

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

  m_eclHitMultip = hits.getEntries();
  for (int ihits = 0; ihits < hits.getEntries() ; ihits++) {
    ECLHit* aECLHits = hits[ihits];

    m_eclHitIdx->push_back(ihits);
    m_eclHitCellId->push_back(aECLHits->getCellId());
    m_eclHitEnergyDep->push_back(aECLHits->getEnergyDep());
    m_eclHitTimeAve->push_back(aECLHits->getTimeAve());

    if (aECLHits->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_hit = aECLHits->getRelated<MCParticle>();
      m_eclHitToMc->push_back(mc_hit->getArrayIndex());
    } else
      m_eclHitToMc->push_back(-1);

  }

  m_eclGammaMultip = gammas.getEntries();
  for (int igammas = 0; igammas < gammas.getEntries() ; igammas++) {
    ECLGamma* aECLgammas = gammas[igammas];

    m_eclGammaIdx->push_back(igammas);
    m_eclGammaEnergy->push_back(aECLgammas->getEnergy());
    m_eclGammaPx->push_back(aECLgammas->getPx());
    m_eclGammaPy->push_back(aECLgammas->getPy());
    m_eclGammaPz->push_back(aECLgammas->getPz());
    m_eclGammaTheta->push_back(aECLgammas->getPositon().Theta());
    m_eclGammaPhi->push_back(aECLgammas->getPositon().Phi());
    m_eclGammaR->push_back(aECLgammas->getPositon().Mag());
    /*
    if (aECLgammas->getRelated<ECLShower>() != (nullptr)) {
    const ECLShower* gamma_shower = aECLgammas->getRelated<ECLShower>();
    m_eclShowerToGamma->push_back(gamma_shower->getArrayIndex());
    }
    else
    m_eclShowerToGamma->push_back(-1);
    */
  }

  m_eclPi0Multip = pi0s.getEntries();
  for (int ipi0s = 0; ipi0s < pi0s.getEntries() ; ipi0s++) {
    ECLPi0* aECLPi0s = pi0s[ipi0s];

    m_eclPi0Idx->push_back(ipi0s);
    m_eclPi0ShowerId1->push_back(aECLPi0s->getShowerId1());
    m_eclPi0ShowerId2->push_back(aECLPi0s->getShowerId2());
    m_eclPi0Energy->push_back(aECLPi0s->getEnergy());
    m_eclPi0Px->push_back(aECLPi0s->getPx());
    m_eclPi0Py->push_back(aECLPi0s->getPy());
    m_eclPi0Pz->push_back(aECLPi0s->getPz());
    m_eclPi0Mass->push_back(aECLPi0s->getMass());
    m_eclPi0MassFit->push_back(aECLPi0s->getMassFit());
    m_eclPi0Chi2->push_back(aECLPi0s->getChi2());
    m_eclPi0PValue->push_back(aECLPi0s->getPValue());

    if (aECLPi0s->getRelated<ECLGamma>() != (nullptr)) {
      const ECLGamma* gamma_pi0 = aECLPi0s->getRelated<ECLGamma>();
      m_eclPi0ToGamma->push_back(gamma_pi0->getArrayIndex());
    } else
      m_eclPi0ToGamma->push_back(-1);
  }

  m_eclClusterMultip = clusters.getEntries();
  for (int iclusters = 0; iclusters < clusters.getEntries() ; iclusters++) {
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
    m_eclClusterPi0Likel->push_back(aECLClusters->getpi0Likelihood());
    m_eclClusterEtaLikel->push_back(aECLClusters->getetaLikelihood());
    m_eclClusterDeltaL->push_back(aECLClusters->getdeltaL());
    m_eclClusterBeta->push_back(aECLClusters->getbeta());

    if (aECLClusters->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_cluster = aECLClusters->getRelated<MCParticle>();
      m_eclClusterToMc->push_back(mc_cluster->getArrayIndex());
    } else
      m_eclClusterToMc->push_back(-1);

    if (aECLClusters->getRelated<ECLShower>() != (nullptr)) {
      const ECLShower* shower_cluster = aECLClusters->getRelated<ECLShower>();
      m_eclClusterToShower->push_back(shower_cluster->getArrayIndex());
    } else
      m_eclClusterToShower->push_back(-1);
  }


  m_eclShowerMultip = showers.getEntries();
  for (int ishowers = 0; ishowers < showers.getEntries() ; ishowers++) {
    ECLShower* aECLshowers = showers[ishowers];

    m_eclShowerIdx->push_back(ishowers);
    m_eclShowerEnergy->push_back(aECLshowers->getEnergy());
    m_eclShowerTheta->push_back(aECLshowers->getTheta());
    m_eclShowerPhi->push_back(aECLshowers->getPhi());
    m_eclShowerR->push_back(aECLshowers->getR());
    m_eclShowerNHits->push_back(aECLshowers->getNHits());
    m_eclShowerE9oE25->push_back(aECLshowers->getE9oE25());
    m_eclShowerUncEnergy->push_back(aECLshowers->getUncEnergy());

    if (aECLshowers->getRelated<MCParticle>() != (nullptr)) {
      const MCParticle* mc_shower = aECLshowers->getRelated<MCParticle>();
      m_eclShowerToMc->push_back(mc_shower->getArrayIndex());
    } else
      m_eclShowerToMc->push_back(-1);

    if (aECLshowers->getRelated<ECLGamma>() != (nullptr)) {
      const ECLGamma* gamma_shower = aECLshowers->getRelated<ECLGamma>();
      m_eclShowerToGamma->push_back(gamma_shower->getArrayIndex());
    } else
      m_eclShowerToGamma->push_back(-1);

  }

  m_mcMultip = mcParticles.getEntries();
  for (int imcpart = 0; imcpart < mcParticles.getEntries(); imcpart++) {
    MCParticle* amcParticle = mcParticles[imcpart];

    m_mcIdx->push_back(amcParticle->getArrayIndex());
    m_mcPdg->push_back(amcParticle->getPDG());
    if (amcParticle->getMother() != NULL) m_mcMothPdg->push_back(amcParticle->getMother()->getPDG());
    else m_mcMothPdg->push_back(-999);
    if (amcParticle->getMother() != NULL && amcParticle->getMother()->getMother() != NULL) m_mcGMothPdg->push_back(amcParticle->getMother()->getMother()->getPDG());
    else m_mcGMothPdg->push_back(-999);
    if (amcParticle->getMother() != NULL && amcParticle->getMother()->getMother() != NULL && amcParticle->getMother()->getMother()->getMother() != NULL)
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

    m_trkMultip = trks.getEntries();
    for (int itrks = 0; itrks < trks.getEntries(); itrks++) {
      TrackFitResult* atrk = trks[itrks];

      m_trkIdx->push_back(itrks);
      m_trkPdg->push_back(atrk->getParticleType().getPDGCode());

      m_trkPx->push_back(atrk->getMomentum().X());
      m_trkPy->push_back(atrk->getMomentum().Y());
      m_trkPz->push_back(atrk->getMomentum().Z());

      m_trkX->push_back(atrk->getPosition().X());
      m_trkY->push_back(atrk->getPosition().Y());
      m_trkZ->push_back(atrk->getPosition().Z());

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

