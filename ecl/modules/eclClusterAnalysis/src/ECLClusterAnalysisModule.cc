/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2011 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Elisa Manoni, Benjamin Oberhof                           *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclClusterAnalysis/ECLClusterAnalysisModule.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationArray.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/ECLCluster.h>
#include <mdst/dataobjects/Track.h>

#include <analysis/ClusterUtility/ClusterUtils.h>

#include <list>
#include <iostream>

//Root
#include <TFile.h>
#include <TTree.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLClusterAnalysis)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLClusterAnalysisModule::ECLClusterAnalysisModule()
  : Module(),
    m_rootFilePtr(0),
    m_writeToRoot(1),
    m_doTracking(1),
    m_tree(0),
    m_iExperiment(0),
    m_iRun(0),
    m_iEvent(0),

    m_eclClusterMultip(0),
    m_eclClusterIdx(0),
    m_eclClusterToMc1(0),
    m_eclClusterToMcWeight1(0),
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
    m_eclClusterPx(0),
    m_eclClusterPy(0),
    m_eclClusterPz(0),
    m_eclClusterIsTrack(0),
    m_eclClusterDeltaL(0),
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
    m_eclpidtrkIdx(0), m_eclpidEnergy(0), m_eclpidEop(0), m_eclpidE9E25(0), m_eclpidNCrystals(0), m_eclpidNClusters(0),
    m_eclLogLikeEl(0), m_eclLogLikeMu(0), m_eclLogLikePi(0)
{
  //Set module properties
  setDescription("This module produces an ntuple with ECL-related quantities starting from mdst");

  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'", bool(true));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclClusterAnalysis"));
  addParam("doTracking", m_doTracking,
           "set true if you want to save the informations from TrackFitResults'rootFileName'", bool(true));
}

ECLClusterAnalysisModule::~ECLClusterAnalysisModule()
{
}


void ECLClusterAnalysisModule::initialize()
{

  B2INFO("[ECLClusterAnalysis Module]: Starting initialization of ECLClusterAnalysis Module.");

  m_eclClusters.isRequired();
  m_mcParticles.isRequired();

  if (m_doTracking == true) {
    m_tracks.isRequired();
    m_trackFitResults.isRequired();
  }

  if (m_writeToRoot == true) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else
    m_rootFilePtr = NULL;

  // initialize tree
  m_tree     = new TTree("m_tree", "ECL Analysis tree");

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

  m_tree->Branch("expNo", &m_iExperiment, "expNo/I");
  m_tree->Branch("runNo", &m_iRun, "runNo/I");
  m_tree->Branch("evtNo", &m_iEvent, "evtNo/I");

  m_tree->Branch("eclClusterMultip",     &m_eclClusterMultip,     "eclClusterMultip/I");
  m_tree->Branch("eclClusterIdx",     "std::vector<int>",       &m_eclClusterIdx);
  m_tree->Branch("eclClusterToMc1",      "std::vector<int>",       &m_eclClusterToMc1);
  m_tree->Branch("eclClusterToMcWeight1",      "std::vector<double>",       &m_eclClusterToMcWeight1);
  m_tree->Branch("eclClusterToMc2",      "std::vector<int>",       &m_eclClusterToMc2);
  m_tree->Branch("eclClusterToMcWeight2",      "std::vector<double>",       &m_eclClusterToMcWeight2);
  m_tree->Branch("eclClusterToMc3",      "std::vector<int>",       &m_eclClusterToMc3);
  m_tree->Branch("eclClusterToMcWeight3",      "std::vector<double>",       &m_eclClusterToMcWeight3);
  m_tree->Branch("eclClusterToMc4",      "std::vector<int>",       &m_eclClusterToMc4);
  m_tree->Branch("eclClusterToMcWeight4",      "std::vector<double>",       &m_eclClusterToMcWeight4);
  m_tree->Branch("eclClusterToMc5",      "std::vector<int>",       &m_eclClusterToMc5);
  m_tree->Branch("eclClusterToMcWeight5",      "std::vector<double>",       &m_eclClusterToMcWeight5);
  m_tree->Branch("eclClusterToBkgWeight",      "std::vector<double>",       &m_eclClusterToBkgWeight);
  m_tree->Branch("eclClusterSimHitSum",      "std::vector<double>",       &m_eclClusterSimHitSum);
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
  m_tree->Branch("eclClusterPx",         "std::vector<double>",    &m_eclClusterPx);
  m_tree->Branch("eclClusterPy",         "std::vector<double>",    &m_eclClusterPy);
  m_tree->Branch("eclClusterPz",         "std::vector<double>",    &m_eclClusterPz);
  m_tree->Branch("eclClusterIsTrack",    "std::vector<bool>",       &m_eclClusterIsTrack);
  m_tree->Branch("eclClusterDeltaL",     "std::vector<double>",    &m_eclClusterDeltaL);

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

  B2INFO("[ECLClusterAnalysis Module]: Initialization of ECLClusterAnalysis Module completed.");


}

void ECLClusterAnalysisModule::beginRun()
{
}


void ECLClusterAnalysisModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ ECLClusterAnalysisModule");

  // re-initialize vars
  m_eclClusterMultip = 0;
  m_mcMultip = 0;  m_trkMultip = 0;

  m_eclClusterEnergy->clear();  m_eclClusterEnergyError->clear();  m_eclClusterTheta->clear();  m_eclClusterThetaError->clear();
  m_eclClusterPhi->clear();  m_eclClusterPhiError->clear();  m_eclClusterR->clear();
  m_eclClusterIdx->clear();  m_eclClusterToMc1->clear(); m_eclClusterToMcWeight1->clear(); m_eclClusterToMc2->clear();
  m_eclClusterToMcWeight2->clear(); m_eclClusterToMc3->clear();
  m_eclClusterToMcWeight3->clear(); m_eclClusterToMc4->clear();
  m_eclClusterToMcWeight4->clear(); m_eclClusterToMc5->clear();
  m_eclClusterToMcWeight5->clear(); m_eclClusterToBkgWeight->clear(); m_eclClusterSimHitSum->clear();
  m_eclClusterEnergyDepSum->clear();  m_eclClusterTiming->clear();  m_eclClusterTimingError->clear();
  m_eclClusterE9oE25->clear();  m_eclClusterHighestE->clear();  m_eclClusterLat->clear();
  m_eclClusterNofCrystals->clear();  m_eclClusterCrystalHealth->clear();
  m_eclClusterPx->clear();  m_eclClusterPy->clear();  m_eclClusterPz->clear();  m_eclClusterIsTrack->clear();
  m_eclClusterDeltaL->clear();

  m_mcIdx->clear();  m_mcPdg->clear();  m_mcMothPdg->clear();  m_mcGMothPdg->clear();  m_mcGGMothPdg->clear();
  m_mcEnergy->clear();  m_mcPx->clear();  m_mcPy->clear();  m_mcPz->clear();
  m_mcDecayVtxX->clear();  m_mcDecayVtxY->clear();  m_mcDecayVtxZ->clear();  m_mcProdVtxX->clear();  m_mcProdVtxY->clear();
  m_mcProdVtxZ->clear();
  m_mcSecondaryPhysProc->clear();

  m_trkIdx->clear();
  m_trkPdg->clear();  m_trkCharge->clear();
  m_trkPx->clear();  m_trkPy->clear();  m_trkPz->clear();
  m_trkPhi->clear();  m_trkTheta->clear();  m_trkPhi->clear();
  m_trkX->clear();  m_trkY->clear();  m_trkZ->clear();

  m_eclpidtrkIdx->clear(); m_eclpidEnergy->clear();   m_eclpidEop->clear();   m_eclpidE9E25->clear();   m_eclpidNCrystals->clear();
  m_eclpidNClusters->clear();
  m_eclLogLikeEl->clear();   m_eclLogLikeMu->clear();   m_eclLogLikePi->clear();

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

  RelationArray ECLClusterToMC(m_eclClusters, m_mcParticles);

  m_eclClusterMultip = m_eclClusters.getEntries();
  for (unsigned int iclusters = 0; iclusters < (unsigned int)m_eclClusters.getEntries() ; iclusters++) {
    ECLCluster* aECLClusters = m_eclClusters[iclusters];

    m_eclClusterIdx->push_back(iclusters);
    m_eclClusterEnergy->push_back(aECLClusters->getEnergy());
    m_eclClusterEnergyError->push_back(aECLClusters->getUncertaintyEnergy());
    m_eclClusterTheta->push_back(aECLClusters->getTheta());
    m_eclClusterThetaError->push_back(aECLClusters->getUncertaintyTheta());
    m_eclClusterPhi->push_back(aECLClusters->getPhi());
    m_eclClusterPhiError->push_back(aECLClusters->getUncertaintyPhi());
    m_eclClusterR->push_back(aECLClusters->getR());
    m_eclClusterEnergyDepSum->push_back(aECLClusters->getEnergyRaw());
    m_eclClusterTiming->push_back(aECLClusters->getTime());
    m_eclClusterTimingError->push_back(aECLClusters->getDeltaTime99());
    m_eclClusterE9oE25->push_back(aECLClusters->getE9oE21());
    m_eclClusterHighestE->push_back(aECLClusters->getEnergyHighestCrystal());
    m_eclClusterLat->push_back(aECLClusters->getLAT());
    m_eclClusterNofCrystals->push_back(aECLClusters->getNumberOfCrystals());
    m_eclClusterCrystalHealth->push_back(aECLClusters->getStatus());

    ClusterUtils C;
    TLorentzVector fourmom = C.Get4MomentumFromCluster(aECLClusters);
    m_eclClusterPx->push_back(fourmom.Px());
    m_eclClusterPy->push_back(fourmom.Py());
    m_eclClusterPz->push_back(fourmom.Pz());
    m_eclClusterIsTrack->push_back(aECLClusters->isTrack());
    m_eclClusterDeltaL->push_back(aECLClusters->getDeltaL());

    double sumHit = 0;
    int idx[10];
    for (int i = 0; i < 10; i++)
      idx[i] = -1;

    if (aECLClusters->getRelated<MCParticle>() != (nullptr)) {
      int ii = 0;
      for (int rel = 0; rel < (int)ECLClusterToMC.getEntries(); rel++) {
        if (ECLClusterToMC[rel].getFromIndex() == iclusters) {
          if ((ECLClusterToMC[rel].getWeight() > 0) && (rel < 10)) {
            idx[ii] = rel;
            ii++;
          }
          sumHit = sumHit + (double)ECLClusterToMC[rel].getWeight();
        }
      }
      int max = 0;
      if ((int)ECLClusterToMC.getEntries() > 9)
        max = 9;
      else
        max = (int)ECLClusterToMC.getEntries();

      int y = 0;
      while (y < max) {
        for (int i = 0; i < max; i++) {
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
      m_eclClusterToMc1->push_back(idx[0]);
      if (idx[0] > -1)
        m_eclClusterToMcWeight1->push_back(ECLClusterToMC[idx[0]].getWeight());
      else
        m_eclClusterToMcWeight1->push_back(-1);
      m_eclClusterToMc2->push_back(idx[1]);
      if (idx[1] > -1)
        m_eclClusterToMcWeight2->push_back(ECLClusterToMC[idx[1]].getWeight());
      else
        m_eclClusterToMcWeight2->push_back(-1);
      m_eclClusterToMc3->push_back(idx[2]);
      if (idx[2] > -1)
        m_eclClusterToMcWeight3->push_back(ECLClusterToMC[idx[2]].getWeight());
      else
        m_eclClusterToMcWeight3->push_back(-1);
      m_eclClusterToMc4->push_back(idx[3]);
      if (idx[3] > -1)
        m_eclClusterToMcWeight4->push_back(ECLClusterToMC[idx[3]].getWeight());
      else
        m_eclClusterToMcWeight4->push_back(-1);
      m_eclClusterToMc5->push_back(idx[4]);
      if (idx[4] > -1)
        m_eclClusterToMcWeight5->push_back(ECLClusterToMC[idx[4]].getWeight());
      else
        m_eclClusterToMcWeight5->push_back(-1);
    } else {
      m_eclClusterToMc1->push_back(-1);
      m_eclClusterToMcWeight1->push_back(-1);
      m_eclClusterToMc2->push_back(-1);
      m_eclClusterToMcWeight2->push_back(-1);
      m_eclClusterToMc3->push_back(-1);
      m_eclClusterToMcWeight3->push_back(-1);
      m_eclClusterToMc4->push_back(-1);
      m_eclClusterToMcWeight4->push_back(-1);
      m_eclClusterToMc5->push_back(-1);
      m_eclClusterToMcWeight5->push_back(-1);
      m_eclClusterToBkgWeight->push_back(aECLClusters->getEnergy() - sumHit);
      m_eclClusterSimHitSum->push_back(-1);
    }
  }

  m_mcMultip = m_mcParticles.getEntries();
  for (int imcpart = 0; imcpart < m_mcParticles.getEntries(); imcpart++) {
    MCParticle* amcParticle = m_mcParticles[imcpart];
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
    m_trkMultip = 0;
    for (const Track& itrk : m_tracks) {
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

      m_trkMultip++;
    }
  }

  m_tree->Fill();

}


void ECLClusterAnalysisModule::endRun()
{
}


void ECLClusterAnalysisModule::terminate()
{

  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_tree->Write();
  }

}
