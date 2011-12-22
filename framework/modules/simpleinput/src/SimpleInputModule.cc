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

SimpleInputModule::SimpleInputModule() : Module()
{
  //Set module properties
  setDescription("simple input");
  setPropertyFlags(c_Input);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_size[jj]    = 0;
    m_sizeObj[jj] = 0;
    m_treeNames[jj]   = "NONE";
    m_tree[jj] = 0;
  }

  m_file = 0;

  m_steerTreeNames.push_back("treeName");
  m_steerTreeNames.push_back("treeNameRun");
  m_steerTreeNames.push_back("treeNamePersistent");

  m_steerBranchNames.push_back("branchNames");
  m_steerBranchNames.push_back("branchNamesRun");
  m_steerBranchNames.push_back("branchNamesPersistent");


  //Parameter definition
  addParam("inputFileName", m_inputFileName, "TFile name.", string("SimpleInput.root"));

  addParam(m_steerTreeNames[0], m_treeNames[0], "TTree name for event data. NONE for no input.", string("tree"));
  addParam(m_steerTreeNames[1], m_treeNames[1], "TTree name for run data. NONE for no input.", string("NONE"));
  addParam(m_steerTreeNames[2], m_treeNames[2], "TTree name for persistent data. NONE for no input.", string("NONE"));

  addParam("eventNumber", m_counterNumber[0], "Skip this number of events before starting.", 0);
  m_counterNumber[1] = 0;
  m_counterNumber[2] = 0;


  vector<string> branchNames;
  addParam(m_steerBranchNames[0], m_branchNames[0], "Names of branches to be read into event map. Empty means all branches.", branchNames);
  addParam(m_steerBranchNames[1], m_branchNames[1], "Names of branches to be read into run map. Empty means all branches.", branchNames);
  addParam(m_steerBranchNames[2], m_branchNames[2], "Names of branches to be read into persistent map. Empty means all branches.", branchNames);
}


SimpleInputModule::~SimpleInputModule()
{}

void SimpleInputModule::initialize()
{
  //Open TFile
  m_file = new TFile(m_inputFileName.c_str(), "READ");
  m_file->cd();
  if (!m_file) {B2FATAL("Input file " + m_inputFileName + " doesn't exist");}
  B2INFO("Opened file " + m_inputFileName);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    //Get TTree
    if (m_treeNames[ii] != "NONE") {
      m_tree[ii] = dynamic_cast<TTree*>(m_file->Get(m_treeNames[ii].c_str()));
      if (!m_tree[ii]) {B2FATAL("TTree " + m_treeNames[ii] + " doesn't exist");}
      B2INFO("Opened tree " + m_treeNames[ii]);



      //Connect the branches to the TObject pointers
      TObjArray* branches = m_tree[ii]->GetListOfBranches();
      TBranch* branch = 0;

      //How many objects, How many arrays
      for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
        branch = validBranch(jj, branches);
        if (branch) {
          //Count none TClonesArrays extra
          if (static_cast<string>(branch->GetClassName()) != "TClonesArray") {
            m_sizeObj[ii]++;
          }
          m_size[ii]++;
        }
      }
      B2DEBUG(150, "m_sizeObj[" << ii << "] : " << m_sizeObj[ii]);
      B2DEBUG(150, "m_size[" << ii << "] : " << m_size[ii]);

      //Create the TObject pointers
      m_objects[ii] = new TObject* [m_size[ii]];
      for (int jj = 0; jj < m_size[ii]; jj++) {
        m_objects[ii][jj] = 0;
      }

      //Go again over the branchlist and connect the branches with TObject pointers
      int iobject = 0;
      int iarray = 0;
      m_objectNames[ii].resize(m_size[ii], "");
      for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
        branch = validBranch(jj, branches);
        if (branch) {
          if (static_cast<string>(branch->GetClassName()) == "TClonesArray") {
            branch->SetAddress(&(m_objects[ii][iarray + m_sizeObj[ii]]));
            m_objectNames[ii][iarray + m_sizeObj[ii]] = static_cast<string>(branch->GetName());
            iarray++;
            branch->GetEntry(0);
          } else {
            branch->SetAddress(&(m_objects[ii][iobject]));
            m_objectNames[ii][iobject] = static_cast<string>(branch->GetName());
            iobject++;
            branch->GetEntry(0);
          }
        }
      }
    }
    // Store arrays in the DataStore
    for (int jj = 0; jj < m_size[ii] - m_sizeObj[ii]; jj++) {
      DataStore::Instance().storeArray(static_cast<TClonesArray*>(m_objects[ii][jj + m_sizeObj[ii]]), m_objectNames[ii][jj + m_sizeObj[ii]]);
      if (m_tree[DataStore::c_Persistent]) {
        readTree(DataStore::c_Persistent);
      }

    }
  }


}


void SimpleInputModule::beginRun()
{
  B2DEBUG(200, "beginRun called.");
  if (m_tree[DataStore::c_Run]) {
    readTree(DataStore::c_Run);
  }
}


void SimpleInputModule::event()
{
  m_file->cd();

  if (m_tree[DataStore::c_Event]) {
    readTree(DataStore::c_Event);
  }
  m_counterNumber[DataStore::c_Event]++;
}


void SimpleInputModule::endRun()
{
  cout << "endRun called" << endl;
}


void SimpleInputModule::terminate()
{
  cout << "Term called" << endl;
}


void SimpleInputModule::setupTFile()
{
}


void SimpleInputModule::readTree(const DataStore::EDurability& durability)
{
  // Check if there are still new entries available.
  B2DEBUG(200, "Durability" << durability)
  if (m_counterNumber[durability] >= m_tree[durability]->GetEntriesFast()) return;

  //Connect the branches to the TObject pointers
  TObjArray* branches = m_tree[durability]->GetListOfBranches();
  TBranch* branch = 0;
  int ii = durability;
  //How many objects, How many arrays
  m_sizeObj[ii] = 0;
  m_size[ii] = 0;
  for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
    branch = validBranch(jj, branches);
    if (branch) {
      //Count none TClonesArrays extra
      if (static_cast<string>(branch->GetClassName()) != "TClonesArray") {
        m_sizeObj[ii]++;
      }
      m_size[ii]++;
    }
  }
  B2DEBUG(150, "m_sizeObj[" << ii << "] : " << m_sizeObj[ii]);
  B2DEBUG(150, "m_size["    << ii << "] : " << m_size[ii]);

  //Create the TObject pointers
  m_objects[ii] = new TObject* [m_size[ii]];
  for (int jj = 0; jj < m_size[ii]; jj++) {
    m_objects[ii][jj] = 0;
  }

  //Go again over the branchlist and connect the branches with TObject pointers
  int iobject = 0;
  //int iarray = 0;
  m_objectNames[ii].resize(m_size[ii], "");
  for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
    branch = validBranch(jj, branches);
    if (branch) {
      if (static_cast<string>(branch->GetClassName()) == "TClonesArray") {
        /*
          branch->SetAddress(&(m_objects[ii][iarray + m_sizeObj[ii]]));
          m_objectNames[ii][iarray + m_sizeObj[ii]] = static_cast<string>(branch->GetName());
          iarray++;*/
      } else {
        branch->SetAddress(&(m_objects[ii][iobject]));
        m_objectNames[ii][iobject] = static_cast<string>(branch->GetName());
        iobject++;
      }
    }
  }


  m_tree[durability]->GetEntry(m_counterNumber[durability]);


  // Store objects in the DataStore
  for (int jj = 0; jj < m_sizeObj[durability]; jj++) {
    DataStore::Instance().storeObject(m_objects[durability][jj], m_objectNames[durability][jj]);
  }
  // Store arrays in the DataStore
  /*  for (int jj = 0; jj < m_size[durability] - m_sizeObj[durability]; jj++) {
      DataStore::Instance().storeArray(static_cast<TClonesArray*>(m_objects[durability][jj+m_sizeObj[durability]]), m_objectNames[durability][jj+m_sizeObj[durability]]);
    }*/

}

TBranch* SimpleInputModule::validBranch(int& ibranch, TObjArray* branches)
{
  TBranch* branch = static_cast<TBranch*>(branches->At(ibranch));
  if (!branch) {
    return 0;
  }

  // check if the branch is in the corresponding branch list
  // an empty list will cause all branches to be accepted
  string name = branch->GetName();
  DataStore::EDurability durability = DataStore::c_Event; // TODO

  vector<string>::iterator found_itr = find(m_branchNames[durability].begin(), m_branchNames[durability].end(), name);
  const bool found = (found_itr != m_branchNames[durability].end());

  if (m_branchNames[durability].size() != 0 && !found)
    return 0;


  return branch;
}
