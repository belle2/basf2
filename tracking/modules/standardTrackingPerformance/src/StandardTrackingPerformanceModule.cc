/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Michael Ziegler                                          *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <tracking/modules/standardTrackingPerformance/StandardTrackingPerformanceModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <genfit/Track.h>

#include <root/TFile.h>
#include <root/TTree.h>

#include <boost/foreach.hpp>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StandardTrackingPerformance)

StandardTrackingPerformanceModule::StandardTrackingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL), m_nGeneratedChargedStableMcParticles(-999),
  m_nFittedChargedStabletracks(-999)
{

  setDescription("Module to test the tracking efficiency. Writes information about the tracks and MCParticles in a ROOT file.");
  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("StandardTrackingPerformanceOutput.root"));
  addParam("gfTrackColName", m_gfTrackColName, "Name of genfit::Track collection.",
           std::string(""));
  addParam("trackColName", m_trackColName, "Name of Track collection.",
           std::string(""));
  addParam("daughterPDGs", m_signalDaughterPDGs, "PDG codes of B daughters.",
           std::vector<int>(0));
}

StandardTrackingPerformanceModule::~StandardTrackingPerformanceModule()
{

}

void StandardTrackingPerformanceModule::initialize()
{
  // MCParticles and Tracks needed for this module
  StoreArray<MCParticle>::required();
  StoreArray<Track>::required();
  StoreArray< genfit::Track >::required(m_gfTrackColName);
  StoreArray<TrackFitResult>::required();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TDirectory* oldDir = gDirectory;
  m_outputFile->cd();
  m_dataTree = new TTree("data", "data");
  oldDir->cd();

  setupTree();

}

void StandardTrackingPerformanceModule::beginRun()
{

}

void StandardTrackingPerformanceModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  unsigned long eventNumber = eventMetaData->getEvent();
  unsigned long runNumber = eventMetaData->getRun();
  unsigned long expNumber = eventMetaData->getExperiment();

  B2DEBUG(99,
          "Processes experiment " << expNumber << " run " << runNumber << " event " << eventNumber);

  StoreArray< genfit::Track > gfTracks(m_gfTrackColName);
  StoreArray<MCParticle> mcParticles;

  m_nGeneratedChargedStableMcParticles = 0;
  m_nFittedChargedStabletracks = 0;

  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {
    // check status of mcParticle
    if (isPrimaryMcParticle(mcParticle) && isChargedStable(mcParticle) && mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
      setVariablesToDefaultValue();

      int pdgCode = mcParticle.getPDG();
      B2DEBUG(99, "Primary MCParticle has PDG code " << pdgCode);

      m_nGeneratedChargedStableMcParticles++;

      m_trackProperties.cosTheta_gen = mcParticle.getMomentum().CosTheta();
      m_trackProperties.ptot_gen = mcParticle.getMomentum().Mag();
      m_trackProperties.pt_gen = mcParticle.getMomentum().Pt();

      genfit::Track* gfTrack = findRelatedTrack(mcParticle, gfTracks);


      if (gfTrack != NULL) { // genfit::Track found
        // find related TrackFitResult, if none is found, something went wrong during fit or extrapolation
        const TrackFitResult* fitResult = findRelatedTrackFitResult(gfTrack);
        if (fitResult != NULL) { // valid TrackFitResult found
          m_nFittedChargedStabletracks++;
          // write some data to the root tree
          m_trackProperties.cosTheta = fitResult->getMomentum().CosTheta();
          m_trackProperties.ptot = fitResult->getMomentum().Mag();
          m_trackProperties.pt = fitResult->getMomentum().Pt();
        }
      }

      m_dataTree->Fill(); // write data to tree
    }
  }
}

void StandardTrackingPerformanceModule::endRun()
{

}

void StandardTrackingPerformanceModule::terminate()
{
  writeData();
}

bool StandardTrackingPerformanceModule::isPrimaryMcParticle(
  MCParticle& mcParticle)
{
  return mcParticle.hasStatus(MCParticle::c_PrimaryParticle);
}

bool StandardTrackingPerformanceModule::isChargedStable(MCParticle& mcParticle)
{
  return Const::chargedStableSet.find(abs(mcParticle.getPDG()))
         != Const::invalidParticle;
}

genfit::Track* StandardTrackingPerformanceModule::findRelatedTrack(
  MCParticle& mcParticle, StoreArray<genfit::Track>& gfTracks)
{
  B2DEBUG(99, "Entered findRelatedTrack function.");
  int iMcParticle = mcParticle.getArrayIndex();
  B2DEBUG(99, "MCParticle array index: " << iMcParticle);
  genfit::Track* resultGfTrack = NULL;

  for (genfit::Track & gfTrack : gfTracks) {
    const genfit::TrackCand* aTrackCandPtr = DataStore::getRelatedToObj<genfit::TrackCand>(&gfTrack);
    if (!aTrackCandPtr) {
      B2ERROR("No Track Candidate for track.  Skipping.");
      continue;
    }
    if (aTrackCandPtr->getMcTrackId() == iMcParticle
        && aTrackCandPtr->getPdgCode() == mcParticle.getPDG()) {
      B2DEBUG(99, "Found genfit::Track<->MCParticle relation.");
      resultGfTrack = &gfTrack;
    }
  }

  return resultGfTrack;
}

void StandardTrackingPerformanceModule::setupTree()
{
  if (m_dataTree == NULL) {
    B2FATAL("Data tree was not created.");
  }

  m_dataTree->Branch("cosTheta_gen", &m_trackProperties.cosTheta_gen, "cosTheta_gen/D");
  m_dataTree->Branch("cosTheta", &m_trackProperties.cosTheta,
                     "cosTheta/D");
  m_dataTree->Branch("ptot_gen", &m_trackProperties.ptot_gen, "ptot_gen/D");
  m_dataTree->Branch("ptot", &m_trackProperties.ptot, "ptot/D");

  m_dataTree->Branch("pt_gen", &m_trackProperties.pt_gen, "pt_gen/D");
  m_dataTree->Branch("pt", &m_trackProperties.pt, "pt/D");

  m_dataTree->Branch("nGenChargedFS", &m_nGeneratedChargedStableMcParticles, "nGenChargedFS/D");
  m_dataTree->Branch("nFitChargedFS", &m_nFittedChargedStabletracks, "nFitChargedFS/D");

}

const TrackFitResult* StandardTrackingPerformanceModule::findRelatedTrackFitResult(
  const genfit::Track* gfTrack)
{
  // search for a related TrackFitResult
  RelationIndex<genfit::Track, TrackFitResult> relGfTracksToTrackFitResults;

  typedef RelationIndex<genfit::Track, TrackFitResult>::Element relElement_t;

  std::vector<const TrackFitResult*> fitResults;

  BOOST_FOREACH(const relElement_t & relGfTrackToTrackFitResult, relGfTracksToTrackFitResults.getElementsFrom(gfTrack)) {
    B2DEBUG(99, "----> Related TrackFitResult found!!!");

    fitResults.push_back(relGfTrackToTrackFitResult.to);
  }

  int numberTrackFitResults = fitResults.size();

  if (numberTrackFitResults == 1) {
    return fitResults[0];
  }
  if (numberTrackFitResults == 0) {
    return NULL;
  }
  if (numberTrackFitResults > 1) {
    B2DEBUG(99,
            "genfit::Track has " << numberTrackFitResults << " related TrackFitResults. No TrackFitResult is returned.");
  }

  return NULL;
}

void StandardTrackingPerformanceModule::writeData()
{
  if (m_dataTree != NULL) {
    TDirectory* oldDir = gDirectory;
    if (m_outputFile)
      m_outputFile->cd();
    m_dataTree->Write();
    oldDir->cd();
  }
  if (m_outputFile != NULL) {
    m_outputFile->Close();
  }
}

void StandardTrackingPerformanceModule::findSignalMCParticles(
  StoreArray<MCParticle>& mcParticles)
{
  std::sort(m_signalDaughterPDGs.begin(), m_signalDaughterPDGs.end());

  std::vector<MCParticle*> daughterMcParticles;
  BOOST_FOREACH(MCParticle & mcParticle, mcParticles) {
    // continue if mcParticle is not a B meson
    if (abs(mcParticle.getPDG()) != 511 && abs(mcParticle.getPDG()) != 521)
      continue;

    if (isSignalDecay(mcParticle)) {
      addChargedStable(&mcParticle);
      break;
    }
  }
}

bool StandardTrackingPerformanceModule::isSignalDecay(MCParticle& mcParticle)
{
  std::vector<int> daughterPDGs;
  std::vector<MCParticle*> daughterMcParticles = mcParticle.getDaughters();

  // remove photons from list
  daughterMcParticles = removeFinalStateRadiation(daughterMcParticles);

  for (auto mcParticle : daughterMcParticles) {
    daughterPDGs.push_back(mcParticle->getPDG());
  }

  std::sort(daughterPDGs.begin(), daughterPDGs.end());

  bool isSignal = (daughterPDGs == m_signalDaughterPDGs);

  if (isSignal)
    m_signalMCParticles = daughterMcParticles;

  return isSignal;
}

/** remove all Photons in a  given MCParticle* vector, assumption that all phtons come from FSR
 * return std::vector< MCParticle* > daughterWOFSR */
std::vector<MCParticle*> StandardTrackingPerformanceModule::removeFinalStateRadiation(
  const std::vector<MCParticle*>& in_daughters)
{
  std::vector<MCParticle*> daughtersWOFSR;
  for (unsigned int iDaughter = 0; iDaughter < in_daughters.size(); iDaughter++)
    if (abs(in_daughters[iDaughter]->getPDG()) != 22)
      daughtersWOFSR.push_back(in_daughters[iDaughter]);

  return daughtersWOFSR;
}

void StandardTrackingPerformanceModule::addChargedStable(
  MCParticle* mcParticle)
{

  // mcparticle is not a charged stable decays into daughters
  // loop over daughters and add the charged stable particles recursively to the vector
  std::vector<MCParticle*> daughters = mcParticle->getDaughters();
  if (daughters.size() != 0) {
    for (auto daughterMcParticle : daughters) {
      addChargedStable(daughterMcParticle);
    }
  }

  // charged stable particle is added to the interesting particle vector
  if (isChargedStable(*mcParticle) && isPrimaryMcParticle(*mcParticle)
      && mcParticle->hasStatus(MCParticle::c_StableInGenerator)) {
//   B2DEBUG(99,
//          "Found a charged stable particle. Add it to interesting MCParticles. PDG(" << mcParticle->getPDG() << ").");
    m_interestingChargedStableMcParcticles.push_back(mcParticle);
    return;
  }

  return;
}

void StandardTrackingPerformanceModule::setVariablesToDefaultValue()
{
  m_trackProperties.cosTheta = -999;
  m_trackProperties.mass = -999;
  m_trackProperties.pt = -999;
  m_trackProperties.ptot = -999;

  m_trackProperties.cosTheta_gen = -999;
  m_trackProperties.mass_gen = -999;
  m_trackProperties.pt_gen = -999;
  m_trackProperties.ptot_gen = -999;
}
