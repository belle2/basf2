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
#include <top/modules/TOPUnpacker/TOPUnpackerModule.h>

#include <framework/core/ModuleManager.h>

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

  TOPUnpackerModule::TOPUnpackerModule() : Module(),
    m_topgp(TOPGeometryPar::Instance())

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

    StoreArray<TOPRawWaveform> waveforms(m_outputWaveformsName);
    waveforms.registerInDataStore(DataStore::c_DontWriteOut);

    if (!m_topgp->isInitialized()) {
      GearDir content("/Detector/DetectorComponent[@name='TOP']/Content");
      m_topgp->Initialize(content);
    }
    if (!m_topgp->isInitialized()) B2ERROR("Component TOP not found in Gearbox");

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
    StoreArray<TOPRawWaveform> waveforms(m_outputWaveformsName);
    waveforms.clear();

    for (auto& raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);
        if (bufferSize < 1)
          continue;
        unsigned dataFormat = (buffer[0] >> 24) & 0xFF;
        switch (dataFormat) {
          case 1:
            unpackProductionFormat(buffer, bufferSize, digits);
            break;
          case 2:
            unpackWaveformFormat(buffer, bufferSize, waveforms);
            break;
          default:
            B2ERROR("TOPUnpacker: unknown data format type = " << dataFormat);
        }
      }
    }

  }


  void TOPUnpackerModule::unpackProductionFormat(const int* buffer, int bufferSize,
                                                 StoreArray<TOPDigit>& digits)
  {

    unsigned short scrodID = buffer[0] & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return;
    }
    int barID = feemap->getBarID();
    int boardstack = feemap->getBoardstackNumber();
    const auto& mapper = m_topgp->getChannelMapper();

    unsigned version = (buffer[0] >> 16) & 0xFF;
    switch (version) {
      case 0:
        for (int i = 1; i < bufferSize; i++) {
          int word = buffer[i];
          int tdc = word & 0xFFFF;
          unsigned chan = ((word >> 16) & 0x7F) + boardstack * 128;
          unsigned flags = (word >> 24) & 0xFF;
          int pixelID = mapper.getPixelID(chan);
          auto* digit = digits.appendNew(barID, pixelID, tdc);
          digit->setTime(m_topgp->getTime(tdc));
          digit->setChannel(chan);
          digit->setHitQuality((TOPDigit::EHitQuality) flags);
        }
        break;
      default:
        B2ERROR("TOPUnpacker::unpackProductionFormat: unknown data format version "
                << version);
    }

  }


  void TOPUnpackerModule::unpackWaveformFormat(const int* buffer, int bufferSize,
                                               StoreArray<TOPRawWaveform>& waveforms)
  {
    DataArray array(buffer, bufferSize);
    unsigned word = array.getWord();
    unsigned short scrodID = word & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return;
    }

    unsigned version = (word >> 16) & 0xFF;
    int err = 0;
    switch (version) {
      case 1:
        err = unpackWaveformFormatV1(array, feemap, waveforms);
        if (err != 0)
          B2ERROR("TOPUnpacker::unpackWaveformFormat: " << err <<
                  " words of data buffer not used");
        break;
      case 2:
        err = unpackWaveformFormatV2(array, feemap, waveforms);
        if (err != 0)
          B2ERROR("TOPUnpacker::unpackWaveformFormat: " << err <<
                  " words of data buffer not used");
        break;
      default:
        B2ERROR("TOPUnpacker::unpackWaveformFormat: unknown data format version "
                << version);
    }

  }


  int TOPUnpackerModule::unpackWaveformFormatV1(TOP::DataArray& array,
                                                const TOPFrontEndMap* feemap,
                                                StoreArray<TOPRawWaveform>& waveforms)
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    int barID = feemap->getBarID();
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
        waveforms.appendNew(barID, pixelID, channel, scrod, freezeDate,
                            triggerType, flags, referenceASIC, segmentASIC,
                            mapper.getType(), mapper.getName(), wfdata);
      } // iseg
    } // k
    return array.getRemainingWords();

  }


  int TOPUnpackerModule::unpackWaveformFormatV2(TOP::DataArray& array,
                                                const TOPFrontEndMap* feemap,
                                                StoreArray<TOPRawWaveform>& waveforms)
  {

    StoreObjPtr<EventMetaData> evtMetaData;
    int barID = feemap->getBarID();
    int boardstack = feemap->getBoardstackNumber();
    const auto& mapper = m_topgp->getChannelMapper(ChannelMapper::c_IRSX);

    unsigned word = 0;
    unsigned numPackets = array.getWord();
    for (unsigned packet = 0; packet < numPackets; packet++) {
      word = array.getWord();
      // unsigned eventNumber = word & 0x7FFFF;
      unsigned numWindows = (word >> 19) & 0x1FF; numWindows++; // should be incremented!
      unsigned trigPattern = (word >> 28);
      for (unsigned win = 0; win < numWindows; win++) {
        word = array.getWord();
        unsigned convertedAddr = word & 0x1FF; // storage window
        unsigned scrod = (word >> 9) & 0x7F;
        unsigned lastWriteAddr = (word >> 16) & 0x1FF; // reference window
        unsigned asic = (word >> 28) & 0x03; // used to be called asicCol
        unsigned carrier = (word >> 30) & 0x03; // used to be called asicRow
        for (unsigned chan = 0; chan < 8; chan++) {
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
          waveforms.appendNew(barID, pixelID, channel, scrod, 0,
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

