/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/simpleoutput/SimpleOutputModule.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimpleOutput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SimpleOutputModule::SimpleOutputModule() : Module()
{
  //Set module properties
  setDescription("simple output");
  setPropertyFlags(c_Output);

  //Initialization of some member variables
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    m_size[jj] = 0;
    m_treeNames[jj]   = "NONE";
  }

  m_file = 0;

  m_steerTreeNames.push_back("treeName");
  m_steerTreeNames.push_back("treeNameRun");
  m_steerTreeNames.push_back("treeNamePersistent");

  m_steerBranchNames.push_back("branchNames");
  m_steerBranchNames.push_back("branchNamesRun");
  m_steerBranchNames.push_back("branchNamesPersistent");


  //Parameter definition
  addParam("outputFileName"  , m_outputFileName, "TFile name.", string("SimpleOutput.root"));
  addParam("compressionLevel", m_compressionLevel, "Compression Level: 0 for no, 1 for low, 9 for high compression.", 1);

  addParam(m_steerTreeNames[0], m_treeNames[0], "TTree name for event data. NONE for no output.", string("tree"));
  addParam(m_steerTreeNames[1], m_treeNames[1], "TTree name for run data. NONE for no output.", string("NONE"));
  addParam(m_steerTreeNames[2], m_treeNames[2], "TTree name for peristent data. NONE for no output.", string("NONE"));

  vector<string> branchNames;
  addParam(m_steerBranchNames[0], m_branchNames[0], "Names of branches to be written from event map. Empty means all branches.", branchNames);
  addParam(m_steerBranchNames[1], m_branchNames[1], "Names of branches to be written from run map. Empty means all branches.", branchNames);
  addParam(m_steerBranchNames[2], m_branchNames[2], "Names of branches to be written from persistent map. Empty means all branches.", branchNames);

  addParam("switchBranchNameMeaning", m_switchBranchNameMeaning, "If true, given branch names are excluded instead of taken.", false);
}


SimpleOutputModule::~SimpleOutputModule()
{
  if (m_file) {
    delete m_file;
  }

  for (size_t jj = 0; jj < DataStore::c_NDurabilityTypes; jj++) {
    for (size_t ii = 0; ii < m_size[jj]; ii++) {
      if (m_objects[jj]) {
        if (m_objects[jj][ii]) {
          delete m_objects[jj][ii];
        }
      }
    }
  }
}

void SimpleOutputModule::initialize()
{
  // check for same treeNames
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    for (int jj = 0; jj < DataStore::c_NDurabilityTypes; ++jj) {
      if ((ii != jj) && (m_treeNames[ii] != "NONE") && (m_treeNames[ii] == m_treeNames[jj])) {
        B2ERROR(m_steerTreeNames[ii] << " and " << m_steerTreeNames[jj] << " are the same: " << m_treeNames[ii]);
      }
    }
  }

  //
  setupTFile();

  // get iterators
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ii++) {
    m_iter[2*ii]   = DataStore::Instance().getObjectIterator(static_cast<DataStore::EDurability>(ii));
    m_iter[2*ii+1] = DataStore::Instance().getArrayIterator(static_cast<DataStore::EDurability>(ii));
    m_done[ii]     = false;
  }

  //warning for duplicates in the branchNames vectors
  for (int jj = 0; jj < DataStore::c_NDurabilityTypes; ++jj) {
    size_t size = m_branchNames[jj].size();
    m_branchNames[jj].resize(unique(m_branchNames[jj].begin(), m_branchNames[jj].end()) - m_branchNames[jj].begin());

    if (size != m_branchNames[jj].size()) {
      B2WARNING(m_steerBranchNames[jj] << " has duplicate entries.");
    }
  }

  B2INFO("SimpleOutput initialised.");
}


void SimpleOutputModule::beginRun()
{
  B2INFO("beginRun called.");
}


void SimpleOutputModule::event()
{
  //Restore Object Count:
  if (!m_done[DataStore::c_Event]) {
    m_nObjID = TProcessID::GetObjectCount();
  } else {
    TProcessID::SetObjectCount(m_nObjID);
  }

  //fill Event data
  if (m_treeNames[0] != "NONE") {
    fillTree(DataStore::c_Event);
  }

  //make sure setup is done only once
  m_done[DataStore::c_Event] = true;
}


void SimpleOutputModule::endRun()
{
  //fill Run data
  if (m_treeNames[1] != "NONE") {
    fillTree(DataStore::c_Run);
  }

  //make sure setup is done only once
  m_done[DataStore::c_Run] = true;

  B2INFO("endRun done.");
}


void SimpleOutputModule::terminate()
{
  //fill Persistent data
  if (m_treeNames[2] != "NONE") {
    fillTree(DataStore::c_Persistent);
  }

  //write the trees
  m_file->cd();
  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii] != "NONE") {
      B2INFO("Write TTree " << m_treeNames[ii]);
      m_tree[ii]->Write();
    }
  }

  B2INFO("terminate called");
}

size_t SimpleOutputModule::getSize(const int& mapID)
{
  int sizeCounter = 0;
  m_iter[mapID]->first();
  while (!m_iter[mapID]->isDone()) {
    if (!(m_branchNames[mapID/2].size())) {
      sizeCounter++;
    } else {
      for (size_t ii = 0; ii < m_branchNames[mapID/2].size(); ++ii) {
        if (m_branchNames[mapID/2][ii] == m_iter[mapID]->key()) {
          sizeCounter++;
        }
      }
    }
    m_iter[mapID]->next();
  }
  return sizeCounter;
}

void SimpleOutputModule::setupTFile()
{
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "basf2 Event File");
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int ii = 0; ii < DataStore::c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii] != "NONE") {
      m_tree[ii] = new TTree(m_treeNames[ii].c_str(), m_treeNames[ii].c_str());
      m_tree[ii]->SetAutoSave(1000000000);
    }
  }
}

void SimpleOutputModule::fillTree(const DataStore::EDurability& durability)
{
  size_t sizeCounter = 0;

  //setup tree
  if (!m_done[durability]) {
    if (m_switchBranchNameMeaning) {
      switchBranchNameMeaning(durability);
    }

    if ((m_branchNames[durability].size())) {
      //sorting the branchNames should later speed up things
      sort(m_branchNames[durability].begin(), m_branchNames[durability].end());

      // print out branch names
      B2INFO("Sorted list of branch names for EDurability map " << durability << ":");
      for (vector<string>::iterator stringIter = m_branchNames[durability].begin(); stringIter != m_branchNames[durability].end(); ++stringIter) {
        B2INFO(*stringIter)
      }
    }

    m_sizeObj[durability] = getSize(2 * durability);
    m_size[durability] = m_sizeObj[durability] + getSize(2 * durability + 1);

    if (m_size[durability]) {
      m_objects[durability] = new TObject* [m_size[durability]];
    }

    for (int ii = 2 * durability; ii < 2*durability + 2; ii++) {
      m_iter[ii]->first();
      while (!m_iter[ii]->isDone()) {
        if (!(m_branchNames[durability].size())) {
          m_objects[durability][sizeCounter] = m_iter[ii]->value();
          m_tree[durability]->Branch((m_iter[ii]->key()).c_str(), &(m_objects[durability][sizeCounter]));
          sizeCounter++;
        } else {
          for (size_t jj = 0; jj < m_branchNames[durability].size(); jj++) {
            if (m_branchNames[durability][jj] == m_iter[ii]->key()) {
              m_objects[durability][sizeCounter] = m_iter[ii]->value();
              m_tree[durability]->Branch((m_iter[ii]->key()).c_str(), &(m_objects[durability][sizeCounter]));
              sizeCounter++;
              break; // if branch is found get out of for loop.
            }
          }
        }
        m_iter[ii]->next();
      }
    }
    if ((m_branchNames[durability].size()) && sizeCounter != m_branchNames[durability].size() && !m_switchBranchNameMeaning) {
      B2WARNING("Number of saved branches is not the same as size of steered branchName list for durability " << durability);
    }
  } else {
    // no need to reconnect the arrays, as the TClonesArrays aren't deleted
    m_iter[2*durability]->first();
    while (!m_iter[2*durability]->isDone()) {
      if (!(m_branchNames[durability].size())) {
        m_objects[durability][sizeCounter] = m_iter[2*durability]->value();
        sizeCounter++;
      } else {
        for (size_t jj = 0; jj < m_branchNames[durability].size(); ++jj) {
          if (m_branchNames[durability][jj] == m_iter[2*durability]->key()) {
            m_objects[durability][sizeCounter] = m_iter[2*durability]->value();
            sizeCounter++;
          }
        }
      }
      m_iter[2*durability]->next();
      if (sizeCounter > m_sizeObj[durability]) {B2FATAL("More elements than in first event.");}
    }
  }
  m_tree[durability]->Fill();
}


void SimpleOutputModule::switchBranchNameMeaning(const DataStore::EDurability& durability)
{
  vector<string> branchNameDummy;
  for (int ii = 2 * durability; ii < 2*durability + 2; ii++) {
    m_iter[ii]->first();
    while (!m_iter[ii]->isDone()) {
      bool take = true;
      for (vector<string>::iterator iter = m_branchNames[durability].begin(); iter != m_branchNames[durability].end(); iter++) {
        if (m_iter[ii]->key() == *iter) {
          m_branchNames[durability].erase(iter);
          take = false;
          break;
        }
      }
      if (take) {
        branchNameDummy.push_back(m_iter[ii]->key());
      }
      m_iter[ii]->next();
    }
  }
  if (m_branchNames[durability].size()) {
    B2WARNING(m_branchNames[durability].size() << " Element(s) of the branchNames vector with EDurability " << durability << " is(are) not in the DataStore");
    B2INFO("These members are: ")
    for (vector<string>::iterator iter = m_branchNames[durability].begin(); iter != m_branchNames[durability].end(); iter++) {
      B2INFO(*iter);
    }
  }
  m_branchNames[durability] = branchNameDummy;
  if (branchNameDummy.size()) {
  } else {
    m_branchNames[durability].push_back("NONE");
    B2WARNING("Tree with EDurability " << durability << ": " << m_treeNames[durability] << " will be empty");
  }
}
