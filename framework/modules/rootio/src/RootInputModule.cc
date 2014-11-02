/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2012 - Belle II Collaboration                             *
 :cc
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck, Christian Pulvermacher, Thomas Kuhr         *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/


#include <framework/modules/rootio/RootInputModule.h>

#include <framework/modules/rootio/RootIOUtilities.h>
#include <framework/core/InputController.h>
#include <framework/pcore/Mergeable.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/dataobjects/EventMetaData.h>

#include <TClonesArray.h>
#include <TSystem.h>
#include <TFile.h>

#include <wordexp.h>

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
RootInputModule::RootInputModule() : Module(), m_counterNumber(0), m_tree(0), m_persistent(0)
{
  //Set module properties
  setDescription("Reads objects/arrays from one or more .root files and makes them available through the DataStore. Files do not necessarily have to be local, http:// and root:// (for files in xrootd) URLs are supported as well.");
  setPropertyFlags(c_Input);

  //Parameter definition
  vector<string> emptyvector;
  addParam("inputFileName", m_inputFileName, "Input file name. For multiple files, use inputFileNames or wildcards instead. Can be overridden using the -i argument to basf2.", string(""));
  addParam("inputFileNames", m_inputFileNames, "List of input files. You may use shell-like expansions to specify multiple files, e.g. 'somePrefix_*.root' or 'file_[a,b]_[1-15].root'. Can be overridden using the -i argument to basf2.", emptyvector);

  addParam("eventNumber", m_counterNumber, "Skip this number of events before starting.", 0);

  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of branches to be read into event map. Empty means all branches. (EventMetaData is always read)", emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of branches to be read into persistent map. Empty means all branches. (FileMetaData is always read)", emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be read into event map. Takes precedence over branchNames.", emptyvector);
  vector<string> excludePersistent({"ProcessStatistics"});
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be read into persistent map. Takes precedence over branchNamesPersistent.", excludePersistent);

  addParam("parentLevel", m_parentLevel, "Number of generations of parent files (files used as input when creating a file) to be read. This can be useful if a file is missing some information available in its parent.", 0);
}


RootInputModule::~RootInputModule() { }

void RootInputModule::initialize()
{
  gSystem->Load("libdataobjects");
  gSystem->Load("libTreePlayer");
  gSystem->Load("libgenfit2");    // Because genfit2 classes need custom streamers.
  gSystem->Load("libvxd");
  gSystem->Load("libsvd");
  gSystem->Load("libpxd");
  gSystem->Load("libcdc");

  const vector<string>& inputFiles = getInputFiles();
  if (inputFiles.empty()) {
    B2FATAL("You have to set either the 'inputFileName' or the 'inputFileNames' parameter, or start basf2 with the '-i MyFile.root' option.");
    return;
  }
  if (!m_inputFileName.empty() && !m_inputFileNames.empty()) {
    B2FATAL("Cannot use both 'inputFileName' and 'inputFileNames' parameters!");
    return;
  }

  //expand any expansions in inputFiles
  wordexp_t expansions;
  wordexp("", &expansions, 0);
  for (const string & pattern : inputFiles) {
    if (wordexp(pattern.c_str(), &expansions, WRDE_APPEND | WRDE_NOCMD | WRDE_UNDEF) != 0) {
      B2ERROR("Failed to expand pattern '" << pattern << "'!");
    }
  }
  m_inputFileNames.resize(expansions.we_wordc);
  for (unsigned int i = 0; i < expansions.we_wordc; i++) {
    m_inputFileNames[i] = expansions.we_wordv[i];
  }
  wordfree(&expansions);

  //we'll only use m_inputFileNames from now on
  m_inputFileName = "";

  if (m_inputFileNames.empty()) {
    B2FATAL("No valid files specified!");
  }

  //Open TFile
  TDirectory* dir = gDirectory;
  for (const string & fileName : m_inputFileNames) {
    TFile* f = TFile::Open(fileName.c_str(), "READ");
    if (!f || !f->IsOpen()) {
      B2FATAL("Couldn't open input file " + fileName);
      return;
    }
    delete f;
  }
  dir->cd();

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (makeBranchNamesUnique(m_branchNames[ii]))
      B2WARNING(c_SteerBranchNames[ii] << " has duplicate entries.");
    if (makeBranchNamesUnique(m_excludeBranchNames[ii]))
      B2WARNING(c_SteerExcludeBranchNames[ii] << " has duplicate entries.");
    //m_branchNames[ii] and its exclusion list are now sorted alphabetically and unique
  }

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
    m_counterNumber = nextEntry;
  } else if (InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0 && InputController::getNextEvent() >= 0) {
    const long entry = RootIOUtilities::getEntryNumberWithEvtRunExp(m_tree->GetTree(), InputController::getNextEvent(), InputController::getNextRun(), InputController::getNextExperiment());
    if (entry >= 0) {
      const long chainentry = m_tree->GetChainEntryNumber(entry);
      B2INFO("RootInput: will read entry " << chainentry << " (entry " << entry << " in current file) next.");
      m_counterNumber = chainentry;
    }
  }
  InputController::eventLoaded(m_counterNumber);

  readTree();
  m_counterNumber++;
}


void RootInputModule::terminate()
{
  B2DEBUG(200, "Term called");
  delete m_tree;
  delete m_persistent;
  for (auto entry : m_parentTrees) delete entry.second->GetCurrentFile();
}


void RootInputModule::readTree()
{
  if (!m_tree)
    return;

  const string prevFile = m_tree->GetCurrentFile()->GetName();

  // Check if there are still new entries available.
  int localEntryNumber = m_tree->LoadTree(m_counterNumber);

  if (localEntryNumber == -2) {
    return; //end of file
  } else if (localEntryNumber < 0) {
    B2FATAL("Failed to load tree, corrupt file? Check standard error for additional messages. (TChain::LoadTree() returned error " << localEntryNumber << ")");
    return;
  }

  B2DEBUG(200, "Reading file entry " << m_counterNumber);

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
    B2FATAL("Could not read 'tree' entry " << m_counterNumber << " in file " << m_tree->GetCurrentFile()->GetName());
  }

  if (prevFile != m_tree->GetCurrentFile()->GetName()) {
    // file changed, read the FileMetaData object from the persistent tree and update the parent file metadata
    B2DEBUG(100, "File changed, loading persistent data.");
    readPersistentEntry(m_tree->GetTreeNumber());
    StoreObjPtr<FileMetaData> fileMetaData("", DataStore::c_Persistent);
    FileCatalog::Instance().getParentMetaData(m_parentLevel, 0, *fileMetaData, m_parentMetaData);
    m_currentParent.resize(0);
  }

  for (auto entry : m_storeEntries) {
    entry->ptr = entry->object;
  }

  if (m_parentLevel > 0) readParentTrees();
}


bool RootInputModule::connectBranches(TTree* tree, DataStore::EDurability durability, StoreEntries* storeEntries)
{
  DataStore::StoreEntryMap& map = DataStore::Instance().getStoreEntryMap(durability);

  //Go over the branchlist and connect the branches with DataStore entries
  const TObjArray* branches = tree->GetListOfBranches();
  if (!branches) {
    B2FATAL("Tree '" << tree->GetName() << "' doesn't contain any branches!");
    return false;
  }
  for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
    TBranch* branch = static_cast<TBranch*>(branches->At(jj));
    if (!branch) continue;
    const std::string branchName = branch->GetName();
    //skip already connected branches
    if (m_connectedBranches[durability].find(branchName) != m_connectedBranches[durability].end()) continue;
    //skip excluded branches, and branches not in m_branchNames (if it is not empty)
    if (binary_search(m_excludeBranchNames[durability].begin(), m_excludeBranchNames[durability].end(), branchName) ||
        (!m_branchNames[durability].empty() && !binary_search(m_branchNames[durability].begin(), m_branchNames[durability].end(), branchName))) {
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
        TTree* persistent = (TTree*) file->Get(c_treeNames[DataStore::c_Persistent].c_str());
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
            tree = (TTree*) file->Get(c_treeNames[DataStore::c_Event].c_str());
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

void RootInputModule::readPersistentEntry(long entry)
{
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

  int bytesRead = m_persistent->GetEntry(entry);
  if (bytesRead <= 0) {
    B2FATAL("Could not read 'persistent' TTree #" << entry << " in file " << m_tree->GetCurrentFile()->GetName());
  }

  for (auto entry : m_persistentStoreEntries) {
    bool isMergeable = entry->object->InheritsFrom(Mergeable::Class());
    if (isMergeable) {
      const Mergeable* oldObj = static_cast<Mergeable*>(entry->ptr);
      Mergeable* newObj = static_cast<Mergeable*>(entry->object);
      newObj->merge(oldObj);

      delete entry->ptr;
    }
    entry->ptr = entry->object;
  }
}
