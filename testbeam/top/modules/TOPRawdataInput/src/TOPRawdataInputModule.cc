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

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// DataStore classes
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawTOP.h>

#include <sstream>

using namespace std;

namespace Belle2 {

  using namespace TOPTB;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPRawdataInput)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPRawdataInputModule::TOPRawdataInputModule() : Module(),
    m_listIndex(0), m_evtNumber(0), m_runNumber(0), m_expNumber(0), m_bytesRead(0),
    m_packetsRead(0), m_eventsRead(0), m_scrodData(0), m_err(false)
  {
    // set module description
    setDescription("Raw data reader for TOP beam tests. Converts data to standard format (RawTOP) and puts on the data store.");

    // Add parameters
    addParam("inputFileName", m_inputFileName, "Input file name (.dat format)",
             string(""));
    std::vector<std::string> defaultFileNames;
    addParam("inputFileNames", m_inputFileNames,
             "List of input file names (.dat format)", defaultFileNames);


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

    StoreArray<RawTOP> rawData;
    rawData.registerInDataStore();

  }


  void TOPRawdataInputModule::beginRun()
  {
  }


  void TOPRawdataInputModule::event()
  {
    // create data store objects
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();

    // read event
    int err = 0;
    do {
      err = readEvent();
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

    B2DEBUG(100, "exp " << evtMetaData->getExperiment() <<
            " run " << evtMetaData->getRun() <<
            " event " << evtMetaData->getEvent());

  }


  void TOPRawdataInputModule::endRun()
  {
  }


  void TOPRawdataInputModule::terminate()
  {
    if (m_stream) closeFile();
  }


  bool TOPRawdataInputModule::openFile(const std::string& fileName)
  {
    m_bytesRead = 0;
    m_packetsRead = 0;
    m_eventsRead = 0;
    m_err = false;

    if (fileName.empty()) {
      B2ERROR("openFile: file name is empty");
      return false;
    }

    m_stream.open(fileName.c_str(), std::ios::binary);
    if (!m_stream) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return false;
    }

    bool ok = setExpRunNumbers(fileName);
    if (!ok) B2WARNING("Cannot parse experiment and run numbers from file name");

    beginRun();
    B2INFO("openFile: " << fileName);
    return true;
  }


  bool TOPRawdataInputModule::setExpRunNumbers(const std::string& fileName)
  {

    std::size_t i1 = fileName.rfind("-e");
    std::size_t i2 = fileName.rfind("-f");
    int len = i2 - i1 - 2;
    if (len <= 0) return false;

    i1 += 2;
    std::string substr = fileName.substr(i1, len);
    i2 = substr.find("r");
    if (i2 == std::string::npos) return false;

    substr.replace(i2, 1 , " ");
    std::stringstream ss;
    ss << substr;
    ss >> m_expNumber >> m_runNumber;
    if (ss.fail()) return false;

    return true;

  }


  void TOPRawdataInputModule::closeFile()
  {
    // close the file

    if (!m_stream.is_open()) return;
    m_stream.close();

    endRun();
    B2INFO("closeFile: " << m_inputFileNames[m_listIndex] << ", "
           << m_eventsRead << " events "
           << m_packetsRead << " packets "
           << m_bytesRead << " bytes read.");
  }


  int TOPRawdataInputModule::readEvent()
  {

    if (m_err) return -1;

    std::vector<ScrodData*> event;

    if (!m_scrodData) {
      m_scrodData = readScrodData();
      if (!m_scrodData) return -1;
    }

    m_evtNumber = m_scrodData->getEventNumber();
    auto evtNumber = m_evtNumber;
    while (evtNumber == m_evtNumber) {
      event.push_back(m_scrodData);
      m_scrodData = readScrodData();
      if (!m_scrodData) break;
      evtNumber = m_scrodData->getEventNumber();
    }

    StoreArray<RawTOP> rawData;

    RawCOPPERPackerInfo info;
    info.exp_num = m_expNumber;
    info.run_subrun_num = (m_runNumber << 8);
    info.eve_num = m_evtNumber;
    info.node_id = TOP_ID;
    info.tt_ctime = 0;
    info.tt_utime = 0;
    info.b2l_ctime = 0;
    info.hslb_crc16_error_bit = 0;
    info.truncation_mask = 0;
    info.type_of_data = 0;

    int* buffer[4] = {0, 0, 0, 0};
    int bufferSize[4] = {0, 0, 0, 0};
    int k = 0;
    for (auto & scrodData : event) {
      buffer[k] = scrodData->getData();
      bufferSize[k] = scrodData->getDataSize();
      k++;
      if (k == 4) {
        auto* raw = rawData.appendNew();
        raw->PackDetectorBuf(buffer[0], bufferSize[0],
                             buffer[1], bufferSize[1],
                             buffer[2], bufferSize[2],
                             buffer[3], bufferSize[3],
                             info);
        k = 0;
        info.node_id++;
      }
    }
    if (k > 0) {
      for (int i = k; i < 4; i++) {
        buffer[i] = 0;
        bufferSize[i] = 0;
      }
      auto* raw = rawData.appendNew();
      raw->PackDetectorBuf(buffer[0], bufferSize[0],
                           buffer[1], bufferSize[1],
                           buffer[2], bufferSize[2],
                           buffer[3], bufferSize[3],
                           info);
    }

    m_eventsRead++;
    for (auto & scrodData : event) delete scrodData;
    return 0;

  }


  TOPTB::ScrodData* TOPRawdataInputModule::readScrodData()
  {
    ScrodData* scrodData = new ScrodData();
    bool ok = scrodData->read(m_stream);
    m_bytesRead += scrodData->getBytesRead();
    m_packetsRead += scrodData->getPacketsRead();
    m_err = !ok;
    if (ok) return scrodData;

    delete scrodData;
    return 0;
  }


} // end Belle2 namespace

