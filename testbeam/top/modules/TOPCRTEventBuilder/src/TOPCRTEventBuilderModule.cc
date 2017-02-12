/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2016 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <testbeam/top/modules/TOPCRTEventBuilder/TOPCRTEventBuilderModule.h>
#include <top/RawDataTypes.h>

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
#include <rawdata/dataobjects/RawDataBlock.h>
#include <rawdata/dataobjects/RawTOP.h>

#include <sstream>

using namespace std;

namespace Belle2 {

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPCRTEventBuilder)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPCRTEventBuilderModule::TOPCRTEventBuilderModule() : Module()

  {
    // set module description (e.g. insert text)
    setDescription("Event builder for PocketDAQ root files of winter 2016 CRT tests."
                   " These files are readout-frame based."
                   " Module opens the root file, reads 'tree' entries"
                   " until it detects next event, converts RawDataBlocks branches into"
                   " RawTOP objects, fix the raw data format and appends objects to"
                   " StoreArray<RawTOP>. Format fix includes adding of header word,"
                   " removal of repeated word and byte swapping.");

    // Add parameters
    addParam("inputFileName", m_inputFileName,
             "input file name (basf2 root file)", string(""));
    addParam("inputFileNames", m_inputFileNames,
             "list of input file names (basf2 root file)", m_inputFileNames);

  }

  TOPCRTEventBuilderModule::~TOPCRTEventBuilderModule()
  {
  }

  void TOPCRTEventBuilderModule::initialize()
  {

    if (!m_inputFileName.empty()) m_inputFileNames.push_back(m_inputFileName);

    if (m_inputFileNames.empty()) {
      B2FATAL("No input files have been specified");
      return;
    }

    m_tree = new TChain("tree");

    int numFiles = 0;
    for (const auto& fileName : m_inputFileNames) {

      // wildcarding is not allowed
      if (TString(fileName.c_str()).Contains("*")) {
        B2WARNING(fileName << ": skip it, wildcards in file name are not allowed!");
        continue;
      }

      // check for the file existance
      TFile* f = TFile::Open(fileName.c_str(), "READ");
      if (!f) {
        B2WARNING(fileName << ": file not found");
        continue;
      }
      if (!f->IsOpen()) {
        B2WARNING(fileName << ": can't open file");
        continue;
      }

      // check for the existanece of 'tree'
      TTree* tree = (TTree*)f->Get("tree");
      if (!tree) {
        B2WARNING(fileName << ": TTree 'tree' not found -> "
                  << "skip it, not a basf2 root file");
        continue;
      }
      // check for existence of a branch 'RawDataBlocks'
      if (!tree->GetBranch("RawDataBlocks")) {
        B2WARNING(fileName << ": TBranch 'RawDataBlocks' not found -> "
                  << "skip it, not a PocketDAQ root file");
        continue;
      }

      f->Close();

      int num = m_tree->Add(fileName.c_str());
      numFiles += num;

      if (num != 0) {
        m_runNumbers.push_back(getRunNumber(fileName));
      } else {
        B2WARNING(fileName << ": not added to TChain");
      }
    }

    if (numFiles == 0) {
      B2FATAL("no PocketDAQ file(s) in the list");
      return;
    }

    m_numEntries = m_tree->GetEntries();

    // set branch address
    if (!m_tree->GetBranch("RawDataBlocks")) {
      B2FATAL(": TBranch 'RawDataBlocks' not found in the 'tree' -> "
              << "not PocketDAQ root file(s)");
      return;
    }
    m_tree->SetBranchAddress("RawDataBlocks", &m_cloneArray);

    // register in DataStore
    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.registerInDataStore();

    StoreArray<RawTOP> rawData;
    rawData.registerInDataStore();

  }

  void TOPCRTEventBuilderModule::beginRun()
  {

  }

  void TOPCRTEventBuilderModule::event()
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    evtMetaData.create();

    if (m_entryCount >= m_numEntries) {
      evtMetaData->setEndOfData();
      return;
    }

    if (m_runNumber < 0) {
      B2ERROR("Cannot proceed with unknown run number");
      evtMetaData->setEndOfData();
      return;
    }

    StoreArray<RawTOP> rawData;

    while (m_entryCount < m_numEntries) {
      m_tree->GetEntry(m_entryCount);
      m_runNumber = m_runNumbers[m_tree->GetTreeNumber()];

      int numEntries = m_cloneArray->GetEntriesFast();
      if (numEntries != 1)
        B2ERROR("RawDataBlocks: expect single entry, got " << numEntries <<
                " first entry will be used only");

      auto* rawDataBlock = static_cast<RawDataBlock*>(m_cloneArray->AddrAt(0));
      int numEvents = rawDataBlock->GetNumEvents();
      if (numEvents != 1)
        B2ERROR("RawDataBlocks: expect single event, got " << numEvents <<
                " first event will be used only");
      int numNodes = rawDataBlock->GetNumNodes();
      if (numNodes != 1)
        B2ERROR("RawDataBlocks: expect single node, got " << numNodes <<
                " first node will be used only");
      int blkid = 0;
      int nwords = rawDataBlock->GetBlockNwords(blkid);
      RawTOP raw;
      raw.SetBuffer(rawDataBlock->GetBuffer(blkid), nwords, 0, 1, 1);
      int expno = raw.GetExpNo(0);
      int runno = raw.GetRunNo(0);
      int subno = raw.GetSubRunNo(0);
      int eveno = raw.GetEveNo(0);
      if (m_entryCount ==  0) m_evtNumber = eveno;
      if (eveno == -1) eveno = m_evtNumber;
      if (eveno != m_evtNumber) {
        evtMetaData->setEvent(m_evtNumber);
        evtMetaData->setRun(m_runNumber);
        evtMetaData->setExperiment(m_expNumber);
        m_evtNumber = eveno;
        return;
      }
      appendData(raw, rawData);
      m_entryCount++;
      B2DEBUG(100, "entry " << m_entryCount << " clonesArray entries " << numEntries
              << " " << expno << " " << runno << " " << subno << " " << eveno);
    }

    evtMetaData->setEvent(m_evtNumber);
    evtMetaData->setRun(m_runNumber);
    evtMetaData->setExperiment(m_expNumber);

  }


  void TOPCRTEventBuilderModule::endRun()
  {
  }

  void TOPCRTEventBuilderModule::terminate()
  {
    delete m_tree;
  }


  int TOPCRTEventBuilderModule::getRunNumber(const std::string& fileName)
  {

    m_expNumber = 0; // Fuji tests

    auto ii = fileName.find("run");
    if (ii == std::string::npos) {
      B2WARNING("Cannot deduce run number from file name");
      return -1;
    }
    auto i1 = ii + 3;
    auto i2 = fileName.rfind(".root");
    auto substr = fileName.substr(i1, i2 - i1);
    std::stringstream ss;
    int runNumber = 0;
    ss << substr;
    ss >> runNumber;
    if (!ss.fail()) return abs(runNumber);

    i2 = substr.find("_");
    substr = substr.substr(i1, i2 - i1);
    ss << substr;
    ss >> runNumber;
    if (ss.fail()) {
      B2WARNING("Cannot deduce run number from file name");
      return -1;
    }
    if (fileName.rfind("cpr3015") != std::string::npos) m_expNumber = 1; // Tsukuba tests
    if (fileName.rfind("cpr3006") != std::string::npos) m_expNumber = 2; // BelleII tests
    return abs(runNumber);
  }


  void TOPCRTEventBuilderModule::appendData(RawTOP& rawTOP, StoreArray<RawTOP>& rawData)
  {

    const unsigned scrods[4] = {45, 50, 48, 46}; // fix for some runs

    vector<int> Buffer[4];
    for (int finesse = 0; finesse < 4; finesse++) {
      int size = rawTOP.GetDetectorNwords(0, finesse);
      if (size < 2) {
        B2ERROR("RawTOP: data buffer is empty");
        continue;
      }
      int* data = rawTOP.GetDetectorBuffer(0, finesse);
      if (data[size - 1] != 0x7473616c) {
        B2ERROR("RawTOP: inconsistent magic trailer word -> data buffer dropped");
        continue;
      }
      int i0 = 0;
      if (data[0] == data[1]) i0 = 1;
      int word = swap32(data[i0 + 1]);
      unsigned scrodID = (word >> 9) & 0x7F;
      unsigned dataFormat = static_cast<unsigned>(TOP::RawDataType::c_GigE);
      if (scrodID == 16 and m_expNumber == 0 and
          m_runNumber >= 1700 and m_runNumber < 2220) {
        scrodID = scrods[finesse]; // fix since ID is wrong
      }
      Buffer[finesse].push_back(scrodID + (dataFormat << 16));
      Buffer[finesse].push_back(1); // number of gigE packets
      for (int i = i0; i < size - 1; i++) {
        Buffer[finesse].push_back(swap32(data[i]));
      }
    }

    RawCOPPERPackerInfo info;
    info.exp_num = m_expNumber;
    info.run_subrun_num = (m_runNumber << 8);
    info.eve_num = m_evtNumber;
    info.node_id = TOP_ID + (rawTOP.GetNodeID(0) & COPPERID_MASK);
    info.tt_ctime = rawTOP.GetTTCtime(0);
    info.tt_utime = rawTOP.GetTTUtime(0);
    info.b2l_ctime = 0; // ???
    info.hslb_crc16_error_bit = rawTOP.GetErrorBitFlag(0);
    info.truncation_mask = rawTOP.GetTruncMask(0);
    info.type_of_data = rawTOP.GetDataType(0);

    auto* raw = rawData.appendNew();
    raw->PackDetectorBuf(Buffer[0].data(), Buffer[0].size(),
                         Buffer[1].data(), Buffer[1].size(),
                         Buffer[2].data(), Buffer[2].size(),
                         Buffer[3].data(), Buffer[3].size(),
                         info);
  }

} // end Belle2 namespace

