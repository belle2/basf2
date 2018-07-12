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
#include <tracking/spacePointCreation/SpacePointTrackCand.h>

#include <framework/datastore/StoreArray.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/datastore/StoreObjPtr.h>

#include <algorithm>
#include <numeric>
// #include <mdst/dataobjects/MCParticle.h> // already in header

#include <TVector3.h> // COULDDO: B2Vector3.h

using namespace std;
using namespace Belle2;

REG_MODULE(PhaseSpaceAnalysis)

PhaseSpaceAnalysisModule::PhaseSpaceAnalysisModule() : Module()
{
  setDescription("Module for analysing the phase space covered by TrackCands (resp. their related MCParticles)");

  std::vector<std::string> defaultNames = { std::string("") };
  addParam("containerNames", m_PARAMcontainerNames, "Collection names of trackCands to be analized.", defaultNames);
  addParam("trackCandTypes", m_PARAMtrackCandTypes, "Either 'GFTC' for genfit::TrackCand or 'SPTC' for SpacePointTrackCand." \
           "If there are more 'containerNames' than 'trackCandTypes' the last type is assumed for all the names that cannot be matched"\
           "one-to-one with a trackCandType. If there are as much names as types the i-th name is assumed to be of the i-th type");

  addParam("diffAnalysis", m_PARAMdiffAnalysis,
           "Use the first container as reference and only collect data for those MCParticle that"\
           " cannot be found in the other containers.", false);

  std::vector<std::string> defaultRootFName = { "PhaseSpaceAnalysis", "RECREATE" };
  addParam("rootFileName", m_PARAMrootFileName, "Name of the output root file without '.root' file ending + write mode"\
           " ('UPDATE' or 'RECREATE')", defaultRootFName);

  // initialize pointers to NULL (cppcheck)
  m_rootFilePtr = NULL;
  initializeCounters(0);
}

// ===================================================== INITIALIZE ===============================================================
void PhaseSpaceAnalysisModule::initialize()
{
  B2INFO("PhaseSpaceAnalysis ------------------------ initialize --------------------------");

  StoreArray<MCParticle> mcParticles;
  mcParticles.isRequired();

  size_t nNames = m_PARAMcontainerNames.size();
  size_t nTypes = m_PARAMtrackCandTypes.size();

  if (nTypes > nNames) { B2FATAL("Passed " << nTypes << " trackCandTypese but only " << nNames << " containerNames!"); }

  for (size_t iName = 0; iName < nNames; ++iName) {
    size_t iType = iName < nTypes ? iName : nTypes - 1; // only acces values that are possible
    string tcType = m_PARAMtrackCandTypes.at(iType);
    if (tcType.compare(string("GFTC")) != 0 && tcType.compare(string("SPTC")) != 0) {
      B2FATAL("Found id " << tcType << " in 'trackCandTypes' but only 'GFTC' and 'SPTC' are allowed!");
    }

    string contName = m_PARAMcontainerNames.at(iName);
    if (tcType.compare(string("GFTC")) == 0) {
      StoreArray<genfit::TrackCand> trackCands(contName);
      trackCands.isRequired();
      m_tcStoreArrays.push_back(make_pair(trackCands, c_gftc));
    } else {
      StoreArray<SpacePointTrackCand> trackCands(contName);
      trackCands.isRequired();
      m_tcStoreArrays.push_back(make_pair(trackCands, c_sptc));
    }
    if (m_PARAMrootFileName.size() != 2 || (m_PARAMrootFileName[1] != "UPDATE" && m_PARAMrootFileName[1] != "RECREATE")) {
      string output;
      for (string id : m_PARAMrootFileName) { output += "'" + id + "' "; }
      B2FATAL("PhaseSpaceAnalysis::initialize() : rootFileName is set wrong: entries are: " << output);
    }

    m_treeNames.push_back(contName);
    if (m_PARAMdiffAnalysis && iName != 0) m_treeNames.push_back(contName + "_diff");
  }

  initializeRootFile(m_PARAMrootFileName[0] + string(".root"), m_PARAMrootFileName[1], m_treeNames);
  initializeCounters(m_treeNames.size());
}


// ============================================================= EVENT ============================================================
void PhaseSpaceAnalysisModule::event()
{
  // print out the number of the event in debug output
  StoreObjPtr<EventMetaData> eventMetaDataPtr("EventMetaData", DataStore::c_Event);
  const int eventCounter = eventMetaDataPtr->getEvent();
  string arrayNames;
  for (string name : m_PARAMcontainerNames) { arrayNames += " " + name; }
  B2DEBUG(10, "PhaseSpaceAnalysis::event(). Processing event " << eventCounter << " for StoreArray names :" << arrayNames);

  StoreArray<MCParticle> mcParticles;
  const int nMCParticles = mcParticles.getEntries();
  B2DEBUG(25, "Found " << nMCParticles << " MCParticles for this event");

  vector<vector<int> > mcPartIds; // collect all mcParticle Ids of all trackCands

  for (pair<boost::any, e_trackCandType> storeArray : m_tcStoreArrays) {
    // COULDDO: wrap this in try-catch, but since it is known what has to be casted, nothing should really happen
    if (storeArray.second == c_gftc) {
      mcPartIds.push_back(getMCParticleIDs(boost::any_cast<StoreArray<genfit::TrackCand> >(storeArray.first)));
    } else if (storeArray.second == c_sptc) {
      mcPartIds.push_back(getMCParticleIDs(boost::any_cast<StoreArray<SpacePointTrackCand> >(storeArray.first)));
    }
    B2DEBUG(25, mcPartIds.back().size() << " MCParticles of this event were set for this container");
  }

  if (m_PARAMdiffAnalysis) { mcPartIds = getDiffIds(mcPartIds); }
  // TODO: get the referee Status of a SPTC into the getValuesForRoot stuff

  // now loop over all MCParticle IDs
  for (size_t iTree = 0; iTree < mcPartIds.size(); ++iTree) {
    B2DEBUG(25, "Now collecting values for vector of MCParticle Ids " << iTree);
    m_mcPartCtr[iTree]++; // increase counter

    m_rootVariables = RootVariables(); // clear root variables for each run
    for (int id : mcPartIds[iTree]) {
      if (id < 0) {
        B2WARNING("Found a negative id in mcParticleId: " << id << \
                  ". It seems that it has not been set properly, I will skip this MC Particle");
        m_skippedTCsCtr++;
        continue;
      }
      MCParticle* mcParticle = mcParticles[id];
      if (mcParticle == NULL) { // safety measure
        m_noMcPartCtr++;
        continue;
      }
      getValuesForRoot(mcParticle, m_rootVariables);
    }
    m_treePtrs[iTree]->Fill(); // write values to TTree
  }
}

// ================================================================= TERMINATE ====================================================
// TODO: update to new version and new counters
void PhaseSpaceAnalysisModule::terminate()
{
  stringstream furtherInfo;
  if (m_skippedTCsCtr || m_noMcPartCtr) {
    furtherInfo << " There were " << m_skippedTCsCtr << " negative mcParticle IDs and " << m_noMcPartCtr <<
                " NULL pointers to MCParticles";
  }
  unsigned int nMCParts = accumulate(m_mcPartCtr.begin(), m_mcPartCtr.end(), 0);
  B2INFO("PhaseSpaceAnalysis::terminate(): Collected mcParticle info in " << m_PARAMcontainerNames.size() << " containers."\
         " Collected information from " << nMCParts << " MCParticles and wrote them into " << m_treeNames.size() << " different trees."\
         << furtherInfo.str());

  if (LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1, PACKAGENAME())) {
    stringstream output;
    output << "tree-wise summary (no of mcParticles):";
    for (unsigned int ctr : m_mcPartCtr) output << " " << ctr;
    B2DEBUG(1, output.str());
  }

  // do ROOT stuff
  if (m_rootFilePtr != NULL) {
    m_rootFilePtr->cd(); //important! without this the famework root I/O could mix with the root I/O of this module
    for (TTree* tree : m_treePtrs) { tree->Write(); }
    m_rootFilePtr->Close();
  }
}

// ================================== initialize root ===============================
void PhaseSpaceAnalysisModule::initializeRootFile(std::string fileName, std::string writeOption, std::vector<std::string> treeNames)
{
  B2DEBUG(10, "initializing root file. fileName: " << fileName << ", writeOption: " << writeOption);
  m_rootFilePtr = new TFile(fileName.c_str(), writeOption.c_str());

  for (size_t i = 0; i < treeNames.size(); ++i) {
    TTree* tree = new TTree(treeNames[i].c_str(), "data of mc particles");
    tree->Branch("momentum_X", &m_rootVariables.MomX);
    tree->Branch("momentum_Y", &m_rootVariables.MomY);
    tree->Branch("momentum_Z", &m_rootVariables.MomZ);

    tree->Branch("eta", &m_rootVariables.Eta);
    tree->Branch("p_T", &m_rootVariables.pT);

    tree->Branch("vertex_X", &m_rootVariables.VertX);
    tree->Branch("vertex_Y", &m_rootVariables.VertY);
    tree->Branch("vertex_Z", &m_rootVariables.VertZ);

    tree->Branch("energy", &m_rootVariables.Energy);
    tree->Branch("mass", &m_rootVariables.Mass);

    tree->Branch("charge", &m_rootVariables.Charge);
    tree->Branch("pdg_code", &m_rootVariables.PDG);

    m_treePtrs.push_back(tree);
  }
}

// =============================== collect values for root ====================================
void PhaseSpaceAnalysisModule::getValuesForRoot(Belle2::MCParticle* mcParticle, RootVariables& rootVariables)
{
  B2DEBUG(100, "Collecting values for MCParticle " << mcParticle->getArrayIndex());
  // collect all the momentum
  const TVector3 momentum = mcParticle->getMomentum();
  rootVariables.MomX.push_back(momentum.X());
  rootVariables.MomY.push_back(momentum.Y());
  rootVariables.MomZ.push_back(momentum.Z());

  rootVariables.pT.push_back(momentum.Pt());
  rootVariables.Eta.push_back(momentum.Eta());

  B2DEBUG(499, "TVector3 momentum: (" << momentum.X() << "," << momentum.Y() << "," << momentum.Z() << \
          "). This leads to p_T = " << momentum.Pt() << " and eta = " << momentum.Eta());

  const TVector3 vertex = mcParticle->getVertex();
  rootVariables.VertX.push_back(vertex.Y());
  rootVariables.VertY.push_back(vertex.Y());
  rootVariables.VertZ.push_back(vertex.Z());

  B2DEBUG(499, "TVector3 vertex: (" << vertex.X() << "," << vertex.Y() << "," << vertex.Z() << ")");

  rootVariables.Charge.push_back(mcParticle->getCharge());
  rootVariables.Energy.push_back(mcParticle->getEnergy());
  rootVariables.PDG.push_back(mcParticle->getPDG());
  rootVariables.Mass.push_back(mcParticle->getMass());

  B2DEBUG(499, "Charge " << mcParticle->getCharge() << ", PDG code" << mcParticle->getPDG() << ", Energy " << \
          mcParticle->getEnergy() << ", Mass " << mcParticle->getMass());
}

// ==================================================== GET MCPARTICLE IDS ========================================================
template<typename TrackCandType>
std::vector<int> PhaseSpaceAnalysisModule::getMCParticleIDs(Belle2::StoreArray<TrackCandType> trackCands)
{
  const int nTrackCands = trackCands.getEntries();
  B2DEBUG(25, "In StoreArray " << trackCands.getName() << ": " << nTrackCands << " track candidates for this event");
  std::vector<int> mcPartIds;
  for (int i = 0; i < nTrackCands; ++i) mcPartIds.push_back(trackCands[i]->getMcTrackId());
  // COULDDO: more debug output
  return mcPartIds;
}

// ================================================== GET DIFF IDS ================================================================
// TODO: test
std::vector<std::vector<int> > PhaseSpaceAnalysisModule::getDiffIds(const std::vector<std::vector<int> >& allIDs)
{
  if (allIDs.size() < 2) { // do nothing if there are less than two entries
    B2DEBUG(50, "There are no more than 1 vectors passed to getDiffIds. No comparison possible. Returning passed vector");
    return allIDs;
  }

//   // COULDDO: remove (left here for checking if working properly, simply prints out all the entries in the vector of vectors)
//   if(LogSystem::Instance().isLevelEnabled(LogConfig::c_Debug, 1999, PACKAGENAME())) { // very verbose dev output
//     stringstream output;
//     output << "content of allIDs: " << endl;
//     for(vector<int> vec: allIDs) {
//       output << "vector: ";
//       for(int i : vec) output << i << " ";
//       output << endl;
//     }
//     B2DEBUG(1999, output.str())
//   }

  vector<vector<int> > diffIds;
  vector<int> referenceIds = allIDs.at(0); // the first vector is the reference
  diffIds.push_back(referenceIds);

  // loop over other vectors and look for missing IDs
  for (size_t iVec = 1; iVec < allIDs.size(); ++iVec) {
    vector<int> compareIds = allIDs[iVec];
    diffIds.push_back(compareIds);
    vector<int> notFoundIds;
    for (int id : referenceIds) {
      if (find(compareIds.begin(), compareIds.end(), id) == compareIds.end()) { // not found
        notFoundIds.push_back(id);
      }
    }
    diffIds.push_back(notFoundIds); // push_back empty vectors too! otherwise the filling of the TTrees will be wrong!
  }

  return diffIds;
}

// ================================================== GET REFEREE STATUSES ========================================================
// std::vector<unsigned short int>
// PhaseSpaceAnalysisModule::getRefereeStatuses(Belle2::StoreArray<Belle2::SpacePointTrackCand> trackCands)
// {
//   std::vector<unsigned short int> statuses;
//   for(int i = 0; i < trackCands.getEntries(); ++i) { statuses.push_back(trackCands[i]->getRefereeStatus()); }
//   return statuses;
// }
