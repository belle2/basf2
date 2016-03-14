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

#include <framework/io/RootIOUtilities.h>
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
  addParam("outputFileName"  , m_outputFileName, "Name of the output file. Can be overridden using the -o argument to basf2.",
           string("RootOutput.root"));
  addParam("ignoreCommandLineOverride"  , m_ignoreCommandLineOverride,
           "Ignore override of file name via command line argument -o. Useful if you have multiple output modules in one path.", false);
  addParam("compressionLevel", m_compressionLevel,
           "0 for no, 1 for low, 9 for high compression. Level 1 usually reduces size by >50%, higher levels have no noticeable effect. On typical hard disks, disabling compression reduces write time by 10-20 %, but almost doubles read time, so you probably should leave this turned on.",
           1);
  addParam("splitLevel", m_splitLevel,
           "Branch split level: determines up to which depth object members will be saved in separate sub-branches in the tree. For arrays or objects with custom streamers, -1 is used instead to ensure the streamers are used. The default (99) usually gives the highest read performance with RootInput.",
           99);
  addParam("updateFileCatalog", m_updateFileCatalog,
           "Flag that specifies whether the file metadata catalog (Belle2FileCatalog.xml) is updated. (You can also set the BELLE2_FILECATALOG environment variable to NONE to get the same effect.)",
           true);

  vector<string> emptyvector;
  addParam(c_SteerBranchNames[0], m_branchNames[0],
           "Names of event durability branches to be saved. Empty means all branches. Objects with c_DontWriteOut flag added here will also be saved. (EventMetaData is always saved)",
           emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1],
           "Names of persistent durability branches to be saved. Empty means all branches. Objects with c_DontWriteOut flag added here will also be saved. (FileMetaData is always saved)",
           emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0],
           "Names of event durability branches NOT to be saved. Branches also in branchNames are not saved.", emptyvector);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1],
           "Names of persistent durability branches NOT to be saved. Branches also in branchNamesPersistent are not saved.", emptyvector);
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
  }
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; durability++) {
    DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(DataStore::EDurability(durability));
    set<string> branchList;
    for (auto pair : map)
      branchList.insert(pair.first);
    //skip branches the user doesn't want
    branchList = filterBranches(branchList, m_branchNames[durability], m_excludeBranchNames[durability], durability);

    //create the tree and branches
    m_tree[durability] = new TTree(c_treeNames[durability].c_str(), c_treeNames[durability].c_str());
    for (DataStore::StoreEntryIter iter = map.begin(); iter != map.end(); ++iter) {
      const std::string& branchName = iter->first;
      //skip transient entries (allow overriding via branchNames)
      if (iter->second.dontWriteOut
          && find(m_branchNames[durability].begin(), m_branchNames[durability].end(), branchName) == m_branchNames[durability].end())
        continue;
      //skip branches the user doesn't want
      if (branchList.count(branchName) == 0) {
        //make sure FileMetaData and EventMetaData are always included in the output
        if (((branchName != "FileMetaData") || (durability == DataStore::c_Event)) &&
            ((branchName != "EventMetaData") || (durability == DataStore::c_Persistent)))
          continue;
      }

      const TClass* entryClass = iter->second.objClass;

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
      m_tree[durability]->Branch(branchName.c_str(), &iter->second.object, bufsize, splitLevel);
      m_tree[durability]->SetBranchAddress(branchName.c_str(), &iter->second.object);
      m_entries[durability].push_back(&iter->second);
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
    if ((experiment < m_experimentLow) || ((experiment == m_experimentLow) && ((run < m_runLow) || ((run == m_runLow)
                                           && (event < m_eventLow))))) {
      m_experimentLow = experiment;
      m_runLow = run;
      m_eventLow = event;
    }
    if ((experiment > m_experimentHigh) || ((experiment == m_experimentHigh) && ((run > m_runHigh) || ((run == m_runHigh)
                                            && (event > m_eventHigh))))) {
      m_experimentHigh = experiment;
      m_runHigh = run;
      m_eventHigh = event;
    }
  }
}

void RootOutputModule::fillFileMetaData()
{
  //get pointer to file level metadata
  StoreObjPtr<FileMetaData> fileMetaDataPtr("", DataStore::c_Persistent);
  fileMetaDataPtr.create(true);

  if (m_tree[DataStore::c_Event]) {
    //create an index for the event tree
    TTree* tree = m_tree[DataStore::c_Event];
    unsigned long numEntries = tree->GetEntries();
    if (numEntries > 10000000) {
      //10M events correspond to about 240MB for the TTreeIndex object. for more than ~45M entries this causes crashes, broken files :(
      B2WARNING("Not building TTree index because of large number of events. The index object would conflict with ROOT limits on object size and cause problems.");
    } else if (tree->GetBranch("EventMetaData")) {
      tree->SetBranchAddress("EventMetaData", 0);
      RootIOUtilities::buildIndex(tree);
    }

    fileMetaDataPtr->setEvents(numEntries);
    fileMetaDataPtr->setLow(m_experimentLow, m_runLow, m_eventLow);
    fileMetaDataPtr->setHigh(m_experimentHigh, m_runHigh, m_eventHigh);
  }

  //fill more file level metadata
  fileMetaDataPtr->setParents(m_parents);
  const char* release = getenv("BELLE2_RELEASE");
  if (!release) release = "unknown";
  string site;
  const char* belle2_site = getenv("BELLE2_SITE");
  if (belle2_site) {
    site = belle2_site;
  } else {
    char hostname[1024];
    gethostname(hostname, 1023); //will not work well for ipv6
    hostname[1023] = '\0'; //if result is truncated, terminating null byte may be missing
    site = hostname;
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
}


void RootOutputModule::terminate()
{
  fillFileMetaData();

  //fill Persistent data
  fillTree(DataStore::c_Persistent);

  //write the trees
  TDirectory* dir = gDirectory;
  m_file->cd();
  for (int durability = 0; durability < DataStore::c_NDurabilityTypes; ++durability) {
    if (m_tree[durability]) {
      B2INFO("Write TTree " << c_treeNames[durability]);
      m_tree[durability]->Write(c_treeNames[durability].c_str(), TObject::kWriteDelete);
    }
  }
  dir->cd();

  delete m_file;

  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_entries[jj].clear();
  }
  m_parents.clear();

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
