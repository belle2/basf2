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

  }

  TOPUnpackerModule::~TOPUnpackerModule()
  {
  }

  void TOPUnpackerModule::initialize()
  {

    StoreArray<RawTOP> rawData(m_inputRawDataName);
    rawData.isRequired();

    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.registerInDataStore();

    StoreArray<TOPRawDigit> rawDigits(m_outputRawDigitsName);
    rawDigits.registerInDataStore();

    StoreArray<TOPRawWaveform> waveforms(m_outputWaveformsName);
    waveforms.registerInDataStore(DataStore::c_DontWriteOut);

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

    StoreArray<RawTOP> rawData(m_inputRawDataName);
    StoreArray<TOPDigit> digits(m_outputDigitsName);
    digits.clear();
    StoreArray<TOPRawDigit> rawDigits(m_outputRawDigitsName);
    rawDigits.clear();
    StoreArray<TOPRawWaveform> waveforms(m_outputWaveformsName);
    waveforms.clear();

    for (auto& raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);
        if (bufferSize < 1) continue;

        int err = 0;
        DataArray array(buffer, bufferSize, m_swapBytes);
        unsigned word = array.getWord();
        int dataFormat = (word >> 16);
        switch (dataFormat) {
          case static_cast<int>(TOP::RawDataType::c_Type0Ver16):
            unpackType0Ver16(buffer, bufferSize, rawDigits);
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
                                           StoreArray<TOPRawDigit>& rawDigits)
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
        std::vector<unsigned short> wfdata;
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
          std::vector<unsigned short> wfdata;
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

