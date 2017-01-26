/**************************************************************************
 * BASF2 (Belle Analysis Framework 2)                                     *
 * Copyright(C) 2014 - Belle II Collaboration                             *
 *                                                                        *
 * Author: The Belle II Collaboration                                     *
 * Contributors: Marko Staric                                             *
 *                                                                        *
 * This software is provided "as is" without any warranty.                *
 **************************************************************************/

// Own include
#include <top/modules/TOPUnpacker/TOPUnpackerModule.h>
#include <top/RawDataTypes.h>

// framework - DataStore
#include <framework/datastore/DataStore.h>
#include <framework/datastore/StoreArray.h>
#include <framework/datastore/StoreObjPtr.h>

// framework aux
#include <framework/gearbox/Unit.h>
#include <framework/gearbox/Const.h>
#include <framework/logging/Logger.h>

// Dataobject classes
#include <framework/dataobjects/EventMetaData.h>
#include <rawdata/dataobjects/RawTOP.h>
#include <top/dataobjects/TOPDigit.h>
#include <top/dataobjects/TOPRawWaveform.h>
#include <top/dataobjects/TOPRawDigit.h>
#include <top/dataobjects/TOPSlowData.h>

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
             "name of TOPRawWaveform store array", string(""));
    addParam("outputRawDigitsName", m_outputRawDigitsName,
             "name of TOPRawDigit store array", string(""));
    addParam("swapBytes", m_swapBytes, "if true, swap bytes", false);
    addParam("dataFormat", m_dataFormat,
             "data format as defined in top/include/RawDataTypes.h, 0 = auto detect", 0);

  }

  TOPUnpackerModule::~TOPUnpackerModule()
  {
  }

  void TOPUnpackerModule::initialize()
  {

    // input

    StoreArray<RawTOP> rawData(m_inputRawDataName);
    rawData.isRequired();

    // output

    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.registerInDataStore();

    StoreArray<TOPRawDigit> rawDigits(m_outputRawDigitsName);
    rawDigits.registerInDataStore();

    StoreArray<TOPSlowData> slowData;
    slowData.registerInDataStore();

    StoreArray<TOPRawWaveform> waveforms(m_outputWaveformsName);
    waveforms.registerInDataStore(DataStore::c_DontWriteOut);

    rawDigits.registerRelationTo(waveforms, DataStore::c_Event, DataStore::c_DontWriteOut);

    // check if front end mappings are available
    const auto& mapper = m_topgp->getFrontEndMapper();
    int mapSize = mapper.getMapSize();
    if (mapSize == 0) B2ERROR("No front-end mapping available for TOP");

  }

  void TOPUnpackerModule::beginRun()
  {
  }

  void TOPUnpackerModule::event()
  {

    // input
    StoreArray<RawTOP> rawData(m_inputRawDataName);

    // output
    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.clear();
    StoreArray<TOPRawDigit> rawDigits(m_outputRawDigitsName);
    rawDigits.clear();
    StoreArray<TOPRawWaveform> waveforms(m_outputWaveformsName);
    waveforms.clear();
    StoreArray<TOPSlowData> slowData;
    slowData.clear();

    for (auto& raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);
        if (bufferSize < 1) continue;

        int err = 0;
        int dataFormat = m_dataFormat;
        if (dataFormat == 0) { // auto detect data format
          DataArray array(buffer, bufferSize, m_swapBytes);
          unsigned word = array.getWord();
          dataFormat = (word >> 16);
        }
        switch (dataFormat) {
          case static_cast<int>(TOP::RawDataType::c_Type0Ver16):
            unpackType0Ver16(buffer, bufferSize, rawDigits, slowData);
            break;
          case static_cast<int>(TOP::RawDataType::c_Type2Ver1):
            err = unpackType2or3Ver1(buffer, bufferSize, rawDigits, waveforms, false);
            break;
          case static_cast<int>(TOP::RawDataType::c_Type3Ver1):
            err = unpackType2or3Ver1(buffer, bufferSize, rawDigits, waveforms, true);
            break;
          case static_cast<int>(TOP::RawDataType::c_Draft):
            unpackProductionDraft(buffer, bufferSize, digits);
            break;
          case static_cast<int>(TOP::RawDataType::c_GigE):
            err = unpackWaveformsGigE(buffer, bufferSize, waveforms);
            break;
          case static_cast<int>(TOP::RawDataType::c_IRS3B):
            err = unpackWaveformsIRS3B(buffer, bufferSize, waveforms);
            break;
          default:
            B2ERROR("TOPUnpacker: unknown data format, Type = " <<
                    (dataFormat >> 8) << ", Version = " <<
                    (dataFormat & 0xFF));
        }

        if (err != 0)
          B2ERROR("TOPUnpacker: " << err << " words of data buffer not used");

      } // finesse loop
    } // rawData loop

  }


  void TOPUnpackerModule::unpackProductionDraft(const int* buffer, int bufferSize,
                                                StoreArray<TOPDigit>& digits)
  {

    B2DEBUG(100, "Unpacking ProductionDraft to TOPDigits, dataSize = " << bufferSize);

    unsigned short scrodID = buffer[0] & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return;
    }
    int moduleID = feemap->getModuleID();
    int boardstack = feemap->getBoardstackNumber();
    const auto& mapper = m_topgp->getChannelMapper();

    const auto* geo = m_topgp->getGeometry();

    for (int i = 1; i < bufferSize; i++) {
      int word = buffer[i];
      int tdc = word & 0xFFFF;
      unsigned chan = ((word >> 16) & 0x7F) + boardstack * 128;
      unsigned flags = (word >> 24) & 0xFF;
      int pixelID = mapper.getPixelID(chan);
      auto* digit = digits.appendNew(moduleID, pixelID, tdc);
      digit->setTime(geo->getNominalTDC().getTime(tdc));
      digit->setChannel(chan);
      digit->setHitQuality((TOPDigit::EHitQuality) flags);
    }

  }


  void TOPUnpackerModule::unpackType0Ver16(const int* buffer, int bufferSize,
                                           StoreArray<TOPRawDigit>& rawDigits,
                                           StoreArray<TOPSlowData>& slowData)
  {

    B2DEBUG(100, "Unpacking Type0Ver16 to TOPRawDigits, dataSize = " << bufferSize);

    DataArray array(buffer, bufferSize, m_swapBytes);
    unsigned word = array.getWord();
    unsigned short scrodID = word & 0x0FFF;

    unsigned last = array.getLastWord();
    int Nhits = last & 0x01FF;
    if (bufferSize != 5 * Nhits + 2) {
      B2ERROR("TOPUnpacker: corrupted data (feature-extraction format) for SCROD ID = "
              << scrodID);
      return;
    }

    short SDType = last >> 24;
    short SDValue = (last >> 12) & 0x0FFF;
    if (SDType != 0) slowData.appendNew(scrodID, SDType, SDValue);

    unsigned short errorFlags = 0;
    if (((word >> 12) & 0x0F) != 0x0A) errorFlags |= TOPRawDigit::c_HeadMagic;
    if (((last >> 9) & 0x07) != 0x05) errorFlags |= TOPRawDigit::c_TailMagic;

    for (int hit = 0; hit < Nhits; hit++) {
      auto* digit = rawDigits.appendNew(scrodID);

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


  int TOPUnpackerModule::unpackType2or3Ver1(const int* buffer, int bufferSize,
                                            StoreArray<TOPRawDigit>& rawDigits,
                                            StoreArray<TOPRawWaveform>& waveforms,
                                            bool pedestalSubtracted)
  {

    if (pedestalSubtracted) {
      B2DEBUG(100, "Unpacking Type3Ver1 to TOPRawDigits and TOPRawWaveforms, dataSize = "
              << bufferSize);
    } else {
      B2DEBUG(100, "Unpacking Type2Ver1 to TOPRawDigits and TOPRawWaveforms, dataSize = "
              << bufferSize);
    }

    DataArray array(buffer, bufferSize, m_swapBytes);
    unsigned word = array.getWord(); // header word 0
    unsigned short scrodID = word & 0x0FFF;
    word = array.getWord(); // header word 1 (what it contains?)

    while (array.getRemainingWords() > 15) {

      unsigned header = array.getWord(); // word 0
      if (header != 0xaaaa0103 and header != 0xaaaa0100) {
        B2ERROR("TOPUnpacker: corrupted data - invalid FE header word");
        return array.getRemainingWords();
      }

      word = array.getWord(); // word 1
      unsigned short ScrodID = word >> 25;
      unsigned short convertedAddr = (word >> 16) & 0x1FF;
      if (ScrodID != scrodID) {
        B2ERROR("TOPUnpacker: corrupted data - scrodID's differ " << scrodID << " "
                << ScrodID);
        return array.getRemainingWords();
      }

      word = array.getWord(); // word 2 (what it contains?)

      // feature-extracted data (positive signal)
      word = array.getWord(); // word 3
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
      short qualityFlags_p = (word >> 16) & 0xFFFF;

      // feature-extracted data (negative signal)
      word = array.getWord(); // word 9
      word = array.getWord(); // word 10
      short samplePeak_n = word & 0xFFFF;
      short valuePeak_n = (word >> 16) & 0xFFFF;

      word = array.getWord(); // word 11
      short sampleRise_n = word & 0xFFFF;
      short valueRise0_n = (word >> 16) & 0xFFFF;

      word = array.getWord(); // word 12
      short valueRise1_n = word & 0xFFFF;
      short sampleFall_n = (word >> 16) & 0xFFFF;

      word = array.getWord(); // word 13
      short valueFall0_n = word & 0xFFFF;
      short valueFall1_n = (word >> 16) & 0xFFFF;

      word = array.getWord(); // word 14
      short integral_n = word & 0xFFFF;
      short qualityFlags_n = (word >> 16) & 0xFFFF;

      // magic word
      word = array.getWord(); // word 15
      if (word != 0x7473616c) {
        B2ERROR("TOPUnpacker: corrupted data - no magic word at the end of FE header");
        return array.getRemainingWords();
      }

      if (header != 0xaaaa0103) continue;

      // waveform header
      word = array.getWord(); // word 16
      word = array.getWord(); // word 17
      word = array.getWord(); // word 18
      word = array.getWord(); // word 19
      // int numPoints = (word >> 16) & 0xFFFF;
      unsigned short carrier = (word >> 14) & 0x03;
      unsigned short asic = (word >> 12) & 0x03;
      unsigned short asicChannel = (word >> 9) & 0x07;
      unsigned short window = word & 0x1FF;
      if (window != convertedAddr)
        B2ERROR("TOPUnpacker: Type2or3Ver1 - window numbers differ " << window <<
                " " << convertedAddr);

      // store to raw digits (carrier/asic/channel not available before waveform header!)
      std::vector<TOPRawDigit*> digits; // needed for creating relations to waveforms
      if (abs(valuePeak_p) != 9999) {
        auto* digit = rawDigits.appendNew(scrodID);
        digit->setCarrierNumber(carrier);
        digit->setASICNumber(asic);
        digit->setASICChannel(asicChannel);
        digit->setASICWindow(window);
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
        digits.push_back(digit);
      }
      if (abs(valuePeak_n) != 9999) {
        auto* digit = rawDigits.appendNew(scrodID);
        digit->setCarrierNumber(carrier);
        digit->setASICNumber(asic);
        digit->setASICChannel(asicChannel);
        digit->setASICWindow(window);
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
        digits.push_back(digit);
      }

      word = array.getWord(); // word 20
      word = array.getWord(); // word 21
      word = array.getWord(); // word 22

      int numWords = 4 * 32; // (numPoints + 1) / 2;
      if (array.getRemainingWords() < numWords) {
        B2ERROR("TOPUnpacker: too few words for waveform data, needed " << numWords);
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
      int moduleID = 0;
      int boardstack = 0;
      const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
      if (feemap) {
        moduleID = feemap->getModuleID();
        boardstack = feemap->getBoardstackNumber();
      } else {
        B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      }

      // determine hardware channel and pixelID (valid only if feemap available!)
      const auto& mapper = m_topgp->getChannelMapper();
      unsigned channel = mapper.getChannel(boardstack, carrier, asic, asicChannel);
      int pixelID = mapper.getPixelID(channel);

      // store to raw waveforms
      unsigned lastWriteAddr = 0; // not important, but maybe available somewhere?
      auto* waveform = waveforms.appendNew(moduleID, pixelID, channel, scrodID, 0,
                                           0, 0, lastWriteAddr, window,
                                           mapper.getType(), mapper.getName(), adcData);
      waveform->setPedestalSubtractedFlag(pedestalSubtracted);

      // create relations btw. raw digits and waveform
      for (auto& digit : digits) digit->addRelationTo(waveform);

    }

    return array.getRemainingWords();

  }


  int TOPUnpackerModule::unpackWaveformsIRS3B(const int* buffer, int bufferSize,
                                              StoreArray<TOPRawWaveform>& waveforms)
  {

    B2DEBUG(100, "Unpacking IRS3B to TOPRawWaveforms, dataSize = " << bufferSize);

    DataArray array(buffer, bufferSize, m_swapBytes);
    unsigned word = array.getWord();
    unsigned short scrodID = word & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return array.getRemainingWords();
    }

    StoreObjPtr<EventMetaData> evtMetaData;
    int moduleID = feemap->getModuleID();
    int boardstack = feemap->getBoardstackNumber();
    const auto& mapper = m_topgp->getChannelMapper(ChannelMapper::c_IRS3B);

    unsigned scrod = array.getWord();
    unsigned freezeDate = array.getWord();
    unsigned eventNumber = array.getWord();
    if (eventNumber != evtMetaData->getEvent()) {
      B2ERROR("TOPUnpacker::unpackWaveformFormat: inconsistent event number, "
              "expect " << evtMetaData->getEvent() << " got " << eventNumber);
      return array.getRemainingWords();
    }
    unsigned triggerType = array.getWord();
    unsigned flags = array.getWord();
    int numofWaveforms = array.getWord();
    for (int k = 0; k < numofWaveforms; k++) {
      unsigned referenceASIC = array.getWord();
      int numofSegments = array.getWord();
      for (int iseg = 0; iseg < numofSegments; iseg++) {
        unsigned segmentASIC = array.getWord();
        unsigned chan = (segmentASIC >> 9) & 0x0007;
        unsigned asic = (segmentASIC >> 14) & 0x0003; // called also asicCol
        unsigned carrier = (segmentASIC >> 12) & 0x0003; // called also asicRow
        unsigned channel = mapper.getChannel(boardstack, carrier, asic, chan);
        int pixelID = mapper.getPixelID(channel);

        int numofPoints = array.getWord();
        std::vector<short> wfdata;
        for (int i = 0; i < numofPoints / 2; i++) {
          unsigned data = array.getWord();
          wfdata.push_back(data & 0xFFFF);
          wfdata.push_back(data >> 16);
        }
        if (numofPoints % 2 != 0) {
          unsigned data = array.getWord();
          wfdata.push_back(data & 0xFFFF);
        }
        waveforms.appendNew(moduleID, pixelID, channel, scrod, freezeDate,
                            triggerType, flags, referenceASIC, segmentASIC,
                            mapper.getType(), mapper.getName(), wfdata);
      } // iseg
    } // k
    return array.getRemainingWords();

  }


  int TOPUnpackerModule::unpackWaveformsGigE(const int* buffer, int bufferSize,
                                             StoreArray<TOPRawWaveform>& waveforms)
  {

    B2DEBUG(100, "Unpacking GigE to TOPRawWaveforms, dataSize = " << bufferSize);

    DataArray array(buffer, bufferSize, m_swapBytes);
    unsigned word = array.getWord();
    unsigned short scrodID = word & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return array.getRemainingWords();
    }

    StoreObjPtr<EventMetaData> evtMetaData;
    int moduleID = feemap->getModuleID();
    int boardstack = feemap->getBoardstackNumber();
    const auto& mapper = m_topgp->getChannelMapper(ChannelMapper::c_IRSX);

    unsigned numPackets = array.getWord();
    for (unsigned packet = 0; packet < numPackets; packet++) {
      word = array.getWord();
      int numWindows = (word >> 19) & 0x1FF; numWindows++; // must be incremented
      unsigned trigPattern = (word >> 28);
      if (trigPattern != 0x0F and array.getRemainingWords() >= numWindows * 257)
        trigPattern = 0x0F; // unsparsified gigE
      int numBits = 0;
      for (int i = 0; i < 4; i++) {
        if (trigPattern & (1 << i)) numBits++;
      }
      if (array.getRemainingWords() < (1 + numBits * 32) * numWindows) {
        B2ERROR("TOPUnpacker::unpackWaveformFormat: missing data - packet is too short");
        return 0;
      }
      for (int win = 0; win < numWindows; win++) {
        word = array.getWord();
        unsigned convertedAddr = word & 0x1FF; // storage window
        unsigned scrod = (word >> 9) & 0x7F;
        unsigned lastWriteAddr = (word >> 16) & 0x1FF; // reference window
        unsigned asic = (word >> 28) & 0x03; // used to be called asicCol
        unsigned carrier = (word >> 30) & 0x03; // used to be called asicRow
        for (unsigned chan = 0; chan < 8; chan++) {
          if ((trigPattern & (1 << (chan / 2))) == 0) continue;
          std::vector<short> wfdata;
          for (unsigned i = 0; i < 32; i++) {
            unsigned data = array.getWord();
            wfdata.push_back(data & 0xFFFF);
            wfdata.push_back(data >> 16);
          }
          unsigned channel = mapper.getChannel(boardstack, carrier, asic, chan);
          int pixelID = mapper.getPixelID(channel);
          unsigned segmentASIC = convertedAddr + (chan << 9) + (carrier << 12) +
                                 (asic << 14);
          waveforms.appendNew(moduleID, pixelID, channel, scrod, 0,
                              trigPattern, 0, lastWriteAddr, segmentASIC,
                              mapper.getType(), mapper.getName(), wfdata);
        } // chan
      } // win
    } // packet

    return array.getRemainingWords();

  }


  void TOPUnpackerModule::endRun()
  {
  }

  void TOPUnpackerModule::terminate()
  {
  }


} // end Belle2 namespace

