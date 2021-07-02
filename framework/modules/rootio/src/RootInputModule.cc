/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/


#include <framework/modules/rootio/RootInputModule.h>

#include <framework/io/RootIOUtilities.h>
#include <framework/io/RootFileInfo.h>
#include <framework/core/FileCatalog.h>
#include <framework/core/InputController.h>
#include <framework/pcore/Mergeable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/DependencyMap.h>
#include <framework/dataobjects/EventMetaData.h>
#include <framework/utilities/NumberSequence.h>
#include <framework/utilities/ScopeGuard.h>
#include <framework/database/Configuration.h>

#include <TClonesArray.h>
#include <TEventList.h>
#include <TObjArray.h>
#include <TChainElement.h>
#include <TError.h>

#include <iomanip>

using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

REG_MODULE(RootInput)

RootInputModule::RootInputModule() : Module(), m_nextEntry(0), m_lastPersistentEntry(-1), m_tree(nullptr), m_persistent(nullptr)
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
  addParam("cacheSize", m_cacheSize,
           "file cache size in Mbytes. If negative, use root default", 0);

  addParam("discardErrorEvents", m_discardErrorEvents,
           "Discard events with an error flag != 0", m_discardErrorEvents);
  addParam("silentErrrorDiscardMask", m_discardErrorMask,
           "Bitmask of error flags to silently discard without raising a WARNING. Should be a combination of the ErrorFlags defined "
           "in the EventMetaData. No Warning will be issued when discarding an event if the error flag consists exclusively of flags "
           "present in this mask", m_discardErrorMask);
}

RootInputModule::~RootInputModule() = default;

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

  const vector<string>& inputFiles = getFileNames();
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
  // we'll only use m_inputFileNames from now on

  // so let's create the chain objects ...
  m_persistent = new TChain(c_treeNames[DataStore::c_Persistent].c_str());
  m_tree = new TChain(c_treeNames[DataStore::c_Event].c_str());

  // time for sanity checks. The problem is that this needs to read a few bytes
  // from every input file so for jobs with large amount of input files this
  // will not be efficient.
  // TODO: We might want to create a different input module which will not
  //       check anything and require manual input like the number of events in
  //       each file and the global tags to use.  That would be more efficient
  //       but also less safe

  // list of required branches. We keep this empty for now and only fill
  // it after we checked the first file to make sure all other files have the
  // same branches available.
  std::set<std::string> requiredEventBranches;
  std::set<std::string> requiredPersistentBranches;
  // Event metadata from all files, keep it around for sanity checks and globaltag replay
  std::vector<FileMetaData> fileMetaData;
  // and if so, what is the sum
  std::result_of<decltype(&FileMetaData::getMcEvents)(FileMetaData)>::type sumInputMCEvents{0};

  // scope for local variables
  {
    // temporarily disable some root warnings
    auto rootWarningGuard = ScopeGuard::guardValue(gErrorIgnoreLevel, kWarning + 1);
    // do all files have a consistent number of MC events? that is all positive or all zero
    bool validInputMCEvents{true};
    for (const string& fileName : m_inputFileNames) {
      // read metadata and create sum of MCEvents and global tags
      try {
        RootIOUtilities::RootFileInfo fileInfo(fileName);
        FileMetaData meta = fileInfo.getFileMetaData();
        if (meta.getNEvents() == 0) {
          B2WARNING("File appears to be empty, skipping" << LogVar("filename", fileName));
          continue;
        }
        realDataWorkaround(meta);
        fileMetaData.push_back(meta);
        // make sure we only look at data or MC. For the first file this is trivially true
        if (fileMetaData.front().isMC() != meta.isMC()) {
          throw std::runtime_error("Mixing real data and simulated data for input files is not supported");
        }
        // accumulate number of inputMCEvents now
        if (validInputMCEvents) {
          // make sure that all files have either a non-zero or zero mcevents.
          if ((sumInputMCEvents > 0 and meta.getMcEvents() == 0)) {
            B2WARNING("inconsistent input files: zero mcEvents, setting total number of MC events to zero" << LogVar("filename", fileName));
            validInputMCEvents = false;
          }
          // So accumulate the number of MCEvents but let's be careful to not have an overflow here
          if (__builtin_add_overflow(sumInputMCEvents, meta.getMcEvents(), &sumInputMCEvents)) {
            B2FATAL("Number of MC events is too large and cannot be represented anymore");
          }
        }
        // for the first file we don't know what branches are required but now we can determine them as we know the file can be opened
        if (requiredEventBranches.empty()) {
          // make sure we have event meta data
          fileInfo.checkMissingBranches({"EventMetaData"}, false);
          requiredEventBranches = fileInfo.getBranchNames(false);
          // filter the branches depending on what the user selected. Note we
          // do the same thing again in connectBranches but we leave it like
          // that because we also want to read branches from parent files
          // selectively and thus we need to filter the branches there anyway.
          // Here we just do it to ensure all files we read directly (which is
          // 99% of the use case) contain all the branches we want.
          requiredEventBranches = RootIOUtilities::filterBranches(requiredEventBranches, m_branchNames[DataStore::c_Event],
                                                                  m_excludeBranchNames[DataStore::c_Event], DataStore::c_Event);
          // but make sure we always have EventMetaData ...
          requiredEventBranches.emplace("EventMetaData");

          // Same for persistent data ...
          requiredPersistentBranches = fileInfo.getBranchNames(true);
          // filter the branches depending on what the user selected
          requiredPersistentBranches = RootIOUtilities::filterBranches(requiredPersistentBranches, m_branchNames[DataStore::c_Persistent],
                                       m_excludeBranchNames[DataStore::c_Persistent], DataStore::c_Persistent);
        } else {
          // ok we already have the list ... so let's make sure following files have the same branches
          fileInfo.checkMissingBranches(requiredEventBranches, false);
          fileInfo.checkMissingBranches(requiredPersistentBranches, true);
        }
        // ok, so now we have the file, add it to the chain. We trust the amount of events from metadata here.
        if (m_tree->AddFile(fileName.c_str(), meta.getNEvents()) == 0 || m_persistent->AddFile(fileName.c_str(), 1) == 0) {
          throw std::runtime_error("Could not add file to TChain");
        }
        B2INFO("Added file " + fileName);
      } catch (std::exception& e) {
        B2FATAL("Could not open input file " << std::quoted(fileName) << ": " << e.what());
      }
    }
  }

  if (m_tree->GetNtrees() == 0) B2FATAL("No file could be opened, aborting");
  // Set cache size TODO: find out if files are remote and use a bigger default
  // value if at least one file is non-local
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
    TChainElement* chEl = nullptr;
    while ((chEl = (TChainElement*)next())) {
      if (!unique_filenames.insert(chEl->GetTitle()).second) {
        B2WARNING("The input file '" << chEl->GetTitle() << "' was specified more than once");
        // seems we have duplicate files so we process events more than once. Disable forwarding of MC event number
        m_processingAllEvents = false;
      }
    }
    if ((unsigned int)m_tree->GetNtrees() != unique_filenames.size() && m_entrySequences.size() > 0) {
      B2FATAL("You specified a file multiple times, and specified a sequence of entries which should be used for each file. "
              "Please specify each file only once if you're using the sequence feature!");
    }
  }

  if (m_entrySequences.size() > 0) {
    auto* elist = new TEventList("input_event_list");
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

  B2DEBUG(33, "Opened tree '" + c_treeNames[DataStore::c_Persistent] + "'" << LogVar("entries", m_persistent->GetEntriesFast()));
  B2DEBUG(33, "Opened tree '" + c_treeNames[DataStore::c_Event] + "'" << LogVar("entries", m_tree->GetEntriesFast()));

  connectBranches(m_persistent, DataStore::c_Persistent, &m_persistentStoreEntries);
  readPersistentEntry(0);

  if (!connectBranches(m_tree, DataStore::c_Event, &m_storeEntries)) {
    delete m_tree;
    m_tree = nullptr; //don't try to read from there
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

  // Let's check check if we process everything
  //   * all filenames unique (already done above)
  //   * no event skipping either with skipN, entry sequences or skipToEvent
  //   * no -n or process(path, N) with N <= the number of entries in our files
  unsigned int maxEvent = Environment::Instance().getNumberEventsOverride();
  m_processingAllEvents &= m_skipNEvents == 0 && m_entrySequences.size() == 0;
  m_processingAllEvents &= (maxEvent == 0 || maxEvent >= InputController::numEntries());

  if (!m_skipToEvent.empty()) {
    // Skipping to some specific event is also not processing all events ...
    m_processingAllEvents = false;
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

  // Processing everything so forward number of MC events
  if (m_processingAllEvents) {
    Environment::Instance().setNumberOfMCEvents(sumInputMCEvents);
  }
  // And setup global tag replay ...
  Conditions::Configuration::getInstance().setInputMetadata(fileMetaData);
}


void RootInputModule::event()
{
  if (!m_tree)
    return;

  while (true) {
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

    // check for events with errors
    unsigned int errorFlag = 0;
    if (m_discardErrorEvents && (m_nextEntry >= 0)) {
      const StoreObjPtr<EventMetaData> eventMetaData;
      errorFlag = eventMetaData->getErrorFlag();
      if (errorFlag != 0) {
        if (errorFlag & ~m_discardErrorMask) {
          B2WARNING("Discarding corrupted event" << LogVar("errorFlag", errorFlag) << LogVar("experiment", eventMetaData->getExperiment())
                    << LogVar("run", eventMetaData->getRun()) << LogVar("event", eventMetaData->getEvent()));
        }
        // make sure this event is not used if it's the last one in the file
        eventMetaData->setEndOfData();
      }
    }
    if (errorFlag == 0) break;
  }
}


void RootInputModule::terminate()
{
  if (m_collectStatistics and m_tree) {
    //add stats for last file
    m_readStats.addFromFile(m_tree->GetFile());
  }
  delete m_tree;
  delete m_persistent;
  ReadStats parentReadStats;
  for (const auto& entry : m_parentTrees) {
    TFile* f = entry.second->GetCurrentFile();
    if (m_collectStatistics)
      parentReadStats.addFromFile(f);

    delete f;
  }

  if (m_collectStatistics) {
    B2INFO("Statistics for event tree: " << m_readStats.getString());
    B2INFO("Statistics for event tree (parent files): " << parentReadStats.getString());
  }

  for (auto& branch : m_connectedBranches) {
    branch.clear();
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
    m_nextEntry = -2;
    return; //end of file
  } else if (localEntryNumber < 0) {
    B2FATAL("Failed to load tree, corrupt file? Check standard error for additional messages. (TChain::LoadTree() returned error " <<
            localEntryNumber << ")");
  }
  B2DEBUG(39, "Reading file entry " << m_nextEntry);

  //Make sure transient members of objects are reinitialised
  for (auto entry : m_storeEntries) {
    entry->resetForGetEntry();
  }
  for (const auto& storeEntries : m_parentStoreEntries) {
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
    B2INFO("Loading new input file"
           << LogVar("filename", m_tree->GetFile()->GetName())
           << LogVar("metadata LFN", fileMetaData->getLfn()));
  }
  realDataWorkaround(*fileMetaData);

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

  // Nooow, if the object didn't exist in the event when we wrote it to File we still have it in the file but it's marked as invalid Object.
  // So go through everything and check for the bit and invalidate as necessary
  for (auto entry : m_storeEntries) {
    if (entry->object->TestBit(kInvalidObject)) entry->invalidate();
  }
  for (const auto& storeEntries : m_parentStoreEntries) {
    for (auto entry : storeEntries) {
      if (entry->object->TestBit(kInvalidObject)) entry->invalidate();
    }
  }
}

bool RootInputModule::connectBranches(TTree* tree, DataStore::EDurability durability, StoreEntries* storeEntries)
{
  B2DEBUG(30, "File changed, loading persistent data.");
  DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(durability);
  //Go over the branchlist and connect the branches with DataStore entries
  const TObjArray* branchesObjArray = tree->GetListOfBranches();
  if (!branchesObjArray) {
    B2FATAL("Tree '" << tree->GetName() << "' doesn't contain any branches!");
  }
  std::vector<TBranch*> branches;
  set<string> branchList;
  for (int jj = 0; jj < branchesObjArray->GetEntriesFast(); jj++) {
    auto* branch = static_cast<TBranch*>(branchesObjArray->At(jj));
    if (!branch) continue;
    branchList.insert(branch->GetName());
    branches.emplace_back(branch);
    // start with all branches disabled and only enable what we read
    setBranchStatus(branch, false);
  }
  //skip branches the user doesn't want
  branchList = filterBranches(branchList, m_branchNames[durability], m_excludeBranchNames[durability], durability, true);
  for (TBranch* branch : branches) {
    const std::string branchName = branch->GetName();
    //skip already connected branches
    if (m_connectedBranches[durability].find(branchName) != m_connectedBranches[durability].end())
      continue;

    if ((branchList.count(branchName) == 0) and
        ((branchName != "FileMetaData") || (tree != m_persistent)) and
        ((branchName != "EventMetaData") || (tree != m_tree))) {
      continue;
    }
    auto found = setBranchStatus(branch, true);
    B2DEBUG(32, "Enabling branch" << LogVar("branchName", branchName)
            << LogVar("children found", found));

    //Get information about the object in the branch
    TObject* objectPtr = nullptr;
    branch->SetAddress(&objectPtr);
    branch->GetEntry();
    bool array = (string(branch->GetClassName()) == "TClonesArray");
    TClass* objClass = nullptr;
    if (array)
      objClass = (static_cast<TClonesArray*>(objectPtr))->GetClass();
    else
      objClass = objectPtr->IsA();
    delete objectPtr;

    //Create a DataStore entry and connect the branch address to it
    if (!DataStore::Instance().registerEntry(branchName, durability, objClass, array, DataStore::c_WriteOut)) {
      B2FATAL("Cannot connect branch to datastore" << LogVar("branchName", branchName));
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
  EventMetaData* eventMetaData = nullptr;
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
      B2ERROR("Couldn't open parent file. Maybe you need to create a file catalog using b2file-catalog-add?"
              << LogVar("LFN", parentLfn) << LogVar("PFN", parentPfn));
      return false;
    }

    // get the event tree and connect its branches
    auto* tree = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Event].c_str()));
    if (!tree) {
      B2ERROR("No tree " << c_treeNames[DataStore::c_Event] << " found in " << parentPfn);
      return false;
    }
    if (int(m_parentStoreEntries.size()) <= level)  m_parentStoreEntries.resize(level + 1);
    connectBranches(tree, DataStore::c_Event, &m_parentStoreEntries[level]);
    m_parentTrees.insert(std::make_pair(parentLfn, tree));

    // get the persistent tree and read its branches
    auto* persistent = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Persistent].c_str()));
    if (!persistent) {
      B2ERROR("No tree " << c_treeNames[DataStore::c_Persistent] << " found in " << parentPfn);
      return false;
    }
    connectBranches(persistent, DataStore::c_Persistent, nullptr);

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
      B2DEBUG(30, "Opening parent file" << LogVar("PFN", parentPfn));
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
  auto* elist = new TEventList("parent_entrylist");
  //TEntryListArray* elist = new TEntryListArray();

  TBranch* branch = m_tree->GetBranch("EventMetaData");
  auto* address = branch->GetAddress();
  EventMetaData* eventMetaData = nullptr;
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

void RootInputModule::entryNotFound(const std::string& entryOrigin, const std::string& name, bool fileChanged)
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
    B2FATAL("Could not read 'persistent' TTree #" << fileEntry << " in file " << name);
  }

  for (auto entry : m_persistentStoreEntries) {
    if (entry->object) {
      bool isMergeable = entry->object->InheritsFrom(Mergeable::Class());
      if (isMergeable) {
        const Mergeable* oldObj = static_cast<Mergeable*>(entry->ptr);
        auto* newObj = static_cast<Mergeable*>(entry->object);
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

void RootInputModule::realDataWorkaround(FileMetaData& metaData)
{
  if ((metaData.getSite().find("bfe0") == 0) && (metaData.getDate().compare("2019-06-30") < 0) &&
      (metaData.getExperimentLow() > 0) && (metaData.getExperimentHigh() < 9) && (metaData.getRunLow() > 0)) {
    metaData.declareRealData();
  }
}
