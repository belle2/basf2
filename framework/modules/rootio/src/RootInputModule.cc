/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/rootio/RootInputModule.h>

#include <framework/modules/rootio/RootIOUtilities.h>
#include <framework/core/InputController.h>
#include <framework/core/Environment.h>

#include <TSystem.h>
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

RootInputModule::RootInputModule() : Module()
{
  //Set module properties
  setDescription("Reads objects/arrays from one or more .root files and makes them available through the DataStore.");
  setPropertyFlags(c_Input | c_InitializeInProcess);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_counterNumber[jj] = 0;
    m_tree[jj] = 0;
  }


  //Parameter definition
  vector<string> emptyvector;
  addParam("inputFileName", m_inputFileName, "Input file name. For multiple files, use inputFileNames instead.", string(""));
  addParam("inputFileNames", m_inputFileNames, "List of input files. You may use wildcards to specify multiple files, e.g. 'somePrefix_*.root'.", emptyvector);

  addParam(c_SteerTreeNames[0], m_treeNames[0], "TTree name for event data. Empty string to disable.", string("tree"));
  addParam(c_SteerTreeNames[1], m_treeNames[1], "TTree name for persistent data. Empty string to disable.", string(""));

  addParam("eventNumber", m_counterNumber[0], "Skip this number of events before starting.", 0);

  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of branches to be read into event map. Empty means all branches.", emptyvector);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of branches to be read into persistent map. Empty means all branches.", emptyvector);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be read into event map. Takes precedence over branchNames.", emptyvector);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be read into persistent map. Takes precedence over branchNamesPersistent.", emptyvector);
}


RootInputModule::~RootInputModule() { }

void RootInputModule::initialize()
{
  gSystem->Load("libdataobjects");
  const std::string& inputFileArgument = Environment::Instance().getInputFileOverride();
  if (!inputFileArgument.empty()) {
    m_inputFileName = "";
    m_inputFileNames.clear();
    m_inputFileNames.push_back(inputFileArgument);
  }

  if (m_inputFileName.empty() && m_inputFileNames.empty()) {
    B2FATAL("You have to set either the 'inputFileName' or the 'inputFileNames' parameter!");
    return;
  }
  if (!m_inputFileName.empty() && !m_inputFileNames.empty()) {
    B2FATAL("Cannot use both 'inputFileName' and 'inputFileNames' parameters!");
    return;
  }

  if (!m_inputFileName.empty())
    m_inputFileNames.push_back(m_inputFileName);
  //we'll only use m_inputFileNames from now on


  //Open TFile
  TDirectory* dir = gDirectory;
  for (unsigned int iFile = 0; iFile < m_inputFileNames.size(); iFile++) {
    //If file name uses wildcarding, we can't check the files here
    if (!TString(m_inputFileNames[iFile].c_str()).Contains("*")) {
      TFile f(m_inputFileNames[iFile].c_str(), "READ");
      if (!f.IsOpen()) {
        B2FATAL("Couldn't open input file " + m_inputFileNames[iFile]);
        return;
      }
    }
  }
  dir->cd();

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii].empty())
      continue;

    //check for duplicate tree names
    for (int jj = 0; jj < ii; ++jj) {
      if (m_treeNames[ii] == m_treeNames[jj]) {
        B2ERROR(c_SteerTreeNames[ii] << " and " << c_SteerTreeNames[jj] << " are the same: " << m_treeNames[ii]);
      }
    }

    if (makeBranchNamesUnique(m_branchNames[ii]))
      B2WARNING(c_SteerBranchNames[ii] << " has duplicate entries.");
    if (makeBranchNamesUnique(m_excludeBranchNames[ii]))
      B2WARNING(c_SteerExcludeBranchNames[ii] << " has duplicate entries.");
    //m_branchNames[ii] and its exclusion list are now sorted alphabetically and unique

    //Get TTree
    m_tree[ii] = new TChain(m_treeNames[ii].c_str());
    for (unsigned int iFile = 0; iFile < m_inputFileNames.size(); iFile++) {
      m_tree[ii]->Add(m_inputFileNames[iFile].c_str());
      B2INFO("Added file " + m_inputFileNames[iFile]);
    }
    B2INFO("Opened tree '" + m_treeNames[ii] + "' with " + m_tree[ii]->GetEntries() << " entries.");

    const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(ii));

    //Go over the branchlist and connect the branches with DataStore entries
    const TObjArray* branches = m_tree[ii]->GetListOfBranches();
    if (!branches) {
      B2ERROR("Tree '" << m_treeNames[ii] << "' doesn't contain any branches!");
      delete m_tree[ii];
      m_tree[ii] = 0; //don't try to read from there
      continue;
    }
    for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
      TBranch* branch = static_cast<TBranch*>(branches->At(jj));
      if (!branch) continue;
      const std::string branchName = branch->GetName();
      //skip excluded branches, and branches not in m_branchNames (if it is not empty)
      if (binary_search(m_excludeBranchNames[ii].begin(), m_excludeBranchNames[ii].end(), branchName) ||
          (!m_branchNames[ii].empty() && !binary_search(m_branchNames[ii].begin(), m_branchNames[ii].end(), branchName))) {
        continue;
      }

      //Get information about the object in the branch
      TObject* objectPtr = 0;
      branch->SetAddress(&objectPtr);
      bool array = (static_cast<string>(branch->GetClassName()) == "TClonesArray");
      TClass* objClass = objectPtr->IsA();
      if (array) {
        objClass = (static_cast<TClonesArray*>(objectPtr))->GetClass();
      }
      delete objectPtr;

      //Create a DataStore entry and connect the branch address to it
      if (!DataStore::Instance().createEntry(branchName, (DataStore::EDurability)ii, objClass, array, false, false)) {
        m_tree[ii]->SetBranchStatus(branch->GetName(), 0);
        continue;
      }
      DataStore::StoreEntry* entry = (map.find(branchName))->second;
      m_tree[ii]->SetBranchAddress(branch->GetName(), &(entry->object));
      m_entries[ii].push_back(entry);

      //Read the persistent objects
      if (ii == DataStore::c_Persistent) {
        branch->GetEntry(0);
        entry->ptr = entry->object;
      }
    }
  }
  if (m_tree[DataStore::c_Event]) {
    InputController::setCanControlInput(true);
    InputController::setNumEntries(m_tree[DataStore::c_Event]->GetEntries());
  }
}


void RootInputModule::event()
{
  if (!m_tree[DataStore::c_Event])
    return;

  const long nextEntry = InputController::getNextEntry();
  if (nextEntry >= 0 && nextEntry < InputController::numEntries()) {
    B2INFO("RootInput: will read " << nextEntry << " next.");
    m_counterNumber[DataStore::c_Event] = nextEntry;
  } else if (InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0 && InputController::getNextEvent() >= 0) {
    const int major = 1000000 * InputController::getNextExperiment() + InputController::getNextRun();
    const int minor = InputController::getNextEvent();
    const long entry = m_tree[DataStore::c_Event]->GetTree()->GetEntryNumberWithIndex(major, minor);
    if (entry == -1) {
      B2ERROR("Couldn't find entry with index " << major << ", " << minor);
    } else {
      const long chainentry = m_tree[DataStore::c_Event]->GetChainEntryNumber(entry);
      B2INFO("RootInput: will read entry " << chainentry << " (entry " << entry << " in current file) next.");
      m_counterNumber[DataStore::c_Event] = chainentry;
    }
  }
  InputController::eventLoaded(m_counterNumber[DataStore::c_Event]);

  readTree(DataStore::c_Event);
  m_counterNumber[DataStore::c_Event]++;
}


void RootInputModule::terminate()
{
  B2DEBUG(200, "Term called");
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    delete m_tree[ii];
  }
}


void RootInputModule::readTree(DataStore::EDurability durability)
{
  if (!m_tree[durability])
    return;

  // Check if there are still new entries available.
  B2DEBUG(200, "Reading file entry for durability " << durability)
  if (m_counterNumber[durability] >= m_tree[durability]->GetEntriesFast()) return;

  for (unsigned int i = 0; i < m_entries[durability].size(); i++) {
    //Make sure transient members of objects are reinitialised
    DataStore::StoreEntry* entry = m_entries[durability][i];
    if (!entry->isArray) {
      delete entry->object;
      entry->object = 0;
    }
  }
  m_tree[durability]->GetEntry(m_counterNumber[durability]);
  for (unsigned int i = 0; i < m_entries[durability].size(); i++) {
    DataStore::StoreEntry* entry = m_entries[durability][i];
    entry->ptr = entry->object;
  }
}
