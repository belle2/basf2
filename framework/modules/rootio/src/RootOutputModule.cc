/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Thomas Kuhr                                 *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/rootio/RootOutputModule.h>

#include <framework/modules/rootio/RootIOUtilities.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/dataobjects/FileMetaData.h>
#include <framework/core/FileCatalog.h>
#include <framework/core/RandomNumbers.h>

#include <TClonesArray.h>

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

RootOutputModule::RootOutputModule() : Module(), m_file(0), m_experimentLow(1), m_runLow(0), m_eventLow(0),
  m_experimentHigh(0), m_runHigh(0), m_eventHigh(0)
{
  //Set module properties
  setDescription("Writes DataStore objects into a .root file. Data is stored in a TTree 'tree' for event-dependent and in 'persistent' for peristent data. You can use RootInput to read the files back into basf2.");
  setPropertyFlags(c_Output);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_tree[jj] = 0;
  }

  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "Name of the output file. Can be overridden using the -o argument to basf2.", string("RootOutput.root"));
  addParam("ignoreCommandLineOverride"  , m_ignoreCommandLineOverride, "Ignore override of filname via command line argument -o. Useful if you have multiple output modules in one path.", false);
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by 50%, higher levels have no noticeable effect.", 1);
  addParam("splitLevel", m_splitLevel, "Branch split level. For arrays or objects with custom streamers, -1 is used instead to ensure the streamers are used.", 99);
  addParam("updateFileCatalog", m_updateFileCatalog, "Flag that specifies whether the file metadata catalog is updated.", true);

  vector<string> emptyvector;
  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of branches to be written from event map. Empty means all branches. Objects with c_DontWriteOut flag added here will also be saved. (EventMetaData is always saved)", emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of branches to be written from persistent map. Empty means all branches. Objects with c_DontWriteOut flag added here will also be saved. (FileMetaData is always saved)", emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be written from event map. Branches also in branchNames are not written.", emptyvector);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be written from persistent map. Branches also in branchNamesPersistent are not written.", emptyvector);
}


RootOutputModule::~RootOutputModule() { }

void RootOutputModule::initialize()
{
  //buffer size in bytes (default value used by root)
  const int bufsize = 32000;

  //create a file level metadata object in the data store
  StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
  fileMetaData.registerInDataStore();

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

    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(ii));

    //check for branch names that are not in the DataStore
    for (unsigned int iBranch = 0; iBranch < m_branchNames[ii].size(); iBranch++) {
      if (map.find(m_branchNames[ii][iBranch]) == map.end()) {
        B2INFO("The branch " << m_branchNames[ii][iBranch] << " has no entry in the DataStore.");
      }
    }
    for (unsigned int iBranch = 0; iBranch < m_excludeBranchNames[ii].size(); iBranch++) {
      if (map.find(m_excludeBranchNames[ii][iBranch]) == map.end()) {
        B2INFO("The excluded branch " << m_excludeBranchNames[ii][iBranch] << " has no entry in the DataStore.");
      }
    }

    //create the tree and branches
    m_tree[ii] = new TTree(c_treeNames[ii].c_str(), c_treeNames[ii].c_str());
    for (DataStore::StoreEntryIter iter = map.begin(); iter != map.end(); ++iter) {
      const std::string& branchName = iter->first;
      //skip transient entries, excluded branches, and branches not in m_branchNames (if it is not empty)
      if ((iter->second.dontWriteOut && !binary_search(m_branchNames[ii].begin(), m_branchNames[ii].end(), branchName)) ||
          binary_search(m_excludeBranchNames[ii].begin(), m_excludeBranchNames[ii].end(), branchName) ||
          (!m_branchNames[ii].empty() && !binary_search(m_branchNames[ii].begin(), m_branchNames[ii].end(), branchName))) {
        //make sure FileMetaData and EventMetaData are always included in the output
        if (((branchName != "FileMetaData") || (ii == DataStore::c_Event)) &&
            ((branchName != "EventMetaData") || (ii == DataStore::c_Persistent))) continue;
      }

      TClass* entryClass = iter->second.object->IsA();
      if (iter->second.isArray) {
        entryClass = static_cast<TClonesArray*>(iter->second.object)->GetClass();
      }

      if (!hasStreamer(entryClass))
        B2ERROR("The version number in the ClassDef() macro for class " << entryClass->GetName() << " must be at least 1 to enable I/O!");

      int splitLevel = m_splitLevel;
      if (hasCustomStreamer(entryClass)) {
        B2DEBUG(100, entryClass->GetName() << " has custom streamer, setting split level -1 for this branch.");

        splitLevel = -1;
        if (iter->second.isArray) {
          //for arrays, we also don't want TClonesArray to go around our streamer
          static_cast<TClonesArray*>(iter->second.object)->BypassStreamer(kFALSE);
        }
      }
      m_tree[ii]->Branch(branchName.c_str(), &iter->second.object, bufsize, splitLevel);
      m_tree[ii]->SetBranchAddress(branchName.c_str(), &iter->second.object);
      m_entries[ii].push_back(&iter->second);
      B2DEBUG(150, "The branch " << branchName << " was created.");
    }
  }
  dir->cd();
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
  unsigned long experiment =  eventMetaDataPtr->getExperiment();
  unsigned long run =  eventMetaDataPtr->getRun();
  unsigned long event = eventMetaDataPtr->getEvent();
  if (m_experimentLow > m_experimentHigh) { //starting condition
    m_experimentLow = m_experimentHigh = experiment;
    m_runLow = m_runHigh = run;
    m_eventLow = m_eventHigh = event;
  } else {
    if ((experiment < m_experimentLow) || ((experiment == m_experimentLow) && ((run < m_runLow) || ((run == m_runLow) && (event < m_eventLow))))) {
      m_experimentLow = experiment;
      m_runLow = run;
      m_eventLow = event;
    }
    if ((experiment > m_experimentHigh) || ((experiment == m_experimentHigh) && ((run > m_runHigh) || ((run == m_runHigh) && (event > m_eventHigh))))) {
      m_experimentHigh = experiment;
      m_runHigh = run;
      m_eventHigh = event;
    }
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
    unsigned long numEntries = tree->GetEntries();
    if (numEntries > 10000000) {
      //10M events correspond to about 240MB for the TTreeIndex object. from ~45M on this causes crashes, broken files :(
      B2WARNING("Not building TTree index because of large number of events. The index object would conflict with ROOT limits on object size and cause problems.");
    } else if (tree->GetBranch("EventMetaData")) {
      tree->SetBranchAddress("EventMetaData", 0);
      RootIOUtilities::buildIndex(tree);
    }

    //fill the file level metadata
    fileMetaDataPtr->setEvents(numEntries);
    fileMetaDataPtr->setLow(m_experimentLow, m_runLow, m_eventLow);
    fileMetaDataPtr->setHigh(m_experimentHigh, m_runHigh, m_eventHigh);
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

  //register the file in the catalog
  if (m_updateFileCatalog) {
    FileCatalog::Instance().registerFile(m_outputFileName, *fileMetaDataPtr);
  }

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

  B2DEBUG(200, "terminate() finished");
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

  const bool writeError = m_file->TestBit(TFile::kWriteError);
  if (writeError) {
    //m_file deleted first so we have a chance of closing it (though that will probably fail)
    delete m_file;
    B2FATAL("A write error occured while saving '" << m_outputFileName  << "', please check if enough disk space is available.");
  }
}
