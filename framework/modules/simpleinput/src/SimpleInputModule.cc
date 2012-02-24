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
  setPropertyFlags(c_Input);

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

  addParam(c_SteerExcludeBranchNames[0], m_excludeBranchNames[0], "Names of branches NOT to be written from event map. Branches also in branchNames are not written.", branchNames);
  addParam(c_SteerExcludeBranchNames[1], m_excludeBranchNames[1], "Names of branches NOT to be written from run map. Branches also in branchNamesRun are not written.", branchNames);
  addParam(c_SteerExcludeBranchNames[2], m_excludeBranchNames[2], "Names of branches NOT to be written from persistent map. Branches also in branchNamesPersistent are not written.", branchNames);
}


SimpleInputModule::~SimpleInputModule()
{
  delete m_file;
}

void SimpleInputModule::initialize()
{
  //Open TFile
  m_file = new TFile(m_inputFileName.c_str(), "READ");
  if (!m_file) {
    B2FATAL("Couldn't open input file " + m_inputFileName);
    return;
  }
  B2INFO("Opened file " + m_inputFileName);
  m_file->cd();

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
    //m_branchNames[ii] and it's exclusion list are now sorted alphabetically and unique

    //Get TTree
    m_tree[ii] = dynamic_cast<TTree*>(m_file->Get(m_treeNames[ii].c_str()));
    if (!m_tree[ii]) {
      B2FATAL("TTree " + m_treeNames[ii] + " doesn't exist");
      return;
    }
    B2INFO("Opened tree " + m_treeNames[ii]);

    //Go over the branchlist and connect the branches with TObject pointers
    for (int jj = 0; jj < m_tree[ii]->GetNbranches(); jj++) {
      TBranch* branch = validBranch(jj, (DataStore::EDurability)ii);
      if (branch) {
        TObject* objectPtr = 0;
        branch->SetAddress(&objectPtr);

        const string objectName = static_cast<string>(branch->GetName());
        if (static_cast<string>(branch->GetClassName()) == "TClonesArray") {
          DataStore::Instance().storeArray(static_cast<TClonesArray*>(objectPtr), objectName, (DataStore::EDurability)ii);
        } else {
          DataStore::Instance().storeObject(objectPtr, objectName, (DataStore::EDurability)ii);
        }
        branch->GetEntry(0);
      }
    }
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

  readTree(DataStore::c_Event);
  m_counterNumber[DataStore::c_Event]++;
}


void SimpleInputModule::endRun()
{
  B2DEBUG(200, "endRun called");
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
  if (m_counterNumber[durability] == 0) return; //first entry is read in initialize()
  if (m_counterNumber[durability] >= m_tree[durability]->GetEntriesFast()) return;


  //Go again over the branchlist and connect the branches with TObject pointers
  for (int jj = 0; jj < m_tree[durability]->GetNbranches(); jj++) {
    TBranch* branch = validBranch(jj, durability);
    if (branch) {
      if (static_cast<string>(branch->GetClassName()) != "TClonesArray") {
        TObject* objectPtr = 0;
        branch->SetAddress(&objectPtr);
        DataStore::Instance().storeObject(objectPtr, static_cast<string>(branch->GetName()), durability);
      }

      //this will also (re)fill the TClonesArrays in the DataStore, we don't need to reassign any pointers
      branch->GetEntry(m_counterNumber[durability]);
    }
  }
}

TBranch* SimpleInputModule::validBranch(int ibranch, DataStore::EDurability durability) const
{
  const TObjArray* branches = m_tree[durability]->GetListOfBranches();
  TBranch* branch = static_cast<TBranch*>(branches->At(ibranch));
  if (!branch) {
    return 0;
  }

  //check if branch is in exclusion list
  if (binary_search(m_excludeBranchNames[durability].begin(), m_excludeBranchNames[durability].end(), branch->GetName()))
    return 0;

  // an empty branch list will cause all branches to be accepted
  if (m_branchNames[durability].size() == 0)
    return branch;

  // check if the branch is in the corresponding branch list
  if (binary_search(m_branchNames[durability].begin(), m_branchNames[durability].end(), branch->GetName()))
    return branch;

  return 0; //not found
}

bool SimpleInputModule::makeBranchNamesUnique(std::vector<std::string> &stringlist) const
{
  const size_t oldsize = stringlist.size();
  sort(stringlist.begin(), stringlist.end());
  stringlist.resize(unique(stringlist.begin(), stringlist.end()) - stringlist.begin());

  return (oldsize != stringlist.size());
}
