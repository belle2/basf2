/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributor: Frank Meier                                               *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <ecl/modules/eclMatchingPerformance/ECLMatchingPerformanceModule.h>

#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/RelationIndex.h>
#include <framework/datastore/RelationVector.h>

#include <tracking/dataobjects/RecoTrack.h>

#include <mdst/dataobjects/MCParticle.h>
#include <mdst/dataobjects/Track.h>
#include <mdst/dataobjects/ECLCluster.h>

#include <root/TFile.h>
#include <root/TTree.h>

using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(ECLMatchingPerformance)

ECLMatchingPerformanceModule::ECLMatchingPerformanceModule() :
  Module(), m_outputFile(NULL), m_dataTree(NULL)
{
  setDescription("Module to test the matching efficiency between tracks and ECLClusters. Additionally, information about the tracks are written into a ROOT file.");
  setPropertyFlags(c_ParallelProcessingCertified);

  addParam("outputFileName", m_outputFileName, "Name of output root file.",
           std::string("ECLMatchingPerformanceOutput.root"));
}

void ECLMatchingPerformanceModule::initialize()
{
  // MCParticles and Tracks needed for this module
  StoreArray<MCParticle>::required();
  StoreArray<RecoTrack>::required();
  StoreArray<Track>::required();
  StoreArray<TrackFitResult>::required();
  StoreArray<ECLCluster>::required();

  m_outputFile = new TFile(m_outputFileName.c_str(), "RECREATE");
  TDirectory* oldDir = gDirectory;
  m_outputFile->cd();
  m_dataTree = new TTree("data", "data");
  oldDir->cd();

  setupTree();
}

void ECLMatchingPerformanceModule::event()
{
  StoreObjPtr<EventMetaData> eventMetaData("EventMetaData", DataStore::c_Event);
  m_iEvent = eventMetaData->getEvent();
  m_iRun = eventMetaData->getRun();
  m_iExperiment = eventMetaData->getExperiment();

  B2DEBUG(99, "Processes experiment " << m_iExperiment << " run " << m_iRun << " event " << m_iEvent);

  StoreArray<Track> tracks;

  for (const Track& track : tracks) {
    setVariablesToDefaultValue();
    const RecoTrack* recoTrack = track.getRelated<RecoTrack>();
    if (recoTrack) {
      const TrackFitResult* fitResult = track.getTrackFitResult(Const::pion);
      B2ASSERT("Related Belle2 Track has no related track fit result!", fitResult);

      // write some data to the root tree
      TVector3 mom = fitResult->getMomentum();
      m_trackProperties.cosTheta = mom.CosTheta();
      m_trackProperties.phi = mom.Phi();
      m_trackProperties.ptot = mom.Mag();
      m_trackProperties.pt = mom.Pt();
      m_trackProperties.px = mom.Px();
      m_trackProperties.py = mom.Py();
      m_trackProperties.pz = mom.Pz();
      m_trackProperties.x = fitResult->getPosition().X();
      m_trackProperties.y = fitResult->getPosition().Y();
      m_trackProperties.z = fitResult->getPosition().Z();

      m_pValue = fitResult->getPValue();

      ECLCluster* eclCluster = track.getRelatedTo<ECLCluster>();
      if (eclCluster != nullptr) {
        m_matchedToECLCluster = 1;
        m_hypothesisOfMatchedECLCluster = eclCluster->getHypothesisId();
        for (const MCParticle& eclClusterMCParticle : eclCluster->getRelationsTo<MCParticle>()) {
          if (eclClusterMCParticle.getPDG() == 22) {
            m_photonCluster = 1;
            break;
          }
        }
      }
      m_dataTree->Fill(); // write data to tree
    }
  }
}


void ECLMatchingPerformanceModule::terminate()
{
  writeData();
}

void ECLMatchingPerformanceModule::setupTree()
{
  if (m_dataTree == NULL) {
    B2FATAL("Data tree was not created.");
  }
  addVariableToTree("expNo", m_iExperiment);
  addVariableToTree("runNo", m_iRun);
  addVariableToTree("evtNo", m_iEvent);

  addVariableToTree("cosTheta", m_trackProperties.cosTheta);

  addVariableToTree("phi", m_trackProperties.phi);

  addVariableToTree("px", m_trackProperties.px);
  addVariableToTree("py", m_trackProperties.py);
  addVariableToTree("pz", m_trackProperties.pz);

  addVariableToTree("x", m_trackProperties.x);
  addVariableToTree("y", m_trackProperties.y);
  addVariableToTree("z", m_trackProperties.z);

  addVariableToTree("pt", m_trackProperties.pt);

  addVariableToTree("ptot", m_trackProperties.ptot);

  addVariableToTree("pValue", m_pValue);

  addVariableToTree("ECLMatch", m_matchedToECLCluster);
  addVariableToTree("PhotonCluster", m_photonCluster);
  addVariableToTree("HypothesisID", m_hypothesisOfMatchedECLCluster);
}

void ECLMatchingPerformanceModule::writeData()
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

void ECLMatchingPerformanceModule::setVariablesToDefaultValue()
{
  m_trackProperties = -999;

  m_pValue = -999;

  m_matchedToECLCluster = 0;

  m_photonCluster = 0;

  m_hypothesisOfMatchedECLCluster = 0;
}

void ECLMatchingPerformanceModule::addVariableToTree(const std::string& varName, double& varReference)
{
  std::stringstream leaf;
  leaf << varName << "/D";
  m_dataTree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}

void ECLMatchingPerformanceModule::addVariableToTree(const std::string& varName, int& varReference)
{
  std::stringstream leaf;
  leaf << varName << "/I";
  m_dataTree->Branch(varName.c_str(), &varReference, leaf.str().c_str());
}
