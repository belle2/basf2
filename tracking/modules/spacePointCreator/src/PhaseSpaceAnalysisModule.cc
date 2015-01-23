/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014  Belle II Collaboration                              *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Thomas Madlener                                          *
 *                                                                        *
 **************************************************************************/

#include <tracking/modules/spacePointCreator/PhaseSpaceAnalysisModule.h>

#include <genfit/TrackCand.h>

#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <framework/datastore/RelationArray.h> // not sure if I actually use this

#include <algorithm>
#include <mdst/dataobjects/MCParticle.h>

#include <TVector3.h> // COULDDO: B2Vector3.h

using namespace std;
using namespace Belle2;

REG_MODULE(PhaseSpaceAnalysis)

PhaseSpaceAnalysisModule::PhaseSpaceAnalysisModule() : Module()
{
  setDescription("Module for analysing the phase space covered by genfit::TrackCands (resp. their related MCParticles)");

  std::vector<std::string> defaultGFTCnames;
  defaultGFTCnames.push_back(std::string("")); defaultGFTCnames.push_back(std::string(""));
  addParam("GFTCname", m_PARAMgfTCNames, "Collection names of GFTCs to be analized. If you want to analyze only one provide as second item in list an empty string. If two StoreArray names are provided only those entries will be written to the root file, that are not in the second StoreArray but only in the first.", defaultGFTCnames);

  std::vector<std::string> defaultRootFName;
  defaultRootFName.push_back("PhaseSpaceAnalysis");
  defaultRootFName.push_back("RECREATE");
  addParam("rootFileName", m_PARAMrootFileName, "Name of the output root file without '.root' file ending + write mode ('UPDATE' or 'RECREATE')", defaultRootFName);

  // initialize pointers to NULL (cppcheck)
  m_rootFilePtr = NULL;
  m_treePtr = NULL;
  diffAnalysis = false;
}

void PhaseSpaceAnalysisModule::initialize()
{
  B2INFO("PhaseSpaceAnalysis ------------------------ initialize --------------------------");

  StoreArray<MCParticle>::required();
  StoreArray<genfit::TrackCand>::required(m_PARAMgfTCNames[0]);

  // if a second container name was provided check if the StoreArray exists
  if (!m_PARAMgfTCNames[1].empty()) {
    StoreArray<genfit::TrackCand>::required(m_PARAMgfTCNames[1]);
    diffAnalysis = true;
  }

  if (m_PARAMrootFileName.size() != 2 || (m_PARAMrootFileName[1] != "UPDATE" && m_PARAMrootFileName[1] != "RECREATE")) {
    string output;
    for (string entry : m_PARAMrootFileName) { output += "'" + entry + "' "; }
    B2FATAL("CurlingTrackCandSplitter::initialize() : rootFileName is set wrong: entries are: " << output)
  }
  // initialize root file
  initializeRootFile(m_PARAMrootFileName[0] + std::string(".root"), m_PARAMrootFileName[1]);

  // initalize counter here (if there are more to come, do this in separate method)
  m_gftcCtr = 0;
  m_noMcPartCtr = 0;
  m_skippedTCsCtr = 0;
  m_noMissingTCsCtr = 0;
}

void PhaseSpaceAnalysisModule::event()
{
  // print out the number of the event in debug output
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  B2DEBUG(10, "PhaseSpaceAnalysis::event(). Processing event " << eventCounter << " for StoreArray name: " << m_PARAMgfTCNames[0]);

  // get the entries from the StoreArray and print some basic information from it
  StoreArray<genfit::TrackCand> genfitTCs(m_PARAMgfTCNames[0]);
  const int nGenfitTCs = genfitTCs.getEntries();
  StoreArray<MCParticle> mcParticles;
  const int nMCParticles = mcParticles.getEntries();

  // no error is thrown this way, even if there is no StoreArray with an empty string (or whatever string is here)
  StoreArray<genfit::TrackCand> diffGFTCs(m_PARAMgfTCNames[1]);
  const int nDiffGFTCs = diffGFTCs.getEntries();

  B2DEBUG(10, "Found " << nGenfitTCs << " genfit::TrackCands in Array " << m_PARAMgfTCNames[0] << " and " << nDiffGFTCs << " in Array " << m_PARAMgfTCNames[1] << " as well as " << nMCParticles << " MC Particles for this event");

  RootVariables rootVariables;

  // count the number of entries that will be written to each branch, since there should be no entries in a StoreArray<TrackCand> without name, this should work
  // CAUTION: this assumption does not have to be true
  m_gftcCtr += nGenfitTCs - nDiffGFTCs;

  // first collect all mc particle ids that shall be checked
  std::vector<int> mcParticleIds;
  if (diffAnalysis) {
    // only do something if there are actually missing genfit::TrackCands
    if (nGenfitTCs != nDiffGFTCs) {
      std::vector<int> firstPartInds; // mcPartIds of first StoreArray
      std::vector<int> secondPartInds; // mcPartIds of second StoreArray

      // collect
      for (int i = 0; i < nGenfitTCs; ++i) { firstPartInds.push_back(genfitTCs[i]->getMcTrackId()); }
      for (int i = 0; i < nDiffGFTCs; ++i) { secondPartInds.push_back(diffGFTCs[i]->getMcTrackId()); }

      // debug output
      if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 20, PACKAGENAME())) {
        stringstream firstInds; stringstream secondInds;
        for (int entry : firstPartInds) { firstInds << entry << " "; }
        for (int entry : secondPartInds) { secondInds << entry << " "; }
        B2DEBUG(20, "McParticleIds of first TrackCands: " << firstInds.str() << ", McParticleIds of second TrackCands: " << secondInds.str())
      }

      // for now just doing this brute-force, surely there is a more elegant solution to get the non-shared mc particle mcParticleIds
      for (int entry : firstPartInds) {
        if (std::find(secondPartInds.begin(), secondPartInds.end(), entry) == secondPartInds.end()) {
          B2DEBUG(20, "Did not find Id " << entry << " from the first StoreArray in the second StoreArray. Addint it to mcParticleIds");
          mcParticleIds.push_back(entry);
        }
      }
    } else {
      B2DEBUG(20, "There are no missing genfit::TrackCands for this event!")
      m_noMissingTCsCtr++;
    }
  } else {
    B2DEBUG(10, "Only MC Particles with TrackCandidates in Array " << m_PARAMgfTCNames[0] << " will be analyzed")
    for (int iTC = 0; iTC < nGenfitTCs; ++iTC) {
      genfit::TrackCand* trackCand = genfitTCs[iTC];
      const int mcPartId = trackCand->getMcTrackId();
      B2DEBUG(20, "For genfit::TrackCand " << iTC << ": MCParticleID is: " << mcPartId << ". Added to mcParticleIds!");
      mcParticleIds.push_back(mcPartId);
    }
  }

  // second collect all the values wanted
  for (int iPart : mcParticleIds) {
    if (iPart < 0) {
      B2WARNING("Found a negative entry in mcParticleId: " << iPart << ". It seems that it has not been set properly, I will skip this MC Particle");
      m_skippedTCsCtr++;
      continue; // start over with nextGFTC
    }

    // get MCParticle
    MCParticle* mcParticle = mcParticles[iPart];
    if (mcParticle == NULL) { // safety measure
      m_noMcPartCtr++;
      continue;
    }

    B2DEBUG(20, "Collecting values for mcParticleId " << iPart);
    getValuesForRoot(mcParticle, rootVariables);
  }
  writeToRoot(rootVariables);
}

void PhaseSpaceAnalysisModule::terminate()
{
  if (!diffAnalysis) {
    B2INFO("found " << m_gftcCtr << " genfit::TrackCands for collection name: " << m_PARAMgfTCNames[0] << ". In " << m_noMcPartCtr << " cases a NULL pointer was found for mcParticle and in " << m_skippedTCsCtr << " a negative mcTrackId was assigned in the GFTC");
  } else {
    B2INFO("collected values for " << m_gftcCtr << " genfit::TrackCands that where present in collection: " << m_PARAMgfTCNames[0] << " but not in " << m_PARAMgfTCNames[1] << ". In " << m_noMcPartCtr << " cases a NULL pointer was found for mcParticle and in " << m_skippedTCsCtr << " a negative mcTrackId was assigned in the GFTC. Furthermore in " << m_noMissingTCsCtr << " events there were no missing genfit::TrackCands");
  }
  // do ROOT stuff
  if (m_treePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O (SimpleOutput etc) could mix with the root I/O of this module
    m_treePtr->Write();
    m_rootFilePtr->Close();
  }
}

// ================================== initialize root ===============================
void PhaseSpaceAnalysisModule::initializeRootFile(std::string fileName, std::string writeOption)
{
  B2DEBUG(10, "initializing root file. fileName: " << fileName << ", writeOption: " << writeOption)
  m_rootFilePtr = new TFile(fileName.c_str(), writeOption.c_str());
  m_treePtr = new TTree("mcPartData", "data of mc particles");

  m_treePtr->Branch("momentum_X", &m_rootMomX);
  m_treePtr->Branch("momentum_Y", &m_rootMomY);
  m_treePtr->Branch("momentum_Z", &m_rootMomZ);

  m_treePtr->Branch("eta", &m_rootEta);
  m_treePtr->Branch("p_T", &m_rootPt);

  m_treePtr->Branch("vertex_X", &m_rootVertX);
  m_treePtr->Branch("vertex_Y", &m_rootVertY);
  m_treePtr->Branch("vertex_Z", &m_rootVertZ);

  m_treePtr->Branch("energy", &m_rootEnergy);
  m_treePtr->Branch("mass", &m_rootMass);

  m_treePtr->Branch("charge", &m_rootCharge);
  m_treePtr->Branch("pdg_code", &m_rootPdg);
}

// =============================== collect values for root ====================================
void PhaseSpaceAnalysisModule::getValuesForRoot(Belle2::MCParticle* mcParticle, RootVariables& rootVariables)
{
  // collect all the momentum
  const TVector3 momentum = mcParticle->getMomentum();
  rootVariables.MomX.push_back(momentum.X());
  rootVariables.MomY.push_back(momentum.Y());
  rootVariables.MomZ.push_back(momentum.Z());

  rootVariables.pT.push_back(momentum.Pt());
  rootVariables.Eta.push_back(momentum.Eta());

  B2DEBUG(30, "TVector3 momentum: (" << momentum.X() << "," << momentum.Y() << "," << momentum.Z() << "). This leads to p_T = " << momentum.Pt() << " and eta = " << momentum.Eta());

  const TVector3 vertex = mcParticle->getVertex();
  rootVariables.VertX.push_back(vertex.Y());
  rootVariables.VertY.push_back(vertex.Y());
  rootVariables.VertZ.push_back(vertex.Z());

  B2DEBUG(30, "TVector3 vertex: (" << vertex.X() << "," << vertex.Y() << "," << vertex.Z() << ")");

  rootVariables.Charge.push_back(mcParticle->getCharge());
  rootVariables.Energy.push_back(mcParticle->getEnergy());
  rootVariables.PDG.push_back(mcParticle->getPDG());
  rootVariables.Mass.push_back(mcParticle->getMass());

  B2DEBUG(30, "Charge " << mcParticle->getCharge() << ", PDG code" << mcParticle->getPDG() << ", Energy " << mcParticle->getEnergy() << ", Mass " << mcParticle->getMass());
}


// ========================================== write to root =================================
void PhaseSpaceAnalysisModule::writeToRoot(RootVariables& rootVariables)
{
  m_rootMomX = rootVariables.MomX;
  m_rootMomY = rootVariables.MomY;
  m_rootMomZ = rootVariables.MomZ;

  m_rootEta = rootVariables.Eta;
  m_rootPt = rootVariables.pT;

  m_rootVertX = rootVariables.VertX;
  m_rootVertY = rootVariables.VertY;
  m_rootVertZ = rootVariables.VertZ;

  m_rootEnergy = rootVariables.Energy;
  m_rootMass = rootVariables.Mass;
  m_rootCharge = rootVariables.Charge;
  m_rootPdg = rootVariables.PDG;

  m_treePtr->Fill();
}