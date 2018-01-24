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

#include <tracking/dataobjects/RecoTrack.h>
#include <genfit/TrackPoint.h>
#include <genfit/KalmanFitterInfo.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>

#include <pxd/reconstruction/PXDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit.h>
#include <svd/reconstruction/SVDRecoHit2D.h>
#include <cdc/dataobjects/CDCRecoHit.h>

#include <root/TFile.h>
#include <root/TTree.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(StandardTrackingPerformance)

StandardTrackingPerformanceModule::StandardTrackingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL), m_pValue(-999), m_nGeneratedChargedStableMcParticles(-999),
  m_nReconstructedChargedStableTracks(-999), m_nFittedChargedStabletracks(-999)
{
  setDescription("Module to test the tracking efficiency. Writes information about the tracks and MCParticles in a ROOT file.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("StandardTrackingPerformanceOutput.root"));
  addParam("recoTracksStoreArrayName", m_recoTracksStoreArrayName, "Name of the RecoTracks StoreArray.",
           std::string(""));
  addParam("daughterPDGs", m_signalDaughterPDGs, "PDG codes of B daughters.",
           std::vector<int>(0));
}

void StandardTrackingPerformanceModule::initialize()
{
  // MCParticles and Tracks needed for this module
  StoreArray<MCParticle>::required();
  StoreArray<Track>::required();
  StoreArray<RecoTrack>::required(m_recoTracksStoreArrayName);
  StoreArray<TrackFitResult>::required();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TDirectory* oldDir = gDirectory;
  m_outputFile->cd();
  m_dataTree = new TTree("data", "data");
  oldDir->cd();

  setupTree();
}

void StandardTrackingPerformanceModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  unsigned long eventNumber = eventMetaData->getEvent();
  unsigned long runNumber = eventMetaData->getRun();
  unsigned long expNumber = eventMetaData->getExperiment();

  B2DEBUG(99,
          "Processes experiment " << expNumber << " run " << runNumber << " event " << eventNumber);

  StoreArray<RecoTrack> recoTracks(m_recoTracksStoreArrayName);
  StoreArray<MCParticle> mcParticles;

  m_nGeneratedChargedStableMcParticles = 0;
  m_nReconstructedChargedStableTracks = 0;
  m_nFittedChargedStabletracks = 0;

  for (MCParticle& mcParticle : mcParticles) {
    // check status of mcParticle
    if (isPrimaryMcParticle(mcParticle) && isChargedStable(mcParticle) && mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
      setVariablesToDefaultValue();

      int pdgCode = mcParticle.getPDG();
      B2DEBUG(99, "Primary MCParticle has PDG code " << pdgCode);

      m_nGeneratedChargedStableMcParticles++;

      m_trackProperties.cosTheta_gen = mcParticle.getMomentum().CosTheta();
      m_trackProperties.ptot_gen = mcParticle.getMomentum().Mag();
      m_trackProperties.pt_gen = mcParticle.getMomentum().Pt();
      m_trackProperties.px_gen = mcParticle.getMomentum().Px();
      m_trackProperties.py_gen = mcParticle.getMomentum().Py();
      m_trackProperties.pz_gen = mcParticle.getMomentum().Pz();
      m_trackProperties.x_gen = mcParticle.getVertex().X();
      m_trackProperties.y_gen = mcParticle.getVertex().Y();
      m_trackProperties.z_gen = mcParticle.getVertex().Z();


      const RecoTrack* recoTrack = nullptr;
      double maximumWeight = -2;
      /*// MIMIK OLD MODULE, which took always the last one (which is probably a CDC track).
      for(const Track& relatedTrack : mcParticle.getRelationsWith<Track>()) {
        b2Track = &relatedTrack;
      }*/
      // find highest rated Track
      const auto& relatedRecoTracks = mcParticle.getRelationsWith<RecoTrack>(m_recoTracksStoreArrayName);
      for (unsigned int index = 0; index < relatedRecoTracks.size(); ++index) {
        const RecoTrack* relatedRecoTrack = relatedRecoTracks.object(index);
        const double weight = relatedRecoTracks.weight(index);

        const unsigned int numberOfRelatedTracks = relatedRecoTrack->getRelationsWith<Track>().size();
        B2ASSERT("B2Track <-> RecoTrack is not a 1:1 relation as expected!", numberOfRelatedTracks <= 1);
        // use only the fitted reco tracks
        if (numberOfRelatedTracks == 1) {
          if (weight > maximumWeight) {
            maximumWeight = weight;
            recoTrack = relatedRecoTrack;
          }
        }
      }

      if (recoTrack) {
        const Track* b2Track = recoTrack->getRelated<Track>();
        const TrackFitResult* fitResult = b2Track->getTrackFitResult(Const::pion);
        B2ASSERT("Related Belle2 Track has no related track fit result!", fitResult);

        m_nFittedChargedStabletracks++;
        // write some data to the root tree
        TVector3 mom = fitResult->getMomentum();
        m_trackProperties.cosTheta = mom.CosTheta();
        m_trackProperties.ptot = mom.Mag();
        m_trackProperties.pt = mom.Pt();
        m_trackProperties.px = mom.Px();
        m_trackProperties.py = mom.Py();
        m_trackProperties.pz = mom.Pz();
        m_trackProperties.x = fitResult->getPosition().X();
        m_trackProperties.y = fitResult->getPosition().Y();
        m_trackProperties.z = fitResult->getPosition().Z();

        m_pValue = fitResult->getPValue();

        // Count hits
        m_trackProperties.nPXDhits = 0;
        m_trackProperties.nSVDhits = 0;
        m_trackProperties.nCDChits = 0;
        m_trackProperties.nWeights = 0;
        for (genfit::TrackPoint* tp : recoTrack->getHitPointsWithMeasurement()) {
          for (genfit::AbsMeasurement* m : tp->getRawMeasurements()) {
            if (dynamic_cast<PXDRecoHit*>(m))
              ++m_trackProperties.nPXDhits;
            else if (dynamic_cast<SVDRecoHit*>(m))
              ++m_trackProperties.nSVDhits;
            else if (dynamic_cast<SVDRecoHit2D*>(m))
              m_trackProperties.nSVDhits += 2;
            else if (dynamic_cast<CDCRecoHit*>(m))
              ++m_trackProperties.nCDChits;
            else
              B2ERROR("Unknown AbsMeasurement in track.");

            std::vector<double> weights;
            genfit::KalmanFitterInfo* kalmanInfo = tp->getKalmanFitterInfo();
            if (kalmanInfo)
              weights = kalmanInfo->getWeights();

            for (size_t i = 0;
                 (i < weights.size()
                  && m_trackProperties.nWeights < ParticleProperties::maxNweights);
                 ++i) {
              m_trackProperties.weights[m_trackProperties.nWeights++] = weights[i];
            }
          }
        }
      }

      m_dataTree->Fill(); // write data to tree
    }
  }
}


void StandardTrackingPerformanceModule::terminate()
{
  writeData();
}

bool StandardTrackingPerformanceModule::isPrimaryMcParticle(const MCParticle& mcParticle)
{
  return mcParticle.hasStatus(MCParticle::c_PrimaryParticle);
}

bool StandardTrackingPerformanceModule::isChargedStable(const MCParticle& mcParticle)
{
  return Const::chargedStableSet.find(abs(mcParticle.getPDG()))
         != Const::invalidParticle;
}

void StandardTrackingPerformanceModule::setupTree()
{
  if (m_dataTree == NULL) {
    B2FATAL("Data tree was not created.");
  }

  addVariableToTree("cosTheta", m_trackProperties.cosTheta);
  addVariableToTree("cosTheta_gen",  m_trackProperties.cosTheta_gen);

  addVariableToTree("px", m_trackProperties.px);
  addVariableToTree("px_gen", m_trackProperties.px_gen);

  addVariableToTree("py", m_trackProperties.py);
  addVariableToTree("py_gen", m_trackProperties.py_gen);

  addVariableToTree("pz", m_trackProperties.pz);
  addVariableToTree("pz_gen", m_trackProperties.pz_gen);

  addVariableToTree("x", m_trackProperties.x);
  addVariableToTree("x_gen", m_trackProperties.x_gen);

  addVariableToTree("y", m_trackProperties.y);
  addVariableToTree("y_gen", m_trackProperties.y_gen);

  addVariableToTree("z", m_trackProperties.z);
  addVariableToTree("z_gen", m_trackProperties.z_gen);

  addVariableToTree("pt", m_trackProperties.pt);
  addVariableToTree("pt_gen", m_trackProperties.pt_gen);

  addVariableToTree("ptot", m_trackProperties.ptot);
  addVariableToTree("ptot_gen", m_trackProperties.ptot_gen);

  addVariableToTree("mass", m_trackProperties.mass);
  addVariableToTree("mass_gen", m_trackProperties.mass_gen);

  addVariableToTree("pValue", m_pValue);

  addVariableToTree("nPXDhits", m_trackProperties.nPXDhits);
  addVariableToTree("nSVDhits", m_trackProperties.nSVDhits);
  addVariableToTree("nCDChits", m_trackProperties.nCDChits);

  m_dataTree->Branch("nWeights", &m_trackProperties.nWeights, "nWeights/I");
  m_dataTree->Branch("weights", &m_trackProperties.weights, "weights[nWeights]/F");
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

void StandardTrackingPerformanceModule::findSignalMCParticles(const StoreArray<MCParticle>& mcParticles)
{
  std::sort(m_signalDaughterPDGs.begin(), m_signalDaughterPDGs.end());

  std::vector<MCParticle*> daughterMcParticles;
  for (const MCParticle& mcParticle : mcParticles) {
    // continue if mcParticle is not a B meson
    if (abs(mcParticle.getPDG()) != 511 && abs(mcParticle.getPDG()) != 521)
      continue;

    if (isSignalDecay(mcParticle)) {
      addChargedStable(mcParticle);
      break;
    }
  }
}

bool StandardTrackingPerformanceModule::isSignalDecay(const MCParticle& mcParticle)
{
  std::vector<int> daughterPDGs;
  std::vector<MCParticle*> daughterMcParticles = mcParticle.getDaughters();

  // remove photons from list
  daughterMcParticles = removeFinalStateRadiation(daughterMcParticles);

  for (auto daughterMCParticle : daughterMcParticles) {
    daughterPDGs.push_back(daughterMCParticle->getPDG());
  }

  std::sort(daughterPDGs.begin(), daughterPDGs.end());

  bool isSignal = (daughterPDGs == m_signalDaughterPDGs);

  if (isSignal)
    m_signalMCParticles = daughterMcParticles;

  return isSignal;
}

/** remove all Photons in a  given MCParticle* vector, assumption that all phtons come from FSR
 * return std::vector< MCParticle* > daughterWOFSR */
std::vector<MCParticle*> StandardTrackingPerformanceModule::removeFinalStateRadiation(const std::vector<MCParticle*>& in_daughters)
{
  std::vector<MCParticle*> daughtersWOFSR;
  for (unsigned int iDaughter = 0; iDaughter < in_daughters.size(); iDaughter++)
    if (abs(in_daughters[iDaughter]->getPDG()) != 22)
      daughtersWOFSR.push_back(in_daughters[iDaughter]);

  return daughtersWOFSR;
}

void StandardTrackingPerformanceModule::addChargedStable(const MCParticle& mcParticle)
{

  // mcparticle is not a charged stable decays into daughters
  // loop over daughters and add the charged stable particles recursively to the vector
  std::vector<MCParticle*> daughters = mcParticle.getDaughters();
  if (daughters.size() != 0) {
    for (auto daughterMcParticle : daughters) {
      addChargedStable(*daughterMcParticle);
    }
  }

  // charged stable particle is added to the interesting particle vector
  if (isChargedStable(mcParticle) && isPrimaryMcParticle(mcParticle)
      && mcParticle.hasStatus(MCParticle::c_StableInGenerator)) {
//   B2DEBUG(99,
//          "Found a charged stable particle. Add it to interesting MCParticles. PDG(" << mcParticle->getPDG() << ").");
    m_interestingChargedStableMcParcticles.push_back(&mcParticle);
    return;
  }

  return;
}

void StandardTrackingPerformanceModule::setVariablesToDefaultValue()
{
  m_trackProperties = -999;

  m_pValue = -999;
}

void StandardTrackingPerformanceModule::addVariableToTree(const std::string& varName, double& varReference)
{
  std::stringstream leaf;
  leaf << varName << "/D";
  m_dataTree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

void StandardTrackingPerformanceModule::addVariableToTree(const std::string& varName, int& varReference)
{
  std::stringstream leaf;
  leaf << varName << "/I";
  m_dataTree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}
