/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Martin Heck                                              *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

#include <framework/modules/simpleoutput/simpleoutput.h>


using namespace std;
using namespace Belle2;

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(SimpleOutput, "SimpleOutput")

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

SimpleOutput::SimpleOutput() : Module()
{
  //Set module properties
  setDescription("simple output");
  setPropertyFlags(c_WritesDataSingleProcess | c_RequiresSingleProcess);

  //Initialization of some member variables
  for (int jj = 0; jj < c_NDurabilityTypes; jj++) {
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
  addParam("outputFileName"  , m_outputFileName, string("SimpleOutput.root"), "TFile name.");
  addParam("compressionLevel", m_compressionLevel, 1, "Compression Level: 0 for no, 1 for low, 9 for high compression.");

  addParam(m_steerTreeNames[0], m_treeNames[0], string("tree"), "TTree name for event data. NONE for no output.");
  addParam(m_steerTreeNames[1], m_treeNames[1], string("NONE"), "TTree name for run data. NONE for no output.");
  addParam(m_steerTreeNames[2], m_treeNames[2], string("NONE"), "TTree name for peristent data. NONE for no output.");

  vector<string> branchNames;
  addParam(m_steerBranchNames[0], m_branchNames[0], branchNames, "Names of branches to be written from event map. Empty means all branches.");
  addParam(m_steerBranchNames[1], m_branchNames[1], branchNames, "Names of branches to be written from run map. Empty means all branches.");
  addParam(m_steerBranchNames[2], m_branchNames[2], branchNames, "Names of branches to be written from persistent map. Empty means all branches.");

  addParam("switchBranchNameMeaning", m_switchBranchNameMeaning, false, "If true, given branch names are excluded instead of taken.");
}


SimpleOutput::~SimpleOutput()
{
  if (m_file) {
    delete m_file;
  }

  for (size_t jj = 0; jj < c_NDurabilityTypes; jj++) {
    for (size_t ii = 0; ii < m_size[jj]; ii++) {
      if (m_objects[jj]) {
        if (m_objects[jj][ii]) {
          delete m_objects[jj][ii];
        }
      }
    }
  }
}

void SimpleOutput::initialize()
{
  // check for same treeNames
  for (int ii = 0; ii < c_NDurabilityTypes; ++ii) {
    for (int jj = 0; jj < c_NDurabilityTypes; ++jj) {
      if ((ii != jj) && (m_treeNames[ii] != "NONE") && (m_treeNames[ii] == m_treeNames[jj])) {
        ERROR(m_steerTreeNames[ii] << " and " << m_steerTreeNames[jj] << " are the same: " << m_treeNames[ii]);
      }
    }
  }

  //
  setupTFile();

  // get iterators
  for (int ii = 0; ii < c_NDurabilityTypes; ii++) {
    m_iter[2*ii]   = DataStore::Instance().getObjectIterator(static_cast<EDurability>(ii));
    m_iter[2*ii+1] = DataStore::Instance().getArrayIterator(static_cast<EDurability>(ii));
    m_done[ii]     = false;
  }

  //warning for duplicates in the branchNames vectors
  for (int jj = 0; jj < c_NDurabilityTypes; ++jj) {
    size_t size = m_branchNames[jj].size();
    m_branchNames[jj].resize(unique(m_branchNames[jj].begin(), m_branchNames[jj].end()) - m_branchNames[jj].begin());

    if (size != m_branchNames[jj].size()) {
      WARNING(m_steerBranchNames[jj] << " has duplicate entries.");
    }
  }

  INFO("SimpleOutput initialised.");
}


void SimpleOutput::beginRun()
{
  INFO("beginRun called.");
}


void SimpleOutput::event()
{
  //Restore Object Count:
  if (!m_done[c_Event]) {
    m_nObjID = TProcessID::GetObjectCount();
  } else {
    TProcessID::SetObjectCount(m_nObjID);
  }

  //fill Event data
  if (m_treeNames[0] != "NONE") {
    fillTree(c_Event);
  }

  //make sure setup is done only once
  m_done[c_Event] = true;
}


void SimpleOutput::endRun()
{
  //fill Run data
  if (m_treeNames[1] != "NONE") {
    fillTree(c_Run);
  }

  //make sure setup is done only once
  m_done[c_Run] = true;

  INFO("endRun done.");
}


void SimpleOutput::terminate()
{
  //fill Persistent data
  if (m_treeNames[2] != "NONE") {
    fillTree(c_Persistent);
  }

  //write the trees
  m_file->cd();
  for (int ii = 0; ii < c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii] != "NONE") {
      INFO("Write TTree " << m_treeNames[ii]);
      m_tree[ii]->Write();
    }
  }

  INFO("terminate called");
}

size_t SimpleOutput::getSize(const int& mapID)
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

void SimpleOutput::setupTFile()
{
  m_file = new TFile(m_outputFileName.c_str(), "RECREATE", "basf2 Event File");
  m_file->SetCompressionLevel(m_compressionLevel);

  for (int ii = 0; ii < c_NDurabilityTypes; ++ii) {
    if (m_treeNames[ii] != "NONE") {
      m_tree[ii] = new TTree(m_treeNames[ii].c_str(), m_treeNames[ii].c_str());
      m_tree[ii]->SetAutoSave(1000000000);
    }
  }
}

void SimpleOutput::fillTree(const EDurability& durability)
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
      INFO("Sorted list of branch names for EDurability map " << durability << ":");
      for (vector<string>::iterator stringIter = m_branchNames[durability].begin(); stringIter != m_branchNames[durability].end(); ++stringIter) {
        INFO(*stringIter)
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
      WARNING("Number of saved branches is not the same as size of steered branchName list for durability " << durability);
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
      if (sizeCounter > m_sizeObj[durability]) {FATAL("More elements than in first event.");}
    }
  }
  m_tree[durability]->Fill();
}


void SimpleOutput::switchBranchNameMeaning(const EDurability& durability)
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
    WARNING(m_branchNames[durability].size() << " Element(s) of the branchNames vector with EDurability " << durability << " is(are) not in the DataStore");
    INFO("These members are: ")
    for (vector<string>::iterator iter = m_branchNames[durability].begin(); iter != m_branchNames[durability].end(); iter++) {
      INFO(*iter);
    }
  }
  m_branchNames[durability] = branchNameDummy;
  if (branchNameDummy.size()) {
  } else {
    m_branchNames[durability].push_back("NONE");
    WARNING("Tree with EDurability " << durability << ": " << m_treeNames[durability] << " will be empty");
  }
}
