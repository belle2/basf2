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

#include <analysis/dataobjects/Particle.h>

#include <generators/dataobjects/MCParticle.h>
#include <tracking/dataobjects/Track.h>

#include <genfit/GFTrack.h>

#include <root/TFile.h>
#include <root/TTree.h>

#include <boost/foreach.hpp>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StandardTrackingPerformance)

StandardTrackingPerformanceModule::StandardTrackingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL), m_cosinePolarAngleMC(-999), m_cosinePolarAngleTrack(
    -999), m_momentumMC(-999), m_momentumTrack(-999), m_momentumDif(-9990), m_bMassMC(
      -999), m_bMass(-999), m_ptotB_MC(-999), m_ptotB(-999), m_generatedChargedStableMcParticles(-999),
  m_fittedChargedStabletracks(-999)
{
  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("StandardTrackingPerformanceOutput.root"));
  addParam("gfTrackColName", m_gfTrackColName, "Name of GFTrack collection.",
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
  StoreArray< GFTrack >::required(m_gfTrackColName);
  StoreArray<TrackFitResult>::required();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  m_dataTree = new TTree("data", "data");

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

  B2INFO(
    "Processes experiment " << expNumber << " run " << runNumber << " event " << eventNumber);

  StoreArray< GFTrack > gfTracks(m_gfTrackColName);
  StoreArray<MCParticle> mcParticles;



  BOOST_FOREACH(MCParticle mcParticle, mcParticles) {
    // check status of mcParticle
    if (isPrimaryMcParticle(mcParticle) && isChargedStable(mcParticle) && mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
      m_cosinePolarAngleTrack = -999; // set everything to default values
      m_cosinePolarAngleMC = -999;
      m_momentumMC = -999;
      m_momentumTrack = -999;
      m_momentumDif = -999;

      int pdgCode = mcParticle.getPDG();
      B2DEBUG(99, "Primary MCParticle has PDG code " << pdgCode);

      m_cosinePolarAngleMC = mcParticle.getMomentum().CosTheta();
      m_momentumMC = mcParticle.getMomentum().Mag();

      GFTrack* gfTrack = findRelatedTrack(mcParticle, gfTracks);


      if (gfTrack != NULL) { // GFTrack found
        // find related TrackFitResult, if none is found, something went wrong during fit or extrapolation
        const TrackFitResult* fitResult = findRelatedTrackFitResult(gfTrack);
        if (fitResult != NULL) { // valid TrackFitResult found
          // write some data to the root tree
          m_cosinePolarAngleTrack = fitResult->getMomentum().CosTheta();
          m_cosinePolarAngleTrack = mcParticle.getMomentum().CosTheta();
          m_momentumTrack = fitResult->getMomentum().Mag();

          m_momentumDif = m_momentumTrack - m_momentumMC;

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

double StandardTrackingPerformanceModule::calculateModulus(double xx,
                                                           double yy, double zz)
{
  return sqrt(xx * xx + yy * yy + zz * zz);
}

GFTrack* StandardTrackingPerformanceModule::findRelatedTrack(
  MCParticle& mcParticle, StoreArray<GFTrack>& gfTracks)
{
  int iMcParticle = mcParticle.getArrayIndex();

  GFTrack* resultGfTrack = NULL;

  for (GFTrack & gfTrack : gfTracks) {
    if (gfTrack.getCand().getMcTrackId() == iMcParticle
        && gfTrack.getCand().getPdgCode() == mcParticle.getPDG()) {
      B2DEBUG(99, "Found GFTrack<->MCParticle relation.");
      resultGfTrack = &gfTrack;
    }
  }

  return resultGfTrack;
}

bool StandardTrackingPerformanceModule::findRelatedTrack(
  MCParticle& mcParticle)
{
  RelationVector< Track > relatedTracks = mcParticle.getRelationsTo< Track >();

  int nRelatedTracks = relatedTracks.size();
  B2DEBUG(99, "MCParticle: pdg = " << mcParticle.getPDG());
  B2DEBUG(99, "# related tracks: " << nRelatedTracks);

  const TrackFitResult* trackResult = NULL;
  if (nRelatedTracks == 1) {
    trackResult = relatedTracks[0]->getTrackFitResult(Const::ChargedStable(abs(mcParticle.getPDG())));
  }

  bool foundResult = bool (trackResult != NULL);
  // create a Particle from track if a valid TrackFitResult was found
  if (foundResult) {
    createParticleFromTrack(relatedTracks[0], mcParticle.getPDG());
  }
  return foundResult;
}

void StandardTrackingPerformanceModule::createParticleFromTrack(Track* track, int pdg)
{
  Particle particle(track, Const::ChargedStable(abs(pdg)));
  // add Particle to m_chargedStableParticles
  m_chargedStableParticles.push_back(particle);
}

void StandardTrackingPerformanceModule::setupTree()
{
  if (m_dataTree == NULL) {
    B2FATAL("Data tree was not created.");
  }

  m_dataTree->Branch("cosTheta_MC", &m_cosinePolarAngleMC, "cosTheta_MC/D");
  m_dataTree->Branch("cosTheta_Track", &m_cosinePolarAngleTrack,
                     "cosTheta_Track/D");
  m_dataTree->Branch("ptot_MC", &m_momentumMC, "ptot_MC/D");
  m_dataTree->Branch("ptot_Track", &m_momentumTrack, "ptot_Track/D");
  m_dataTree->Branch("ptot_Dif", &m_momentumDif, "ptot_Dif/D");
  m_dataTree->Branch("Bmass_MC", &m_bMassMC, "Bmass_MC/D");
  m_dataTree->Branch("Bptot_MC", &m_ptotB_MC, "Bptot_MC/D");
  m_dataTree->Branch("Bptot", &m_ptotB, "Bptot/D");
  m_dataTree->Branch("nGenChargedFS", &m_generatedChargedStableMcParticles, "nGenChargedFS/D");
  m_dataTree->Branch("nFitChargedFS", &m_fittedChargedStabletracks, "nFitChargedFS/D");
}

const TrackFitResult* StandardTrackingPerformanceModule::findRelatedTrackFitResult(
  const GFTrack* gfTrack)
{
  // search for a related TrackFitResult
  RelationIndex<GFTrack, TrackFitResult> relGfTracksToTrackFitResults;

  typedef RelationIndex<GFTrack, TrackFitResult>::Element relElement_t;

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
            "GFTrack has " << numberTrackFitResults << " related TrackFitResults. No TrackFitResult is returned.");
  }

  return NULL;

}

void StandardTrackingPerformanceModule::writeData()
{
  if (m_dataTree != NULL) {
    m_dataTree->Write();
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
