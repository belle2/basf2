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
  setPropertyFlags(c_Output);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_tree[jj] = 0;
    m_size[jj] = 0;
    m_objects[jj] = 0;
    m_treeNames[jj] = "NONE";
    m_done[jj] = false;
  }

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "TFile name.", string("SimpleOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 1);

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

  addParam("switchBranchNameMeaning", m_switchBranchNameMeaning, "DEPRECATED: switches branchNames and excludeBranchNames parameters.", false);
}


SimpleOutputModule::~SimpleOutputModule()
{
  delete m_file;

  for (size_t jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    delete[] m_objects[jj];
  }
}

void SimpleOutputModule::initialize()
{
  //create a file level metadata object in the data store
  StoreObjPtr<FileMetaData> fileMetaDataPtr("", DataStore::c_Persistent);

  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "basf2 Event File");
  if (m_file->IsZombie()) {
    B2FATAL("Couldn't open file '" << m_outputFileName << "' for writing!");
    return;
  }
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
    if (m_treeNames[ii] == "NONE")
      return; //nothing to do

    // check for duplicate treeNames
    for (int jj = 0; jj < ii; ++jj) {
      if (m_treeNames[ii] == m_treeNames[jj]) {
        B2ERROR(c_SteerTreeNames[ii] << " and " << c_SteerTreeNames[jj] << " are the same: " << m_treeNames[ii]);
      }
    }

    m_tree[ii] = new TTree(m_treeNames[ii].c_str(), m_treeNames[ii].c_str());

    if (makeBranchNamesUnique(m_branchNames[ii]))
      B2WARNING(c_SteerBranchNames[ii] << " has duplicate entries.");
    if (makeBranchNamesUnique(m_excludeBranchNames[ii]))
      B2WARNING(c_SteerExcludeBranchNames[ii] << " has duplicate entries.");
    //m_branchNames[ii] and it's exclusion list are now sorted alphabetically and unique
  }
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
  //Restore Object Count:
  if (!m_done[DataStore::c_Event]) {
    m_nObjID = TProcessID::GetObjectCount();
  } else {
    TProcessID::SetObjectCount(m_nObjID);
  }

  //fill Event data
  fillTree(DataStore::c_Event);

  //check for new parent file
  StoreObjPtr<FileMetaData> fileMetaDataPtr("", DataStore::c_Persistent);
  int id = fileMetaDataPtr->getId();
  if (id && (m_parents.empty() || (m_parents.back() != id))) {
    m_parents.push_back(id);
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
  //get pointer to event and file level metadata
  StoreObjPtr<FileMetaData> fileMetaDataPtr("", DataStore::c_Persistent);

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
  m_file->cd();
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_tree[ii]) {
      B2INFO("Write TTree " << m_treeNames[ii]);
      m_tree[ii]->Write(m_treeNames[ii].c_str(), TObject::kWriteDelete);
    }
  }

  B2DEBUG(1, "terminate called");
}

void SimpleOutputModule::fillTree(const DataStore::EDurability& durability)
{
  if (!m_tree[durability])
    return;

  if (!m_done[durability]) {
    //branches need to be set up here, because we need a list of object/array names from the data store
    //TODO: once all modules initialize saved arrays in their initialize() method,
    //this bit can also be moved to SimpleOutput::initialize().
    setupBranches(durability);

    //make sure setup is done only once
    m_done[durability] = true;
  } else {
    // gather the object pointers for this entry
    // no need to reconnect the arrays, as the TClonesArrays aren't deleted
    size_t sizeCounter = 0;
    const DataStore::StoreObjMap& map = DataStore::Instance().getObjectMap(durability);
    for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
      if (binary_search(m_branchNames[durability].begin(), m_branchNames[durability].end(), iter->first)) {
        m_objects[durability][sizeCounter] = iter->second;
        sizeCounter++;
      }
    }
    if (sizeCounter > m_sizeObj[durability]) {
      B2FATAL("More elements than in first event.");
      return;
    }
  }
  m_tree[durability]->Fill();
}

void SimpleOutputModule::setupBranches(DataStore::EDurability durability)
{
  if (m_switchBranchNameMeaning) {
    switchBranchNameMeaning(durability);
  }

  std::vector<std::string> branchesToBeSaved;
  for (int iMap = 0; iMap < 2; iMap++) {
    //first objects, then arrays
    const DataStore::StoreObjMap& map = (iMap == 0) ? DataStore::Instance().getObjectMap(durability) : DataStore::Instance().getArrayMap(durability);
    for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
      const std::string& branchName = iter->first;
      //check if branchName is not excluded, and that it's in m_branchNames (which also may be empty for all branches)
      if (!binary_search(m_excludeBranchNames[durability].begin(), m_excludeBranchNames[durability].end(), branchName)
          && (m_branchNames[durability].empty() || binary_search(m_branchNames[durability].begin(), m_branchNames[durability].end(), branchName))) {
        branchesToBeSaved.push_back(branchName);
      }
    }
    if (iMap == 0) { //we just made a list of all object branches
      m_sizeObj[durability] = branchesToBeSaved.size();
    }
  }
  //as we also added arrays, this is now the total size
  m_size[durability] = branchesToBeSaved.size();
  if (m_size[durability]) {
    m_objects[durability] = new TObject* [m_size[durability]];
  }

  m_branchNames[durability] = branchesToBeSaved;
  if (!m_branchNames[durability].empty()) {
    //sort new branch name list
    makeBranchNamesUnique(m_branchNames[durability]);

    // print out branch names
    B2DEBUG(2, "Sorted list of branch names for EDurability map " << durability << ":");
    for (vector<string>::iterator stringIter = m_branchNames[durability].begin(); stringIter != m_branchNames[durability].end(); ++stringIter) {
      B2DEBUG(2, *stringIter)
    }
  }


  //loop over all objects/arrays in store and create branches if they're in m_branchNames (=enabled)
  size_t sizeCounter = 0;
  for (int iMap = 0; iMap < 2; iMap++) {
    //first objects, then arrays
    const DataStore::StoreObjMap& map = (iMap == 0) ? DataStore::Instance().getObjectMap(durability) : DataStore::Instance().getArrayMap(durability);
    for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
      if (binary_search(m_branchNames[durability].begin(), m_branchNames[durability].end(), iter->first)) {
        m_objects[durability][sizeCounter] = iter->second;
        m_tree[durability]->Branch(iter->first.c_str(), &(m_objects[durability][sizeCounter]));
        sizeCounter++;
      }
    }
  }
  if (sizeCounter != m_branchNames[durability].size()) {
    B2FATAL("Number of initialized branches is different from constructed branch name list!");
  }
}


void SimpleOutputModule::switchBranchNameMeaning(const DataStore::EDurability& durability)
{
  m_branchNames[durability].swap(m_excludeBranchNames[durability]);
}
