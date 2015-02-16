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

#include <TClonesArray.h>
#include <TFile.h>

using namespace std;
using namespace Belle2;
using namespace RootIOUtilities;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(RootInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------
RootInputModule::RootInputModule() : Module(), m_nextEntry(0), m_lastPersistentEntry(-1), m_tree(0), m_persistent(0)
{
  //Set module properties
  setDescription("Reads objects/arrays from one or more .root files saved by the RootOutput module and makes them available through the DataStore. Files do not necessarily have to be local, http:// and root:// (for files in xrootd) URLs are supported as well.");
  setPropertyFlags(c_Input);

  //Parameter definition
  vector<string> emptyvector;
  addParam("inputFileName", m_inputFileName, "Input file name. For multiple files, use inputFileNames or wildcards instead. Can be overridden using the -i argument to basf2.", string(""));
  addParam("inputFileNames", m_inputFileNames, "List of input files. You may use shell-like expansions to specify multiple files, e.g. 'somePrefix_*.root' or 'file_[a,b]_[1-15].root'. Can be overridden using the -i argument to basf2.", emptyvector);

  addParam("skipNEvents", m_skipNEvents, "Skip this number of events before starting.", 0);

  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of event durability branches to be read. Empty means all branches. (EventMetaData is always read)", emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of persistent durability branches to be read. Empty means all branches. (FileMetaData is always read)", emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of event durability branches NOT to be read. Takes precedence over branchNames.", emptyvector);
  vector<string> excludePersistent({"ProcessStatistics"});
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of persistent durability branches NOT to be read. Takes precedence over branchNamesPersistent.", excludePersistent);

  addParam("parentLevel", m_parentLevel, "Number of generations of parent files (files used as input when creating a file) to be read. This can be useful if a file is missing some information available in its parent. See https://belle2.cc.kek.jp/~twiki/bin/view/Software/ParentFiles for details.", 0);
}


RootInputModule::~RootInputModule() { }

void RootInputModule::initialize()
{
  m_nextEntry = m_skipNEvents;
  m_lastPersistentEntry = -1;

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
  m_inputFileName = "";
  //we'll only use m_inputFileNames from now on

  //Open TFile
  TDirectory* dir = gDirectory;
  for (const string & fileName : m_inputFileNames) {
    TFile* f = TFile::Open(fileName.c_str(), "READ");
    if (!f || !f->IsOpen()) {
      B2FATAL("Couldn't open input file " + fileName);
    }
    delete f;
  }
  dir->cd();

  //Get TTree
  m_persistent = new TChain(c_treeNames[DataStore::c_Persistent].c_str());
  m_tree = new TChain(c_treeNames[DataStore::c_Event].c_str());
  for (const string & fileName : m_inputFileNames) {
    m_persistent->AddFile(fileName.c_str());
    m_tree->AddFile(fileName.c_str());
    B2INFO("Added file " + fileName);
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
    StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
    FileCatalog::Instance().getParentMetaData(m_parentLevel, 0, *fileMetaData, m_parentMetaData);
    createParentStoreEntries();
  } else if (m_parentLevel < 0) {
    B2ERROR("parentLevel must be >= 0!");
    return;
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
  } else if (InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0 && InputController::getNextEvent() >= 0) {
    const long entry = RootIOUtilities::getEntryNumberWithEvtRunExp(m_tree->GetTree(), InputController::getNextEvent(), InputController::getNextRun(), InputController::getNextExperiment());
    if (entry >= 0) {
      const long chainentry = m_tree->GetChainEntryNumber(entry);
      B2INFO("RootInput: will read entry " << chainentry << " (entry " << entry << " in current file) next.");
      m_nextEntry = chainentry;
    }
  }
  InputController::eventLoaded(m_nextEntry);

  readTree();
  m_nextEntry++;
}


void RootInputModule::terminate()
{
  B2DEBUG(200, "Term called");
  delete m_tree;
  delete m_persistent;
  for (auto entry : m_parentTrees) delete entry.second->GetCurrentFile();

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    m_connectedBranches[ii].clear();
  }
  m_storeEntries.clear();
  m_persistentStoreEntries.clear();
  m_currentParent.clear();
  m_parentStoreEntries.clear();
  m_parentTrees.clear();
}


void RootInputModule::readTree()
{
  if (!m_tree)
    return;

  // Check if there are still new entries available.
  int localEntryNumber = m_tree->LoadTree(m_nextEntry);

  if (localEntryNumber == -2) {
    return; //end of file
  } else if (localEntryNumber < 0) {
    B2FATAL("Failed to load tree, corrupt file? Check standard error for additional messages. (TChain::LoadTree() returned error " << localEntryNumber << ")");
  }

  B2DEBUG(200, "Reading file entry " << m_nextEntry);

  //Make sure transient members of objects are reinitialised
  for (auto entry : m_storeEntries) {
    if (!entry->object)
      continue;
    entry->resetForGetEntry();
  }
  for (auto storeEntries : m_parentStoreEntries) {
    for (auto entry : storeEntries) {
      if (!entry->object)
        continue;
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
  const long treeNum = m_tree->GetTreeNumber();
  if (m_lastPersistentEntry != treeNum) {
    // file changed, read the FileMetaData object from the persistent tree and update the parent file metadata
    readPersistentEntry(treeNum);
    StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
    FileCatalog::Instance().getParentMetaData(m_parentLevel, 0, *fileMetaData, m_parentMetaData);
    m_currentParent.resize(0);
  }

  for (auto entry : m_storeEntries) {
    if (!entry->object)
      entryNotFound("Event durability tree (global entry: " + std::to_string(m_nextEntry) + ")", entry->name);
    entry->ptr = entry->object;
  }

  if (m_parentLevel > 0) readParentTrees();
}


bool RootInputModule::connectBranches(TTree* tree, DataStore::EDurability durability, StoreEntries* storeEntries)
{
  B2DEBUG(100, "File changed, loading persistent data.");
  DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(durability);

  //Go over the branchlist and connect the branches with DataStore entries
  const TObjArray* branches = tree->GetListOfBranches();
  if (!branches) {
    B2FATAL("Tree '" << tree->GetName() << "' doesn't contain any branches!");
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
  StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
  unsigned int experiment = fileMetaData->getExperimentLow();
  unsigned int run = fileMetaData->getRunLow();
  unsigned int event = fileMetaData->getEventLow();

  // loop over parents and get their metadata
  FileCatalog::ParentMetaData* parentMetaData = &m_parentMetaData;
  for (int level = 0; level < m_parentLevel; level++) {
    bool parentFound = false;
    for (unsigned int iParent = 0; iParent < parentMetaData->size(); iParent++) {
      FileMetaData& metaData = (*parentMetaData)[iParent].metaData;

      // pick a parent that contains the given event
      if (metaData.containsEvent(experiment, run, event)) {
        TDirectory* dir = gDirectory;
        TFile* file = TFile::Open(metaData.getLfn().c_str(), "READ");
        dir->cd();
        if (!file || !file->IsOpen()) {
          B2ERROR("Couldn't open parent file " << metaData.getLfn());
          continue;
        }

        // get the event tree and connect its branches
        TTree* tree = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Event].c_str()));
        if (!tree) {
          B2ERROR("No tree " << c_treeNames[DataStore::c_Event] << " found in " << metaData.getLfn());
          continue;
        }
        if (int(m_parentStoreEntries.size()) <= level) {
          m_parentStoreEntries.resize(level + 1);
          connectBranches(tree, DataStore::c_Event, &m_parentStoreEntries[level]);
        }
        if (int(m_currentParent.size()) <= level) m_currentParent.resize(level + 1);
        m_currentParent[level] = iParent;
        m_parentTrees.insert(std::make_pair(metaData.getId(), tree));

        // get the persistent tree and read its branches
        TTree* persistent = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Persistent].c_str()));
        if (!persistent) {
          B2ERROR("No tree " << c_treeNames[DataStore::c_Persistent] << " found in " << metaData.getLfn());
        } else {
          connectBranches(persistent, DataStore::c_Persistent, 0);
        }

        parentFound = true;
        parentMetaData = &((*parentMetaData)[iParent].parents);
        break;
      }
    }
    if (!parentFound) {
      B2ERROR("Failed to connect parent file at level " << level);
      return false;
    }
  }

  return true;
}


bool RootInputModule::readParentTrees()
{
  StoreObjPtr<EventMetaData> eventMetaData;
  unsigned int experiment = eventMetaData->getExperiment();
  unsigned int run = eventMetaData->getRun();
  unsigned int event = eventMetaData->getEvent();
  long entry = -1;

  FileCatalog::ParentMetaData* parentMetaData = &m_parentMetaData;
  for (int level = 0; level < m_parentLevel; level++) {

    // check whether the current parent tree contains the current event
    TTree* tree = 0;
    if (int(m_currentParent.size()) > level) {
      int iParent = m_currentParent[level];
      if (iParent < 0) {
        B2ERROR("Failed to get parent data");
        return false;
      }
      FileMetaData& metaData = (*parentMetaData)[iParent].metaData;
      tree = m_parentTrees[metaData.getId()];
      entry = RootIOUtilities::getEntryNumberWithEvtRunExp(tree, event, run, experiment);
      if (entry != -1) {
        parentMetaData = &((*parentMetaData)[iParent].parents);
      } else {
        tree = 0;
      }
    }

    // if the current tree does not contain the current event, loop over parents and find the one that does
    if (!tree) {
      m_currentParent.resize(level + 1);
      m_currentParent[level] = -1;
      for (int iParent = 0; iParent < int(parentMetaData->size()); iParent++) {
        if (iParent == m_currentParent[level]) continue;
        FileMetaData& metaData = (*parentMetaData)[iParent].metaData;

        // only consider parents who can contain the current event according to their metadata
        if (metaData.containsEvent(experiment, run, event)) {

          // check whether the parent tree is already open, otherwise load it
          if (m_parentTrees.find(metaData.getId()) != m_parentTrees.end()) {
            tree = m_parentTrees[metaData.getId()];
          } else {
            TDirectory* dir = gDirectory;
            TFile* file = TFile::Open(metaData.getLfn().c_str(), "READ");
            dir->cd();
            if (!file || !file->IsOpen()) {
              B2ERROR("Couldn't open parent file " << metaData.getLfn());
              continue;
            }
            tree = dynamic_cast<TTree*>(file->Get(c_treeNames[DataStore::c_Event].c_str()));
            if (!tree) {
              B2ERROR("No tree " << c_treeNames[DataStore::c_Event] << " found in " << metaData.getLfn());
              continue;
            }
            m_parentTrees.insert(std::make_pair(metaData.getId(), tree));
          }

          // if the tree does contain the current event, connect its branches and exit parents loop
          entry = RootIOUtilities::getEntryNumberWithEvtRunExp(tree, event, run, experiment);
          if (entry != -1) {
            for (auto entry : m_parentStoreEntries[level]) {
              tree->SetBranchAddress(entry->name.c_str(), &(entry->object));
            }
            parentMetaData = &((*parentMetaData)[iParent].parents);
            m_currentParent[level] = iParent;
            break;
          } else {
            tree = 0;
          }
        }
      }
    }

    // check whether we found a parent tree
    if (!tree) {
      B2ERROR("Failed to get parent data");
      return false;
    }

    // read the tree and mark the data read in the data store
    tree->GetEntry(entry);
    for (auto entry : m_parentStoreEntries[level]) {
      entry->ptr = entry->object;
    }
  }

  return true;
}

void RootInputModule::entryNotFound(std::string entryOrigin, std::string name)
{
  if (name == "ProcessStatistics" or DataStore::Instance().getDependencyMap().isUsedAs(name, DependencyMap::c_Input)) {
    B2FATAL(entryOrigin << " in " << m_tree->GetFile()->GetName() << " does not contain required object " << name << ", aborting.")
  } else {
    B2WARNING(entryOrigin << " in " << m_tree->GetFile()->GetName() << " does not contain object " << name << " that was present in a previous entry.")
  }
}

void RootInputModule::readPersistentEntry(long fileEntry)
{
  m_lastPersistentEntry = fileEntry;

  for (auto entry : m_persistentStoreEntries) {
    if (!entry->object)
      continue;
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
    B2FATAL("Could not read 'persistent' TTree #" << fileEntry << " in file " << m_tree->GetCurrentFile()->GetName());
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
    } else {
      entryNotFound("Persistent tree", entry->name);
    }
    entry->ptr = entry->object;
  }
}
