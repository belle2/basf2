/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/rootio/RootOutputModule.h>

#include <framework/modules/rootio/RootIOUtilities.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/core/RandomNumbers.h>

#include <TClonesArray.h>
#include <TBaseClass.h>
#include <TTreeIndex.h>
#include <TProcessID.h>
#include <TSystem.h>

#include <time.h>


using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RootOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

RootOutputModule::RootOutputModule() : Module(), m_file(0), m_experiment(0), m_runLow(0), m_eventLow(0),
  m_runHigh(0), m_eventHigh(0)
{
  //Set module properties
  setDescription("Writes DataStore objects into a .root file. Use RootInput to read them again.");
  setPropertyFlags(c_Output);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_tree[jj] = 0;
  }

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "Name of the output file. Can be overridden using the -o argument to basf2.", string("RootOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticeable effect.", 1);
  addParam("splitLevel", m_splitLevel, "Branch split level.", 99);

  vector<string> emptyvector;
  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of branches to be written from event map. Empty means all branches. Transient objects added here will also be saved. (EventMetaData is always saved)", emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of branches to be written from persistent map. Empty means all branches. Transient objects added here will also be saved. (FileMetaData is always saved)", emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be written from event map. Branches also in branchNames are not written.", emptyvector);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be written from persistent map. Branches also in branchNamesPersistent are not written.", emptyvector);
}


RootOutputModule::~RootOutputModule() { }

void RootOutputModule::initialize()
{
  //needed if parallel processing is used
  gSystem->Load("libdataobjects");

  //buffer size in bytes (default value used by root)
  const int bufsize = 32000;

  //create a file level metadata object in the data store
  StoreObjPtr<FileMetaData>::registerPersistent("", DataStore::c_Persistent, false);

  m_outputFileName = getOutputFile();
  TDirectory* dir = gDirectory;
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "basf2 Event File");
  if (m_file->IsZombie()) {
    B2FATAL("Couldn't open file '" << m_outputFileName << "' for writing!");
    return;
  }
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
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
    m_tree[ii] = new TTree(c_treeNames[ii].c_str(), c_treeNames[ii].c_str());
    for (DataStore::StoreObjConstIter iter = map.begin(); iter != map.end(); ++iter) {
      const std::string& branchName = iter->first;
      //skip transient entries, excluded branches, and branches not in m_branchNames (if it is not empty)
      if ((iter->second->isTransient && !binary_search(m_branchNames[ii].begin(), m_branchNames[ii].end(), branchName)) ||
          binary_search(m_excludeBranchNames[ii].begin(), m_excludeBranchNames[ii].end(), branchName) ||
          (!m_branchNames[ii].empty() && !binary_search(m_branchNames[ii].begin(), m_branchNames[ii].end(), branchName))) {
        //make sure FileMetaData and EventMetaData are always included in the output
        if (((branchName != "FileMetaData") || (ii == DataStore::c_Event)) &&
            ((branchName != "EventMetaData") || (ii == DataStore::c_Persistent))) continue;
      }

      TClass* entryClass = iter->second->object->IsA();
      if (iter->second->isArray) {
        entryClass = static_cast<TClonesArray*>(iter->second->object)->GetClass();
      }

      if (!hasStreamers(entryClass))
        B2ERROR("The version number in the ClassDef() macro for class " << entryClass->GetName() << " must be at least 1 to enable I/O!");

      m_tree[ii]->Branch(branchName.c_str(), &iter->second->object, bufsize, m_splitLevel);
      m_tree[ii]->SetBranchAddress(branchName.c_str(), &iter->second->object);
      m_entries[ii].push_back(iter->second);
      B2DEBUG(50, "The branch " << branchName << " was created.");
    }
  }
  dir->cd();
}

bool RootOutputModule::hasStreamers(TClass* cl)
{
  if (cl == TObject::Class())
    return false;

  if (cl->GetClassVersion() <= 0) {
    // version number == 0 means no streamers for this class, check base classes
    TList* baseClasses = cl->GetListOfBases();
    TIter it(baseClasses);
    while (TBaseClass* base = static_cast<TBaseClass*>(it())) {
      if (hasStreamers(base->GetClassPointer()))
        return true;
    }
    //nothing found
    return false;
  } else {
    return true;
  }
}

void RootOutputModule::beginRun()
{
  B2DEBUG(1, "beginRun called.");

  StoreObjPtr<EventMetaData> eventMetaDataPtr;
  if (m_experiment && (m_experiment != eventMetaDataPtr->getExperiment())) {
    B2ERROR("The output file " << m_outputFileName << " contains more than one experiment.");
  }
}


void RootOutputModule::event()
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


void RootOutputModule::terminate()
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
      B2INFO("Write TTree " << c_treeNames[ii]);
      m_tree[ii]->Write(c_treeNames[ii].c_str(), TObject::kWriteDelete);
    }
  }
  dir->cd();

  delete m_file;

  B2DEBUG(1, "terminate() finished");
}


void RootOutputModule::fillTree(DataStore::EDurability durability)
{
  if (!m_tree[durability]) return;

  //Check for entries whose object was not created.
  for (unsigned int i = 0; i < m_entries[durability].size(); i++) {
    if (!m_entries[durability][i]->ptr) {
      //create object owned and deleted by branch
      m_tree[durability]->SetBranchAddress(m_entries[durability][i]->name.c_str(), 0);
    } else {
      m_tree[durability]->SetBranchAddress(m_entries[durability][i]->name.c_str(), &m_entries[durability][i]->object);
    }
  }
  m_tree[durability]->Fill();
}
