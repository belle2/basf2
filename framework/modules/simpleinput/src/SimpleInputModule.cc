/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/simpleinput/SimpleInputModule.h>

#include <framework/core/InputController.h>
#include <framework/core/Environment.h>

#include <TSystem.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimpleInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

const std::string SimpleInputModule::c_SteerTreeNames[] = { "treeName", "treeNameRun", "treeNamePersistent" };
const std::string SimpleInputModule::c_SteerBranchNames[] = { "branchNames", "branchNamesRun", "branchNamesPersistent" };
const std::string SimpleInputModule::c_SteerExcludeBranchNames[] = { "excludeBranchNames", "excludeBranchNamesRun", "excludeBranchNamesPersistent" };

SimpleInputModule::SimpleInputModule() : Module()
{
  //Set module properties
  setDescription("This module reads objects/arrays from a root file and writes them into the DataStore.");
  setPropertyFlags(c_Input | c_InitializeInProcess);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_counterNumber[jj] = 0;
    m_tree[jj] = 0;
  }

  m_file = 0;

  //Parameter definition
  addParam("inputFileName", m_inputFileName, "TFile name.", string("SimpleInput.root"));

  addParam(c_SteerTreeNames[0], m_treeNames[0], "TTree name for event data. NONE for no input.", string("tree"));
  addParam(c_SteerTreeNames[1], m_treeNames[1], "TTree name for run data. NONE for no input.", string("NONE"));
  addParam(c_SteerTreeNames[2], m_treeNames[2], "TTree name for persistent data. NONE for no input.", string("NONE"));

  addParam("eventNumber", m_counterNumber[0], "Skip this number of events before starting.", 0);


  vector<string> branchNames;
  addParam(c_SteerBranchNames[0], m_branchNames[0], "Names of branches to be read into event map. Empty means all branches.", branchNames);
  addParam(c_SteerBranchNames[1], m_branchNames[1], "Names of branches to be read into run map. Empty means all branches.", branchNames);
  addParam(c_SteerBranchNames[2], m_branchNames[2], "Names of branches to be read into persistent map. Empty means all branches.", branchNames);

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be read into event map. Takes precedence over branchNames.", branchNames);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be read into run map. Takes precedence over branchNamesRun.", branchNames);
  addParam(c_SteerExcludeBranchNames[2], m_excludeBranchNames[2], "Names of branches NOT to be read into persistent map. Takes precedence over branchNamesPersistent.", branchNames);
}


SimpleInputModule::~SimpleInputModule()
{
  delete m_file;
}

void SimpleInputModule::initialize()
{
  gSystem->Load("libdataobjects");
  const std::string& inputFileArgument = Environment::Instance().getInputFileOverride();
  if (!inputFileArgument.empty())
    m_inputFileName = inputFileArgument;

  //Open TFile
  TDirectory* dir = gDirectory;
  m_file = new TFile(m_inputFileName.c_str(), "READ");
  dir->cd();
  if (!m_file or !m_file->IsOpen()) {
    B2FATAL("Couldn't open input file " + m_inputFileName);
    return;
  }
  B2INFO("Opened file " + m_inputFileName);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii] == "NONE")
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
    m_tree[ii] = dynamic_cast<TTree*>(m_file->Get(m_treeNames[ii].c_str()));
    if (!m_tree[ii]) {
      B2FATAL("TTree " + m_treeNames[ii] + " doesn't exist");
      return;
    }
    B2INFO("Opened tree " + m_treeNames[ii]);

    const DataStore::StoreObjMap& map = DataStore::Instance().getStoreObjectMap(DataStore::EDurability(ii));

    //Go over the branchlist and connect the branches with DataStore entries
    const TObjArray* branches = m_tree[ii]->GetListOfBranches();
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

      //Create a DataStore entry and connect the branch address to it
      if (!DataStore::Instance().createEntry(branchName, (DataStore::EDurability)ii, objClass, array, false, true)) {
        branch->SetStatus(0);
        continue;
      }
      DataStore::StoreEntry* entry = (map.find(branchName))->second;
      branch->SetAddress(&(entry->object));
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


void SimpleInputModule::beginRun()
{
  B2DEBUG(200, "beginRun called.");
  readTree(DataStore::c_Run);
  m_counterNumber[DataStore::c_Run]++;
}


void SimpleInputModule::event()
{
  m_file->cd();

  if (!m_tree[DataStore::c_Event])
    return;

  const long nextEntry = InputController::getNextEntry();
  if (nextEntry >= 0 && nextEntry < InputController::numEntries()) {
    B2INFO("SimpleInput: will read " << nextEntry << " next.");
    m_counterNumber[DataStore::c_Event] = nextEntry;
  } else if (InputController::getNextExperiment() >= 0 && InputController::getNextRun() >= 0 && InputController::getNextEvent() >= 0) {
    const int major = 1000000 * InputController::getNextExperiment() + InputController::getNextRun();
    const int minor = InputController::getNextEvent();
    const long entry = m_tree[DataStore::c_Event]->GetEntryNumberWithIndex(major, minor);
    if (entry == -1) {
      B2ERROR("Couldn't find entry with index " << major << ", " << minor);
    } else {
      B2INFO("SimpleInput: will read " << entry << " next.");
      m_counterNumber[DataStore::c_Event] = entry;
    }
  }
  InputController::eventLoaded(m_counterNumber[DataStore::c_Event]);

  readTree(DataStore::c_Event);
  m_counterNumber[DataStore::c_Event]++;
}


void SimpleInputModule::terminate()
{
  B2DEBUG(200, "Term called");
}


void SimpleInputModule::readTree(DataStore::EDurability durability)
{
  if (!m_tree[durability])
    return;

  // Check if there are still new entries available.
  B2DEBUG(200, "Durability" << durability)
  if (m_counterNumber[durability] >= m_tree[durability]->GetEntriesFast()) return;

  m_tree[durability]->GetEntry(m_counterNumber[durability]);
  for (unsigned int i = 0; i < m_entries[durability].size(); i++) {
    DataStore::StoreEntry* entry = m_entries[durability][i];
    entry->ptr = entry->object;
  }
}

bool SimpleInputModule::makeBranchNamesUnique(std::vector<std::string> &stringlist) const
{
  const size_t oldsize = stringlist.size();
  sort(stringlist.begin(), stringlist.end());
  stringlist.resize(unique(stringlist.begin(), stringlist.end()) - stringlist.begin());

  return (oldsize != stringlist.size());
}
