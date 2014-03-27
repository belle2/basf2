/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2010 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPRawdataInput/TOPRawdataInputModule.h>

#include <framework/core/ModuleManager.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>
#include <framework/datastore/RelationArray.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <top/dataobjects/TOPDigit.h>
#include <testbeam/top/dataobjects/TOPTBDigit.h>
#include <testbeam/top/dataobjects/TOPWaveform.h>


using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPRawdataInput)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPRawdataInputModule::TOPRawdataInputModule() : Module(),
    m_listIndex(0), m_evtNumber(0), m_runNumber(0), m_expNumber(0), m_wordsRead(0)
  {
    // set module description
    setDescription("Raw data reader for TOP beam tests");

    // Add parameters
    addParam("inputFileName", m_inputFileName, "Input file name (raw data format)",
             string(""));
    std::vector<std::string> defaultFileNames;
    addParam("inputFileNames", m_inputFileNames,
             "List of input file names (raw data format)", defaultFileNames);


  }


  TOPRawdataInputModule::~TOPRawdataInputModule()
  {
  }


  void TOPRawdataInputModule::initialize()
  {
    if (!m_inputFileName.empty()) m_inputFileNames.push_back(m_inputFileName);
    if (m_inputFileNames.empty()) B2FATAL("No input file name given");

    // open the first accessible file
    bool ok = false;
    for (unsigned i = 0; i < m_inputFileNames.size(); ++i) {
      ok = openFile(m_inputFileNames[i]);
      if (ok) {m_listIndex = i; break;}
    }
    if (!ok) B2FATAL("None of the input files can be open");

    // data store objects registration
    StoreObjPtr<EventMetaData>::registerPersistent();
    StoreArray<TOPDigit>::registerPersistent();
    StoreArray<TOPTBDigit>::registerPersistent();
    StoreArray<TOPWaveform>::registerPersistent();

  }


  void TOPRawdataInputModule::beginRun()
  {
  }


  void TOPRawdataInputModule::event()
  {
    // create data store objects
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();
    StoreArray<TOPDigit> digits;
    digits.create();
    StoreArray<TOPTBDigit> tbDigits;
    tbDigits.create();
    StoreArray<TOPWaveform> waveforms;
    waveforms.create();

    // read event
    int err = 0;
    do {
      err = readDataRecord();
      if (err < 0) {
        closeFile();
        bool ok = false;
        for (unsigned i = m_listIndex + 1; i < m_inputFileNames.size(); ++i) {
          ok = openFile(m_inputFileNames[i]);
          if (ok) {m_listIndex = i; break;}
        }
        if (!ok) {
          evtMetaData->setEndOfData(); // stop event processing
          return;
        }
      }
    } while (err != 0);

    // set event metadata
    evtMetaData->setEvent(m_evtNumber);
    evtMetaData->setRun(m_runNumber);
    evtMetaData->setExperiment(m_expNumber);

    B2INFO("evt run exp: "
           << evtMetaData->getEvent() << " "
           << evtMetaData->getRun() << " "
           << evtMetaData->getExperiment());

  }


  void TOPRawdataInputModule::endRun()
  {
  }


  void TOPRawdataInputModule::terminate()
  {
  }


  bool TOPRawdataInputModule::openFile(const std::string& fileName)
  {
    m_evtNumber = 0;
    m_wordsRead = 0;

    if (fileName.empty()) {
      B2ERROR("openFile: file name is empty");
      return false;
    }

    // open the file; if failed, issue B2ERROR and return false

    /* <implement code here> */

    // file successfuly open
    m_runNumber++; // obtain it from the file name (?)
    beginRun();
    B2INFO("openFile: " << fileName << " is open for reading");
    return true;
  }


  void TOPRawdataInputModule::closeFile()
  {
    // close the file

    /* <implement code here> */

    endRun();
    B2INFO("closeFile: " << m_inputFileNames[m_listIndex] << " file is closed, "
           << m_wordsRead << " words read");
  }


  int TOPRawdataInputModule::readDataRecord()
  {

    /* <implement code here> */

    // --- just for my tests -> please, remove this code
    unsigned i = m_wordsRead;
    if (i < m_inputFileNames[m_listIndex].size()) {
      m_wordsRead++;
      if (i == 0) {
        B2INFO("header record found");
        return 1;
      }
      m_evtNumber++;
      if (m_inputFileNames[m_listIndex][i] == string(".")) {
        B2WARNING("corrupted record found");
        return 2; // corrupted record
      }
      return 0; // data record successfully read and decoded
    }
    return -1; // EOF reached
    // --- end of test code
  }


} // end Belle2 namespace

