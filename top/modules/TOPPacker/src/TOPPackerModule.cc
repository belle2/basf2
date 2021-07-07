/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPPacker/TOPPackerModule.h>
#include <top/RawDataTypes.h>

// framework - DataStore
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// Dataobject classes
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <framework/dataobjects/EventMetaData.h>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPPacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPPackerModule::TOPPackerModule() : Module()
  {
    // set module description (e.g. insert text)
    setDescription("Raw data packer for TOP");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputDigitsName", m_inputDigitsName,
             "name of TOPDigit store array", string(""));
    addParam("inputRawDigitsName", m_inputRawDigitsName,
             "name of TOPRawDigit store array", string(""));
    addParam("outputRawDataName", m_outputRawDataName,
             "name of RawTOP store array", string(""));
    addParam("format", m_format,
             "data format (draft, FE, production)", string("production"));

  }

  TOPPackerModule::~TOPPackerModule()
  {
  }

  void TOPPackerModule::initialize()
  {

    if (m_format == "draft") {
      m_dataType = TOP::RawDataType::c_Draft;
      StoreArray<TOPDigit> digits(m_inputDigitsName);
      digits.isRequired();
    } else if (m_format == "FE") {
      m_dataType = TOP::RawDataType::c_Type0Ver16;
      StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
      rawDigits.isRequired();
    } else if (m_format == "production") {
      m_dataType = TOP::RawDataType::c_ProductionDebug01;
      StoreArray<TOPRawDigit> rawDigits(m_inputRawDigitsName);
      rawDigits.isRequired();
    } else {
      B2ERROR("TOPPacker: unknown data format."
              << LogVar("format", m_format));
    }

    StoreArray<RawTOP> rawData(m_outputRawDataName);
    rawData.registerInDataStore();

    // check if front end mappings are available
    const auto& mapper = m_topgp->getFrontEndMapper();
    if (!mapper.isValid()) B2ERROR("No front-end mapping available for TOP");

  }

  void TOPPackerModule::beginRun()
  {
  }

  void TOPPackerModule::event()
  {

    switch (m_dataType) {
      case TOP::RawDataType::c_Type0Ver16:
        packType0Ver16();
        break;
      case TOP::RawDataType::c_ProductionDebug01:
        packProductionDebug();
        break;
      case TOP::RawDataType::c_Draft:
        packProductionDraft();
        break;
      default:
        B2ERROR("TOPPacker: data format unknown or not implemented");
    }

  }


  void TOPPackerModule::endRun()
  {
  }

  void TOPPackerModule::terminate()
  {
  }


  void TOPPackerModule::packProductionDraft()
  {
    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<TOPDigit> digits(m_inputDigitsName);
    StoreArray<RawTOP> rawData(m_outputRawDataName);

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) return;

    vector<const TOPDigit*>* sortedDigits = new vector<const TOPDigit*>[mapSize];

    for (const auto& digit : digits) {
      int moduleID = digit.getModuleID();
      int boardstack = digit.getChannel() / 128;
      const auto* feemap = mapper.getMap(moduleID, boardstack);
      if (!feemap) {
        B2ERROR("TOPPacker: no front-end map available."
                << LogVar("moduleID", moduleID)
                << LogVar("boardstack", boardstack));
        continue;
      }
      sortedDigits[feemap->getIndex()].push_back(&digit);
    }

    auto subBits = m_topgp->getGeometry()->getNominalTDC().getSubBits();
    int sampleDivisions = 0x1 << subBits;

    for (const auto& copperID : mapper.getCopperIDs()) {
      vector<int> Buffer[4];
      for (int finesse = 0; finesse < 4; finesse++) {
        const auto* feemap = mapper.getMapFromCopper(copperID, finesse);
        if (!feemap) continue;
        unsigned scrodID = feemap->getScrodID();
        unsigned dataFormat = static_cast<unsigned>(TOP::RawDataType::c_Draft);
        Buffer[finesse].push_back(scrodID + (dataFormat << 16));
        for (const auto& digit : sortedDigits[feemap->getIndex()]) {
          double rawTime = digit->getRawTime();
          unsigned tdc = int(rawTime * sampleDivisions) & 0xFFFF;
          unsigned chan = digit->getChannel() % 128;
          unsigned flags = (unsigned) digit->getHitQuality();
          Buffer[finesse].push_back(tdc + (chan << 16) + (flags << 24));
        }
      }
      RawCOPPERPackerInfo info;
      info.exp_num = evtMetaData->getExperiment();
      // run number : 14bits, subrun # : 8bits
      info.run_subrun_num = (evtMetaData->getRun() << 8) +
                            (evtMetaData->getSubrun() & 0xFF);
      info.eve_num = evtMetaData->getEvent();
      info.node_id = TOP_ID + copperID;
      info.tt_ctime = 0;
      info.tt_utime = 0;
      info.b2l_ctime = 0;
      info.hslb_crc16_error_bit = 0;
      info.truncation_mask = 0;
      info.type_of_data = 0;

      auto* raw = rawData.appendNew();
      raw->PackDetectorBuf(Buffer[0].data(), Buffer[0].size(),
                           Buffer[1].data(), Buffer[1].size(),
                           Buffer[2].data(), Buffer[2].size(),
                           Buffer[3].data(), Buffer[3].size(),
                           info);
    }
    delete [] sortedDigits;
  }


  void TOPPackerModule::packType0Ver16()
  {
    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<TOPRawDigit> digits(m_inputRawDigitsName);
    StoreArray<RawTOP> rawData(m_outputRawDataName);

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) return;

    auto* sortedDigits = new vector<const TOPRawDigit*>[mapSize];

    for (const auto& digit : digits) {
      auto scrodID = digit.getScrodID();
      const auto* feemap = mapper.getMap(scrodID);
      if (!feemap) {
        B2ERROR("TOPPacker: no front-end map available."
                << LogVar("scrodID", scrodID));
        continue;
      }
      sortedDigits[feemap->getIndex()].push_back(&digit);
    }

    for (const auto& copperID : mapper.getCopperIDs()) {
      vector<int> Buffer[4];
      for (int finesse = 0; finesse < 4; finesse++) {
        const auto* feemap = mapper.getMapFromCopper(copperID, finesse);
        if (!feemap) continue;
        unsigned scrodID = feemap->getScrodID();
        unsigned dataFormat = static_cast<unsigned>(TOP::RawDataType::c_Type0Ver16);

        // production data v2.1 (data_format_v2_1.xlsx from Lynn 06/26/2016)
        unsigned head = (dataFormat << 16) | (0xA << 12) | (scrodID & 0x0FFF);
        Buffer[finesse].push_back(head);
        unsigned Nhits = 0;
        for (const auto& digit : sortedDigits[feemap->getIndex()]) {
          unsigned word1 =
            (digit->getCarrierNumber() << 30) |
            ((digit->getASICNumber() & 0x3) << 28) |
            ((digit->getASICChannel() & 0x7) << 25) |
            ((digit->getASICWindow() & 0x1FF) << 16) |
            (0xB << 12) |
            ((digit->getTFine() & 0xF) << 8);
          Buffer[finesse].push_back(word1);
          unsigned word2 =
            ((digit->getValuePeak() & 0x1FFF) << 16) |
            (digit->getIntegral() & 0xFFFF);
          Buffer[finesse].push_back(word2);
          unsigned word3 =
            ((digit->getValueRise0() & 0x1FFF) << 16) |
            (digit->getValueRise1() & 0x1FFF);
          Buffer[finesse].push_back(word3);
          unsigned word4 =
            ((digit->getValueFall0() & 0x1FFF) << 16) |
            (digit->getValueFall1() & 0x1FFF);
          Buffer[finesse].push_back(word4);
          unsigned word5 =
            (digit->getSampleRise() << 24) |
            ((digit->getDeltaSamplePeak() & 0xF) << 20) |
            ((digit->getDeltaSampleFall() & 0xF) << 16);
          short checkSum = -(sumShorts(word1) + sumShorts(word2) + sumShorts(word3) +
                             sumShorts(word4) + sumShorts(word5));
          word5 |= (checkSum & 0xFFFF);
          Buffer[finesse].push_back(word5);
          Nhits++;
        }
        unsigned tail = (0x5 << 9) | (Nhits & 0x1FF);
        Buffer[finesse].push_back(tail);
      }
      RawCOPPERPackerInfo info;
      info.exp_num = evtMetaData->getExperiment();
      // run number : 14bits, subrun # : 8bits
      info.run_subrun_num = (evtMetaData->getRun() << 8) +
                            (evtMetaData->getSubrun() & 0xFF);
      info.eve_num = evtMetaData->getEvent();
      info.node_id = TOP_ID + copperID;
      info.tt_ctime = 0;
      info.tt_utime = 0;
      info.b2l_ctime = 0;
      info.hslb_crc16_error_bit = 0;
      info.truncation_mask = 0;
      info.type_of_data = 0;

      auto* raw = rawData.appendNew();
      raw->PackDetectorBuf(Buffer[0].data(), Buffer[0].size(),
                           Buffer[1].data(), Buffer[1].size(),
                           Buffer[2].data(), Buffer[2].size(),
                           Buffer[3].data(), Buffer[3].size(),
                           info);
    }
    delete [] sortedDigits;
  }


  void TOPPackerModule::packProductionDebug()
  {
    StoreObjPtr<EventMetaData> evtMetaData;
    StoreArray<TOPRawDigit> digits(m_inputRawDigitsName);
    StoreArray<RawTOP> rawData(m_outputRawDataName);

    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) return;

    auto* sortedDigits = new vector<const TOPRawDigit*>[mapSize];

    for (const auto& digit : digits) {
      auto scrodID = digit.getScrodID();
      const auto* feemap = mapper.getMap(scrodID);
      if (!feemap) {
        B2ERROR("TOPPacker: no front-end map available."
                << LogVar("scrodID", scrodID));
        continue;
      }
      sortedDigits[feemap->getIndex()].push_back(&digit);
    }

    unsigned revo9count = 0;
    unsigned phase = 0;
    if (digits.getEntries() > 0) {
      revo9count = digits[0]->getRevo9Counter();
      phase = digits[0]->getPhase();
    }

    for (const auto& copperID : mapper.getCopperIDs()) {
      vector<int> Buffer[4];
      for (int finesse = 0; finesse < 4; finesse++) {
        const auto* feemap = mapper.getMapFromCopper(copperID, finesse);
        if (!feemap) continue;
        unsigned scrodID = feemap->getScrodID();
        unsigned format = static_cast<unsigned>(TOP::RawDataType::c_ProductionDebug01);

        unsigned head0 = (format << 16) | (0xA << 12) | (scrodID & 0x0FFF);
        Buffer[finesse].push_back(head0);

        unsigned numWordsCore = sortedDigits[feemap->getIndex()].size() * 5 + 1;
        unsigned head1 = ((phase & 0xF) << 12) | (numWordsCore & 0xFFF);
        Buffer[finesse].push_back(head1);

        unsigned head2 = revo9count & 0xFFFF;
        Buffer[finesse].push_back(head2);

        unsigned head3 = 0;
        Buffer[finesse].push_back(head3);

        unsigned Nhits = 0;
        for (const auto& digit : sortedDigits[feemap->getIndex()]) {
          unsigned checkSum = 0; // IPv4 checksum
          unsigned word0 =
            (digit->getCarrierNumber() << 30) |
            ((digit->getASICNumber() & 0x3) << 28) |
            ((digit->getASICChannel() & 0x7) << 25) |
            ((digit->getASICWindow() & 0x1FF) << 16) |
            (0xB << 12) |
            ((digit->getTFine() & 0xF) << 8);
          checkSum += (word0 & 0xFFFF) + ((word0 >> 16) & 0xFFFF);
          Buffer[finesse].push_back(word0);
          unsigned word1 =
            ((digit->getValuePeak() & 0x1FFF) << 16) |
            (digit->getIntegral() & 0xFFFF);
          checkSum += (word1 & 0xFFFF) + ((word1 >> 16) & 0xFFFF);
          Buffer[finesse].push_back(word1);
          unsigned word2 =
            ((digit->getValueRise0() & 0x1FFF) << 16) |
            (digit->getValueRise1() & 0x1FFF);
          checkSum += (word2 & 0xFFFF) + ((word2 >> 16) & 0xFFFF);
          Buffer[finesse].push_back(word2);
          unsigned word3 =
            ((digit->getValueFall0() & 0x1FFF) << 16) |
            (digit->getValueFall1() & 0x1FFF);
          checkSum += (word3 & 0xFFFF) + ((word3 >> 16) & 0xFFFF);
          Buffer[finesse].push_back(word3);
          unsigned word4 =
            (digit->getSampleRise() << 24) |
            ((digit->getDeltaSamplePeak() & 0xF) << 20) |
            ((digit->getDeltaSampleFall() & 0xF) << 16);
          checkSum += (word4 & 0xFFFF) + ((word4 >> 16) & 0xFFFF);
          while ((checkSum >> 16) > 0) {
            checkSum = (checkSum & 0xFFFF) + (checkSum >> 16);
          }
          word4 |= ((~checkSum) & 0xFFFF);
          Buffer[finesse].push_back(word4);
          Nhits++;
        }
        unsigned tail = (0x5 << 9) | (Nhits & 0x1FF);
        Buffer[finesse].push_back(tail);
      }

      RawCOPPERPackerInfo info;
      info.exp_num = evtMetaData->getExperiment();
      // run number : 14bits, subrun # : 8bits
      info.run_subrun_num = (evtMetaData->getRun() << 8) +
                            (evtMetaData->getSubrun() & 0xFF);
      info.eve_num = evtMetaData->getEvent();
      info.node_id = TOP_ID + copperID;
      info.tt_ctime = 0;
      info.tt_utime = 0;
      info.b2l_ctime = 0;
      info.hslb_crc16_error_bit = 0;
      info.truncation_mask = 0;
      info.type_of_data = 0;

      auto* raw = rawData.appendNew();
      raw->PackDetectorBuf(Buffer[0].data(), Buffer[0].size(),
                           Buffer[1].data(), Buffer[1].size(),
                           Buffer[2].data(), Buffer[2].size(),
                           Buffer[3].data(), Buffer[3].size(),
                           info);
    }
    delete [] sortedDigits;
  }

} // end Belle2 namespace

