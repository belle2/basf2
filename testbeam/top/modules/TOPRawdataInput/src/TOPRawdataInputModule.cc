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
#include <top/dataobjects/TOPTimeZero.h>

#include <sstream>
#include <map>

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

  TOPRawdataInputModule::TOPRawdataInputModule() : Module()
  {
    // set module description
    setDescription("Raw data reader for TOP beam tests. Converts data to standard format (RawTOP) and puts on the data store.");

    // Add parameters
    addParam("inputFileName", m_inputFileName, "Input file name of binary data",
             string(""));
    std::vector<std::string> defaultFileNames;
    addParam("inputFileNames", m_inputFileNames,
             "List of input file names of binary data", defaultFileNames);
    addParam("dataFormat", m_dataFormat,
             "data format: 0 Kurtis packets, 1 gigE events", 0);
    addParam("camacDir", m_camacDir,
             "relative path to camac data (relative to binary data directory)",
             string(""));
    addParam("ftswBin", m_ftswBin,
             "size of ftsw bin in [ns] - to convert to time", 50e-3);
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

    StoreObjPtr<TOPTimeZero>::registerPersistent();

  }


  void TOPRawdataInputModule::beginRun()
  {
  }


  void TOPRawdataInputModule::event()
  {
    // create data store objects
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();
    StoreObjPtr<TOPTimeZero> timeZero;
    timeZero.create();

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
    m_evtNumber = 0;
    m_runNumber = 0;
    m_expNumber = 0;
    m_bytesRead = 0;
    m_packetsRead = 0;
    m_eventsRead = 0;
    m_err = false;
    m_linesRead = 0;

    if (fileName.empty()) {
      B2ERROR("openFile: file name is empty");
      return false;
    }

    m_stream.open(fileName.c_str(), std::ios::binary);
    if (!m_stream) {
      B2ERROR("openFile: " << fileName << " *** failed to open");
      return false;
    }

    if (!m_camacDir.empty()) {
      string dir;
      string name = fileName;
      auto slash = fileName.rfind("/");
      if (slash != string::npos) {
        dir = fileName.substr(0, slash + 1);
        name = fileName.substr(slash + 1);
      }
      auto dot = name.rfind(".");
      if (dot != string::npos) name = name.substr(0, dot);
      m_camacName = dir + m_camacDir + "/" + name + ".camac";
      m_camac.open(m_camacName.c_str());
      if (!m_camac) {
        B2ERROR("openFile: " << m_camacName << " *** failed to open");
        return false;
      }
    }

    bool ok = setExpRunNumbers(fileName);
    if (!ok) B2WARNING("Cannot parse experiment and run numbers from file name");

    beginRun();

    B2INFO("openFile: " << fileName);
    if (m_camac) B2INFO("openFile: " << m_camacName);
    return true;
  }


  void TOPRawdataInputModule::closeFile()
  {

    if (!m_stream.is_open()) return;
    m_stream.close();

    endRun();

    B2INFO("closeFile: " << m_inputFileNames[m_listIndex] << ", "
           << m_eventsRead << " events "
           << m_packetsRead << " packets "
           << m_bytesRead << " bytes read.");

    if (!m_camac.is_open()) return;
    m_camac.close();
    B2INFO("closeFile: " << m_camacName << ", "
           << m_linesRead << " lines read.");

  }


  bool TOPRawdataInputModule::setExpRunNumbers(const std::string& fileName)
  {

    auto ii = fileName.rfind("run");
    if (ii != std::string::npos) {
      auto i1 = ii + 3;
      auto i2 = fileName.rfind("_");
      if (i2 == std::string::npos) i2 = fileName.rfind(".");
      auto substr = fileName.substr(i1, i2 - i1);
      std::stringstream ss;
      ss << substr;
      ss >> m_runNumber;
      if (ss.fail()) return false;
    } else {
      auto i1 = fileName.rfind("-e");
      auto i2 = fileName.rfind("-f");
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
    }

    return true;

  }


  int TOPRawdataInputModule::readEvent()
  {

    switch (m_dataFormat) {
      case 0:
        return readEventIRS3B();
      case 1:
        return readEventIRSXv1();
      default:
        B2ERROR("unknown dataFormat");
        return -1;
    }

  }

  /* ------ IRSX -----------------------------------------------------------------*/

  int TOPRawdataInputModule::readEventIRSXv1()
  {

    bool ok = true;
    if (m_gigEPacket.empty()) {
      ok = readGigEPacket();
      if (!ok) return -1;
    }

    std::map<unsigned, unsigned> eventNum; // used to find next event
    std::map<unsigned, std::vector<unsigned> > eventData; // a container for event data

    // collect packets of an event

    while (ok) {
      unsigned word = m_gigEPacket[0];
      unsigned evtNumber = word & 0x0007FFFF;
      word = m_gigEPacket[1];
      unsigned scrod = (word >> 9) & 0x7F;
      unsigned carrier = (word >> 30) & 0x03;
      unsigned key = carrier + scrod * 4;
      auto it = eventNum.find(key);
      if (it != eventNum.end()) {
        if (evtNumber != it->second) break; // new event detected
      } else {
        eventNum[key] = evtNumber;
      }

      auto ev = eventData.find(scrod);
      if (ev != eventData.end()) {
        auto& scrodData = ev->second;
        for (const auto& word : m_gigEPacket) scrodData.push_back(word); // append packet
        scrodData[1]++; // increment packet counter
      } else {
        unsigned dataFormat = 2;
        unsigned version = 2;
        auto& scrodData = eventData[scrod];
        scrodData.push_back(scrod + (version << 16) + (dataFormat << 24)); // append header
        scrodData.push_back(0); // append packet counter
        for (const auto& word : m_gigEPacket) scrodData.push_back(word); // append packet
        scrodData[1]++; // increment packet counter
      }

      ok = readGigEPacket();
      if (m_stream.eof()) break;
      if (!ok) return -1;
    }

    // store the event

    m_evtNumber++;

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
    unsigned numPackets = 0;
    int k = 0;
    for (const auto& scrodData : eventData) {
      numPackets += scrodData.second[1];
      buffer[k] = (int*) scrodData.second.data();
      bufferSize[k] = (int) scrodData.second.size();
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

    B2DEBUG(100, "Number of packets saved: " << numPackets);

    // read camac data
    if (m_camac.is_open()) {
      unsigned dummy = 0, ftsw = 0;
      m_camac >> dummy >> ftsw;
      if (!m_camac.good()) {
        B2ERROR("Error reading camac data");
        return -1;
      }
      StoreObjPtr<TOPTimeZero> timeZero;
      timeZero->setFTSW(ftsw);
      double t0 = ftsw * m_ftswBin;
      timeZero->setTime(t0);
      m_linesRead++;
    }

    m_eventsRead++;
    return 0;
  }


  bool TOPRawdataInputModule::readGigEPacket()
  {

    m_gigEPacket.clear();

    unsigned packetSize = 0;
    m_stream.read((char*) &packetSize, sizeof(unsigned));
    m_bytesRead += m_stream.gcount();
    if (!m_stream.good()) {
      if (!m_stream.eof()) B2ERROR("Error reading packet header word");
      return false;
    }

    packetSize--; // Footer to be read separately
    const unsigned maxSize = 257 * 512 + 1; // 257(words) * maxWindows + 1(words)
    if (packetSize > maxSize) {
      B2ERROR("Packet size exceeds the limit of " << maxSize << " words: size = "
              << packetSize << " words");
      return false;
    }

    if (m_gigEPacket.size() != packetSize) m_gigEPacket.resize(packetSize);
    m_stream.read((char*) m_gigEPacket.data(), packetSize * sizeof(unsigned));
    m_bytesRead += m_stream.gcount();
    if (!m_stream.good()) {
      B2ERROR("Error reading packet data");
      m_gigEPacket.clear();
      return false;
    }

    unsigned footer = 0;
    m_stream.read((char*) &footer, sizeof(unsigned));
    m_bytesRead += m_stream.gcount();
    if (!m_stream.good()) {
      B2ERROR("Error reading packet footer word");
      m_gigEPacket.clear();
      return false;
    }

    const char* last = "tsal";
    unsigned* wlast = (unsigned*) last;
    if (footer != *wlast) {
      B2ERROR("Invalid footer word");
      m_gigEPacket.clear();
      return false;
    }

    if ((packetSize - 1) % 257 != 0) {
      B2ERROR("Corrupted packet: packetSize");
      m_gigEPacket.clear();
      return false;
    }

    // check that number of windows is correct
    unsigned numWindows = (packetSize - 1) / 257;
    unsigned word = m_gigEPacket[0];
    unsigned nw = (word >> 19) & 0x1FF; nw++; // should be incremented !
    if (numWindows != nw) {
      B2ERROR("Corrupted packet: numWindows");
      m_gigEPacket.clear();
      return false;
    }

    m_packetsRead++;
    return true;
  }


  /* ------ IRS3B ----------------------------------------------------------------*/

  int TOPRawdataInputModule::readEventIRS3B()
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
    for (auto& scrodData : event) {
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
    for (auto& scrodData : event) delete scrodData;
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

