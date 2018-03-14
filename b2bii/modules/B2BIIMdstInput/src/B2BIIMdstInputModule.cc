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
#include <framework/utilities/FileSystem.h>

// Belle tables
#include "belle_legacy/tables/belletdf.h"

// Belle II dataobjects
#include <framework/utilities/NumberSequence.h>

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
  m_current_file_position = -1;

  //Parameter definition
  addParam("inputFileName", m_inputFileName, "Belle MDST input file name. "
           "For more than one file use inputFileNames", std::string(""));
  addParam("inputFileNames"  , m_inputFileNames, "Belle MDST input file names.",
           m_inputFileNames);

  std::vector<std::string> emptyvector;
  addParam("entrySequences", m_entrySequences,
           "The number sequences (e.g. 23:42,101) defining the entries which are processed for each inputFileName."
           "Must be specified exactly once for each file to be opened."
           "The first event has the number 0.", emptyvector);
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

  auto entrySequencesOverride = Environment::Instance().getEntrySequencesOverride();
  if (entrySequencesOverride.size() > 0)
    m_entrySequences = entrySequencesOverride;

  //Check if there is at least one filename provided
  if (m_inputFileNames.empty()) {
    B2FATAL("Empty list of files supplied, cannot continue");
  }

  if (m_entrySequences.size() > 0 and m_inputFileNames.size() != m_entrySequences.size()) {
    B2FATAL("Number of provided filenames does not match the number of given entrySequences parameters: len(inputFileNames) = "
            << m_inputFileNames.size() << " len(entrySequences) = " << m_entrySequences.size());
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

  m_evtMetaData.registerInDataStore();

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
  m_current_file_position++;
  m_current_file_entry = -1;

  if (m_entrySequences.size() > 0)
    // exclude ":" since this case is not considered in generate_number_sequence
    if (m_entrySequences[m_current_file_position] != ":") {
      m_valid_entries_in_current_file = generate_number_sequence(m_entrySequences[m_current_file_position]);
    }
  return true;
}

bool B2BIIMdstInputModule::readNextEvent()
{

  do {
    m_current_file_entry++;
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

  } while (m_entrySequences.size() > 0
           and (m_entrySequences[m_current_file_position] != ":"
                and m_valid_entries_in_current_file.find(m_current_file_entry) == m_valid_entries_in_current_file.end()));

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

  // Check if RUNHEAD is available if not create one using the event.
  // Basf did something similar in Record::convert_to_begin_run.
  // Some files are missing the RUNHEAD for unknown reasons.
  Belle::Belle_runhead_Manager& rhdmgr = Belle::Belle_runhead_Manager::get_manager();
  Belle::Belle_runhead_Manager::const_iterator belleevt = rhdmgr.begin();

  B2DEBUG(90, "Event number " << m_nevt);
  if (belleevt == rhdmgr.end() || not(*belleevt)) {
    B2WARNING("Missing RUNHEAD: Creating RUNHEAD from Event. This is as far as we know fine and handled correctly.");

    Belle::Belle_runhead& bgr = rhdmgr.add();
    bgr.ExpMC(evt.ExpMC());
    bgr.ExpNo(evt.ExpNo());
    bgr.RunNo(evt.RunNo());
    bgr.Time(evt.Time());
    bgr.Date(evt.Date());
    bgr.Field(evt.MagFieldID());
    bgr.MaxField(evt.BField());
    bgr.Type(0); // basf used 0, and the debug output below of valid entries returned 0 as well
    bgr.ELER(evt.ELER());
    bgr.EHER(evt.EHER());

    // In basf the BELLE_RUNHEAD which is created like this is not cleared anymore
    // m_fd->non_clear("BELLE_RUNHEAD");
    // However, I think in this case we are going to miss reinitializing the BELLE_RUNHEAD
    // at the beginning of a new run without a BELLE_RUNHEAD, I'm not sure how this was handled
    // in basf. I think it is safest to recreate the BELLE_RUNHEAD for each event which is missing
    // the BELLE_RUNHEAD

  } else {
    Belle::Belle_runhead& bgr = rhdmgr[0];
    B2DEBUG(90, "ExpMC " << bgr.ExpMC() << " " << evt.ExpMC());
    B2DEBUG(90, "ExpNo " << bgr.ExpNo() << " " << evt.ExpNo());
    B2DEBUG(90, "RunNo " << bgr.RunNo() << " " << evt.RunNo());
    B2DEBUG(90, "Time " << bgr.Time() << " " << evt.Time());
    B2DEBUG(90, "Date " << bgr.Date() << " " << evt.Date());
    B2DEBUG(90, "Field " << bgr.Field() << " " << evt.MagFieldID());
    B2DEBUG(90, "MaxField " << bgr.MaxField() << " " << evt.BField());
    B2DEBUG(90, "Type " << bgr.Type());
    B2DEBUG(90, "ELER " << bgr.ELER() << " " << evt.ELER());
    B2DEBUG(90, "EHER " << bgr.EHER() << " " << evt.EHER());
  }

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

