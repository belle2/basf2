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

  }

  TOPUnpackerModule::~TOPUnpackerModule()
  {
  }

  void TOPUnpackerModule::initialize()
  {

    StoreArray<RawTOP> rawData;
    rawData.isRequired();

    StoreArray<TOPDigit> digits;
    digits.registerInDataStore();

    StoreArray<TOPRawWaveform> waveforms;
    waveforms.registerInDataStore();

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

    StoreArray<RawTOP> rawData;
    StoreArray<TOPDigit> digits;
    digits.clear();

    for (auto& raw : rawData) {
      for (int finesse = 0; finesse < 4; finesse++) {
        const int* buffer = raw.GetDetectorBuffer(0, finesse);
        int bufferSize = raw.GetDetectorNwords(0, finesse);
        if (bufferSize < 1)
          continue;
        unsigned dataFormat = (buffer[0] >> 24) & 0xFF;
        switch (dataFormat) {
          case 1:
            unpackProductionFormat(buffer, bufferSize);
            break;
          case 2:
            unpackWaveformFormat(buffer, bufferSize);
            break;
          default:
            B2ERROR("TOPUnpacker: unknown data format type = " << dataFormat);
        }
      }
    }

  }


  void TOPUnpackerModule::unpackProductionFormat(const int* buffer, int bufferSize)
  {

    StoreArray<TOPDigit> digits;

    unsigned short scrodID = buffer[0] & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return;
    }
    int barID = feemap->barID;
    int column = feemap->column;

    unsigned version = (buffer[0] >> 16) & 0xFF;
    switch (version) {
      case 0:
        for (int i = 1; i < bufferSize; i++) {
          int word = buffer[i];
          int tdc = word & 0xFFFF;
          unsigned chan = ((word >> 16) & 0x7F) + column * 128;
          unsigned flags = (word >> 24) & 0xFF;
          int channelID = m_topgp->getChannelID(chan);
          auto* digit = digits.appendNew(barID, channelID, tdc);
          digit->setHardwareChannelID(chan);
          digit->setHitQuality((TOPDigit::EHitQuality) flags);
        }
        break;
      default:
        B2ERROR("TOPUnpacker::unpackProductionFormat: unknown data format version = "
                << version);
    }

  }


  void TOPUnpackerModule::unpackWaveformFormat(const int* buffer, int bufferSize)
  {

    StoreArray<TOPRawWaveform> waveforms;
    StoreObjPtr<EventMetaData> evtMetaData;

    unsigned short scrodID = buffer[0] & 0xFFFF;
    const auto* feemap = m_topgp->getFrontEndMapper().getMap(scrodID);
    if (!feemap) {
      B2ERROR("TOPUnpacker: no front-end map available for SCROD ID = " << scrodID);
      return;
    }
    int barID = feemap->barID;
    int column = feemap->column;

    DataArray array(buffer, bufferSize);
    unsigned version = (buffer[0] >> 16) & 0xFF;
    switch (version) {
      case 1: {
        unsigned scrod = array.getWord();
        unsigned freezeDate = array.getWord();
        unsigned eventNumber = array.getWord();
        if (eventNumber != evtMetaData->getEvent()) {
          B2ERROR("TOPUnpacker::unpackWaveformFormat: inconsistent event number, "
                  "expect " << evtMetaData->getEvent() << " got " << eventNumber);
          return;
        }
        unsigned triggerType = array.getWord();
        unsigned flags = array.getWord();
        int numofWaveforms = array.getWord();
        for (int k = 0; k < numofWaveforms; k++) {
          unsigned referenceASIC = array.getWord();
          int numofSegments = array.getWord();
          for (int iseg = 0; iseg < numofSegments; iseg++) {
            unsigned segmentASIC = array.getWord();
            unsigned hardChID = m_topgp->getHardwareChannelID(segmentASIC >> 9, column);
            int channelID = m_topgp->getChannelID(hardChID);
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
            waveforms.appendNew(barID, channelID, hardChID, scrod, freezeDate,
                                triggerType, flags, referenceASIC, segmentASIC,
                                wfdata);
          }
        }
        int n = bufferSize - array.getIndex() - 1;
        if (n > 0) {
          B2ERROR("TOPUnpacker::unpackWaveformFormat: " << n <<
                  " words of data buffer unused");
        }
      }
      break;
      default:
        B2ERROR("TOPUnpacker::unpackWaveformFormat: unknown data format version = "
                << version);
    }

  }



  void TOPUnpackerModule::endRun()
  {
  }

  void TOPUnpackerModule::terminate()
  {
  }


} // end Belle2 namespace

