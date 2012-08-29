/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/simpleoutput/SimpleOutputModule.h>

#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/core/RandomNumbers.h>
#include <framework/core/Environment.h>

#include <TTreeIndex.h>
#include <TProcessID.h>
#include <TSystem.h>

#include <time.h>
#include <algorithm>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimpleOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

const std::string SimpleOutputModule::c_SteerTreeNames[] = { "treeName", "treeNameRun", "treeNamePersistent" };
const std::string SimpleOutputModule::c_SteerBranchNames[] = { "branchNames", "branchNamesRun", "branchNamesPersistent" };
const std::string SimpleOutputModule::c_SteerExcludeBranchNames[] = { "excludeBranchNames", "excludeBranchNamesRun", "excludeBranchNamesPersistent" };

SimpleOutputModule::SimpleOutputModule() : Module(), m_file(0), m_experiment(0), m_runLow(0), m_eventLow(0),
  m_runHigh(0), m_eventHigh(0)
{
  //Set module properties
  setDescription("Writes datastore objects into a .root file");
  setPropertyFlags(c_Output | c_InitializeInProcess);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_tree[jj] = 0;
    m_treeNames[jj] = "NONE";
  }

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "TFile name.", string("SimpleOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 1);
  addParam("splitLevel", m_splitLevel, "Branch split level.", 99);

  addParam(c_SteerTreeNames[0], m_treeNames[0], "TTree name for event data. NONE for no output.", string("tree"));
  addParam(c_SteerTreeNames[1], m_treeNames[1], "TTree name for run data. NONE for no output.", string("run"));
  addParam(c_SteerTreeNames[2], m_treeNames[2], "TTree name for peristent data. NONE for no output.", string("persistent"));

  vector<string> branchNames;
  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of branches to be written from event map. Empty means all branches.", branchNames);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of branches to be written from run map. Empty means all branches.", branchNames);
  addParam(c_SteerBranchNames[2], m_branchNames[2], "Names of branches to be written from persistent map. Empty means all branches.", branchNames);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be written from event map. Branches also in branchNames are not written.", branchNames);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be written from run map. Branches also in branchNamesRun are not written.", branchNames);
  addParam(c_SteerExcludeBranchNames[2], m_excludeBranchNames[2], "Names of branches NOT to be written from persistent map. Branches also in branchNamesPersistent are not written.", branchNames);
}


SimpleOutputModule::~SimpleOutputModule() { }

void SimpleOutputModule::initialize()
{
  //needed if preprocessing is used
  gSystem->Load("libdataobjects");
  const int bufsize = 32000;

  //create a file level metadata object in the data store
  StoreObjPtr<FileMetaData>::registerPersistent("", DataStore::c_Persistent);

  const std::string& outputFileArgument = Environment::Instance().getOutputFileOverride();
  if (!outputFileArgument.empty())
    m_outputFileName = outputFileArgument;

  TDirectory* dir = gDirectory;
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "basf2 Event File");
  if (m_file->IsZombie()) {
    B2FATAL("Couldn't open file '" << m_outputFileName << "' for writing!");
    return;
  }
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
    if (m_treeNames[ii] == "NONE")
      continue; //nothing to do

    // check for duplicate treeNames
    for (int jj = 0; jj < ii; ++jj) {
      if (m_treeNames[ii] == m_treeNames[jj]) {
        B2ERROR(c_SteerTreeNames[ii] << " and " << c_SteerTreeNames[jj] << " are the same: " << m_treeNames[ii]);
      }
    }

    // check for duplicate branch names
    if (makeBranchNamesUnique(m_branchNames[ii]))
      B2WARNING(c_SteerBranchNames[ii] << " has duplicate entries.");
    if (makeBranchNamesUnique(m_excludeBranchNames[ii]))
      B2WARNING(c_SteerExcludeBranchNames[ii] << " has duplicate entries.");
    //m_branchNames[ii] and it's exclusion list are now sorted alphabetically and unique

    const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(ii));

    //check for branch names that are not in the DataStore
    for (unsigned int iBranch = 0; iBranch < m_branchNames[ii].size(); iBranch++) {
      if (map.find(m_branchNames[ii][iBranch]) == map.end()) {
        B2WARNING("The branch " << m_branchNames[ii][iBranch] << " has no entry in the DataStore.");
      }
    }
    for (unsigned int iBranch = 0; iBranch < m_excludeBranchNames[ii].size(); iBranch++) {
      if (map.find(m_excludeBranchNames[ii][iBranch]) == map.end()) {
        B2WARNING("The excluded branch " << m_excludeBranchNames[ii][iBranch] << " has no entry in the DataStore.");
      }
    }

    //create the tree and branches
    m_tree[ii] = new TTree(m_treeNames[ii].c_str(), m_treeNames[ii].c_str());
    for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
      const std::string& branchName = iter->first;
      //skip transient entries, excluded branches, and branches not in m_branchNames (if it is not empty)
      if (iter->second->isTransient ||
          binary_search(m_excludeBranchNames[ii].begin(), m_excludeBranchNames[ii].end(), branchName) ||
          (!m_branchNames[ii].empty() && !binary_search(m_branchNames[ii].begin(), m_branchNames[ii].end(), branchName))) {
        continue;
      }

      m_tree[ii]->Branch(branchName.c_str(), &iter->second->object, bufsize, m_splitLevel);
      m_tree[ii]->SetBranchAddress(branchName.c_str(), &iter->second->object);
      m_entries[ii].push_back(iter->second);
      B2DEBUG(50, "The branch " << branchName << " was created.");
    }
  }
  dir->cd();
  B2DEBUG(1, "SimpleOutput initialised.");
}

bool SimpleOutputModule::makeBranchNamesUnique(std::vector<std::string> &stringlist) const
{
  const size_t oldsize = stringlist.size();
  sort(stringlist.begin(), stringlist.end());
  stringlist.resize(unique(stringlist.begin(), stringlist.end()) - stringlist.begin());

  return (oldsize != stringlist.size());
}

void SimpleOutputModule::beginRun()
{
  B2DEBUG(1, "beginRun called.");

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (m_experiment && (m_experiment != eventMetaDataPtr->getExperiment())) {
    B2ERROR("The output file " << m_outputFileName << " contains more than one experiment.");
  }
}


void SimpleOutputModule::event()
{
  //fill Event data
  fillTree(DataStore::c_Event);

  //check for new parent file
  StoreObjPtr<FileMetaData> fileMetaDataPtr("", DataStore::c_Persistent);
  if (fileMetaDataPtr) {
    int id = fileMetaDataPtr->getId();
    if (id && (m_parents.empty() || (m_parents.back() != id))) {
      m_parents.push_back(id);
    }
  }

  // keep track of file level metadata
  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (!m_experiment) {
    m_experiment = eventMetaDataPtr->getExperiment();
  }
  if (!m_runLow || (eventMetaDataPtr->getRun() < m_runLow) || ((eventMetaDataPtr->getRun() == m_runLow) && (eventMetaDataPtr->getEvent() < m_eventLow))) {
    m_runLow = eventMetaDataPtr->getRun();
    m_eventLow = eventMetaDataPtr->getEvent();
  }
  if (!m_runHigh || (eventMetaDataPtr->getRun() > m_runHigh) || ((eventMetaDataPtr->getRun() == m_runHigh) && (eventMetaDataPtr->getEvent() > m_eventHigh))) {
    m_runHigh = eventMetaDataPtr->getRun();
    m_eventHigh = eventMetaDataPtr->getEvent();
  }
}


void SimpleOutputModule::endRun()
{
  //fill Run data
  fillTree(DataStore::c_Run);

  B2DEBUG(1, "endRun done.");
}


void SimpleOutputModule::terminate()
{
  //get pointer to file level metadata
  StoreObjPtr<FileMetaData> fileMetaDataPtr("", DataStore::c_Persistent);
  fileMetaDataPtr.create(true);

  if (m_tree[DataStore::c_Event]) {
    //create an index for the event tree
    TTree* tree = m_tree[DataStore::c_Event];
    if (tree->GetBranch("EventMetaData")) {
      tree->SetBranchAddress("EventMetaData", 0);
      tree->BuildIndex("1000000*EventMetaData.m_experiment+EventMetaData.m_run", "EventMetaData.m_event");
    }

    //fill the file level metadata
    fileMetaDataPtr->setEvents(tree->GetEntries());
    fileMetaDataPtr->setExperiment(m_experiment);
    fileMetaDataPtr->setLow(m_runLow, m_eventLow);
    fileMetaDataPtr->setHigh(m_runHigh, m_eventHigh);
  }

  //fill more file level metadata
  fileMetaDataPtr->setParents(m_parents);
  const char* release = getenv("BELLE2_RELEASE");
  if (!release) release = "unknown";
  char* site = getenv("BELLE2_SITE");
  if (!site) {
    char hostname[1024];
    gethostname(hostname, 1023);
    site = static_cast<char*>(hostname);
  }
  const char* user = getenv("BELLE2_USER");
  if (!user) user = getenv("USER");
  if (!user) user = getlogin();
  if (!user) user = "unknown";
  fileMetaDataPtr->setCreationData(release, time(0), site, user);
  fileMetaDataPtr->setRandom(RandomNumbers::getInitialSeed(), RandomNumbers::getInitialRandom());
  fileMetaDataPtr->setSteering(Environment::Instance().getSteering());

  //fill Persistent data
  fillTree(DataStore::c_Persistent);

  //write the trees
  TDirectory* dir = gDirectory;
  m_file->cd();
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_tree[ii]) {
      B2INFO("Write TTree " << m_treeNames[ii]);
      m_tree[ii]->Write(m_treeNames[ii].c_str(), TObject::kWriteDelete);
    }
  }
  dir->cd();

  // Clean up (moved from destructor)
  delete m_file;

  B2DEBUG(1, "terminate called");

}


void SimpleOutputModule::fillTree(DataStore::EDurability durability)
{
  if (!m_tree[durability]) return;

  //Check for entries whose object was not created.
  for (unsigned int i = 0; i < m_entries[durability].size(); i++) {
    if (!m_entries[durability][i]->ptr) {
      B2WARNING("Trying to write non-existing object to branch " << m_entries[durability][i]->name << ". Using default object.");
      //create object owned and deleted by branch
      m_tree[durability]->SetBranchAddress(m_entries[durability][i]->name.c_str(), 0);
    } else {
      m_tree[durability]->SetBranchAddress(m_entries[durability][i]->name.c_str(), &m_entries[durability][i]->object);
    }
  }
  m_tree[durability]->Fill();
}
