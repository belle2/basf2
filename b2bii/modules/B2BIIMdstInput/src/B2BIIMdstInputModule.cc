//+
// File : B2BIIMdstInutModule.cc
// Description : A module to read panther records in basf2
//
// Author : Ryosuke Itoh, IPNS, KEK
// Date : 16 - Feb - 2015
//
// Contributors: Anze Zupanc, Martin Ritter
//-

#include <b2bii/modules/B2BIIMdstInput/B2BIIMdstInputModule.h>

#include <framework/core/Environment.h>
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/utilities/FileSystem.h>

// Belle tables
#include "belle_legacy/tables/belletdf.h"

// Belle II dataobjects
#include <framework/dataobjects/EventMetaData.h>

#include <cstdlib>
#include <algorithm>

using namespace std;
using namespace Belle2;


/*
Some code for simple globing from
http://stackoverflow.com/questions/8401777/simple-glob-in-c-on-unix-system
In RootInput this is handled by ROOT
*/
#include <glob.h>
#include <vector>
#include <string>
vector<string> globbing(const vector<string>& patterns)
{
  vector<string> ret;
  for (const auto& pat : patterns) {
    glob_t glob_result;
    glob(pat.c_str(), GLOB_TILDE, NULL, &glob_result);
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
      ret.push_back(string(glob_result.gl_pathv[i]));
    }
    globfree(&glob_result);
  }
  return ret;
}

//-----------------------------------------------------------------
//                 Register the Module
//-----------------------------------------------------------------
REG_MODULE(B2BIIMdstInput)

//-----------------------------------------------------------------
//                 Implementation
//-----------------------------------------------------------------

B2BIIMdstInputModule::B2BIIMdstInputModule() : Module()
{
  //Set module properties
  setDescription("Module to read Belle MDST files.");
  setPropertyFlags(c_Input);

  m_nevt = -1;

  //Parameter definition
  addParam("inputFileName", m_inputFileName, "Belle MDST input file name. "
           "For more than one file use inputFileNames", std::string(""));
  addParam("inputFileNames"  , m_inputFileNames, "Belle MDST input file names.",
           m_inputFileNames);
}


B2BIIMdstInputModule::~B2BIIMdstInputModule()
{
}

std::vector<std::string> B2BIIMdstInputModule::getInputFiles() const
{
  std::vector<std::string> inputFiles = Environment::Instance().getInputFilesOverride();
  if (!inputFiles.empty()) {
    return inputFiles;
  }
  inputFiles = m_inputFileNames;
  if (!m_inputFileName.empty())
    inputFiles.push_back(m_inputFileName);
  return inputFiles;
}

void B2BIIMdstInputModule::initialize()
{
  m_inputFileNames = globbing(getInputFiles());

  //Check if there is at least one filename provided
  if (m_inputFileNames.empty()) {
    B2FATAL("Empty list of files supplied, cannot continue");
  }
  //Ok we have files. Since vectors can only remove efficiently from the back
  //we reverse the order and read the files from back to front.
  std::reverse(m_inputFileNames.begin(), m_inputFileNames.end());

  // check environment
  const char* table_dir = getenv("PANTHER_TABLE_DIR");
  if (!table_dir or !FileSystem::isDir(table_dir)) {
    string fixed_table_dir = Environment::Instance().getExternalsPath() + "/share/belle_legacy/panther";
    B2WARNING("PANTHER_TABLE_DIR environment variable not set correctly. This is a known problem with externals v00-05-09, using " <<
              fixed_table_dir << " instead.");
    if (!FileSystem::isDir(fixed_table_dir))
      B2FATAL("Path " << fixed_table_dir << " does not exist, your externals setup seems broken.");
    setenv("PANTHER_TABLE_DIR", fixed_table_dir.c_str(), 1); //overwrite existing value
  }


  // Initialize Panther
  BsInit(0);

  // Initialize Belle II DataStore
  initializeDataStore();

  // open the first file
  openNextFile();

  B2DEBUG(1, "B2BIIMdstInput: initialized.");
}

void B2BIIMdstInputModule::initializeDataStore()
{
  B2DEBUG(99, "[B2BIIMdstInputModule::initializeDataStore] initialization of DataStore started");

  // event meta data Object pointer
  StoreObjPtr<EventMetaData>::registerPersistent();

  B2DEBUG(99, "[B2BIIMdstInputModule::initializeDataStore] initialization of DataStore ended");
}


void B2BIIMdstInputModule::beginRun()
{
  B2DEBUG(99, "B2BIIMdstInput: beginRun called.");
}

bool B2BIIMdstInputModule::openNextFile()
{
  // delete existing FileIO
  BsClrTab(BBS_CLEAR_ALL);
  delete m_fd;
  m_fd = nullptr;

  // check if we have more files to read
  if (m_inputFileNames.empty()) return false;
  // if so, get the last one from the list and remove it.
  const std::string name = m_inputFileNames.back();
  m_inputFileNames.pop_back();

  // Open data file
  m_fd = new Belle::Panther_FileIO(name.c_str(), BBS_READ);

  // Read first record (does not contain event data)
  if (m_fd->read() == -1) {
    B2FATAL("Couldn't read file '" << name << "'!");
  }
  m_nevt++;
  return true;
}

bool B2BIIMdstInputModule::readNextEvent()
{
  // read event
  int rectype = -1;
  while (rectype < 0 && rectype != -2) {
    //clear all previous event data before reading!
    BsClrTab(BBS_CLEAR);
    rectype = m_fd->read();
    if (rectype == -1) {
      B2ERROR("Error while reading panther tables! Record skipped.");
    }
  }
  if (rectype == -2) { // EoF detected
    B2DEBUG(99, "[B2BIIMdstInputModule::Conversion] Conversion stopped at event #" << m_nevt << ". EOF detected!");
    return false;
  }
  return true;
}

void B2BIIMdstInputModule::event()
{
  m_nevt++;

  // Fill EventMetaData
  StoreObjPtr<EventMetaData> evtmetadata;
  evtmetadata.create();

  // Read next event: We try to read the next event from the current file
  // if thist fails, open the next file and try again
  // if we cannot open the next file then stop processing
  while (!readNextEvent()) {
    if (!openNextFile()) {
      evtmetadata->setEndOfData(); // stop event processing
      B2DEBUG(99, "[B2BIIMdstInputModule::Conversion] Conversion stopped at event #" << m_nevt << ". No more files");
      return;
    }
  }

  // Convert the Belle_event -> EventMetaData
  // Get Belle_event_Manager
  Belle::Belle_event_Manager& evman = Belle::Belle_event_Manager::get_manager();
  Belle::Belle_event& evt = evman[0];

  // set exp/run/evt numbers
  evtmetadata->setExperiment(evt.ExpNo());
  evtmetadata->setRun(evt.RunNo());
  evtmetadata->setEvent(evt.EvtNo() & 0x0fffffff);

  // set generated weight (>0 for MC; <0 for real DATA)
  evtmetadata->setGeneratedWeight((evt.ExpMC() == 2) ? 1.0 : -1.0);

  B2DEBUG(90, "[B2BIIMdstInputModule] Convert exp/run/evt: " << evt.ExpNo() << "/" << evt.RunNo() << "/" << int(
            evt.EvtNo() & 0x0fffffff));
}

void B2BIIMdstInputModule::endRun()
{
  B2INFO("B2BIIMdstInput: endRun done.");
}


void B2BIIMdstInputModule::terminate()
{
  delete m_fd;
  B2INFO("B2BIIMdstInput: terminate called");
}

