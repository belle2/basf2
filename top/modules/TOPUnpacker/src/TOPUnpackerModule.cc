/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

// Own include
#include <top/modules/TOPUnpacker/TOPUnpackerModule.h>
#include <top/RawDataTypes.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/logging/Logger.h>

// Dataobject classes
#include <framework/dataobjects/EventMetaData.h>

#include <bitset>
#include <iomanip>
#include <fstream>
#include <algorithm>

using namespace std;

namespace Belle2 {

  using namespace TOP;

  //-----------------------------------------------------------------
  //                 Register module
  //-----------------------------------------------------------------

  REG_MODULE(TOPUnpacker)

  //-----------------------------------------------------------------
  //                 Implementation
  //-----------------------------------------------------------------

  TOPUnpackerModule::TOPUnpackerModule() : Module()
  {
    // set module description
    setDescription("Raw data unpacker for TOP");
    setPropertyFlags(c_ParallelProcessingCertified);

    // Add parameters
    addParam("inputRawDataName", m_inputRawDataName,
             "name of RawTOP store array", string(""));
    addParam("outputDigitsName", m_outputDigitsName,
             "name of TOPDigit store array", string(""));
    addParam("outputWaveformsName", m_outputWaveformsName,
             "name of TOP(Raw/Production)Waveform store array", string(""));
    addParam("outputRawDigitsName", m_outputRawDigitsName,
             "name of TOPRawDigit store array", string(""));
    addParam("outputTemplateFitResultName", m_templateFitResultName,
             "name of TOPTemplateFitResult", string(""));
    addParam("swapBytes", m_swapBytes, "if true, swap bytes", false);
    addParam("dataFormat", m_dataFormat,
             "data format as defined in top/include/RawDataTypes.h, 0 = auto detect", 0);
    addParam("addRelations", m_addRelations,
             "if true, make relations to TOPProductionHitDebugs (production debug data format only)", true);
  }

  TOPUnpackerModule::~TOPUnpackerModule()
  {
  }

  void TOPUnpackerModule::initialize()
  {

    // input

    m_rawData.isRequired(m_inputRawDataName);

    // output

    m_digits.registerInDataStore(m_outputDigitsName);
    m_rawDigits.registerInDataStore(m_outputRawDigitsName);
    m_slowData.registerInDataStore();
    m_interimFEInfos.registerInDataStore(DataStore::c_DontWriteOut);
    m_waveforms.registerInDataStore(m_outputWaveformsName, DataStore::c_DontWriteOut);
    m_productionEventDebugs.registerInDataStore(DataStore::c_DontWriteOut);
    m_productionHitDebugs.registerInDataStore(DataStore::c_DontWriteOut);
    m_templateFitResults.registerInDataStore(m_templateFitResultName, DataStore::c_DontWriteOut);

    m_rawDigits.registerRelationTo(m_waveforms, DataStore::c_Event, DataStore::c_DontWriteOut);
    m_rawDigits.registerRelationTo(m_templateFitResults, DataStore::c_Event, DataStore::c_DontWriteOut);
    m_rawDigits.registerRelationTo(m_interimFEInfos, DataStore::c_Event, DataStore::c_DontWriteOut);
    m_rawDigits.registerRelationTo(m_productionHitDebugs, DataStore::c_Event, DataStore::c_DontWriteOut);
    m_waveforms.registerRelationTo(m_interimFEInfos, DataStore::c_Event, DataStore::c_DontWriteOut);

    // check if front end mappings are available
    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) B2ERROR("TOPUnpacker: No front-end mapping available for TOP");

  }

  void TOPUnpackerModule::beginRun()
  {
    m_channelStatistics.clear();
  }

  void TOPUnpackerModule::event()
  {
    // clear output store arrays
    m_digits.clear();
    m_rawDigits.clear();
    m_waveforms.clear();
    m_productionEventDebugs.clear();
    m_productionHitDebugs.clear();
    m_templateFitResults.clear();
    m_slowData.clear();
    m_interimFEInfos.clear();

    StoreObjPtr<EventMetaData> evtMetaData;
    for (auto& raw : m_rawData) {
      for (int finesse = 0; finesse < raw.GetMaxNumOfCh(0); finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);
        if (bufferSize < 1) continue;

        int err = 0;

        int dataFormat = m_dataFormat;
        if (dataFormat == 0) { // auto detect data format
          DataArray array(buffer, bufferSize, m_swapBytes);
          unsigned word = array.getWord();
          dataFormat = (word >> 16);
          bool isKnownDataFormat = false;
          for (auto& t : TOP::membersRawDataType) {
            if (static_cast<int>(t) == dataFormat) {
              isKnownDataFormat = true;
              break;
            }
          }

          if (!isKnownDataFormat) { //dataformat word is not recognised, might be interim format.
            if (evtMetaData->getExperiment() == 1) { // all exp.1 data was taken with interim format
              dataFormat = 0x0301;
              m_swapBytes = true;
            } else {
              if (unpackHeadersInterimFEVer01(buffer, bufferSize, true)) { //if buffer unpacks without errors assuming it's interim format
                B2DEBUG(22, "Assuming interim FW data format");
                dataFormat = 0x0301; //assume it's interim format
                m_swapBytes = true;
              } else {
                B2WARNING("TOPUnpacker: Could not establish data format.");
                err = bufferSize;
              }
            }
          }
        }

        switch (dataFormat) {
          case static_cast<int>(TOP::RawDataType::c_Type0Ver16):
            unpackType0Ver16(buffer, bufferSize);
            break;
          case static_cast<int>(TOP::RawDataType::c_Type2Ver1):
            err = unpackInterimFEVer01(buffer, bufferSize, false);
            break;
          case static_cast<int>(TOP::RawDataType::c_Type3Ver1):
            err = unpackInterimFEVer01(buffer, bufferSize, true);
            break;
          case static_cast<int>(TOP::RawDataType::c_Draft):
            unpackProductionDraft(buffer, bufferSize);
            break;
          case static_cast<int>(TOP::RawDataType::c_ProductionDebug01):
            err = unpackProdDebug(buffer, bufferSize, TOP::RawDataType::c_ProductionDebug01, true);
            break;
          case static_cast<int>(TOP::RawDataType::c_ProductionDebug02):
            err = unpackProdDebug(buffer, bufferSize, TOP::RawDataType::c_ProductionDebug02, true);
            break;

          default:
            B2ERROR("TOPUnpacker: unknown data format."
                    << LogVar("Type", (dataFormat >> 8))
                    << LogVar("Version", (dataFormat & 0xFF)));
            err = bufferSize;

        }


        if (err != 0) {
          if (raw.GetMaxNumOfCh(0) <= 4) { // COPPER
            stringstream copperName;
            //nodeid format is described on page7 of: https://confluence.desy.de/display/BI/DAQ+WebHome?preview=%2F34029242%2F37487394%2FSetupPocketDAQ_4p1_RawCOPPERDataFormat.pdf
            copperName << "cpr" << ((raw.GetNodeID(0) >> 24) * 1000 + (raw.GetNodeID(0) & 0x3FF)) << char('a' + finesse);
            B2ERROR("TOPUnpacker: error in unpacking data from frontend " << copperName.str()
                    << LogVar("words unused", err)
                    << LogVar("copper", copperName.str()));
          } else { // PCIe40
            int slot = (raw.GetNodeID(0) & 0xF) * 8 - 7 + finesse / 4;
            std::string name = (slot < 10) ? "s0" : "s";
            name += std::to_string(slot) + char('a' + finesse % 4);
            B2ERROR("TOPUnpacker: error in unpacking data from boardstack " << name
                    << LogVar("words unused", err));
          }
        }

      } // finesse loop
    } // m_rawData loop

  }


  void TOPUnpackerModule::unpackProductionDraft(const int* buffer, int bufferSize)
  {

    B2DEBUG(22, "Unpacking ProductionDraft to TOPDigits, dataSize = " << bufferSize);

    unsigned short scrodID = buffer[0] & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available."
              << LogVar("SCROD ID", scrodID));
      return;
    }
    int moduleID = feemap->getModuleID();
    int boardstack = feemap->getBoardstackNumber();
    const auto& mapper = m_topgp->getChannelMapper();

    const auto* geo = m_topgp->getGeometry();
    auto subBits = geo->getNominalTDC().getSubBits();
    int sampleDivisions = 0x1 << subBits;

    for (int i = 1; i < bufferSize; i++) {
      int word = buffer[i];
      int tdc = word & 0xFFFF;
      double rawTime = double(tdc) / sampleDivisions;
      unsigned chan = ((word >> 16) & 0x7F) + boardstack * 128;
      unsigned flags = (word >> 24) & 0xFF;
      int pixelID = mapper.getPixelID(chan);
      auto* digit = m_digits.appendNew(moduleID, pixelID, rawTime);
      digit->setTime(geo->getNominalTDC().getTime(tdc));
      digit->setChannel(chan);
      digit->setHitQuality((TOPDigit::EHitQuality) flags);
    }

  }


  void TOPUnpackerModule::unpackType0Ver16(const int* buffer, int bufferSize)
  {

    B2DEBUG(22, "Unpacking Type0Ver16 to TOPRawDigits, dataSize = " << bufferSize);

    DataArray array(buffer, bufferSize, m_swapBytes);
    unsigned word = array.getWord();
    unsigned short scrodID = word & 0x0FFF;

    unsigned last = array.getLastWord();
    int Nhits = last & 0x01FF;
    if (bufferSize != 5 * Nhits + 2) {
      B2ERROR("TOPUnpacker: corrupted data (feature-extraction format)."
              << LogVar("SCROD ID", scrodID));
      return;
    }

    short SDType = last >> 24;
    short SDValue = (last >> 12) & 0x0FFF;
    if (SDType != 0) m_slowData.appendNew(scrodID, SDType, SDValue);

    unsigned short errorFlags = 0;
    if (((word >> 12) & 0x0F) != 0x0A) errorFlags |= TOPRawDigit::c_HeadMagic;
    if (((last >> 9) & 0x07) != 0x05) errorFlags |= TOPRawDigit::c_TailMagic;

    for (int hit = 0; hit < Nhits; hit++) {
      auto* digit = m_rawDigits.appendNew(scrodID, TOPRawDigit::c_Production);

      word = array.getWord(); // word 1
      digit->setCarrierNumber((word >> 30) & 0x03);
      digit->setASICNumber((word >> 28) & 0x03);
      digit->setASICChannel((word >> 25) & 0x07);
      digit->setASICWindow((word >> 16) & 0x1FF);
      digit->setTFine((word >> 8) & 0x0F);
      auto flags = errorFlags;
      if (((word >> 12) & 0x0F) != 0x0B) flags |= TOPRawDigit::c_HitMagic;
      unsigned short checkSum = sumShorts(word);

      word = array.getWord(); // word 2
      digit->setValuePeak(expand13to16bits(word >> 16));
      digit->setIntegral(word & 0xFFFF);
      checkSum += sumShorts(word);

      word = array.getWord(); // word 3
      digit->setValueRise0(expand13to16bits(word >> 16));
      digit->setValueRise1(expand13to16bits(word));
      checkSum += sumShorts(word);

      word = array.getWord(); // word 4
      digit->setValueFall0(expand13to16bits(word >> 16));
      digit->setValueFall1(expand13to16bits(word));
      checkSum += sumShorts(word);

      word = array.getWord(); // word 5
      digit->setSampleRise(word >> 24);
      digit->setDeltaSamplePeak((word >> 20) & 0x0F);
      digit->setDeltaSampleFall((word >> 16) & 0x0F);
      checkSum += sumShorts(word);
      if (checkSum != 0) flags |= TOPRawDigit::c_HitChecksum;

      digit->setErrorFlags(flags);

    }

  }

  bool TOPUnpackerModule::unpackHeadersInterimFEVer01(const int* buffer, int bufferSize, bool swapBytes)
  {
    B2DEBUG(22, "Checking whether buffer unpacks as InterimFEVer01, dataSize = " << bufferSize);

    DataArray array(buffer, bufferSize, swapBytes);

    array.getWord(); // header word 0
    array.getWord(); // header word 1 (what it contains?)
    while (array.getRemainingWords() > 0) {
      unsigned header = array.getWord(); // word 0
      if ((header & 0xFF) == 0xBE) { //this is a super short FE header
        continue;
      }

      if (header != 0xaaaa0104 and header != 0xaaaa0103 and header != 0xaaaa0100) {//cannot be interim firmware
        return false; //abort
      }


      array.getWord(); // word 1
      array.getWord(); // word 2

      if (header != 0xaaaa0104) {
        // feature-extracted data (positive signal)
        array.getWord(); // word 3
        array.getWord(); // word 4
        array.getWord(); // word 5
        array.getWord(); // word 6
        array.getWord(); // word 7
        array.getWord(); // word 8
        array.getWord(); // word 9
        array.getWord(); // word 10
        array.getWord(); // word 11
        array.getWord(); // word 12
        array.getWord(); // word 13
        array.getWord(); // word 14
      }

      unsigned magicWord = array.getWord(); // word 15
      if (magicWord != 0x7473616c) {//invalid magic word
        return false; //abort
      }

      if (header != 0xaaaa0103) continue;
      array.getWord(); // word 16

      array.getWord(); // word 17
      array.getWord(); // word 18
      array.getWord(); // word 19

      array.getWord(); // word 20

      array.getWord(); // word 21

      array.getWord(); // word 22

      int numWords = 4 * 32; // (numPoints + 1) / 2;
      if (array.getRemainingWords() < numWords) { //not enough remaining words for waveform data
        return false; //abort
      }

      for (int i = 0; i < numWords; i++) {
        array.getWord();
      }
    }

    return true;
  }


  int TOPUnpackerModule::unpackInterimFEVer01(const int* buffer, int bufferSize,
                                              bool pedestalSubtracted)
  {

    B2DEBUG(22, "Unpacking InterimFEVer01 to TOPRawDigits and TOPRawWaveforms, "
            "dataSize = " << bufferSize);

    int moduleID = 0;
    int boardstack = 0;

    DataArray array(buffer, bufferSize, m_swapBytes);

    map<unsigned short, int> evtNumCounter; //counts the occurence of carrier-generated event numbers.
    std::vector<unsigned short> channelCounter(128, 0); //counts occurence of carrier/asic/channel combinations

    unsigned word = array.getWord(); // header word 0
    unsigned short scrodID = word & 0x0FFF;
    auto* info = m_interimFEInfos.appendNew(scrodID, bufferSize);

    word = array.getWord(); // header word 1 (what it contains?)


    while (array.getRemainingWords() > 0) {

      unsigned header = array.getWord(); // word 0

      if ((header & 0xFF) == 0xBE) { //this is a super short FE header
//        B2DEBUG(21, "0b" << std::bitset<8>(header & 0xFF) << " " << std::bitset<8>((header>>8) & 0xFF) << " " << std::bitset<8>((header>>16) & 0xFF) << " " << std::bitset<8>((header>>24) & 0xFF));

        unsigned short scrodID_SSFE;
        unsigned short carrier_SSFE;
        unsigned short asic_SSFE;
        unsigned short channel_SSFE;
        unsigned short evtNum_SSFE;

        evtNum_SSFE = (header >> 8) & 0xFF;
        scrodID_SSFE = (header >> 16) & 0x7F;
        channel_SSFE = (header >> 24) & 0x07;
        asic_SSFE = (header >> 27) & 0x03;
        carrier_SSFE = (header >> 29) & 0x03;

        const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID_SSFE);
        if (feemap) {
          moduleID = feemap->getModuleID();
          boardstack = feemap->getBoardstackNumber();
        } else {
          B2ERROR("TOPUnpacker: no front-end map available."
                  << LogVar("SCROD ID", scrodID_SSFE));
          info->setErrorFlag(TOPInterimFEInfo::c_InvalidScrodID);
        }

        if (scrodID_SSFE != scrodID) {
          B2ERROR("TOPUnpacker: corrupted data - "
                  << "different scrodID's in HLSB and super short FE header."
                  << LogVar("SCROD", scrodID_SSFE)
                  << LogVar("slot", moduleID)
                  << LogVar("BS", boardstack));
          B2DEBUG(21, "Different scrodID's in HLSB and FE header: " << scrodID << " " << scrodID_SSFE << " word = 0x" << std::hex << word);
          info->setErrorFlag(TOPInterimFEInfo::c_DifferentScrodIDs);
          return array.getRemainingWords();
        }

        B2DEBUG(21, scrodID_SSFE << "\t" << carrier_SSFE << "\t"  << asic_SSFE << "\t" << channel_SSFE << "\t" << evtNum_SSFE);

        int channelID = carrier_SSFE * 32 + asic_SSFE * 8 + channel_SSFE;
        channelCounter[channelID] += 1;
        evtNumCounter[evtNum_SSFE] += 1;

        info->incrementFEHeadersCount();
        info->incrementEmptyFEHeadersCount();

        continue;
      }

      if (header != 0xaaaa0104 and header != 0xaaaa0103 and header != 0xaaaa0100) {
        B2ERROR("TOPUnpacker: corrupted data - invalid FE header word");
        B2DEBUG(21, "Invalid FE header word: " << std::hex << header  << " 0b" << std::bitset<32>(header));
        B2DEBUG(21, "SCROD ID: " << scrodID << " " << std::hex << scrodID);

        info->setErrorFlag(TOPInterimFEInfo::c_InvalidFEHeader);
        return array.getRemainingWords();
      }


      word = array.getWord(); // word 1
      unsigned short scrodID_FE = word >> 25;
      unsigned short convertedAddr = (word >> 16) & 0x1FF;
      unsigned short evtNum_numWin_trigPat_FEheader = word & 0xFFFF;
      unsigned short evtNum_FEheader = evtNum_numWin_trigPat_FEheader & 0xFF;
      evtNumCounter[evtNum_FEheader] += 1;

      if (scrodID_FE != scrodID) {
        B2ERROR("TOPUnpacker: different scrodID's in HLSB and FE header."
                << LogVar("scrodID (HSLB)", scrodID)
                << LogVar("scrodID (FE)", scrodID_FE));

        info->setErrorFlag(TOPInterimFEInfo::c_DifferentScrodIDs);
        return array.getRemainingWords();
      }

      word = array.getWord(); // word 2
      //      unsigned lastWrAddr = word & 0x1FF;
      unsigned lastWrAddr = (word & 0x0FF) << 1;
      //unsigned short asicChannelFE = (word >> 9) & 0x07;
      unsigned short asicChannelFE = (word >> 8) & 0x07;
      unsigned short asicFE = (word >> 12) & 0x03;
      unsigned short carrierFE = (word >> 14) & 0x03;
      unsigned short channelID = carrierFE * 32 + asicFE * 8 + asicChannelFE;
      //B2DEBUG(21, "carrier asic chn " << carrierFE << " " << asicFE << " " << asicChannelFE << " " << channelID << " 0b" << std::bitset<32>(word) );

      B2DEBUG(21, scrodID_FE << "\t" << carrierFE << "\t" << asicFE << "\t" << asicChannelFE << "\t" << evtNum_FEheader);

      channelCounter[channelID] += 1;

      std::vector<TOPRawDigit*> digits; // needed for creating relations to waveforms

      if (header != 0xaaaa0104) {
        // feature-extracted data (positive signal)
        array.getWord(); // word 3
        word = array.getWord(); // word 4
        short samplePeak_p = word & 0xFFFF;
        short valuePeak_p = (word >> 16) & 0xFFFF;

        word = array.getWord(); // word 5
        short sampleRise_p = word & 0xFFFF;
        short valueRise0_p = (word >> 16) & 0xFFFF;

        word = array.getWord(); // word 6
        short valueRise1_p = word & 0xFFFF;
        short sampleFall_p = (word >> 16) & 0xFFFF;

        word = array.getWord(); // word 7
        short valueFall0_p = word & 0xFFFF;
        short valueFall1_p = (word >> 16) & 0xFFFF;

        word = array.getWord(); // word 8
        short integral_p = word & 0xFFFF;
        //      short qualityFlags_p = (word >> 16) & 0xFFFF;

        // feature-extracted data (negative signal)
        array.getWord(); // word 9
        //word = array.getWord(); // word 9
        //short n_samp_i = (word >> 16) & 0xFFFF;
        word = array.getWord(); // word 10
        short samplePeak_n = word & 0xFFFF;
        short valuePeak_n = (word >> 16) & 0xFFFF;

        array.getWord(); // word 11
        //word = array.getWord(); // word 11
        //short sampleRise_n = word & 0xFFFF;
        //short valueRise0_n = (word >> 16) & 0xFFFF;

        array.getWord(); // word 12
        //word = array.getWord(); // word 12
        //short valueRise1_n = word & 0xFFFF;
        //short sampleFall_n = (word >> 16) & 0xFFFF;

        array.getWord(); // word 13
        //word = array.getWord(); // word 13
        //short valueFall0_n = word & 0xFFFF;
        //short valueFall1_n = (word >> 16) & 0xFFFF;

        word = array.getWord(); // word 14
        short integral_n = word & 0xFFFF;
        short qualityFlags_n = (word >> 16) & 0xFFFF;

        if (abs(valuePeak_p) != 9999) {
          auto* digit = m_rawDigits.appendNew(scrodID, TOPRawDigit::c_Interim);
          digit->setCarrierNumber(carrierFE);
          digit->setASICNumber(asicFE);
          digit->setASICChannel(asicChannelFE);
          digit->setASICWindow(convertedAddr);
          digit->setLastWriteAddr(lastWrAddr);
          digit->setSampleRise(sampleRise_p);
          digit->setDeltaSamplePeak(samplePeak_p - sampleRise_p);
          digit->setDeltaSampleFall(sampleFall_p - sampleRise_p);
          digit->setValueRise0(valueRise0_p);
          digit->setValueRise1(valueRise1_p);
          digit->setValuePeak(valuePeak_p);
          digit->setValueFall0(valueFall0_p);
          digit->setValueFall1(valueFall1_p);
          digit->setIntegral(integral_p);
          //        digit->setErrorFlags(qualityFlags_p); // not good solution !
          digit->addRelationTo(info);
          digits.push_back(digit);
          //template fit result is saved in negative pulse fe data
          if (valuePeak_p < 150) {
            auto* tlpfResult = m_templateFitResults.appendNew();
            tlpfResult->setBackgroundOffset(samplePeak_n);
            tlpfResult->setAmplitude(valuePeak_n);
            tlpfResult->setChisquare(qualityFlags_n);
            tlpfResult->setRisingEdgeAndConvert(integral_n);
            digit->addRelationTo(tlpfResult);
          }
        }
        /*if (abs(valuePeak_n) != 9999) {
          auto* digit = m_rawDigits.appendNew(scrodID, TOPRawDigit::c_Interim);
          digit->setCarrierNumber(carrierFE);
          digit->setASICNumber(asicFE);
          digit->setASICChannel(asicChannelFE);
          digit->setASICWindow(convertedAddr);
          digit->setLastWriteAddr(lastWrAddr);
          digit->setSampleRise(sampleRise_n);
          digit->setDeltaSamplePeak(samplePeak_n - sampleRise_n);
          digit->setDeltaSampleFall(sampleFall_n - sampleRise_n);
          digit->setValueRise0(valueRise0_n);
          digit->setValueRise1(valueRise1_n);
          digit->setValuePeak(valuePeak_n);
          digit->setValueFall0(valueFall0_n);
          digit->setValueFall1(valueFall1_n);
          digit->setIntegral(integral_n);
          //        digit->setErrorFlags(qualityFlags_n); // not good solution !
          digit->addRelationTo(info);
          digits.push_back(digit);
        }*/
      }

      // magic word
      word = array.getWord(); // word 15
      if (word != 0x7473616c) {
        //B2ERROR("TOPUnpacker: corrupted data - no magic word at the end of FE header");
        //B2DEBUG(21, "No magic word at the end of FE header, found: "
        //<< std::hex << word);
        info->setErrorFlag(TOPInterimFEInfo::c_InvalidMagicWord);
        //return array.getRemainingWords(); do not abort event for now as footer is invalid in current debugging version of firmware
      }

      // store to raw digits

      info->incrementFEHeadersCount();
      if (digits.empty()) info->incrementEmptyFEHeadersCount();

      if (header != 0xaaaa0103) continue;

      // waveform header
      word = array.getWord(); // word 16
      unsigned long evtNum_numWaves_refWin_WFheader = word;
      unsigned short evtNum_WFheader = (evtNum_numWaves_refWin_WFheader >> 24) & 0xFF;

      if (evtNum_WFheader != evtNum_FEheader) {
        B2ERROR("TOPUnpacker: different carrier event number in WF and FE header."
                << LogVar("Event number (FE header)", evtNum_FEheader)
                << LogVar("Event number (WF header)", evtNum_WFheader));
      }

      array.getWord(); // word 17
      array.getWord(); // word 18
      word = array.getWord(); // word 19
      // int numPoints = (word >> 16) & 0xFFFF;
      unsigned short carrier = (word >> 14) & 0x03;
      unsigned short asic = (word >> 12) & 0x03;
      unsigned short asicChannel = (word >> 9) & 0x07;
      unsigned short window = word & 0x1FF;

      // checks for data corruption
      if (carrier != carrierFE) {
        B2ERROR("TOPUnpacker: different carrier numbers in FE and WF header");
        B2DEBUG(21, "Different carrier numbers in FE and WF header: "
                << carrierFE << " " << carrier);
        info->setErrorFlag(TOPInterimFEInfo::c_DifferentCarriers);
      }
      if (asic != asicFE) {
        B2ERROR("TOPUnpacker: different ASIC numbers in FE and WF header");
        B2DEBUG(21, "Different ASIC numbers in FE and WF header: "
                << asicFE << " " << asic);
        info->setErrorFlag(TOPInterimFEInfo::c_DifferentAsics);
      }
      if (asicChannel != asicChannelFE) {
        B2ERROR("TOPUnpacker: different ASIC channel numbers in FE and WF header");
        B2DEBUG(21, "Different ASIC channel numbers in FE and WF header: "
                << asicChannelFE << " " << asicChannel);
        info->setErrorFlag(TOPInterimFEInfo::c_DifferentChannels);
      }
      if (window != convertedAddr) {
        B2ERROR("TOPUnpacker: different window numbers in FE and WF header");
        B2DEBUG(21, "Different window numbers in FE and WF header: "
                << convertedAddr << " " << window);
        info->setErrorFlag(TOPInterimFEInfo::c_DifferentWindows);
      }

      // reading out all four window addresses
      // to be for correcnt alignment of individual readout windows in written waveform
      std::vector<unsigned short> windows;
      windows.push_back(window);

      word = array.getWord(); // word 20
      windows.push_back(word & 0x1FF);

      word = array.getWord(); // word 21
      windows.push_back(word & 0x1FF);

      word = array.getWord(); // word 22
      windows.push_back(word & 0x1FF);

      int numWords = 4 * 32; // (numPoints + 1) / 2;
      if (array.getRemainingWords() < numWords) {
        B2ERROR("TOPUnpacker: too few words for waveform data."
                << LogVar("needed", numWords)
                << LogVar("available", array.getRemainingWords()));
        info->setErrorFlag(TOPInterimFEInfo::c_InsufficientWFData);
        return array.getRemainingWords();
      }

      // unpack waveforms
      std::vector<short> adcData;
      for (int i = 0; i < numWords; i++) {
        word = array.getWord();
        adcData.push_back(word & 0xFFFF);
        adcData.push_back((word >> 16) & 0xFFFF);
      }
      // if (numWords * 2 != numPoints) adcData.pop_back(); // numPoints is even

      // determine slot number (moduleID) and boardstack
      moduleID = 0;
      boardstack = 0;
      const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
      if (feemap) {
        moduleID = feemap->getModuleID();
        boardstack = feemap->getBoardstackNumber();
      } else {
        B2ERROR("TOPUnpacker: no front-end map available."
                << LogVar("SCROD ID", scrodID));
        info->setErrorFlag(TOPInterimFEInfo::c_InvalidScrodID);
      }

      // determine hardware channel and pixelID (valid only if feemap available!)
      const auto& mapper = m_topgp->getChannelMapper();
      unsigned channel = mapper.getChannel(boardstack, carrier, asic, asicChannel);
      int pixelID = mapper.getPixelID(channel);

      // store to raw waveforms
      auto* waveform = m_waveforms.appendNew(moduleID, pixelID, channel, scrodID,
                                             window, 0, adcData);
      waveform->setLastWriteAddr(lastWrAddr);
      waveform->setStorageWindows(windows);
      waveform->setPedestalSubtractedFlag(pedestalSubtracted);
      waveform->addRelationTo(info);
      info->incrementWaveformsCount();

      // create relations btw. raw digits and waveform
      for (auto& digit : digits) digit->addRelationTo(waveform);

    }

    if (evtNumCounter.size() != 1) {
      B2ERROR("TOPUnpacker: Possible frame shift detected "
              << "(More than one unique carrier event number in this readout event)."
              << LogVar("SCROD", scrodID)
              << LogVar("slot", moduleID)
              << LogVar("BS", boardstack));
    }

    int nASICs = 0;

    string evtNumOutputString;
    evtNumOutputString += "Carrier event numbers and their counts for SCROD ID " + std::to_string(scrodID) + ":\n";
    for (auto const& it : evtNumCounter) {
      nASICs += it.second;
      evtNumOutputString += std::to_string(it.first) + ":\t" + std::to_string(it.second) + "\n";
    }
    evtNumOutputString += "Total:\t" + std::to_string(nASICs);
    B2DEBUG(21, evtNumOutputString);

    int nChannels = 0;
    int nChannelsDiff = 0;

    string channelOutputString;
    channelOutputString += "Detected channels and their counts for SCROD ID (channels with count == 1 are omitted)" + std::to_string(
                             scrodID) + ":\n";

    int channelIndex(0);
    for (auto const& it : channelCounter) {
      if (it > 0) {
        nChannelsDiff += 1;
      }
      nChannels += it;

      int channelID = channelIndex;
      int carrier = channelID / 32;
      int asic = (channelID % 32) / 8;
      int chn = channelID % 8;

      if (it != 1) {
        channelOutputString += "carrier: " + std::to_string(carrier) + " asic: " + std::to_string(asic) + " chn: " + std::to_string(
                                 chn) + " occurence: " + std::to_string(it) + "\n";
        B2WARNING("TOPUnpacker: interim FE - ASIC channel seen more than once"
                  << LogVar("ScrodID", scrodID)
                  << LogVar("carrier", carrier)
                  << LogVar("ASIC", asic)
                  << LogVar("channel", chn)
                  << LogVar("times seen", it));
      }
      channelIndex += 1;
    }

    m_channelStatistics[nChannels] += 1;

    channelOutputString += "Total:\t" + std::to_string(nChannels) + " " + std::to_string(nChannelsDiff);
    B2DEBUG(21, channelOutputString);

    return array.getRemainingWords();


  }


  int TOPUnpackerModule::unpackProdDebug(const int* buffer, int bufferSize, TOP::RawDataType dataFormat,
                                         bool pedestalSubtracted)
  {

    B2DEBUG(22, "Unpacking Production firmware debug data format to TOPRawDigits "
            "dataSize = " << bufferSize);

    DataArray array(buffer, bufferSize, m_swapBytes);
    unsigned word;

    word = array.getWord(); // word 0, type(8)/version(8)/0xA(4)/ScrodID(12)
    unsigned int evtType = word >> 24;
    unsigned int evtVersion = (word >> 16) & 0xFF;
    unsigned int evtMagicHeader = (word >> 12) & 0xF;
    unsigned int evtScrodID = word & 0xFFF;

    // determine slot number (moduleID) and boardstack
    int moduleID = 0;
    int boardstack = 0;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(evtScrodID);
    if (feemap) {
      moduleID = feemap->getModuleID();
      boardstack = feemap->getBoardstackNumber();
    } else {
      B2WARNING("TOPUnpacker: no front-end map available."
                << LogVar("SCROD ID", evtScrodID));
    }


    B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
            (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
            << "\tevtType = " << evtType
            << ", evtVersion = " << evtVersion
            << ", evtMagicHeader = " << evtMagicHeader
            << ", evtScrodID = " << evtScrodID);

    if (evtMagicHeader != 0xA) {
      B2WARNING("TOPUnpacker: event header magic word mismatch. should be 0xA."
                << LogVar("Magic word", evtMagicHeader));
      return array.getRemainingWords();
    }

    word = array.getWord(); // word 1, extra(3)/numWordsBonus(13)/phase(4)/numWordsCore(12)
    unsigned int evtExtra = word >> 29;
    unsigned int evtNumWordsBonus = (word >> 16) & 0x1FFF;
    unsigned int evtPhase = (word >> 12) & 0xF;
    unsigned int evtNumWordsCore = word & 0xFFF;

    B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
            (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
            << "\tevtExtra = " << evtExtra
            << ", evtNumWordsBonus = " << evtNumWordsBonus
            << ", evtPhase = " << evtPhase
            << ", numWordsCore = " << evtNumWordsCore);

    word = array.getWord(); // word 2, skipHit(1)/reserved(4)/ctime LSBs(11)/revo9counter(16)
    bool         evtSkipHit = word >> 29;
    unsigned int evtCtime = (word >> 16) & 0x7FF;
    unsigned int evtRevo9Counter = word & 0xFFFF;

    B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
            (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
            << "\tevtSkipHit = " << evtSkipHit
            << ", evtCtime = " << evtCtime
            << ", evtRevo9Counter = " << evtRevo9Counter);

    word = array.getWord(); // word 3, asicMask(16)/eventQueueDepth(8)/eventNumberByte(8)
    unsigned int evtAsicMask = word >> 16;
    unsigned int evtEventQueueDepth = (word >> 8) & 0xFF;
    unsigned int evtEventNumberByte = word & 0xFF;

    B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
            (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
            << "\tevtAsicMask = " << evtAsicMask
            << ", evtEventQueueDepth = " << evtEventQueueDepth
            << ", evtEventNumberByte = " << evtEventNumberByte);

    m_productionEventDebugs.appendNew(evtType,
                                      evtVersion,
                                      evtScrodID,
                                      evtSkipHit,
                                      evtCtime,
                                      evtPhase,
                                      evtAsicMask,
                                      evtEventQueueDepth,
                                      evtEventNumberByte);

    B2DEBUG(22, "end of event header, start of hits:");

    const int numWordsPerHit = 4 + evtExtra;
    unsigned int numHitsFound = 0;
    unsigned int numExpectedWaveforms = 0;

    std::vector<TOPRawDigit*> digitsWithWaveform;  // digits that have waveforms

    while (array.getRemainingWords() > numWordsPerHit //one more full hit + one word of footer
           && array.getIndex() < evtNumWordsCore - 2) {  // -1 for 0-based counting, -1 for hit footer word
      array.resetChecksum();


      //need to predefine some variables here as we need to branch out between two data format versions for the next two words
      unsigned int hitCarrier = 0;
      unsigned int hitAsic = 0;
      unsigned int hitChannel = 0;
      unsigned int hitWindow = 0;
      unsigned int hitMagicHeader = 0;
      unsigned int hitTFine = 0;
      bool         hitHasWaveform = false;
      bool         hitIsOnHeap = false;
      unsigned int hitHeapWindow = 0;
      bool         hitIsOnHeapStraddle = false;
      unsigned int hitHeapWindowStraddle = 0;
      bool         hitIsWindowStraddle = false;
      short        hitIntegral = 0;
      short        hitVPeak = 0;

      if (dataFormat == TOP::RawDataType::c_ProductionDebug01) {    //dataformat 0x0401
        word = array.getWord(); // hit word 0, carrier(2)/asic(2)/channel(3)/window(9)/0xB(4)/tFine(4)/hasWaveform(1)/isOnHeap(1)/heapWindow(6)
        hitCarrier = word >> 30;
        hitAsic = (word >> 28) & 0x3;
        hitChannel = (word >> 25) & 0x7;
        hitWindow = (word >> 16) & 0x1FF;
        hitMagicHeader = (word >> 12) & 0xF;
        hitTFine = (word >> 8) & 0xF;
        hitHasWaveform = (word >> 7) & 0x1;
        hitIsOnHeap = (word >> 6) & 0x1;
        hitHeapWindow = word  & 0x3F;


        B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
                (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
                << "\thitCarrier = " << hitCarrier
                << ", hitAsic = " << hitAsic
                << ", hitChannel = " << hitChannel
                << ", hitWindow = " << hitWindow
                //      << ", hitMagicHeader = " << hitMagicHeader
                //      << ", hitTFine = " << hitTFine
                << ", hitHasWaveform = " << hitHasWaveform
                //      << ", hitIsOnHeap = " << hitIsOnHeap
                //      << ", hitHeapWindow = " << hitHeapWindow
               );


        word = array.getWord(); // hit word 1, reserved(3)/vPeak(13)/integral(16)
        hitVPeak = expand13to16bits(word >> 16);
        hitIntegral = word & 0xFFFF;
        B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
                (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
                << "\thitVPeak = " << hitVPeak
                << ", hitIntegral = " << hitIntegral);

      } else if (dataFormat == TOP::RawDataType::c_ProductionDebug02) { //dataformat 0x0402
        word = array.getWord(); // hit word 0, carrier(2)/asic(2)/channel(3)/window(9)/0xB(4)/tFine(4)/hasWaveform(1)/isWindowStraddle(1)/integral(6)
        hitCarrier = word >> 30;
        hitAsic = (word >> 28) & 0x3;
        hitChannel = (word >> 25) & 0x7;
        hitWindow = (word >> 16) & 0x1FF;
        hitMagicHeader = (word >> 12) & 0xF;
        hitTFine = (word >> 8) & 0xF;
        hitHasWaveform = (word >> 7) & 0x1;
        hitIsWindowStraddle = (word >> 6) & 0x1;
        hitIntegral = word & 0x3F;


        B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
                (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
                << "\thitCarrier = " << hitCarrier
                << ", hitAsic = " << hitAsic
                << ", hitChannel = " << hitChannel
                << ", hitWindow = " << hitWindow
                //      << ", hitMagicHeader = " << hitMagicHeader
                //      << ", hitTFine = " << hitTFine
                << ", hitHasWaveform = " << hitHasWaveform
                << ", hitIsWindowStraddle = " << hitHasWaveform
                << ", hitIntegral = " << hitIntegral
                //      << ", hitHeapWindow = " << hitHeapWindow
               );

        word = array.getWord(); // hit word 1, reserved(3)/vPeak(13)/isOnHeap1(1)/heapWindow1(7)/isOnHeap0(1)/heapWindow0(7)
        hitVPeak = expand13to16bits(word >> 16);
        hitIsOnHeapStraddle = (word >> 15) & 0x1;
        hitHeapWindowStraddle = (word >> 8) & 0x7F;
        hitIsOnHeap = (word >> 7) & 0x1;
        hitHeapWindow = word & 0x1;

        B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
                (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
                << "\thitVPeak = " << hitVPeak
                << ", hitIsOnHeapStraddle = " << hitIsOnHeapStraddle
                << ", hitHeapWindowStraddle = " << hitHeapWindowStraddle
                << ", hitIsOnHeap = " << hitIsOnHeap
                << ", hitHeapWindow = " << hitHeapWindow);
      } else { //could not match the data format
        B2WARNING("TOPUnpacker: could not match data type inside unpackProdDebug()"
                  << LogVar("evtType", evtType) << LogVar("evtVersion", evtVersion));
        return array.getRemainingWords();
      }


      if (hitHasWaveform) {
        numExpectedWaveforms += 1;
      }

      if (hitMagicHeader != 0xB) {
        B2WARNING("TOPUnpacker: hit header magic word mismatch. should be 0xB."
                  << LogVar("Magic word", hitMagicHeader));
        return array.getRemainingWords();
      }

      word = array.getWord(); // hit word 2, reserved(3)/vRise0(13)/reserved(3)/vRise1(13)
      short hitVRise0 = expand13to16bits(word >> 16);
      short hitVRise1 = expand13to16bits(word);
      B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
              (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
              << "\thitVRise0 = " << hitVRise0
              << ", hitVRise1 = " << hitVRise1);

      word = array.getWord(); // hit word 3, reserved(3)/vFall0(13)/reserved(3)/vFall1(13)
      short hitVFall0 = expand13to16bits(word >> 16);
      short hitVFall1 = expand13to16bits(word);
      B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
              (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
              << "\thitVFall0 = " << hitVFall0
              << ", hitVFall1 = " << hitVFall1);

      word = array.getWord(); // hit word 4, sampleRise(8)/dSampPeak(4)/dSampFall(4)/headerChecksum(16)
      unsigned short hitSampleRise = (word >> 24);
      short hitDSampPeak = (word >> 20) & 0xF;
      short hitDSampFall = (word >> 16) & 0xF;
      unsigned short hitHeaderChecksum = word & 0xFFFF;
      B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
              (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
              << "\thitSampleRise = " << hitSampleRise
              << ", hitDSampPeak = " << hitDSampPeak
              << ", hitDSampFall = " << hitDSampFall
              << ", hitheaderChecksum = " << hitHeaderChecksum
              << ", checksum " << (array.validateChecksum() ? "OK" : "NOT OK"));

      if (!array.validateChecksum()) {
        B2WARNING("TOPUnpacker: hit checksum invalid.");
        return array.getRemainingWords();
      }

      // append digit
      auto* digit = m_rawDigits.appendNew(evtScrodID, TOPRawDigit::c_ProductionDebug);
      digit->setCarrierNumber(hitCarrier);
      digit->setASICNumber(hitAsic);
      digit->setASICChannel(hitChannel);
      digit->setASICWindow(hitWindow);
      digit->setLastWriteAddr(0);
      digit->setSampleRise(hitSampleRise);
      digit->setDeltaSamplePeak(hitDSampPeak);
      digit->setDeltaSampleFall(hitDSampFall);
      digit->setValueRise0(hitVRise0);
      digit->setValueRise1(hitVRise1);
      digit->setValuePeak(hitVPeak);
      digit->setValueFall0(hitVFall0);
      digit->setValueFall1(hitVFall1);
      digit->setTFine(hitTFine);
      digit->setIntegral(hitIntegral);
      digit->setRevo9Counter(evtRevo9Counter);
      digit->setPhase(evtPhase);


      if (hitHasWaveform) {
        digitsWithWaveform.push_back(digit);
      }

      TOPProductionHitDebug* hitDebug = 0;
      if (dataFormat == TOP::RawDataType::c_ProductionDebug01) { // dataformat 0x0401
        hitDebug = m_productionHitDebugs.appendNew(hitHasWaveform,
                                                   hitIsOnHeap,
                                                   hitWindow,
                                                   hitIsOnHeap ? 428 + hitHeapWindow : hitWindow, //hitHeapWindow is counted from the start of the heap
                                                   hitIsWindowStraddle);
      } else if (dataFormat == TOP::RawDataType::c_ProductionDebug02) { // dataformat 0x0402
        hitDebug = m_productionHitDebugs.appendNew(hitHasWaveform,
                                                   hitIsOnHeap,
                                                   hitWindow,
                                                   hitIsOnHeap ? 428 + hitHeapWindow : hitWindow, //hitHeapWindow is counted from the start of the heap
                                                   hitIsWindowStraddle,
                                                   hitWindow + 1, //logical address of straddle window is always +1
                                                   hitIsOnHeapStraddle ? 428 + hitHeapWindowStraddle : hitWindow +
                                                   1); //physical address might be entirely different if straddled window is on heap
      }

      //parse extra words if exist:
      for (unsigned int i = 0; i < evtExtra; ++i) {
        word = array.getWord(); // extra hit word i, undefined so far
        B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
                (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
                << "\thit extra word " << i << " (" << evtExtra << ")");
        hitDebug->appendExtraWord(word);
      }

      if (m_addRelations and hitDebug) {
        digit->addRelationTo(hitDebug);
      }

      numHitsFound += 1;
    } // end of hits loop

    word = array.getWord(); // event footer word, sdType(8)/sdData(12)/0x5(3)/nHits(9)
    unsigned int evtSdType = (word >> 24);
    unsigned int evtSdData = (word >> 12) & 0xFFF;
    unsigned int evtMagicFooter = (word >> 9) & 0x7;
    unsigned int evtNHits = word & 0x1FF;
    B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
            (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
            << "\tevtSdType = " << evtSdType
            << ", evtSdData = " << evtSdData
            << ", evtMagicFooter = " << evtMagicFooter
            << ", evtNHits = " << evtNHits << " (" << numHitsFound << ")");

    if (evtSdType != 0) {
      m_slowData.appendNew(evtScrodID, evtSdType, evtSdData);
    }

    if (evtMagicFooter != 0x5) {
      B2WARNING("TOPUnpacker: event footer magic word mismatch. should be 0x5."
                << LogVar("Magic word", evtMagicFooter));
      return array.getRemainingWords();
    }

    B2DEBUG(22, "the rest:");

    unsigned int numParsedWaveforms = 0;
    while (array.peekWord() != 0x6c617374 //next word is not wf footer word
           && array.getRemainingWords() > 0) {

      word = array.getWord(); // waveform word 0, nSamples(16)/0x0(5)/nWindows(3)/0(1)/carrier(2)/asic(2)/channel(3)
      unsigned int wfNSamples = (word >> 16);
      unsigned int wfNWindows = (word >> 8) & 0x7;
      unsigned int wfCarrier = (word >> 5) & 0x3;
      unsigned int wfAsic = (word >> 3) & 0x3;
      unsigned int wfChannel = word & 0x7;

      // determine hardware channel and pixelID (valid only if feemap available!)
      const auto& mapper = m_topgp->getChannelMapper();
      unsigned channel = mapper.getChannel(boardstack, wfCarrier, wfAsic, wfChannel);
      int pixelID = mapper.getPixelID(channel);

      B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
              (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
              << "\twfNSamples = " << wfNSamples
              << ", wfNWindows = " << wfNWindows
              << ", wfCarrier = " << wfCarrier
              << ", wfAsic = " << wfAsic
              << ", wfChannel " << wfChannel);

      if (wfNSamples != 32 && wfNSamples != 16) {
        B2WARNING("TOPUnpacker: suspicious value for wfNSamples."
                  << LogVar("wfNSamples", wfNSamples));
        return array.getRemainingWords();
      }

      word = array.getWord(); // waveform word 1, 0x0(1)/startSamp(6)/logAddress(9)/carrierEventNumber(7)/readAddr(9)
      unsigned int wfStartSample = (word >> 25) & 0x3F;
      unsigned int wfWindowLogic = (word >> 16) & 0x1FF;
      unsigned int wfEventNumber = (word >> 9) & 0x7F;
      unsigned int wfWindowPhysical = word & 0x1FF;

      B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
              (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
              << "\twfStartSample = " << wfStartSample
              << ", wfWindowLogic = " << wfWindowLogic
              << ", wfEventNumber = " << wfEventNumber
              << ", wfWindowPhysical = " << wfWindowPhysical);

      std::vector<short> wfSamples;

      for (unsigned int i = 0; i < wfNSamples / 2; ++i) {
        short wfSampleLast = 0;
        short wfSampleFirst = 0;


        word = array.getWord(); // waveform sample word i, reserved(4)/sample 2*i+1(12)/reserved(4)/sample 2*i(12)
        if (pedestalSubtracted) {
          wfSampleLast = (word >> 16);
          wfSampleFirst = word & 0xFFFF;
        } else {
          wfSampleLast = (word >> 16) & 0xFFF;
          wfSampleFirst = word & 0xFFF;

        }

        B2DEBUG(22, std::dec << array.getIndex() << ":\t" << setfill('0') << setw(4) << std::hex <<
                (word >> 16) << " " << setfill('0') << setw(4) << (word & 0xFFFF) << std::dec
                << "\twfSample" << 2 * i + 1 << " = " << wfSampleLast
                << ", wfSample" << 2 * i << " = " << wfSampleFirst);

        wfSamples.push_back(wfSampleFirst);
        wfSamples.push_back(wfSampleLast);
      }

      // append waveform
      auto* waveform = m_waveforms.appendNew(moduleID, pixelID, channel, evtScrodID,
                                             wfWindowLogic, wfStartSample, wfSamples);
      waveform->setPedestalSubtractedFlag(pedestalSubtracted);
      waveform->setPhysicalWindow(wfWindowPhysical);
      if (numParsedWaveforms < digitsWithWaveform.size()) {
        const auto* digit = digitsWithWaveform[numParsedWaveforms];
        if (digit->getScrodChannel() == channel % 128) {
          digit->addRelationTo(waveform);
        } else {
          B2WARNING("TOPUnpacker: hit and its waveform have different channel number."
                    << LogVar("channel (hit)", digit->getScrodChannel())
                    << LogVar("channel (waveform)", channel % 128));
        }
      }
      numParsedWaveforms += 1;

    } // end of waveform segments loop

    if (numExpectedWaveforms != numParsedWaveforms) {
      B2WARNING("TOPUnpacker: number of expected and parsed waveforms does not match."
                << LogVar("expected", numExpectedWaveforms)
                << LogVar("parsed", numParsedWaveforms));
    }

    return array.getRemainingWords();
  }


  void TOPUnpackerModule::endRun()
  {
    B2INFO("TOPUnpacker: Channels seen per event statistics:");
    B2INFO("TOPUnpacker: nChn\tcount");
    for (auto& entry : m_channelStatistics) {
      B2INFO("TOPUnpacker: " << entry.first << "\t\t" << entry.second);
    }
  }

  void TOPUnpackerModule::terminate()
  {
  }


} // end Belle2 namespace
