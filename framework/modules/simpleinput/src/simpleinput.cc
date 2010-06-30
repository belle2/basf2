/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <../simpleinput/simpleinput.h>

using namespace std;
using namespace Belle2;

REG_MODULE(ModuleSimpleInput)

ModuleSimpleInput::ModuleSimpleInput() : Module("SimpleInput")
{
  //Set module properties
  setDescription("simple input");
  setPropertyFlags(c_TriggersNewRun | c_TriggersEndOfData | c_ReadsDataSingleProcess | c_RequiresSingleProcess);

  //Initialization of some member variables
  for (int jj = 0; jj < c_NDurabilityTypes; jj++) {
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
  addParam("inputFileName", m_inputFileName, string("SimpleInput.root"), "TFile name.");

  addParam(m_steerTreeNames[0], m_treeNames[0], string("tree"), "TTree name for event data. NONE for no input.");
  addParam(m_steerTreeNames[1], m_treeNames[1], string("NONE"), "TTree name for run data. NONE for no input.");
  addParam(m_steerTreeNames[2], m_treeNames[2], string("NONE"), "TTree name for persistent data. NONE for no input.");

  addParam("eventNumber", m_eventNumber, 0, "Skip this number of events before starting.");

  vector<string> branchNames;
  addParam(m_steerBranchNames[0], m_branchNames[0], branchNames, "Names of branches to be read into event map. Empty means all branches.");
  addParam(m_steerBranchNames[1], m_branchNames[1], branchNames, "Names of branches to be read into run map. Empty means all branches.");
  addParam(m_steerBranchNames[2], m_branchNames[2], branchNames, "Names of branches to be read into persistent map. Empty means all branches.");


  INFO("Constructor done.")
}


ModuleSimpleInput::~ModuleSimpleInput()
{}

void ModuleSimpleInput::initialize()
{
  //Open TFile
  m_file = new TFile(m_inputFileName.c_str(), "READ");
  m_file->cd();
  if (!m_file) {FATAL("Input file " + m_inputFileName + " doesn't exist");}
  INFO("Opened file " + m_inputFileName);

  for (int ii = 0; ii < c_NDurabilityTypes; ++ii) {
    //Get TTree
    if (m_treeNames[ii] != "NONE") {
      m_tree[ii] = dynamic_cast<TTree*>(m_file->Get(m_treeNames[ii].c_str()));
      if (!m_tree[ii]) {FATAL("TTree " + m_treeNames[ii] + " doesn't exist");}
      INFO("Opened tree " + m_treeNames[ii]);

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

      //Create the TObject pointers
      m_objects[ii] = new TObject* [m_size[ii]];
      for (int jj = 0; jj < m_size[ii]; jj++) {
        m_objects[ii][jj] = 0;
      }

      //Go again over the branchlist and connect the branches with TObject pointers
      int iobject = 0;
      int iarray = 0;
      for (int jj = 0; jj < branches->GetEntriesFast(); jj++) {
        branch = validBranch(jj, branches);
        if (branch) {
          if (static_cast<string>(branch->GetClassName()) == "TClonesArray") {
            branch->SetAddress(&(m_objects[ii][iarray + m_sizeObj[ii]]));
            m_objectNames[ii].push_back(static_cast<string>(branch->GetName()));
            iarray++;
            branch->GetEntry(0);
          } else {
            branch->SetAddress(&(m_objects[ii][iobject]));
            m_objectNames[ii].push_back(static_cast<string>(branch->GetName()));
            iobject++;
            branch->GetEntry(0);
          }
        }
      }
    }
  }

  if (m_tree[c_Persistent]) {
    readTree(c_Persistent);
  }

}


void ModuleSimpleInput::beginRun()
{
  cout << "beginRun called" << endl;
}


void ModuleSimpleInput::event()
{
  m_file->cd();

  readTree(c_Event);
  m_eventNumber++;
}


void ModuleSimpleInput::endRun()
{
  cout << "endRun called" << endl;
}


void ModuleSimpleInput::terminate()
{
  cout << "Term called" << endl;
}


void ModuleSimpleInput::setupTFile()
{
}


void ModuleSimpleInput::readTree(const EDurability& durability)
{
  // Fill m_objects
  WARNING("Durability" << durability)
  m_tree[durability]->GetEntry(m_eventNumber);


  // Store objects in the DataStore
  for (int jj = 0; jj < m_sizeObj[durability]; jj++) {
    DataStore::Instance().storeObject(m_objects[durability][jj], m_objectNames[durability][jj]);
  }
  // Store arrays in the DataStore
  for (int jj = 0; jj < m_size[durability] - m_sizeObj[durability]; jj++) {
    DataStore::Instance().storeArray(static_cast<TClonesArray*>(m_objects[durability][jj+m_sizeObj[durability]]), m_objectNames[durability][jj]);
  }

}

TBranch* ModuleSimpleInput::validBranch(int& ibranch, TObjArray* branches)
{
  TBranch* branch = static_cast<TBranch*>(branches->At(ibranch));
  if (!branch) {
    return 0;
  }

  // if there is a branch list, count only, if the name is on the list.
  string name = "";
  name = static_cast<string>(branch->GetName());
  string branchNames = "";
  vector<string>::iterator it;
  for (it = m_branchNames[0].begin(); it < m_branchNames[0].end(); it++) {
    branchNames = *it + " ";
  }

  if ((m_branchNames[0].size()) && (branchNames.find(name) == string::npos)) {
    return 0;
  }

  return branch;
}
