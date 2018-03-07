/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Christian Pulvermacher, Thomas Kuhr         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <framework/modules/rootio/RootInputModule.h>

#include <framework/io/RootIOUtilities.h>
#include <framework/core/InputController.h>
#include <framework/pcore/Mergeable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/DependencyMap.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/utilities/NumberSequence.h>

#include <TClonesArray.h>
#include <TEventList.h>
#include <TObjArray.h>
#include <TChainElement.h>

using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

REG_MODULE(RootInput)

RootInputModule::RootInputModule() : Module(), m_nextEntry(0), m_lastPersistentEntry(-1), m_tree(0), m_persistent(0)
{
  //Set module properties
  setDescription("Reads objects/arrays from one or more .root files saved by the RootOutput module and makes them available through the DataStore. Files do not necessarily have to be local, http:// and root:// (for files in xrootd) URLs are supported as well.");
  setPropertyFlags(c_Input);

  //Parameter definition
  vector<string> emptyvector;
  addParam("inputFileName", m_inputFileName,
           "Input file name. For multiple files, use inputFileNames or wildcards instead. Can be overridden using the -i argument to basf2.",
           string(""));
  addParam("inputFileNames", m_inputFileNames,
           "List of input files. You may use shell-like expansions to specify multiple files, e.g. 'somePrefix_*.root' or 'file_[a,b]_[1-15].root'. Can be overridden using the -i argument to basf2.",
           emptyvector);
  addParam("entrySequences", m_entrySequences,
           "The number sequences (e.g. 23:42,101) defining the entries which are processed for each inputFileName."
           "Must be specified exactly once for each file to be opened."
           "The first event has the entry number 0.", emptyvector);
  addParam("ignoreCommandLineOverride"  , m_ignoreCommandLineOverride,
           "Ignore override of file name via command line argument -i.", false);

  addParam("skipNEvents", m_skipNEvents, "Skip this number of events before starting.", 0u);
  addParam("skipToEvent", m_skipToEvent, "Skip events until the event with "
           "the specified (experiment, run, event number) occurs. This parameter "
           "is useful for debugging to start with a specific event.", m_skipToEvent);

  addParam(c_SteerBranchNames[0], m_branchNames[0],
           "Names of event durability branches to be read. Empty means all branches. (EventMetaData is always read)", emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1],
           "Names of persistent durability branches to be read. Empty means all branches. (FileMetaData is always read)", emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0],
           "Names of event durability branches NOT to be read. Takes precedence over branchNames.", emptyvector);
  vector<string> excludePersistent({"ProcessStatistics"});
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1],
           "Names of persistent durability branches NOT to be read. Takes precedence over branchNamesPersistent.", excludePersistent);

  addParam("parentLevel", m_parentLevel,
           "Number of generations of parent files (files used as input when creating a file) to be read. This can be useful if a file is missing some information available in its parent. See https://confluence.desy.de/display/BI/Software+ParentFiles for details.",
           0);

  addParam("collectStatistics"  , m_collectStatistics,
           "Collect statistics on amount of data read and print statistics (seperate for input & parent files) after processing. Data is collected from TFile using GetBytesRead(), GetBytesReadExtra(), GetReadCalls()",
           false);
  addParam("recovery"  , m_recovery,
           "Try recovery when reading corrupted files. Might allow reading some of the event data but FileMetaData likely to be missing.",
           false);
  addParam("cacheSize", m_cacheSize,
           "file cache size in Mbytes. If negative, use root default", 0);
}


RootInputModule::~RootInputModule() { }

void RootInputModule::initialize()
{
  unsigned int skipNEventsOverride = Environment::Instance().getSkipEventsOverride();
  if (skipNEventsOverride != 0)
    m_skipNEvents = skipNEventsOverride;

  auto entrySequencesOverride = Environment::Instance().getEntrySequencesOverride();
  if (entrySequencesOverride.size() > 0)
    m_entrySequences = entrySequencesOverride;

  m_nextEntry = m_skipNEvents;
  m_lastPersistentEntry = -1;
  m_lastParentFileLFN = "";

  loadDictionaries();

  const vector<string>& inputFiles = getInputFiles();
  if (inputFiles.empty()) {
    B2FATAL("You have to set either the 'inputFileName' or the 'inputFileNames' parameter, or start basf2 with the '-i MyFile.root' option.");
  }
  if (!m_inputFileName.empty() && !m_inputFileNames.empty()) {
    B2FATAL("Cannot use both 'inputFileName' and 'inputFileNames' parameters!");
  }
  m_inputFileNames = expandWordExpansions(inputFiles);
  if (m_inputFileNames.empty()) {
    B2FATAL("No valid files specified!");
  }

  if (m_entrySequences.size() > 0 and m_inputFileNames.size() != m_entrySequences.size()) {
    B2FATAL("Number of provided filenames does not match the number of given entrySequences parameters: len(inputFileNames) = "
            << m_inputFileNames.size() << " len(entrySequences) = " << m_entrySequences.size());
  }

  m_inputFileName = "";
  //we'll only use m_inputFileNames from now on

  //Open TFile
  TDirectory* dir = gDirectory;
  for (const string& fileName : m_inputFileNames) {
    std::unique_ptr<TFile> f;
    try {
      f.reset(TFile::Open(fileName.c_str(), "READ"));
    } catch (std::logic_error&) {
      //this might happen for ~invaliduser/foo.root
      //actually undefined behaviour per standard, reported as ROOT-8490 in JIRA
    }
    if (!f || !f->IsOpen()) {
      B2FATAL("Couldn't open input file " + fileName);
    }
  }
  dir->cd();
  const auto loglevel = m_recovery ? LogConfig::c_Warning : LogConfig::c_Fatal;

  //Get TTree
  m_persistent = new TChain(c_treeNames[DataStore::c_Persistent].c_str());
  m_tree = new TChain(c_treeNames[DataStore::c_Event].c_str());
  for (const string& fileName : m_inputFileNames) {
    //nentries = -1 forces AddFile() to read headers
    if (!m_tree->AddFile(fileName.c_str(), -1))
      B2FATAL("Couldn't read header of TTree 'tree' in file '" << fileName << "'");
    if (!m_persistent->AddFile(fileName.c_str(), -1))
      B2LOG(loglevel, 0, "Couldn't read header of TTree 'persistent' in file '" << fileName << "'");
    B2INFO("Added file " + fileName);
  }
  // Set cache size
  if (m_cacheSize >= 0) m_tree->SetCacheSize(m_cacheSize * 1024 * 1024);

  // Check if the files we added to the Chain are unique,
  // if the same file is added multiple times the TEventList used for the eventSequence feature
  // will process each file only once with the union of both given sequences.
  // It is not clear if the user wants this, so we raise a fatal in this situation.
  {
    std::set<std::string> unique_filenames;

    // The following lines are directly from the ROOT documentation
    // see TChain::AddFile
    TObjArray* fileElements = m_tree->GetListOfFiles();
    TIter next(fileElements);
    TChainElement* chEl = 0;
    while ((chEl = (TChainElement*)next())) {
      unique_filenames.insert(chEl->GetTitle());
    }

    if (m_inputFileNames.size() != unique_filenames.size()) {
      if (m_entrySequences.size() > 0) {
        B2FATAL("You specified a file multiple times, and specified a sequence of entries which should be used for each file. "
                "Please specify each file only once if you're using the sequence feature!");
      } else {
        B2WARNING("You specified a file multiple times as input file.");
      }
    }
  }

  if (m_entrySequences.size() > 0) {
    TEventList* elist = new TEventList("input_event_list");
    for (unsigned int iFile = 0; iFile < m_entrySequences.size(); ++iFile) {
      int64_t offset = m_tree->GetTreeOffset()[iFile];
      int64_t next_offset = m_tree->GetTreeOffset()[iFile + 1];
      // check if Sequence consists only of ':', e.g. the whole file is requested
      if (m_entrySequences[iFile] == ":") {
        for (int64_t global_entry = offset; global_entry < next_offset; ++global_entry)
          elist->Enter(global_entry);
      } else {
        for (const auto& entry : generate_number_sequence(m_entrySequences[iFile])) {
          int64_t global_entry = entry + offset;
          if (global_entry >= next_offset) {
            B2WARNING("Given sequence contains entry numbers which are out of range. "
                      "I won't add any further events to the EventList for the current file.");
            break;
          } else {
            elist->Enter(global_entry);
          }
        }
      }
    }
    m_tree->SetEventList(elist);
  }

  B2DEBUG(100, "Opened tree '" + c_treeNames[DataStore::c_Persistent] + "' with " + m_persistent->GetEntries() << " entries.");
  B2DEBUG(100, "Opened tree '" + c_treeNames[DataStore::c_Event] + "' with " + m_tree->GetEntries() << " entries.");

  connectBranches(m_persistent, DataStore::c_Persistent, &m_persistentStoreEntries);
  readPersistentEntry(0);

  if (!connectBranches(m_tree, DataStore::c_Event, &m_storeEntries)) {
    delete m_tree;
    m_tree = 0; //don't try to read from there
  } else {
    InputController::setCanControlInput(true);
    InputController::setChain(m_tree);
  }

  if (m_parentLevel > 0) {
    createParentStoreEntries();
  } else if (m_parentLevel < 0) {
    B2ERROR("parentLevel must be >= 0!");
    return;
  }

  if (!m_skipToEvent.empty()) {
    // make sure the number of entries is exactly 3
    if (m_skipToEvent.size() != 3) {
      B2ERROR("skipToEvent must be a list of three values: experiment, run, event number");
      // ignore the value
      m_skipToEvent.clear();
    } else {
      InputController::setNextEntry(m_skipToEvent[0], m_skipToEvent[1], m_skipToEvent[2]);
    }
    if (m_nextEntry > 0) {
      B2ERROR("You cannot supply a number of events to skip (skipNEvents) and an "
              "event to skip to (skipToEvent) at the same time, ignoring skipNEvents");
      //force the number of skipped events to be zero
      m_nextEntry = 0;
    }
  }
}


void RootInputModule::event()
{
  if (!m_tree)
    return;

  const long nextEntry = InputController::getNextEntry();
  if (nextEntry >= 0 && nextEntry < InputController::numEntries()) {
    B2INFO("RootInput: will read entry " << nextEntry << " next.");
    m_nextEntry = nextEntry;
  } else if (InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0
             && InputController::getNextEvent() >= 0) {
    const long entry = RootIOUtilities::getEntryNumberWithEvtRunExp(m_tree->GetTree(), InputController::getNextEvent(),
                       InputController::getNextRun(), InputController::getNextExperiment());
    if (entry >= 0) {
      const long chainentry = m_tree->GetChainEntryNumber(entry);
      B2INFO("RootInput: will read entry " << chainentry << " (entry " << entry << " in current file) next.");
      m_nextEntry = chainentry;
    } else {
      B2ERROR("Couldn't find entry (" << InputController::getNextEvent() << ", " << InputController::getNextRun() << ", " <<
              InputController::getNextExperiment() << ") in file! Loading entry " << m_nextEntry << " instead.");
    }
  }
  InputController::eventLoaded(m_nextEntry);

  readTree();
  m_nextEntry++;
}


void RootInputModule::terminate()
{
  B2DEBUG(200, "Term called");
  if (m_collectStatistics and m_tree) {
    //add stats for last file
    m_readStats.addFromFile(m_tree->GetFile());
  }
  delete m_tree;
  delete m_persistent;
  ReadStats parentReadStats;
  for (auto entry : m_parentTrees) {
    TFile* f = entry.second->GetCurrentFile();
    if (m_collectStatistics)
      parentReadStats.addFromFile(f);

    delete f;
  }

  if (m_collectStatistics) {
    B2INFO("Statistics for event tree: " << m_readStats.getString());
    B2INFO("Statistics for event tree (parent files): " << parentReadStats.getString());
  }

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    m_connectedBranches[ii].clear();
  }
  m_storeEntries.clear();
  m_persistentStoreEntries.clear();
  m_parentStoreEntries.clear();
  m_parentTrees.clear();
}


void RootInputModule::readTree()
{
  if (!m_tree)
    return;

  //keep snapshot of TFile stats (to use if it changes)
  ReadStats currentEventStats;
  if (m_collectStatistics) {
    currentEventStats.addFromFile(m_tree->GetFile());
  }

  // Check if there are still new entries available.
  int  localEntryNumber = m_nextEntry;
  if (m_entrySequences.size() > 0) {
    localEntryNumber = m_tree->GetEntryNumber(localEntryNumber);
  }
  localEntryNumber = m_tree->LoadTree(localEntryNumber);

  if (localEntryNumber == -2) {
    return; //end of file
  } else if (localEntryNumber < 0) {
    B2FATAL("Failed to load tree, corrupt file? Check standard error for additional messages. (TChain::LoadTree() returned error " <<
            localEntryNumber << ")");
  }
  B2DEBUG(200, "Reading file entry " << m_nextEntry);

  //Make sure transient members of objects are reinitialised
  for (auto entry : m_storeEntries) {
    entry->resetForGetEntry();
  }
  for (auto storeEntries : m_parentStoreEntries) {
    for (auto entry : storeEntries) {
      entry->resetForGetEntry();
    }
  }

  int bytesRead = m_tree->GetTree()->GetEntry(localEntryNumber);
  if (bytesRead <= 0) {
    B2FATAL("Could not read 'tree' entry " << m_nextEntry << " in file " << m_tree->GetCurrentFile()->GetName());
  }

  //In case someone is tempted to change this:
  // TTree::GetCurrentFile() returns a TFile pointer to a fixed location,
  // calling GetName() on the TFile almost works as expected, but starts with the
  // last file in a TChain. (-> we re-read the first persistent tree with TChain,
  // with ill results for Mergeable objects.)
  // GetTreeNumber() also starts at the last entry before we read the first event from m_tree,
  // so we'll save the last persistent tree loaded and only reload on changes.
  StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
  const long treeNum = m_tree->GetTreeNumber();
  const bool fileChanged = (m_lastPersistentEntry != treeNum);
  if (fileChanged) {
    if (m_collectStatistics) {
      m_readStats.add(currentEventStats);
    }
    // file changed, read the FileMetaData object from the persistent tree and update the parent file metadata
    readPersistentEntry(treeNum);
    if (!m_recovery or fileMetaData)
      B2INFO("Loading new input file with physical path:" << FileCatalog::Instance().getPhysicalFileName(fileMetaData->getLfn()));
  }

  for (auto entry : m_storeEntries) {
    if (!entry->object) {
      entryNotFound("Event durability tree (global entry: " + std::to_string(m_nextEntry) + ")", entry->name, fileChanged);
      entry->recoverFromNullObject();
      entry->ptr = nullptr;
    } else {
      entry->ptr = entry->object;
    }
  }

  if (m_parentLevel > 0) {
    if (!readParentTrees())
      B2FATAL("Could not read data from parent file!");
  }

}


bool RootInputModule::connectBranches(TTree* tree, DataStore::EDurability durability, StoreEntries* storeEntries)
{
  B2DEBUG(100, "File changed, loading persistent data.");
  DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(durability);

  //Go over the branchlist and connect the branches with DataStore entries
  const TObjArray* branches = tree->GetListOfBranches();
  if (!branches) {
    const auto loglevel = m_recovery ? LogConfig::c_Warning : LogConfig::c_Fatal;
    B2LOG(loglevel, 0, "Tree '" << tree->GetName() << "' doesn't contain any branches!");
    return false; //stop in case this wasn't fatal
  }
  set<string> branchList;
  for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
    TBranch* branch = static_cast<TBranch*>(branches->At(jj));
    if (branch)
      branchList.insert(branch->GetName());
  }
  //skip branches the user doesn't want
  branchList = filterBranches(branchList, m_branchNames[durability], m_excludeBranchNames[durability], durability);
  for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
    TBranch* branch = static_cast<TBranch*>(branches->At(jj));
    if (!branch) continue;
    const std::string branchName = branch->GetName();
    //skip already connected branches
    if (m_connectedBranches[durability].find(branchName) != m_connectedBranches[durability].end())
      continue;
    if (branchList.count(branchName) == 0) {
      //make sure FileMetaData and EventMetaData of the main file are always loaded
      if (((branchName != "FileMetaData") || (tree != m_persistent)) &&
          ((branchName != "EventMetaData") || (tree != m_tree))) {
        tree->SetBranchStatus(branchName.c_str(), 0);
        continue;
      }
    }

    //Get information about the object in the branch
    TObject* objectPtr = 0;
    branch->SetAddress(&objectPtr);
    branch->GetEntry();
    bool array = (string(branch->GetClassName()) == "TClonesArray");
    TClass* objClass = 0;
    if (array)
      objClass = (static_cast<TClonesArray*>(objectPtr))->GetClass();
    else
      objClass = objectPtr->IsA();
    delete objectPtr;

    //Create a DataStore entry and connect the branch address to it
    if (!DataStore::Instance().registerEntry(branchName, durability, objClass, array, DataStore::c_WriteOut)) {
      tree->SetBranchStatus(branch->GetName(), 0);
      continue;
    }
    DataStore::StoreEntry& entry = (map.find(branchName))->second;
    tree->SetBranchAddress(branch->GetName(), &(entry.object));
    if (storeEntries) storeEntries->push_back(&entry);


    //Keep track of already connected branches
    m_connectedBranches[durability].insert(branchName);
  }

  return true;
}


bool RootInputModule::createParentStoreEntries()
{
  // get the experiment/run/event number and parentLfn of the first entry
  TBranch* branch = m_tree->GetBranch("EventMetaData");
  char* address = branch->GetAddress();
  EventMetaData* eventMetaData = 0;
  branch->SetAddress(&eventMetaData);
  branch->GetEntry(0);
  int experiment = eventMetaData->getExperiment();
  int run = eventMetaData->getRun();
  unsigned int event = eventMetaData->getEvent();
  std::string parentLfn = eventMetaData->getParentLfn();
  branch->SetAddress(address);

  // loop over parents and get their metadata
  for (int level = 0; level < m_parentLevel; level++) {
    // open the parent file
    TDirectory* dir = gDirectory;
    const std::string parentPfn = FileCatalog::Instance().getPhysicalFileName(parentLfn);
    TFile* file = TFile::Open(parentPfn.c_str(), "READ");
    dir->cd();
    if (!file || !file->IsOpen()) {
      B2ERROR("Couldn't open parent file " << parentLfn << " " << parentPfn);
      return false;
    }

    // get the event tree and connect its branches
    TTree* tree = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Event].c_str()));
    if (!tree) {
      B2ERROR("No tree " << c_treeNames[DataStore::c_Event] << " found in " << parentPfn);
      return false;
    }
    if (int(m_parentStoreEntries.size()) <= level)  m_parentStoreEntries.resize(level + 1);
    connectBranches(tree, DataStore::c_Event, &m_parentStoreEntries[level]);
    m_parentTrees.insert(std::make_pair(parentLfn, tree));

    // get the persistent tree and read its branches
    TTree* persistent = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Persistent].c_str()));
    if (!persistent) {
      B2ERROR("No tree " << c_treeNames[DataStore::c_Persistent] << " found in " << parentPfn);
      return false;
    }
    connectBranches(persistent, DataStore::c_Persistent, 0);

    // get parent LFN of parent
    EventMetaData* metaData = nullptr;
    tree->SetBranchAddress("EventMetaData", &metaData);
    long entry = RootIOUtilities::getEntryNumberWithEvtRunExp(tree, event, run, experiment);
    tree->GetBranch("EventMetaData")->GetEntry(entry);
    parentLfn = metaData->getParentLfn();
  }

  return true;
}


bool RootInputModule::readParentTrees()
{
  const StoreObjPtr<EventMetaData> eventMetaData;
  int experiment = eventMetaData->getExperiment();
  int run = eventMetaData->getRun();
  unsigned int event = eventMetaData->getEvent();

  std::string parentLfn = eventMetaData->getParentLfn();
  for (int level = 0; level < m_parentLevel; level++) {
    const std::string& parentPfn = FileCatalog::Instance().getPhysicalFileName(parentLfn);

    // Open the parent file if we haven't done this already
    TTree* tree = nullptr;
    if (m_parentTrees.find(parentLfn) == m_parentTrees.end()) {
      TDirectory* dir = gDirectory;
      B2DEBUG(100, "Opening parent file: " << parentPfn);
      TFile* file = TFile::Open(parentPfn.c_str(), "READ");
      dir->cd();
      if (!file || !file->IsOpen()) {
        B2ERROR("Couldn't open parent file " << parentPfn);
        return false;
      }
      tree = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Event].c_str()));
      if (!tree) {
        B2ERROR("No tree " << c_treeNames[DataStore::c_Event] << " found in " << parentPfn);
        return false;
      }
      for (auto entry : m_parentStoreEntries[level]) {
        tree->SetBranchAddress(entry->name.c_str(), &(entry->object));
      }
      m_parentTrees.insert(std::make_pair(parentLfn, tree));
    } else {
      tree = m_parentTrees[parentLfn];
    }

    // get entry number in parent tree
    long entryNumber = RootIOUtilities::getEntryNumberWithEvtRunExp(tree, event, run, experiment);
    if (entryNumber < 0) {
      B2ERROR("No event " << experiment << "/" << run << "/" << event << " in parent file " << parentPfn);
      return false;
    }

    // read the tree and mark the data read in the data store
    EventMetaData* parentMetaData = nullptr;
    tree->SetBranchAddress("EventMetaData", &parentMetaData);
    tree->GetEntry(entryNumber);
    for (auto entry : m_parentStoreEntries[level]) {
      entry->ptr = entry->object;
    }

    // set the parent LFN to the next level
    parentLfn = parentMetaData->getParentLfn();
  }

  addEventListForIndexFile(parentLfn);

  return true;
}

void RootInputModule::addEventListForIndexFile(const std::string& parentLfn)
{
  //is this really an index file? (=only EventMetaData stored)
  if (!(m_parentLevel > 0 and m_storeEntries.size() == 1))
    return;
  //did we handle the current parent file already?
  if (parentLfn == m_lastParentFileLFN)
    return;
  m_lastParentFileLFN = parentLfn;

  B2INFO("Index file detected, scanning to generate event list.");
  TTree* tree = m_parentTrees.at(parentLfn);

  //both types of list work, TEventList seems to result in slightly less data being read.
  TEventList* elist = new TEventList("parent_entrylist");
  //TEntryListArray* elist = new TEntryListArray();

  TBranch* branch = m_tree->GetBranch("EventMetaData");
  auto* address = branch->GetAddress();
  EventMetaData* eventMetaData = 0;
  branch->SetAddress(&eventMetaData);
  long nEntries = m_tree->GetEntries();
  for (long i = m_nextEntry; i < nEntries; i++) {
    branch->GetEntry(i);
    int experiment = eventMetaData->getExperiment();
    int run = eventMetaData->getRun();
    unsigned int event = eventMetaData->getEvent();
    const std::string& newParentLfn = eventMetaData->getParentLfn();

    if (parentLfn != newParentLfn) {
      //parent file changed, stopping for now
      break;
    }
    long entry = RootIOUtilities::getEntryNumberWithEvtRunExp(tree, event, run, experiment);
    elist->Enter(entry);
  }
  branch->SetAddress(address);

  if (tree) {
    tree->SetEventList(elist);
    //tree->SetEntryList(elist);
  }
}

void RootInputModule::entryNotFound(std::string entryOrigin, std::string name, bool fileChanged)
{
  if (name == "ProcessStatistics" or DataStore::Instance().getDependencyMap().isUsedAs(name, DependencyMap::c_Input)) {
    B2FATAL(entryOrigin << " in " << m_tree->GetFile()->GetName() << " does not contain required object " << name << ", aborting.");
  } else if (fileChanged) {
    B2WARNING(entryOrigin << " in " << m_tree->GetFile()->GetName() << " does not contain object " << name <<
              " that was present in a previous entry.");
  }
}

void RootInputModule::readPersistentEntry(long fileEntry)
{
  m_lastPersistentEntry = fileEntry;

  for (auto entry : m_persistentStoreEntries) {
    bool isMergeable = entry->object->InheritsFrom(Mergeable::Class());
    TObject* copyOfPreviousVersion = nullptr;
    if (isMergeable) {
      copyOfPreviousVersion = entry->object->Clone();
    }
    entry->resetForGetEntry();
    //ptr stores old value (or nullptr)
    entry->ptr = copyOfPreviousVersion;
  }

  int bytesRead = m_persistent->GetEntry(fileEntry);
  if (bytesRead <= 0) {
    const char* name = m_tree->GetCurrentFile() ? m_tree->GetCurrentFile()->GetName() : "<unknown>";
    const auto loglevel = m_recovery ? LogConfig::c_Warning : LogConfig::c_Fatal;
    B2LOG(loglevel, 0, "Could not read 'persistent' TTree #" << fileEntry << " in file " << name);
    return;
  }

  for (auto entry : m_persistentStoreEntries) {
    if (entry->object) {
      bool isMergeable = entry->object->InheritsFrom(Mergeable::Class());
      if (isMergeable) {
        const Mergeable* oldObj = static_cast<Mergeable*>(entry->ptr);
        Mergeable* newObj = static_cast<Mergeable*>(entry->object);
        newObj->merge(oldObj);

        delete entry->ptr;
      }
      entry->ptr = entry->object;
    } else {
      entryNotFound("Persistent tree", entry->name);
      entry->recoverFromNullObject();
      entry->ptr = nullptr;
    }
  }
}
