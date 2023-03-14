/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <iostream>
#include <ecl/modules/eclChargedPIDDataAnalysisExpert/ECLChargedPIDDataAnalysisModule.h>
#include <framework/datastore/RelationVector.h>
#include <framework/logging/Logger.h>
#include <framework/gearbox/Const.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <ecl/dataobjects/ECLShower.h>

using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------

REG_MODULE(ECLChargedPIDDataAnalysis);

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

ECLChargedPIDDataAnalysisModule::ECLChargedPIDDataAnalysisModule() :
  Module(),
  m_rootFilePtr(0),
  m_writeToRoot(1),
  m_eclShowers(eclShowerArrayName()),

  // N1 Hypothesis
  n1_tree(0),
  n1_iExperiment(0),
  n1_iRun(0),
  n1_iEvent(0),

  // Shower
  n1_eclShowerMultip(0),
  n1_eclShowerEnergy(0),
  n1_eclShowerTheta(0),
  n1_eclShowerPhi(0),
  n1_eclShowerR(0),
  n1_eclShowerHypothesisId(0),
  n1_eclShowerAbsZernike40(0),
  n1_eclShowerAbsZernike51(0),

  // MC
  n1_mcMultip(0),
  n1_mcPdg(0),
  n1_mcMothPdg(0),
  n1_mcEnergy(0),
  n1_mcP(0),
  n1_mcTheta(0),
  n1_mcPhi(0),

  // Tracks
  n1_trkMultip(0),
  n1_trkPdg(0),
  n1_trkCharge(0),
  n1_trkP(0),
  n1_trkTheta(0),
  n1_trkPhi(0),

  n1_eclEoP(0),

  // N2 Hypothesis
  n2_tree(0),
  n2_iExperiment(0),
  n2_iRun(0),
  n2_iEvent(0),

  // Shower
  n2_eclShowerMultip(0),
  n2_eclShowerEnergy(0),
  n2_eclShowerTheta(0),
  n2_eclShowerPhi(0),
  n2_eclShowerR(0),
  n2_eclShowerHypothesisId(0),
  n2_eclShowerAbsZernike40(0),
  n2_eclShowerAbsZernike51(0),

  // MC
  n2_mcMultip(0),
  n2_mcPdg(0),
  n2_mcMothPdg(0),
  n2_mcEnergy(0),
  n2_mcP(0),
  n2_mcTheta(0),
  n2_mcPhi(0),

  // Tracks
  n2_trkMultip(0),
  n2_trkPdg(0),
  n2_trkCharge(0),
  n2_trkP(0),
  n2_trkTheta(0),
  n2_trkPhi(0),

  n2_eclEoP(0)
{
  // Set module properties
  setDescription("This module produces an ntuple with ECL-related quantities starting from mdst");
  addParam("writeToRoot", m_writeToRoot,
           "set true if you want to save the informations in a root file named by parameter 'rootFileName'",
           bool(true));
  addParam("rootFileName", m_rootFileName,
           "fileName used for root file where info are saved. Will be ignored if parameter 'writeToRoot' is false (standard)",
           string("eclChargedPID"));
}

ECLChargedPIDDataAnalysisModule::~ECLChargedPIDDataAnalysisModule()
{
}

void ECLChargedPIDDataAnalysisModule::initialize()
{
  B2INFO("[ECLChargedPIDDataAnalysis Module]: Starting initialization of ECLChargedPIDDataAnalysis Module.");

  if (m_writeToRoot) {
    m_rootFilePtr = new TFile(m_rootFileName.c_str(), "RECREATE");
  } else {
    m_rootFilePtr = nullptr;
  }
  // initialize N1 tree
  n1_tree = new TTree("n1_tree", "ECL Charged PID tree: N1 Hypothesis");

  n1_tree->Branch("expNo", &n1_iExperiment, "expNo/I");
  n1_tree->Branch("runNo", &n1_iRun,        "runNo/I");
  n1_tree->Branch("evtNo", &n1_iEvent,      "evtNo/I");

  // shower
  n1_tree->Branch("eclShowerMultip",       &n1_eclShowerMultip,     "eclShowerMultip/I");
  n1_tree->Branch("eclShowerEnergy",       "std::vector<double>",    &n1_eclShowerEnergy);
  n1_tree->Branch("eclShowerTheta",        "std::vector<double>",    &n1_eclShowerTheta);
  n1_tree->Branch("eclShowerPhi",          "std::vector<double>",    &n1_eclShowerPhi);
  n1_tree->Branch("eclShowerR",            "std::vector<double>",    &n1_eclShowerR);
  n1_tree->Branch("eclShowerHypothesisId", "std::vector<int>",       &n1_eclShowerHypothesisId);
  n1_tree->Branch("eclShowerAbsZernike40", "std::vector<double>",    &n1_eclShowerAbsZernike40);
  n1_tree->Branch("eclShowerAbsZernike51", "std::vector<double>",    &n1_eclShowerAbsZernike51);

  // MC particle
  n1_tree->Branch("mcMultip",     &n1_mcMultip,         "mcMultip/I");
  n1_tree->Branch("mcPdg",        "std::vector<int>",    &n1_mcPdg);
  n1_tree->Branch("mcMothPdg",    "std::vector<int>",    &n1_mcMothPdg);
  n1_tree->Branch("mcEnergy",     "std::vector<double>", &n1_mcEnergy);
  n1_tree->Branch("mcP",          "std::vector<double>", &n1_mcP);
  n1_tree->Branch("mcTheta",      "std::vector<double>", &n1_mcTheta);
  n1_tree->Branch("mcPhi",        "std::vector<double>", &n1_mcPhi);

  // tracks
  n1_tree->Branch("trkMultip",     &n1_trkMultip,         "trkMulti/I");
  n1_tree->Branch("trkPdg",        "std::vector<int>",    &n1_trkPdg);
  n1_tree->Branch("trkCharge",     "std::vector<int>",    &n1_trkCharge);
  n1_tree->Branch("trkP",          "std::vector<double>", &n1_trkP);
  n1_tree->Branch("trkTheta",      "std::vector<double>", &n1_trkTheta);
  n1_tree->Branch("trkPhi",        "std::vector<double>", &n1_trkPhi);

  n1_tree->Branch("eclEoP",        "std::vector<double>", &n1_eclEoP);

  // initialize N2 tree
  n2_tree = new TTree("n2_tree", "ECL Charged PID tree: N2 Hypothesis");

  n2_tree->Branch("expNo", &n2_iExperiment, "expNo/I");
  n2_tree->Branch("runNo", &n2_iRun,        "runNo/I");
  n2_tree->Branch("evtNo", &n2_iEvent,      "evtNo/I");

  // shower
  n2_tree->Branch("eclShowerMultip",       &n2_eclShowerMultip,     "eclShowerMultip/I");
  n2_tree->Branch("eclShowerEnergy",       "std::vector<double>",    &n2_eclShowerEnergy);
  n2_tree->Branch("eclShowerTheta",        "std::vector<double>",    &n2_eclShowerTheta);
  n2_tree->Branch("eclShowerPhi",          "std::vector<double>",    &n2_eclShowerPhi);
  n2_tree->Branch("eclShowerR",            "std::vector<double>",    &n2_eclShowerR);
  n2_tree->Branch("eclShowerHypothesisId", "std::vector<int>",       &n2_eclShowerHypothesisId);
  n2_tree->Branch("eclShowerAbsZernike40", "std::vector<double>",    &n2_eclShowerAbsZernike40);
  n2_tree->Branch("eclShowerAbsZernike51", "std::vector<double>",    &n2_eclShowerAbsZernike51);

  // MC particle
  n2_tree->Branch("mcMultip",     &n2_mcMultip,         "mcMultip/I");
  n2_tree->Branch("mcPdg",        "std::vector<int>",    &n2_mcPdg);
  n2_tree->Branch("mcMothPdg",    "std::vector<int>",    &n2_mcMothPdg);
  n2_tree->Branch("mcEnergy",     "std::vector<double>", &n2_mcEnergy);
  n2_tree->Branch("mcP",          "std::vector<double>", &n2_mcP);
  n2_tree->Branch("mcTheta",      "std::vector<double>", &n2_mcTheta);
  n2_tree->Branch("mcPhi",        "std::vector<double>", &n2_mcPhi);

  // tracks
  n2_tree->Branch("trkMultip",     &n2_trkMultip,         "trkMulti/I");
  n2_tree->Branch("trkPdg",        "std::vector<int>",    &n2_trkPdg);
  n2_tree->Branch("trkCharge",     "std::vector<int>",    &n2_trkCharge);
  n2_tree->Branch("trkP",          "std::vector<double>", &n2_trkP);
  n2_tree->Branch("trkTheta",      "std::vector<double>", &n2_trkTheta);
  n2_tree->Branch("trkPhi",        "std::vector<double>", &n2_trkPhi);

  n2_tree->Branch("eclEoP",        "std::vector<double>", &n2_eclEoP);

  B2INFO("[ECLChargedPIDDataAnalysis Module]: Initialization of ECLChargedPIDDataAnalysis Module completed.");
}

void ECLChargedPIDDataAnalysisModule::beginRun()
{
}

void ECLChargedPIDDataAnalysisModule::event()
{

  B2DEBUG(1, "  ++++++++++++++ ECLChargedPIDDataAnalysisModule");

  // Showers
  n1_eclShowerMultip = 0;
  n1_eclShowerEnergy->clear();
  n1_eclShowerTheta->clear();
  n1_eclShowerPhi->clear();
  n1_eclShowerR->clear();
  n1_eclShowerHypothesisId->clear();
  n1_eclShowerAbsZernike40->clear();
  n1_eclShowerAbsZernike51->clear();

  // MC
  n1_mcMultip = 0;
  n1_mcPdg->clear();
  n1_mcMothPdg->clear();
  n1_mcEnergy->clear();
  n1_mcP->clear();
  n1_mcTheta->clear();
  n1_mcPhi->clear();

  // Tracks
  n1_trkMultip = 0;
  n1_trkPdg->clear();
  n1_trkCharge->clear();
  n1_trkP->clear();
  n1_trkTheta->clear();
  n1_trkPhi->clear();

  n1_eclEoP->clear();

  // Showers
  n2_eclShowerMultip = 0;
  n2_eclShowerEnergy->clear();
  n2_eclShowerTheta->clear();
  n2_eclShowerPhi->clear();
  n2_eclShowerR->clear();
  n2_eclShowerHypothesisId->clear();
  n2_eclShowerAbsZernike40->clear();
  n2_eclShowerAbsZernike51->clear();

  // MC
  n2_mcMultip = 0;
  n2_mcPdg->clear();
  n2_mcMothPdg->clear();
  n2_mcEnergy->clear();
  n2_mcP->clear();
  n2_mcTheta->clear();
  n2_mcPhi->clear();

  // Tracks
  n2_trkMultip = 0;
  n2_trkPdg->clear();
  n2_trkCharge->clear();
  n2_trkP->clear();
  n2_trkTheta->clear();
  n2_trkPhi->clear();

  n2_eclEoP->clear();

  if (m_EventMetaData) {
    n1_iExperiment = m_EventMetaData->getExperiment();
    n1_iRun = m_EventMetaData->getRun();
    n1_iEvent = m_EventMetaData->getEvent();
    n2_iExperiment = m_EventMetaData->getExperiment();
    n2_iRun = m_EventMetaData->getRun();
    n2_iEvent = m_EventMetaData->getEvent();
  } else {
    n1_iExperiment = -1;
    n1_iRun = -1;
    n1_iEvent = -1;
    n2_iExperiment = -1;
    n2_iRun = -1;
    n2_iEvent = -1;
  }

  // get the matched MC particle
  for (const MCParticle& imcpart : m_mcParticles) {
    if (!imcpart.hasStatus(MCParticle::c_PrimaryParticle)) continue; // only check primaries
    if (imcpart.hasStatus(MCParticle::c_Initial)) continue; // ignore initial particles
    if (imcpart.hasStatus(MCParticle::c_IsVirtual)) continue; // ignore virtual particles

    n1_mcMultip++;
    n2_mcMultip++;

    // get mc particle kinematics
    n1_mcPdg->push_back(imcpart.getPDG());
    if (imcpart.getMother() != nullptr) n1_mcMothPdg->push_back(imcpart.getMother()->getPDG());
    else n1_mcMothPdg->push_back(-999);
    n1_mcEnergy->push_back(imcpart.getEnergy());
    n1_mcP->push_back(imcpart.getMomentum().R());
    n1_mcTheta->push_back(imcpart.getMomentum().Theta());
    n1_mcPhi->push_back(imcpart.getMomentum().Phi());

    n2_mcPdg->push_back(imcpart.getPDG());
    if (imcpart.getMother() != nullptr) n2_mcMothPdg->push_back(imcpart.getMother()->getPDG());
    else n2_mcMothPdg->push_back(-999);
    n2_mcEnergy->push_back(imcpart.getEnergy());
    n2_mcP->push_back(imcpart.getMomentum().R());
    n2_mcTheta->push_back(imcpart.getMomentum().Theta());
    n2_mcPhi->push_back(imcpart.getMomentum().Phi());

    // loop over all matched tracks to find index of max momentum
    int index = 0;
    int index_max_mom = -1;
    double max_mom = -1;
    for (const auto& itrk : imcpart.getRelationsFrom<Track>()) {
      // get the track fit results
      const TrackFitResult* atrkF = itrk.getTrackFitResult(Const::pion);
      if (atrkF == nullptr) continue; //go to next track if no fit result
      if (atrkF->getMomentum().R() > max_mom) {
        max_mom = atrkF->getMomentum().R();
        index_max_mom = index;
      }
      index++;
    }
    if (index_max_mom == -1) continue;   // go to next mc part if no track found

    // get the track w/ max momentum
    const auto itrack = imcpart.getRelationsFrom<Track>()[index_max_mom];
    // get the track fit results
    const TrackFitResult* atrkF = itrack->getTrackFitResult(Const::pion);

    n1_trkMultip++;
    n2_trkMultip++;

    // get trk kinematics
    n1_trkPdg->push_back(atrkF->getParticleType().getPDGCode());
    n1_trkCharge->push_back(atrkF->getChargeSign());
    n1_trkP->push_back(atrkF->getMomentum().R());
    n1_trkTheta->push_back(atrkF->getMomentum().Theta());
    n1_trkPhi->push_back(atrkF->getMomentum().Phi());

    n2_trkPdg->push_back(atrkF->getParticleType().getPDGCode());
    n2_trkCharge->push_back(atrkF->getChargeSign());
    n2_trkP->push_back(atrkF->getMomentum().R());
    n2_trkTheta->push_back(atrkF->getMomentum().Theta());
    n2_trkPhi->push_back(atrkF->getMomentum().Phi());

    // loop over all matched ECLShowers (N1,N2) to find index of max energy
    int jndex1 = -1;
    int jndex1_max_e = -1;
    double max_e1 = -1;
    for (const auto& i1sh : itrack->getRelationsTo<ECLShower>()) {
      ++jndex1;
      // use HypoID 5 (N1 Photon hypothesis)
      if (i1sh.getHypothesisId() != 5) continue;
      // look only at showers passing the timing selection
      if (abs(i1sh.getTime()) > i1sh.getDeltaTime99()) continue;
      if (i1sh.getEnergy() > max_e1) {
        max_e1 = i1sh.getEnergy();
        jndex1_max_e = jndex1;
      }
    }
    int jndex2 = -1;
    int jndex2_max_e = -1;
    double max_e2 = -1;
    for (const auto& i2sh : itrack->getRelationsTo<ECLShower>()) {
      ++jndex2;
      // use Hypo ID 6 (N2 neutral hadron hypothesis)
      if (i2sh.getHypothesisId() != 6) continue;
      // look only at showers passing the timing selection
      if (abs(i2sh.getTime()) > i2sh.getDeltaTime99()) continue;
      if (i2sh.getEnergy() > max_e2) {
        max_e2 = i2sh.getEnergy();
        jndex2_max_e = jndex2;
      }
    }

    // get the N1, N2 shower w/ max energy
    if (jndex1_max_e != -1) {
      const auto i1shower = itrack->getRelationsTo<ECLShower>()[jndex1_max_e];
      // get shower kinematics
      n1_eclShowerEnergy->push_back(i1shower->getEnergy());
      n1_eclShowerTheta->push_back(i1shower->getTheta());
      n1_eclShowerPhi->push_back(i1shower->getPhi());
      n1_eclShowerR->push_back(i1shower->getR());
      n1_eclShowerHypothesisId->push_back(i1shower->getHypothesisId());
      // get shower Zernike moments
      n1_eclShowerAbsZernike40->push_back(i1shower->getAbsZernikeMoment(4, 0));
      n1_eclShowerAbsZernike51->push_back(i1shower->getAbsZernikeMoment(5, 1));
      // get E/p
      n1_eclEoP->push_back((i1shower->getEnergy()) / (atrkF->getMomentum().R()));
      n1_eclShowerMultip++;
    }
    if (jndex2_max_e != -1) {
      const auto i2shower = itrack->getRelationsTo<ECLShower>()[jndex2_max_e];
      // get shower kinematics
      n2_eclShowerEnergy->push_back(i2shower->getEnergy());
      n2_eclShowerTheta->push_back(i2shower->getTheta());
      n2_eclShowerPhi->push_back(i2shower->getPhi());
      n2_eclShowerR->push_back(i2shower->getR());
      n2_eclShowerHypothesisId->push_back(i2shower->getHypothesisId());
      // get shower Zernike moments
      n2_eclShowerAbsZernike40->push_back(i2shower->getAbsZernikeMoment(4, 0));
      n2_eclShowerAbsZernike51->push_back(i2shower->getAbsZernikeMoment(5, 1));
      // get E/p
      n2_eclEoP->push_back((i2shower->getEnergy()) / (atrkF->getMomentum().R()));
      n2_eclShowerMultip++;
    }
  }

  n1_tree->Fill();
  n2_tree->Fill();

}

void ECLChargedPIDDataAnalysisModule::endRun()
{
}

void ECLChargedPIDDataAnalysisModule::terminate()
{
  if (m_rootFilePtr != nullptr) {
    m_rootFilePtr->cd();
    n1_tree->Write();
    n2_tree->Write();
  }

}


